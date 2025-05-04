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
        assert(_window_handle == nullptr && "Winapi window should be closed before destruction" );
    }

    IProcess & WinapiWindow::getProcess()
    {
        return _process;
    }

    asio::awaitable<void> WinapiWindow::show()
    {
        if(!_strand.running_in_this_thread()){
            co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));
        }

        ShowWindowAsync(_window_handle, SW_SHOW);
        co_return;
    }

    asio::awaitable<void> WinapiWindow::hide()
    {
        if(!_strand.running_in_this_thread()){
            co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));
        }

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
        if(!_strand.running_in_this_thread()){
            co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));
        }

        if(_window_handle == nullptr)co_return;

        auto handle = _window_handle;
        _window_handle = nullptr;

        co_await _process.unregisterWindow(handle);
        co_return;
    }

    native::device_context WinapiWindow::acquireDeviceContext()
    {
        return GetDC(_window_handle);
    }

    bool WinapiWindow::releaseDeviceContext(native::device_context device_context)
    {
        if(device_context == NULL)
        {
            spdlog::error("Wrong device context");
            return false;
        }
        ReleaseDC(_window_handle, device_context);
        return true;
    }

}