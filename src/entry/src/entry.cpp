#include "entry.hpp"

using namespace velora;

namespace velora
{
    static std::filesystem::path BIN_PATH = "";
    static std::filesystem::path RESOURCES_PATH = "";

    std::filesystem::path getBinPath()
    {
        return BIN_PATH;
    }

    std::filesystem::path getResourcesPath()
    {
        return RESOURCES_PATH;
    }
}

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
    #ifdef GOOGLE_PROTOBUF_VERSION
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    #endif

    const unsigned hardware_cores = std::thread::hardware_concurrency();
    const unsigned used_cores = 4;

    assert(used_cores <= hardware_cores);

    spdlog::set_level(spdlog::level::debug);

    spdlog::info("{}", getAsciiLogo());

    spdlog::info("Version: {}.{}.{}\n", MAJOR_VERSION, MINOR_VERSION, PATCH_VERSION);

    spdlog::info("Velora started with {} arguments", argc);
    for(int i = 0; i < argc; ++i)
    {
        spdlog::info("Argument at [{}] = {}", i, argv[i]);
    }

    BIN_PATH = std::filesystem::path(argv[0]).parent_path();
    RESOURCES_PATH = BIN_PATH.parent_path() / "resources";

    spdlog::info("Current working path: {}", std::filesystem::current_path().string());
    spdlog::info("/bin path: {}", getBinPath().string());
    spdlog::info("/resources path: {}", getResourcesPath().string());

    asio::io_context io_context(used_cores);

    // Create a process
    Process process = Process::construct<winapi::WinapiProcess>();

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
    
    std::vector<std::thread> worker_threads;
    worker_threads.reserve(used_cores);
    for(unsigned i = 0; i < used_cores; ++i)
    {
        worker_threads.emplace_back([&io_context]()
        {
            #ifdef WIN32 //TODO
            SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
            #endif
            
            try
            {
                io_context.run();
            }
            catch(std::exception & e)
            {
                spdlog::error("Error: {}", e.what());
            }  
        });
    }
    // Wait for all threads to finish
    for(auto & thread : worker_threads)
    {
        thread.join();
    }
    
    // Wait for the external main to finish
    int return_value = external_main_future.get();

    process->join();

    spdlog::debug("Program finished with code {}", return_value);

    #ifdef GOOGLE_PROTOBUF_VERSION
    google::protobuf::ShutdownProtobufLibrary();
    #endif

    return return_value;
}