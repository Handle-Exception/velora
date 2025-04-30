#pragma once

#include <chrono>
using namespace std::chrono_literals;
#include <filesystem>
#include <String>

#include "native.hpp"
#include "entry.hpp"
#include "type.hpp"
#include "events.hpp"
#include "resolution.hpp"
#include "process.hpp"
#include "window.hpp"
#include "render.hpp"
#include "ecs.hpp"
#include "game.hpp"

#ifdef WIN32
#include "process_winapi.hpp"
#include "window_winapi.hpp"
#endif

#include "opengl.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <asio.hpp>
#include <asio/experimental/awaitable_operators.hpp>
using namespace asio::experimental::awaitable_operators;

#include <glm/glm.hpp>

#include <absl/hash/hash.h>

namespace velora
{
}