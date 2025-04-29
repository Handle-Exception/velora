#include "velora.hpp"

const std::string & getAsciiLogo()
{
    static const std::string logo = R"(
____   ____     .__                       
\   \ /   /____ |  |   ________________   
 \   Y   // __ \|  |  /  _ \_  __ \__  \  
  \     /\  ___/|  |_(  <_> )  | \// __ \_
   \___/  \___  >____/\____/|__|  (____  /
              \/                       \/              
    )";
    
    return logo;
}

int main(int argc, char** argv)
{
    spdlog::set_level(spdlog::level::debug);

    spdlog::info("{}", getAsciiLogo());

    spdlog::info("Version: {}.{}.{}\n", velora::MAJOR_VERSION, velora::MINOR_VERSION, velora::PATCH_VERSION);

    spdlog::info("Velora started with {} arguments", argc);
    for(int i = 0; i < argc; ++i)
    {
        spdlog::info("Argument at [{}] = {}", i, argv[i]);
    }

    spdlog::info("Current working path: {}", std::filesystem::current_path().string());

    asio::io_context io_context;
    
    velora::EntityManager entity_manager;
    velora::ComponentManager component_manager(entity_manager);

    velora::Entity player = entity_manager.createEntity();
    velora::Entity enemy = entity_manager.createEntity();

    component_manager.addComponent<velora::game::PositionComponent>(player, {0.0f, 0.0f});
    component_manager.addComponent<velora::game::HealthComponent>(player, {100});

    component_manager.addComponent<velora::game::PositionComponent>(enemy, {10.0f, 5.0f});


    int return_value = 0;
    
    try
    {
        io_context.run();
    }
    catch(std::exception & e)
    {
        spdlog::error("Error: {}", e.what());
    }
    
    spdlog::debug("Program finished");

    return return_value;
}