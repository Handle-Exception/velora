#include "opengl_debug.hpp"

namespace velora::opengl
{
    bool logOpenGLState()
    {
        switch(glGetError())
        {
            case GL_INVALID_VALUE:
            spdlog::error(std::format("[opengl] Invalid opengl object"));
            return false;

            case GL_INVALID_OPERATION:
            spdlog::error(std::format("[opengl] Invalid operation"));
            return false;

            case GL_INVALID_FRAMEBUFFER_OPERATION:
            spdlog::error(std::format("[opengl] The framebuffer object is not complete"));
            return false;

            case GL_OUT_OF_MEMORY:
            spdlog::error(std::format("[opengl] OpenGL is out of memory"));
            return false;

            case GL_STACK_UNDERFLOW:
            spdlog::error(std::format("[opengl] An attempt has been made to perform an operation that would cause an internal stack to underflow"));
            return false;

            case GL_STACK_OVERFLOW:
            spdlog::error(std::format("[opengl] An attempt has been made to perform an operation that would cause an internal stack to overflow"));
            return false;

            case GL_NO_ERROR:
            spdlog::debug(std::format("[opengl] OpenGL OK state"));
            return true;

            default:
            spdlog::warn(std::format("[opengl] Unidentified OpenGL problem"));
            return false;
        }
    }
}