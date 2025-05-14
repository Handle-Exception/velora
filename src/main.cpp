#include "velora.hpp"

namespace velora
{
    asio::awaitable<int> main(asio::io_context & io_context, IProcess & process)
    {
        spdlog::debug(std::format("[t:{}] Velora main started", std::this_thread::get_id()));
        
        // create system objects
        Window window = co_await Window::construct<winapi::WinapiWindow>(
            asio::use_awaitable, io_context, process, "Velora", Resolution{512, 256});
        if (window->good() == false)
        {
            spdlog::error("Failed to create window");
            co_return -1;
        }

        Renderer renderer = co_await Renderer::construct<opengl::OpenGLRenderer>(asio::use_awaitable, *window, 4, 0);
        if (renderer->good() == false)
        {
            spdlog::error("Failed to create renderer");
            co_return -1;
        }

        co_await renderer->enableVSync();

        // create input system
        auto input_strand = asio::make_strand(io_context);
        game::InputSystem input_system(io_context);

        bool cursor_shown = true;

        // connect input system to window and setup system objects callbacks
        co_await process.setWindowCallbacks(window->getHandle(), 
            WindowCallbacks{.executor = input_strand,
                // process notifies us when the window is destroyed
                .onDestroy = [&window, &renderer]() -> asio::awaitable<void>
                {
                    spdlog::info(std::format("[t:{}] Window callback onDestroy", std::this_thread::get_id()));
                    co_await renderer->close(); // so we notify process to close the renderer
                    co_await window->close(); // and destroy window object
                    co_return;
                },
                .onResize = [&window, &renderer](int width, int height) -> asio::awaitable<void>
                {
                    spdlog::info(std::format("[t:{}] Window callback onResize {}x{}", std::this_thread::get_id(),  width, height));
                    //co_await window->resize(width, height); is it even needed?
                    co_await renderer->updateViewport(Resolution{(std::size_t)width, (std::size_t)height});
                    co_return;
                },
                .onKeyPress = [&input_system](int key) -> asio::awaitable<void>
                {     
                    // need to propagate this information into InputSystem
                    co_await input_system.recordKeyPressed(game::keyToInputCode(key));
                    co_return;
                },
                .onKeyRelease = [&input_system](int key) -> asio::awaitable<void>
                {
                    // need to propagate this information into InputSystem
                    co_await input_system.recordKeyReleased(game::keyToInputCode(key));
                    co_return;
                },
                .onMouseButtonDown = [&process, &input_system, &cursor_shown](int key) -> asio::awaitable<void>
                {
                    spdlog::debug(std::format("[t:{}] Window callback onMouseButtonDown {}", std::this_thread::get_id(), key));
                    
                    cursor_shown = !cursor_shown;
                    
                    if(cursor_shown) co_await process.showCursor();
                    else co_await process.hideCursor();

                    // need to propagate this information into InputSystem
                    co_await input_system.recordKeyPressed(game::keyToInputCode(key));
                    co_return;
                },
                .onMouseButtonUp = [&input_system](int key) -> asio::awaitable<void>
                {
                    spdlog::debug(std::format("[t:{}] Window callback onMouseButtonUp {}", std::this_thread::get_id(), key));
                    // need to propagate this information into InputSystem
                    co_await input_system.recordKeyReleased(game::keyToInputCode(key));
                    co_return;
                },
                .onMouseMove = [&input_system, &cursor_shown](int x, int y, float dx, float dy) -> asio::awaitable<void>
                {
                    if(cursor_shown == true)
                    {
                        dx = 0.0f;
                        dy = 0.0f;
                    }
                    // need to propagate this information into InputSystem
                    co_await input_system.recordMouseMove((float)x, (float)y, dx, dy);
                    co_return;
                },
            });


        // ---------------------------------------------------------------------------------------------------------------------------------------------
        // LOADING ASSETS
        // ---------------------------------------------------------------------------------------------------------------------------------------------
        co_await loadVertexBuffersPrefabs(*renderer);

        co_await loadShadersFromDir(*renderer, getResourcesPath() / "shaders" / "glsl");

        // ---------------------------------------------------------------------------------------------------------------------------------------------
        // 
        // ---------------------------------------------------------------------------------------------------------------------------------------------


        // create level in world in which we will spawn entities
        // level is divided in chunks which is basically octree
        // level should have init, update and shutdown methods

        // then update all subsystems
        // input is handled in other thread
        // it calls callbacks in strand
        // 
        // callbacks are connected to controller component
        // controller component stores actions
        // then eg. Transform component uses actions to move entity
        // eg. Shoot component uses actions to fire bullet ect.
        //
        // then Visual Component uses Transform component to calculate matrices and draw entity
        // then Physics component uses Transform component to move entity
        
        game::TransformSystem transform_system(io_context);

        // Create rendering systems
        game::CameraSystem camera_system(io_context, *renderer);

        // async constructor because visual system must allocate fbo in renderer thread asynchronously
        game::VisualSystem visual_system = co_await game::VisualSystem::asyncConstructor(
                io_context, *renderer, {1280, 720}, camera_system);

        // async constructor because light system must allocate shader input buffer in renderer thread asynchronously
        game::LightSystem light_system = co_await game::LightSystem::asyncConstructor(io_context, visual_system);

        // create scripts system
        game::ScriptSystem script_system(io_context);
        co_await loadScriptsFromDir(script_system, getResourcesPath() / "scripts");

        // create logic systems
        game::HealthSystem health_system(io_context);

        game::TerrainSystem terrain_system(io_context, *renderer);


        // create world
        game::World world(io_context);

        // ---------------------------------------------------------------------------------------------------------------------------------------------
        // LOADING LEVEL
        // ---------------------------------------------------------------------------------------------------------------------------------------------
        ComponentLoaderRegistry components_loader_reg = constructComponentLoaderRegistry();
        ComponentSerializerRegistry components_serializer_reg = constructComponentSerializerRegistry();

        co_await loadWorldFromDir(world, components_loader_reg, getResourcesPath() / "levels", use_json);
        world.setCurrentLevel("level_0");
        
        // ---------------------------------------------------------------------------------------------------------------------------------------------
        // 
        // ---------------------------------------------------------------------------------------------------------------------------------------------

        co_await window->show();

        FpsCounter priority_fps_counter;
        FpsCounter logic_fps_counter;
        std::chrono::high_resolution_clock::time_point last_log_time = std::chrono::high_resolution_clock::now();

        auto NDC_quad_res = renderer->getVertexBuffer("NDC_quad_prefab");
        if(!NDC_quad_res)
        {
            spdlog::error("Failed to load NDC_quad_prefab");
            co_return -1;
        }
        const std::size_t NDC_quad = NDC_quad_res.value();
        
        const auto deferred_lighting_pass_res = renderer->getShader("deferred_lighting_pass");
        if(!deferred_lighting_pass_res)
        {
            spdlog::error("Failed to get deferred lighting pass shader");
            co_return -1;
        } 
        const std::size_t deferred_lighting_pass = *deferred_lighting_pass_res;

        const auto & gbuffer_textures = visual_system.getDeferredFBOTextures();


        FixedStepLoop loop(io_context, 
            // fixed logic step 30 HZ update 
            33.333ms, 
            
            // loop condition
            [&window, &renderer]() -> bool 
            {
                return window->good() && renderer->good();
            },

            // logic loop to be executed at fixed time step 
            [   &world,
                &input_system, &transform_system, &script_system, &health_system,  &terrain_system, 
                &last_log_time, &logic_fps_counter, &priority_fps_counter
            ]
            (std::chrono::duration<double> delta) -> asio::awaitable<void>  
            {
                logic_fps_counter.frame();

                // update fetched input actions in entities
                // input itself is recorded asynchronousy in window callbacks
                co_await world.getCurrentLevel().runSystem(input_system);

                co_await  world.getCurrentLevel().runSystem(transform_system, delta);
                
                co_await world.getCurrentLevel().runSystem(script_system, delta, world.getCurrentLevel());
                
                // co_await (
                //         world.getCurrentLevel().runSystem(health_system, delta) &&
                //         world.getCurrentLevel().runSystem(terrain_system, delta));

                //track fps frames for profiling
                if (std::chrono::high_resolution_clock::now() - last_log_time >= std::chrono::seconds(5)) 
                {
                        spdlog::info(std::format(
                            "[t:{}]\n\tPriority FPS: {:.1f}\n\tLogic FPS:{:.1f}", 
                                std::this_thread::get_id(),
                                priority_fps_counter.getFPS(), 
                                logic_fps_counter.getFPS()
                            )
                        );
                    last_log_time = std::chrono::high_resolution_clock::now();
                }

                co_return;
            },

            // priority loop to be executed as soon as possible
            [&renderer, &world,
            &NDC_quad, &deferred_lighting_pass, &gbuffer_textures,
            &camera_system, &light_system, &visual_system, &priority_fps_counter]
            (float alpha) -> asio::awaitable<void> 
            {
                priority_fps_counter.frame();

                // execute camera calculation in parallel with light calculation
                co_await (
                    world.getCurrentLevel().runSystem(camera_system, alpha) 
                );
                
                co_await renderer->clearScreen({0.8f, 0.8f, 0.8f, 1.0f});

                // visual system will render entities with visual component into its GBuffer
                // interpolate between current and previous transform using alpha
                co_await world.getCurrentLevel().runSystem(visual_system, alpha);
                        
                // light system will render shadows into its FBO
                // and sends light to its shader storage buffer
                // interpolate between current and previous light using alpha
                co_await world.getCurrentLevel().runSystem(light_system, alpha);                

                // render GBuffer to screen
                co_await renderer->render(NDC_quad, deferred_lighting_pass,
                    ShaderInputs{
                        .in_int = {
                            {"lightCount", (int)light_system.getLightsCount()},
                            {"shadowCastersCount", (int)light_system.getShadowCastersCount()}},
                        .in_mat4_array = {{"lightSpaceMatrices", light_system.getShadowMapLightSpaceMatrices()}},
                        .in_samplers = {
                            {"gPosition", gbuffer_textures.at(0)},
                            {"gNormal", gbuffer_textures.at(1)},
                            {"gAlbedoSpec", gbuffer_textures.at(2)}
                        },
                        .in_samplers_array = {
                           {"shadowMaps", light_system.getShadowMapTextures()}
                        },
                        .storage_buffers = {light_system.getLightShaderBufferID()}
                    },
                    RenderOptions{
                        .mode = RenderMode::Solid
                    }
                );

                // swap buffers
                co_await renderer->present();
                
                co_return;
            }
        );

        // start fixed step loop
        co_await loop.run();

        renderer->join();

        if(window->good())co_await window->close();

        co_await saveWorldToDir(world, components_serializer_reg, getResourcesPath() / "saves", use_binary);

        spdlog::debug("Velora main finished with code {}", 0);

        co_return 0;
    }
}