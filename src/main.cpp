#include "velora.hpp"

namespace velora
{
    // TODO
    void moveCamera(const std::chrono::duration<double> & delta, game::TransformComponent * camera_transform_component, game::InputComponent * camera_input_component)
    {
        static glm::vec3 camera_position{0,0,0};
        static const float speed = 0.3f;
        
        if(isInputPresent(game::InputCode::KEY_A, camera_input_component->pressed()))
                camera_position.x -= speed * (float)delta.count();
        if(isInputPresent(game::InputCode::KEY_D, camera_input_component->pressed()))
                camera_position.x += speed * (float)delta.count();

        if(isInputPresent(game::InputCode::KEY_W, camera_input_component->pressed()))
                camera_position.z -= speed * (float)delta.count();
        if(isInputPresent(game::InputCode::KEY_S, camera_input_component->pressed()))
                camera_position.z += speed * (float)delta.count();

        if(isInputPresent(game::InputCode::KEY_Q, camera_input_component->pressed()))
                camera_position.y += speed * (float)delta.count();
        if(isInputPresent(game::InputCode::KEY_E, camera_input_component->pressed()))
                camera_position.y -= speed * (float)delta.count();
            

        camera_transform_component->mutable_position()->set_x(camera_position.x);
        camera_transform_component->mutable_position()->set_y(camera_position.y);
        camera_transform_component->mutable_position()->set_z(camera_position.z);
    }

    void movePlayer(const std::chrono::duration<double> & delta, game::TransformComponent * player_transform_component)
    {
        static float pitch = 45.0f; // X axis
        static float yaw   = 90.0f; // Y axis
        static float roll  = 30.0f; // Z axis
        static glm::quat player_rotation;
        static glm::vec3 player_position{0,0,0};

        pitch += 0.05f * (float)delta.count();
        yaw   += 0.1f * (float)delta.count();
        roll  += 0.03f * (float)delta.count();

        player_rotation = glm::radians(glm::vec3(pitch, yaw, roll));

        player_transform_component->mutable_rotation()->set_w(player_rotation.w);
        player_transform_component->mutable_rotation()->set_x(player_rotation.x);
        player_transform_component->mutable_rotation()->set_y(player_rotation.y);
        player_transform_component->mutable_rotation()->set_z(player_rotation.z);

        player_transform_component->mutable_position()->set_x(player_position.x);
        player_transform_component->mutable_position()->set_y(player_position.y);
        player_transform_component->mutable_position()->set_z(player_position.z);   
    }




    asio::awaitable<int> main(asio::io_context & io_context, IProcess & process)
    {
        // create system objects

        auto main_strand = asio::make_strand(io_context);

        spdlog::debug(std::format("[t:{}] Velora main started", std::this_thread::get_id()));

        auto window = co_await Window::construct<winapi::WinapiWindow>(asio::use_awaitable, io_context, process, "Velora", Resolution{256, 512});
        if (window->good() == false)
        {
            spdlog::error("Failed to create window");
            co_return -1;
        }

        auto renderer = co_await Renderer::construct<opengl::OpenGLRenderer>(asio::use_awaitable, io_context, *window, 4, 0);
        if (renderer->good() == false)
        {
            spdlog::error("Failed to create renderer");
            co_return -1;
        }

        co_await renderer->enableVSync();

        co_await asio::dispatch(asio::bind_executor(main_strand, asio::use_awaitable));

        // create input system

        game::InputSystem input_system(io_context);

        co_await process.setWindowCallbacks(window->getHandle(), 
            WindowCallbacks{.executor = main_strand,
                // process notifies us when the window is destroyed
                .onDestroy = [&window, &renderer]() -> asio::awaitable<void>
                {
                    spdlog::info(std::format("WindowCallbacks Destroyed [{}]", std::this_thread::get_id()));
                    co_await renderer->close(); // so we notify process to close the renderer
                    co_await window->close(); // and destroy window object
                    co_return;
                },
                .onResize = [&window, &renderer](int width, int height) -> asio::awaitable<void>
                {
                    spdlog::info(std::format("WindowCallbacks Resized {}x{}", width, height));
                    //co_await window->resize(width, height);
                    co_await renderer->updateViewport(Resolution{(std::size_t)width, (std::size_t)height});
                    co_return;
                },
                .onKeyPress = [&window, &renderer, &input_system](int key) -> asio::awaitable<void>
                {                                        
                    // need to propagate this information into InputSystem
                    co_await input_system.recordKeyPressed(game::keyToInputCode(key));
                    co_return;
                },
                .onKeyRelease = [&window, &input_system](int key) -> asio::awaitable<void>
                {
                    // need to propagate this information into InputSystem
                    co_await input_system.recordKeyReleased(game::keyToInputCode(key));
                    co_return;
                }
            });


        // ---------------------------------------------------------------------------------------------------------------------------------------------
        // LOADING ASSETS
        // ---------------------------------------------------------------------------------------------------------------------------------------------
        auto vb_id = co_await renderer->constructVertexBuffer("vertex_buffer_0", getCubePrefab());

        if(!vb_id)
        {
            spdlog::error("Failed to create vertex buffer");
            co_return -1;
        }


        auto sh_id = co_await loadShaderFromFile(*renderer, "shaders/glsl/basic_shader");

        if(!sh_id)
        {
            spdlog::error("Failed to create Shader");
            co_return -1;
        }
        // ---------------------------------------------------------------------------------------------------------------------------------------------
        // 
        // ---------------------------------------------------------------------------------------------------------------------------------------------


        // create level in world in which we will spawn entities
        // level is divided in chunks which is basically octree
        // level should have init, update and shutdown methods

        // then update all subsystems
        // input is handled in other thread
        // it calls callbacks in main_strand
        // 
        // callbacks are connected to controller component
        // controller component stores actions
        // then eg. Transform component uses actions to move entity
        // eg. Shoot component uses actions to fire bullet ect.
        //
        // then Visual Component uses Transform component to calculate matrices and draw entity
        // then Physics component uses Transform component to move entity

        // Create game subsystems

        // create world

        ComponentLoaderRegistry components_loader_reg = constructComponentLoaderRegistry();
        ComponentSerializerRegistry components_serializer_reg = constructComponentSerializerRegistry();

        game::World world(io_context, *renderer);

        game::VisualSystem visual_system(*renderer, world.getSystem("CameraSystem"));


        // ---------------------------------------------------------------------------------------------------------------------------------------------
        // LOADING LEVEL
        // ---------------------------------------------------------------------------------------------------------------------------------------------

        co_await loadLevelFromFile(world, components_loader_reg, getResourcesPath() / "levels/level_0.json" );
        world.setCurrentLevel("level_0");
        // ---------------------------------------------------------------------------------------------------------------------------------------------
        // 
        // ---------------------------------------------------------------------------------------------------------------------------------------------
        auto player_entity = world.getCurrentLevel().getEntity("player");
        auto camera_entity = world.getCurrentLevel().getEntity("camera");

        if(!player_entity || !camera_entity)
        {
            spdlog::error("Failed to find entities");
            co_return -1;
        }
        
        auto player_input_component = world.getCurrentLevel().getComponent<game::InputComponent>(*player_entity);
        auto player_transform_component = world.getCurrentLevel().getComponent<game::TransformComponent>(*player_entity);
        auto player_visual_component = world.getCurrentLevel().getComponent<game::VisualComponent>(*player_entity);
        auto player_health_component = world.getCurrentLevel().getComponent<game::HealthComponent>(*player_entity);
        
        auto camera_input_component = world.getCurrentLevel().getComponent<game::InputComponent>(*camera_entity);
        auto camera_transform_component = world.getCurrentLevel().getComponent<game::TransformComponent>(*camera_entity);
        auto camera_camera_component = world.getCurrentLevel().getComponent<game::CameraComponent>(*camera_entity);

        co_await window->show();

        using clock = std::chrono::high_resolution_clock;
        constexpr const std::chrono::duration<double> FIXED_DELTA = 16.66ms; // 60Hz logic update
        constexpr const double MAX_ACCUMULATED_TIME = 0.25f; // avoid spiral of death
        uint64_t simulation_tick = 0;
        std::chrono::steady_clock::time_point now = clock::now();
        auto previous_time = clock::now();
        std::chrono::duration<double> frame_time;
        double delta = 0.0;
        double lag = 0.0;

        FpsCounter fps_counter;
        auto last_log_time = clock::now();

        while (window->good() && renderer->good()) 
        {
            now = clock::now();
            frame_time = now - previous_time;
            // Clamp to avoid spiral of death
            delta = std::min(frame_time.count(), MAX_ACCUMULATED_TIME);
            lag += delta;

            fps_counter.frame();

            // Input runs every frame, regardless of update rate
            co_await world.getCurrentLevel().runSystem(input_system);
        
            // track fps frames for profiling
            if (now - last_log_time >= std::chrono::seconds(3)) {
                spdlog::info(std::format("[t:{}] FPS: {:.1f}", std::this_thread::get_id(), fps_counter.getFPS()));
                last_log_time = now;
            }

            // Apply fixed time steps
            while (lag >= FIXED_DELTA.count()) 
            {
                //TODO
                // Store tick count in input for deterministic use
                //world.getCurrentLevel().visitComponents<InputComponent>([&](InputComponent& input) {
                //    input.tick = simulation_tick;
                //});

                // TODO
                moveCamera(FIXED_DELTA, camera_transform_component, camera_input_component);
                movePlayer(FIXED_DELTA, player_transform_component);

                // Fixed time update
                co_await world.update(FIXED_DELTA); // assumes systems respect fixed delta

                simulation_tick++;
                lag -= FIXED_DELTA.count();
            }

            // clear window
            co_await renderer->clearScreen({1.0f, 1.0f, 1.0f, 1.0f});
            
            // visual system will render entities with visual component
            co_await world.getCurrentLevel().runSystem(visual_system);

            // swap buffers
            co_await renderer->present();
        }

        if(renderer->good())co_await renderer->close();
        if(window->good())co_await window->close();

        co_await saveWorldToFiles(world, components_serializer_reg, getResourcesPath() / "saves" );

        spdlog::debug("Velora main finished with code {}", 0);

        co_return 0;
    }
}