#pragma once

#include <utility>
#include <functional>

#include "native.hpp"
#include <asio.hpp>

namespace velora
{
    struct WindowCallbacks
    {
        asio::any_io_executor executor;
        
        std::function<asio::awaitable<void>()> onDestroy;
        std::function<asio::awaitable<void>(int, int)> onResize;
        std::function<asio::awaitable<void>()> onMove;
        std::function<asio::awaitable<void>()> onFocus;
        std::function<asio::awaitable<void>()> onUnfocus;
        std::function<asio::awaitable<void>(int)> onKeyPress;
        std::function<asio::awaitable<void>(int)> onKeyRelease;
    };
}
