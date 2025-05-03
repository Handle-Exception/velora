#pragma once

#include <utility>
#include <functional>
#include <filesystem>
#include <fstream>

#include "native.hpp"
#include <asio.hpp>
#include "entry.hpp"
#include "render.hpp"

namespace velora
{
    asio::awaitable<std::optional<std::size_t>> loadShader(IRenderer & renderer, std::filesystem::path shader_dir);
}
