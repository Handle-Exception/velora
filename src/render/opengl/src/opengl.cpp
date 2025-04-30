#include "opengl.hpp"

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

    OpenGLRenderer::OpenGLRenderer(asio::io_context & io_context, IWindow & window, native::opengl_context_handle oglctx_handle)
    :   _io_context(io_context),
        _strand(asio::make_strand(io_context)),
        _oglctx_handle(oglctx_handle),
        _window(window)

    {
        spdlog::info("[render] OpenGL renderer created");
    }

    OpenGLRenderer::OpenGLRenderer(OpenGLRenderer && other)
    :   _io_context(other._io_context),
        _strand(std::move(other._strand)),
        _oglctx_handle(other._oglctx_handle),
        _window(other._window)
    {
        other._oglctx_handle = nullptr;
    }


    OpenGLRenderer::~OpenGLRenderer()
    {
        if(_oglctx_handle == nullptr)return;
        
        asio::co_spawn(_io_context, _window.getProcess().unregisterOGLContext(_oglctx_handle), asio::detached);
        _oglctx_handle = nullptr;
        spdlog::debug("[render] OpenGL renderer destroyed");
    }
}