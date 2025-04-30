#include "entry.hpp"

using namespace velora;

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

int main(int argc, char* argv[])
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    const unsigned hardwareCores = std::thread::hardware_concurrency();
    const unsigned usedCores = 4;

    spdlog::set_level(spdlog::level::debug);

    spdlog::info("{}", getAsciiLogo());

    spdlog::info("Version: {}.{}.{}\n", MAJOR_VERSION, MINOR_VERSION, PATCH_VERSION);

    spdlog::info("Velora started with {} arguments", argc);
    for(int i = 0; i < argc; ++i)
    {
        spdlog::info("Argument at [{}] = {}", i, argv[i]);
    }

    spdlog::info("Current working path: {}", std::filesystem::current_path().string());

    asio::io_context io_context;

    // Create a process
    Process process = Process::construct<winapi::WinapiProcess>();

    EntityManager entity_manager;
    ComponentManager component_manager(entity_manager);

    // start external entry point
    std::promise<int> external_main_promise;
    auto external_main_future = external_main_promise.get_future();

    asio::co_spawn(io_context, 
        main(io_context, *process), 
        // when main ended spawn process->close()
        // and set return values
        [&io_context, &process, &external_main_promise](std::exception_ptr, int r)
        {
            auto process_close_result = asio::co_spawn(io_context, process->close(), asio::use_future);
            process_close_result.get();
            external_main_promise.set_value(r);
        }
    );
    
    try
    {
        io_context.run();
    }
    catch(std::exception & e)
    {
        spdlog::error("Error: {}", e.what());
    }
    
    // Wait for the external main to finish
    int return_value = external_main_future.get();

    process->join();

    google::protobuf::ShutdownProtobufLibrary();

    spdlog::debug("Program finished with code {}", return_value);

    return return_value;
}