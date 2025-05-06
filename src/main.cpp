#include "velora.hpp"

namespace velora
{
    // TODO
    void moveCamera(const std::chrono::duration<double> & delta, game::TransformComponent * camera_transform_component, game::InputComponent * camera_input_component)
    {
        static glm::vec3 camera_position{0,0,0};
        static const float speed = 5.0f;
        
        if(isInputPresent(game::InputCode::KEY_A, camera_input_component->pressed()) ||
            isInputPresent(game::InputCode::KEY_A, camera_input_component->just_pressed()))
                camera_position.x -= speed * (float)delta.count();
        if(isInputPresent(game::InputCode::KEY_D, camera_input_component->pressed()) ||
            isInputPresent(game::InputCode::KEY_D, camera_input_component->just_pressed()))
                camera_position.x += speed * (float)delta.count();

        if(isInputPresent(game::InputCode::KEY_W, camera_input_component->pressed()) || 
            isInputPresent(game::InputCode::KEY_W, camera_input_component->just_pressed()))
                camera_position.z -= speed * (float)delta.count();
        if(isInputPresent(game::InputCode::KEY_S, camera_input_component->pressed()) || 
            isInputPresent(game::InputCode::KEY_S, camera_input_component->just_pressed()))
                camera_position.z += speed * (float)delta.count();

        if(isInputPresent(game::InputCode::KEY_Q, camera_input_component->pressed()) || 
            isInputPresent(game::InputCode::KEY_Q, camera_input_component->just_pressed()))
                camera_position.y += speed * (float)delta.count();
        if(isInputPresent(game::InputCode::KEY_E, camera_input_component->pressed()) || 
            isInputPresent(game::InputCode::KEY_E, camera_input_component->just_pressed()))
                camera_position.y -= speed * (float)delta.count();
            

        camera_transform_component->mutable_position()->set_x(camera_position.x);
        camera_transform_component->mutable_position()->set_y(camera_position.y);
        camera_transform_component->mutable_position()->set_z(camera_position.z);
    }

    // TODO
    void movePlayer(const std::chrono::duration<double> & delta, game::TransformComponent * player_transform_component)
    {
        static float pitch = 45.0f; // X axis
        static float yaw   = 90.0f; // Y axis
        static float roll  = 30.0f; // Z axis

        static float pitch_speed = 10.0f; // X axis
        static float yaw_speed   = 20.0f; // Y axis
        static float roll_speed  = 30.0f; // Z axis

        static glm::quat player_rotation;
        static glm::vec3 player_position{0,0,0};

        pitch += pitch_speed * (float)delta.count();
        yaw   += yaw_speed * (float)delta.count();
        roll  += roll_speed * (float)delta.count();

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
        if((co_await renderer->constructVertexBuffer("vertex_buffer_0", getCubePrefab())) == std::nullopt)
        {
            spdlog::error("Failed to create vertex buffer");
            co_return -1;
        }

        if((co_await loadShaderFromFile(*renderer, "shaders/glsl/basic_shader")) == std::nullopt)
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
        // it calls callbacks in strand
        // 
        // callbacks are connected to controller component
        // controller component stores actions
        // then eg. Transform component uses actions to move entity
        // eg. Shoot component uses actions to fire bullet ect.
        //
        // then Visual Component uses Transform component to calculate matrices and draw entity
        // then Physics component uses Transform component to move entity

        // Create rendering systems
        game::CameraSystem camera_system(io_context, *renderer);
        game::LightSystem light_system(io_context, *renderer);
        game::VisualSystem visual_system(io_context, *renderer, camera_system, light_system);

        // create world - will create logic systems
        game::World world(io_context, *renderer);

        // ---------------------------------------------------------------------------------------------------------------------------------------------
        // LOADING LEVEL
        // ---------------------------------------------------------------------------------------------------------------------------------------------
        ComponentLoaderRegistry components_loader_reg = constructComponentLoaderRegistry();
        ComponentSerializerRegistry components_serializer_reg = constructComponentSerializerRegistry();

        co_await loadLevelFromFile(world, components_loader_reg, getResourcesPath() / "levels/level_0.json" );
        world.setCurrentLevel("level_0");
        // ---------------------------------------------------------------------------------------------------------------------------------------------
        // 
        // ---------------------------------------------------------------------------------------------------------------------------------------------
        // --- TODO --- 
        auto player_entity = world.getCurrentLevel().getEntity("player");
        auto camera_entity = world.getCurrentLevel().getEntity("camera");
        if(!player_entity || !camera_entity)
        {
            spdlog::error("Failed to find entities");
            co_return -1;
        }
        //auto player_input_component = world.getCurrentLevel().getComponent<game::InputComponent>(*player_entity);
        auto player_transform_component = world.getCurrentLevel().getComponent<game::TransformComponent>(*player_entity);
        //auto player_visual_component = world.getCurrentLevel().getComponent<game::VisualComponent>(*player_entity);
        //auto player_health_component = world.getCurrentLevel().getComponent<game::HealthComponent>(*player_entity);
        
        auto camera_input_component = world.getCurrentLevel().getComponent<game::InputComponent>(*camera_entity);
        auto camera_transform_component = world.getCurrentLevel().getComponent<game::TransformComponent>(*camera_entity);
        //auto camera_camera_component = world.getCurrentLevel().getComponent<game::CameraComponent>(*camera_entity);
        // ---------------------------------------------------------------------------------------------------------------------------------------------
        // 
        // ---------------------------------------------------------------------------------------------------------------------------------------------

        co_await window->show();

        FpsCounter priority_fps_counter;
        FpsCounter logic_fps_counter;
        std::chrono::high_resolution_clock::time_point last_log_time = std::chrono::high_resolution_clock::now();

        FixedStepLoop loop(io_context, 
            // fixed logic step 30 HZ update 
            33.33ms, 
            
            // loop condition
            [&window, &renderer]() -> bool 
            {
                return window->good() && renderer->good();
            },

            // logic loop to be executed at fixed time step 
            [&world, &input_system, &camera_transform_component, &camera_input_component, &player_transform_component, &last_log_time, &logic_fps_counter, &priority_fps_counter]
            (std::chrono::duration<double> delta) -> asio::awaitable<void>  
            {
                logic_fps_counter.frame();

                // update fetched input actions in entities
                // input itself is recorded asynchronousy in window callbacks
                co_await world.getCurrentLevel().runSystem(input_system);

                // world has topologicaly sorted graph of systems
                // and will execute them in layers which are independent of each other
                // this way we can run them in parallel
                co_await world.update(delta);

                // TODO
                moveCamera(delta, camera_transform_component, camera_input_component);
                movePlayer(delta, player_transform_component);

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
            [&renderer, &world, &camera_system, &light_system, &visual_system, &priority_fps_counter]
            (float alpha) -> asio::awaitable<void> 
            {
                priority_fps_counter.frame();

                // execute camera calculation in parallel with light calculation
                co_await (
                    world.getCurrentLevel().runSystem(light_system) && 
                    world.getCurrentLevel().runSystem(camera_system, alpha) 
                );
                
                co_await renderer->clearScreen({1.0f, 1.0f, 1.0f, 1.0f});

                // visual system will render entities with visual component
                // interpolate between current and previous transform using alpha
                co_await world.getCurrentLevel().runSystem(visual_system, alpha);

                // swap buffers
                co_await renderer->present();
                
                co_return;
            }
        );

        // start fixed step loop
        co_await loop.run();
        
        renderer->join();

        if(window->good())co_await window->close();

        co_await saveWorldToFiles(world, components_serializer_reg, getResourcesPath() / "saves" );

        spdlog::debug("Velora main finished with code {}", 0);

        co_return 0;
    }
}