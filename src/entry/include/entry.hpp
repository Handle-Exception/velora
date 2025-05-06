#pragma once

#include <utility>

#include "process.hpp"
#include "version.hpp"

#if defined(_WIN32) || defined(_WIN64)
#include "process_winapi.hpp"
#endif

#ifdef GOOGLE_PROTOBUF_VERSION
#include <google/protobuf/descriptor.h>
#endif

#include "native.hpp"
#include <asio.hpp>

#include "fixed_step_loop.hpp"

namespace velora
{
    std::filesystem::path getBinPath();
    std::filesystem::path getResourcesPath();

    // Must be defined by executable.
    // Engine will call this function on startup
    extern asio::awaitable<int> main(asio::io_context & io_context, IProcess & process);
}

int main(int argc, char** argv);