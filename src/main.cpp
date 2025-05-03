#include "velora.hpp"

namespace velora
{
    // todo move to a better place
    game::InputCode keyToInputCode(int key)
    {
        switch (key)
        {
            case 87: //W
                return game::InputCode::KEY_W;
            case 65: //A
                return game::InputCode::KEY_A;
            case 83: //S
                return game::InputCode::KEY_S;
            case 68: //D
                return game::InputCode::KEY_D;
            case 81: //Q
                return game::InputCode::KEY_Q;
            case 69: //E
                return game::InputCode::KEY_E;
            
            default:
                return game::InputCode::UNKNOWN;
        }
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
                    co_await input_system.recordKeyPressed(keyToInputCode(key));
                    co_return;
                },
                .onKeyRelease = [&window, &input_system](int key) -> asio::awaitable<void>
                {
                    // need to propagate this information into InputSystem
                    co_await input_system.recordKeyReleased(keyToInputCode(key));
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

        // create level
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

        float pitch = 45.0f; // X axis
        float yaw   = 90.0f; // Y axis
        float roll  = 30.0f; // Z axis

        glm::quat player_rotation;
        glm::vec3 player_position{0,0,0};
        glm::vec3 camera_position{0,0,0};

        co_await window->show();

        FpsCounter fps_counter;
        auto last_log_time = std::chrono::steady_clock::now();
        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

        while (window->good() && renderer->good()) 
        {

            // -----------------------------------------------------------------
            // CAMERA
            // -----------------------------------------------------------------

            if(isInputPresent(game::InputCode::KEY_A, camera_input_component->pressed()))
                camera_position.x -= 0.1f;
            if(isInputPresent(game::InputCode::KEY_D, camera_input_component->pressed()))
                camera_position.x += 0.1f;

            if(isInputPresent(game::InputCode::KEY_W, camera_input_component->pressed()))
                camera_position.z -= 0.1f;
            if(isInputPresent(game::InputCode::KEY_S, camera_input_component->pressed()))
                camera_position.z += 0.1f;

            if(isInputPresent(game::InputCode::KEY_Q, camera_input_component->pressed()))
                camera_position.y += 0.1f;
            if(isInputPresent(game::InputCode::KEY_E, camera_input_component->pressed()))
                camera_position.y -= 0.1f;
            

            camera_transform_component->mutable_position()->set_x(camera_position.x);
            camera_transform_component->mutable_position()->set_y(camera_position.y);
            camera_transform_component->mutable_position()->set_z(camera_position.z);

            // -----------------------------------------------------------------
            // PLAYER
            // -----------------------------------------------------------------

            pitch += 0.05f;
            yaw   += 0.1f;
            roll  += 0.03f;

            player_rotation = glm::radians(glm::vec3(pitch, yaw, roll));

            player_transform_component->mutable_rotation()->set_w(player_rotation.w);
            player_transform_component->mutable_rotation()->set_x(player_rotation.x);
            player_transform_component->mutable_rotation()->set_y(player_rotation.y);
            player_transform_component->mutable_rotation()->set_z(player_rotation.z);

            player_transform_component->mutable_position()->set_x(player_position.x);
            player_transform_component->mutable_position()->set_y(player_position.y);
            player_transform_component->mutable_position()->set_z(player_position.z);            

            // track fps frames for profiling
            fps_counter.frame();
            now = std::chrono::steady_clock::now();
            if (now - last_log_time >= std::chrono::seconds(3)) {
                spdlog::info("FPS: {:.1f}", fps_counter.getFPS());
                last_log_time = now;
            }
            // apply input
            co_await world.getCurrentLevel().runSystem(input_system);

            // clear window
            co_await renderer->clearScreen({1.0f, 1.0f, 1.0f, 1.0f});
            
            // update world state
            // visual system will render entities with visual component
            co_await world.update();

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