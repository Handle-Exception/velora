#include "velora.hpp"

namespace velora
{
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
                .onKeyPress = [&window, &input_system](int key) -> asio::awaitable<void>
                {                    
                    spdlog::info(std::format("[t:{}] WindowCallbacks Key pressed {}", std::this_thread::get_id(), key));

                    // need to propagate this information into InputSystem
                    co_await input_system.recordInput(key);
                    co_return;
                }
            });


        // ---------------------------------------------------------------------------------------------------------------------------------------------
        // LOADING ASSETS
        // ---------------------------------------------------------------------------------------------------------------------------------------------
        auto vb_id = co_await renderer->constructVertexBuffer("vertex_buffer_0",
            {
                // Front face
                0, 1, 2,
                2, 3, 0,

                // Right face
                1, 5, 6,
                6, 2, 1,

                // Back face
                5, 4, 7,
                7, 6, 5,

                // Left face
                4, 0, 3,
                3, 7, 4,

                // Top face
                3, 2, 6,
                6, 7, 3,

                // Bottom face
                4, 5, 1,
                1, 0, 4
            },
            {
            // Front face
            {{-0.5f, -0.5f,  0.5f}, {1, 0, 0}, {0.0f, 0.0f}}, // 0
            {{ 0.5f, -0.5f,  0.5f}, {0, 1, 0}, {1.0f, 0.0f}}, // 1
            {{ 0.5f,  0.5f,  0.5f}, {0, 0, 1}, {1.0f, 1.0f}}, // 2
            {{-0.5f,  0.5f,  0.5f}, {1, 1, 0}, {0.0f, 1.0f}}, // 3

            // Back face
            {{-0.5f, -0.5f, -0.5f}, {1, 0, 1}, {1.0f, 0.0f}}, // 4
            {{ 0.5f, -0.5f, -0.5f}, {0, 1, 1}, {0.0f, 0.0f}}, // 5
            {{ 0.5f,  0.5f, -0.5f}, {1, 1, 1}, {0.0f, 1.0f}}, // 6
            {{-0.5f,  0.5f, -0.5f}, {0, 0, 0}, {1.0f, 1.0f}}, // 7
            }
        );

        if(!vb_id)
        {
            spdlog::error("Failed to create vertex buffer");
            co_return -1;
        }


        auto sh_id = co_await loadShader(*renderer, "shaders/glsl/basic_shader");

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

        // create level
        // ---------------------------------------------------------------------------------------------------------------------------------------------
        // LOADING LEVEL
        // ---------------------------------------------------------------------------------------------------------------------------------------------
        if(world.constructLevel("level_0") == false)
        {
            spdlog::error("Failed to create level");
            co_return -1;
        }
        world.setCurrentLevel("level_0");

        auto player_entity = world.getCurrentLevel().spawnEntity("player");
        if(player_entity.has_value() == false)
        {
            spdlog::error("Failed to spawn player entity");
            co_return -1;
        }

        // player entity definition
        // should be loaded from level file 
        // just like the rest of entities
        world.getCurrentLevel().addComponent(*player_entity, 
        game::TransformComponent{
                .position = {0, 0, 0},
                .rotation = glm::radians(glm::vec3(45.0f, 30.0f, 14.0f)),
                .scale = {1, 1, 1}}
        );

        world.getCurrentLevel().addComponent(*player_entity, game::HealthComponent());
        world.getCurrentLevel().getComponent<game::HealthComponent>(*player_entity)->set_health(100);

        world.getCurrentLevel().addComponent(*player_entity, game::VisualComponent());
                world.getCurrentLevel().getComponent<game::VisualComponent>(*player_entity)->set_visible(true);
        world.getCurrentLevel().getComponent<game::VisualComponent>(*player_entity)->set_vertex_buffer_name("vertex_buffer_0");
        world.getCurrentLevel().getComponent<game::VisualComponent>(*player_entity)->set_shader_name("basic_shader");

        world.getCurrentLevel().addComponent(*player_entity, game::InputComponent{});


        auto input_component = world.getCurrentLevel().getComponent<game::InputComponent>(*player_entity);
        auto transform_component = world.getCurrentLevel().getComponent<game::TransformComponent>(*player_entity);
        // ---------------------------------------------------------------------------------------------------------------------------------------------
        // 
        // ---------------------------------------------------------------------------------------------------------------------------------------------
        float pitch = 45.0f; // X axis
        float yaw   = 90.0f; // Y axis
        float roll  = 30.0f; // Z axis

        co_await window->show();

        while (window->good() && renderer->good()) 
        {
            pitch += 0.05f;
            yaw   += 0.1f;
            roll  += 0.03f;

            transform_component->rotation = glm::radians(glm::vec3(pitch, yaw, roll));

            if(input_component->action == 37)
            {
                transform_component->position.x -= 0.1f;
            }

            if(input_component->action == 38)
            {
                transform_component->position.y += 0.1f;
            }

            if(input_component->action == 39)
            {
                transform_component->position.x += 0.1f;
            }

            if(input_component->action == 40)
            {
                transform_component->position.y -= 0.1f;
            }

            // clear window
            co_await renderer->clearScreen({1.0f, 1.0f, 1.0f, 1.0f});
            
            // apply input
            co_await world.getCurrentLevel().runSystem(input_system);
            
            // update world state
            // visual system will render entities with visual component
            co_await world.update();

            // swap buffers
            co_await renderer->present(); 
        }

        if(renderer->good())co_await renderer->close();
        if(window->good())co_await window->close();

        spdlog::debug("Velora main finished with code {}", 0);

        co_return 0;
    }
}