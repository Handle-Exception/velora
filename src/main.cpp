#include "velora.hpp"

namespace velora
{
    asio::awaitable<int> main(asio::io_context & io_context, IProcess & process)
    {
        // create system objects

        auto main_strand = asio::make_strand(io_context);

        spdlog::debug(std::format("[t:{}] Velora main started", std::this_thread::get_id()));

        auto window = co_await Window::construct<winapi::WinapiWindow>(asio::use_awaitable, io_context, process, "Velora", Resolution{512, 256});
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
                .onResize = [&window](int width, int height) -> asio::awaitable<void>
                {
                    spdlog::info(std::format("WindowCallbacks Resized {}x{}", width, height));
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

        auto id_res = co_await renderer->constructVertexBuffer({0,1,2}, {Vertex{{0,0,0}, {1,0,0}, {0,1}}});

        if(!id_res)
        {
            spdlog::error("Failed to create vertex buffer");
            co_return -1;
        }

        if((co_await renderer->eraseVertexBuffer(id_res.value())) == false)
        {
            spdlog::error("Failed to erase vertex buffer");
            co_return -1;
        }

        co_await window->show();

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
        game::World world(io_context, *renderer);

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
        world.getCurrentLevel().addComponent(*player_entity, game::TransformComponent{.position = {0, 0, 0}});
        world.getCurrentLevel().addComponent(*player_entity, game::HealthComponent{.health = 100});
        world.getCurrentLevel().addComponent(*player_entity, game::VisualComponent{});
        world.getCurrentLevel().addComponent(*player_entity, game::InputComponent{});

        auto player_transform_component =  world.getCurrentLevel().getComponent<game::TransformComponent>(*player_entity);
        if(player_transform_component == nullptr)
        {
            spdlog::error("Failed to get player transform component");
            co_return -1;
        }

        player_transform_component->position = {0, 10, 0};
        player_transform_component->rotation = glm::vec3(0.0f, glm::radians(90.0f), 0.0f);
        player_transform_component->scale = {1, 1, 1};


        auto input_component = world.getCurrentLevel().getComponent<game::InputComponent>(*player_entity);
        auto transform_component = world.getCurrentLevel().getComponent<game::TransformComponent>(*player_entity);

        while (window->good()) 
        {

            if(input_component->action == 38)
            {
                spdlog::info("Player pressed action 38");
                transform_component->position.y += 0.1f;
                spdlog::info(std::format("Player position: {} {} {}", transform_component->position.x, transform_component->position.y, transform_component->position.z));
            }


            // clear window
            
            // swap buffers
            //co_await renderer->present();
            // update window
            //co_await window->present();
            // apply input
            co_await world.getCurrentLevel().runSystem(input_system);
            // update world state
            co_await world.update();
        }

        if(renderer->good())co_await renderer->close();
        if(window->good())co_await window->close(); // here we want to notifiy the process to close the window

        spdlog::debug("Velora main finished with code {}", 0);
        co_return 0;
    }
}