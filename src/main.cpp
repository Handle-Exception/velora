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