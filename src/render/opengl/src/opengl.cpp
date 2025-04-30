#include "opengl.hpp"

namespace velora::opengl
{
    asio::awaitable<OpenGLRenderer> OpenGLRenderer::asyncConstructor(asio::io_context & io_context, IWindow & window, int major_version, int minor_version)
    {
        auto window_handle = window.getHandle();
        auto & process = window.getProcess();

        native::opengl_context_handle oglctx_handle = co_await process.registerOGLContext(window_handle, major_version, minor_version);

        co_return OpenGLRenderer(io_context, window, oglctx_handle);
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

    bool OpenGLRenderer::good() const
    {
        return _oglctx_handle != nullptr;
    }

    asio::awaitable<void> OpenGLRenderer::destroy()
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));
        _oglctx_handle = nullptr;
        co_return;
    }

    asio::awaitable<void> OpenGLRenderer::close()
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));
        co_await _window.getProcess().unregisterOGLContext(_oglctx_handle);
        co_await destroy();
        co_return;
    }
}