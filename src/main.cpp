#include "velora.hpp"

namespace velora
{

    struct FixedStepLoop
    {
        using clock = std::chrono::high_resolution_clock;

        FixedStepLoop(asio::io_context & io_context, const std::chrono::duration<double> fixed_logic_step,
                std::function<bool()> condition,
                std::function<asio::awaitable<void>(std::chrono::duration<double>)> logic,
                std::function<asio::awaitable<void>(float)> priority) 
        :   _strand(asio::make_strand(io_context)),
            _fixed_logic_step(std::move(fixed_logic_step)),
            _condition(std::move(condition)),
            _logic(std::move(logic)),
            _priority(std::move(priority)) 
        {}

        asio::awaitable<void> run()
        {
            spdlog::debug("Starting fixed step loop");

            FpsCounter priority_fps_counter;
            FpsCounter logic_fps_counter;

            _previous_alpha = 0.0f;
            _raw_alpha = 0.0f;

            auto last_log_time = clock::now();

            while (_condition()) 
            {
                _total_time.duration = _total_time.end - _total_time.start;
                _total_time.start = clock::now();

                // Clamp to avoid spiral of death
                _lag += _total_time.duration; //std::min(_total_time.duration, _MAX_ACCUMULATED_TIME);
                
                _logic_time.duration = _logic_time.end - _logic_time.start;
                _logic_time.start = clock::now();
                // Apply fixed time steps
                while (_lag >= _fixed_logic_step) 
                {
                    logic_fps_counter.frame();

                    // Fixed time update
                    co_await _logic(_fixed_logic_step);

                    _simulation_tick++;
                    _lag -= _fixed_logic_step;

                    //track fps frames for profiling
                    if (_total_time.start - last_log_time >= std::chrono::seconds(5)) 
                    {
                        spdlog::info(std::format(
                            "[t:{}]\n\tPriority FPS: {:.1f}\n\tLogic FPS:{:.1f}\n\tLast frame took: {:.1f}ms", 
                                std::this_thread::get_id(),
                                priority_fps_counter.getFPS(), 
                                logic_fps_counter.getFPS(),
                                _total_time.duration.count() * 1000.0f 
                            )
                        );
                       last_log_time = _total_time.start;
                    }
                }
                _logic_time.end = clock::now();

                _priority_time.duration = _priority_time.end - _priority_time.start;
                _priority_time.start = clock::now();

                priority_fps_counter.frame();
                
                _raw_alpha = _lag / _fixed_logic_step;
                _raw_alpha  = std::clamp(_raw_alpha, 0.0f, 1.0f);
                if (std::isnan(_raw_alpha)) _raw_alpha = 0.0f;

                // Exponential smoothing
                _alpha = std::lerp(_previous_alpha, _raw_alpha, _ALPHA_SMOOTHING);
                _previous_alpha = _alpha; // store for next frame

                co_await _priority(_alpha);

                _priority_time.end = clock::now();
                _total_time.end = clock::now();
            }
            spdlog::debug("fixed step loop ended");

            co_return;
        }

        private:
            asio::strand<asio::any_io_executor> _strand;

            const std::chrono::duration<double> _fixed_logic_step;

            const std::function<bool()> _condition;
            const std::function<asio::awaitable<void>(std::chrono::duration<double>)> _logic;
            const std::function<asio::awaitable<void>(float)> _priority;

            constexpr static const std::chrono::duration<double> _MAX_ACCUMULATED_TIME = std::chrono::milliseconds(25); // avoid spiral of death
            uint64_t _simulation_tick = 0;


            struct TimeSpent
            {
                std::chrono::steady_clock::time_point start = clock::now();
                std::chrono::steady_clock::time_point end = clock::now();
                std::chrono::duration<double> duration = std::chrono::duration<double>::zero();
            };

            TimeSpent _total_time;
            TimeSpent _logic_time;
            TimeSpent _priority_time;

            std::chrono::duration<double> _lag = std::chrono::duration<double>::zero();

            constexpr static const float _ALPHA_SMOOTHING = 0.5f;
            float _raw_alpha = 0.0f;
            float _previous_alpha = 0.0f;
            float _alpha = 0.0f;
    };


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
        // create system objects

        spdlog::debug(std::format("[t:{}] Velora main started", std::this_thread::get_id()));

        auto window = co_await Window::construct<winapi::WinapiWindow>(asio::use_awaitable, io_context, process, "Velora", Resolution{256, 512});
        if (window->good() == false)
        {
            spdlog::error("Failed to create window");
            co_return -1;
        }

        auto renderer = co_await Renderer::construct<opengl::OpenGLRenderer>(asio::use_awaitable, *window, 4, 0);
        if (renderer->good() == false)
        {
            spdlog::error("Failed to create renderer");
            co_return -1;
        }

        co_await renderer->enableVSync();

        // create input system
        auto input_strand = asio::make_strand(io_context);

        game::InputSystem input_system(io_context);

        co_await process.setWindowCallbacks(window->getHandle(), 
            WindowCallbacks{.executor = input_strand,
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


        game::World world(io_context, *renderer);

        game::CameraSystem camera_system(io_context, *renderer);
        game::VisualSystem visual_system(io_context, *renderer, camera_system);

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

        FixedStepLoop loop(io_context, 33.33ms, // fixed logic step 30 HZ update 
            [&window, &renderer]() -> bool // condition
            {
                return window->good() && renderer->good();
            },
            [&world, &input_system, &camera_transform_component, &camera_input_component, &player_transform_component]
            (std::chrono::duration<double> delta) -> asio::awaitable<void>  // logic
            {
                co_await world.getCurrentLevel().runSystem(input_system);
                co_await world.update(delta);
                // TODO
                moveCamera(delta, camera_transform_component, camera_input_component);
                movePlayer(delta, player_transform_component);

                co_return;
            },
            [&renderer, &world, &camera_system, &visual_system](float alpha) -> asio::awaitable<void> // priority as fast as possible
            {
                co_await (
                    world.getCurrentLevel().runSystem(camera_system, alpha) && 
                    renderer->clearScreen({1.0f, 1.0f, 1.0f, 1.0f})
                );
                
                // visual system will render entities with visual component
                // interpolate between current and previous transform using alpha
                co_await world.getCurrentLevel().runSystem(visual_system, alpha);

                // swap buffers
                co_await renderer->present();
                
                co_return;
            }
        );

        co_await loop.run();
        
        spdlog::debug("joining renderer thread");
        renderer->join();

        if(window->good())co_await window->close();

        co_await saveWorldToFiles(world, components_serializer_reg, getResourcesPath() / "saves" );

        spdlog::debug("Velora main finished with code {}", 0);

        co_return 0;
    }
}