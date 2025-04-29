#pragma once

#include <chrono>
using namespace std::chrono_literals;
#include <filesystem>
#include <String>

#include "native.hpp"
#include "ecs.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <asio.hpp>
#include <asio/experimental/awaitable_operators.hpp>
using namespace asio::experimental::awaitable_operators;


namespace velora
{
    const short int MAJOR_VERSION = 0;
    const short int MINOR_VERSION = 0;
    const short int PATCH_VERSION = 1;

    const short int DEFAULT_PORT = 5555;
}