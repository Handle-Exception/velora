#pragma once

#include <expected>
#include <string>

#include <GL/glew.h>
#include <spdlog/spdlog.h>

namespace velora::opengl
{
    std::expected<void, std::string> checkOpenGLState();
}