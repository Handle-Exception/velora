#include "window_winapi.hpp"

namespace velora::winapi
{
    WinapiWindow::WinapiWindow(asio::io_context & io_context, IProcess & process, native::window_handle window_handle)
    :   _io_context(io_context),
        _strand(asio::make_strand(io_context)),
        _window_handle(window_handle),
        _process(process) 
    {
        spdlog::info("[window] Winapi window created");
    }

    WinapiWindow::WinapiWindow(WinapiWindow && other)
    :   _io_context(other._io_context),
        _strand(std::move(other._strand)),
        _window_handle(other._window_handle),
        _process(other._process)
    {
        other._window_handle = nullptr;
    }

    WinapiWindow::~WinapiWindow()
    {
        if(_window_handle == nullptr)return;
        
        close();
        spdlog::debug("[window] Winapi window destroyed");
    }

    IProcess & WinapiWindow::getProcess()
    {
        return _process;
    }

    asio::awaitable<void> WinapiWindow::show()
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));
        ShowWindowAsync(_window_handle, SW_SHOW);
        co_return;
    }

    asio::awaitable<void> WinapiWindow::hide()
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));
        ShowWindowAsync(_window_handle, SW_HIDE);
        co_return;
    }

    native::window_handle WinapiWindow::getHandle() const
    {
        return _window_handle;
    }

    bool WinapiWindow::good() const
    {
        return _window_handle != nullptr;
    }
    
    asio::awaitable<void> WinapiWindow::close()
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));

        PostMessage(_window_handle, WM_CLOSE, 0, 0);
        co_await destroy();
        co_return;
    }

    asio::awaitable<void> WinapiWindow::destroy()
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));
        _window_handle = nullptr;
        co_return;
    }

}