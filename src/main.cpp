#include "velora.hpp"

namespace velora
{
    // asio::awaitable<Renderer> constructRenderer(asio::io_context & io_context, IProcess & process, IWindow & window, int minor_version, int major_version)
    // {
    //     auto oglctx_handle = co_await process.registerOGLContext(window.getHandle(), minor_version, major_version);
    //     co_return Renderer::construct<opengl::OpenGLRenderer>(io_context, oglctx_handle);
    // }

    asio::awaitable<int> main(asio::io_context & io_context, IProcess & process)
    {
        auto main_strand = asio::make_strand(io_context);

        spdlog::debug(std::format("[t:{}] Velora main started", std::this_thread::get_id()));

        auto window = co_await Window::construct<winapi::WinapiWindow>(asio::use_awaitable, io_context, process, "Velora", Resolution{256, 512});
        if (window->good() == false)
        {
            spdlog::error("Failed to create window");
            co_return -1;
        }
        ShowWindow(window->getHandle(), SW_SHOW);

        auto renderer = co_await Renderer::construct<opengl::OpenGLRenderer>(asio::use_awaitable, io_context, *window, 4, 0);
        if (renderer->good() == false)
        {
            spdlog::error("Failed to create renderer");
            co_return -1;
        }

        co_await asio::dispatch(asio::bind_executor(main_strand, asio::use_awaitable));


        int return_value = 0;


        int i = 1000;
        while(i-- > 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        
        spdlog::debug("Velora main finished with code {}", return_value);
        co_return return_value;
    }
}