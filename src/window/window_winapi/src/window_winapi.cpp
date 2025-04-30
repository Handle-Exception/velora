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
        
        asio::co_spawn(_io_context, _process.unregisterWindow(_window_handle), asio::detached);
        _window_handle = nullptr;
        spdlog::debug("[window] Winapi window destroyed");
    }
}