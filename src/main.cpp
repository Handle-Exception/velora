#include "velora.hpp"

namespace velora
{
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

        auto renderer = co_await Renderer::construct<opengl::OpenGLRenderer>(asio::use_awaitable, io_context, *window, 4, 0);
        if (renderer->good() == false)
        {
            spdlog::error("Failed to create renderer");
            co_return -1;
        }

        co_await process.setWindowCallbacks(window->getHandle(), 
            WindowCallbacks{.executor = main_strand,
                // process notifies us when the window is destroyed
                .onDestroy = [&window, &renderer]() -> asio::awaitable<void>
                {
                    spdlog::info(std::format("WindowCallbacks Destroyed [{}]", std::this_thread::get_id()));
                    co_await renderer->close(); // so we notify process to close the renderer
                    co_await window->destroy(); // and destroy window object
                    co_return;
                },
                .onResize = [&window](int width, int height) -> asio::awaitable<void>
                {
                    spdlog::info(std::format("WindowCallbacks Resized {}x{}", width, height));
                    co_return;
                },
                .onKeyPress = [&window](int key) -> asio::awaitable<void>
                {
                    spdlog::info(std::format("WindowCallbacks Key pressed {}", key));
                    co_return;
                }
            });

        co_await asio::dispatch(asio::bind_executor(main_strand, asio::use_awaitable));
        co_await window->show();

        int return_value = 0;
        asio::steady_timer timer(main_strand);

        while (window->good()) 
        {
            timer.expires_after(std::chrono::seconds(1));
            co_await timer.async_wait(asio::use_awaitable);
        }

        if(renderer->good())co_await renderer->close();
        if(window->good())co_await window->close(); // here we want to notifiy the process to close the window

        spdlog::debug("Velora main finished with code {}", return_value);
        co_return return_value;
    }
}