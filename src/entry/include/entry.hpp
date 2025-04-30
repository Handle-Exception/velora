#pragma once

#include <utility>

#include "process.hpp"
#include "ecs.hpp"
#include "version.hpp"

#include <google/protobuf/descriptor.h>

#include <asio.hpp>

namespace velora
{
    // Must be defined by executable.
    // Engine will call this function on startup
    extern asio::awaitable<int> main(asio::io_context & io_context, IProcess & process);
}

int main(int argc, char** argv);