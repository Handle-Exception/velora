#include "opengl.hpp"

namespace velora::opengl
{
    asio::awaitable<OpenGLRenderer> OpenGLRenderer::asyncConstructor(asio::io_context & io_context, IWindow & window, int major_version, int minor_version)
    {
        auto window_handle = window.getHandle();
        native::opengl_context_handle oglctx_handle = co_await window.getProcess().registerOGLContext(window_handle, major_version, minor_version);

        co_return OpenGLRenderer(io_context, window, oglctx_handle);
    }

    OpenGLRenderer::OpenGLRenderer(asio::io_context & io_context, IWindow & window, native::opengl_context_handle oglctx_handle)
    :   _io_context(io_context),
        _window(window),
        _strand(asio::make_strand(io_context)),
        _oglctx_handle(oglctx_handle),

        _vertex_buffers()

    {
        spdlog::info("[render] OpenGL renderer created");
    }

    OpenGLRenderer::OpenGLRenderer(OpenGLRenderer && other)
    :   _io_context(other._io_context),
        _window(other._window),
        _strand(std::move(other._strand)),
        _oglctx_handle(other._oglctx_handle),

        _vertex_buffers(std::move(other._vertex_buffers))
    {
        other._oglctx_handle = nullptr;
    }


    OpenGLRenderer::~OpenGLRenderer()
    {
        assert(_oglctx_handle == nullptr && "OpenGL context should be closed before destruction" );
    }

    bool OpenGLRenderer::good() const
    {
        return _oglctx_handle != nullptr;
    }

    asio::awaitable<void> OpenGLRenderer::close()
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));
        auto handle = _oglctx_handle;

        if(_oglctx_handle == nullptr)co_return;

        native::device_context device_context = _window.acquireDeviceContext();
        // bind context
        if(wglMakeCurrent(device_context, _oglctx_handle) == FALSE)
        {
            spdlog::error(std::format("[t{}] Cannot activate opengl context", std::this_thread::get_id()));
        }

        _vertex_buffers.clear();

        // unbind and realease
        wglMakeCurrent(0, 0);
        _window.releaseDeviceContext(device_context);
        
        _oglctx_handle = nullptr;

        co_await _window.getProcess().unregisterOGLContext(handle);
        co_return;
    }

    asio::awaitable<std::optional<std::size_t>> OpenGLRenderer::constructVertexBuffer(std::vector<unsigned int> indices, std::vector<Vertex> vertices)
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));
        
        // bind context
        native::device_context device_context = _window.acquireDeviceContext();
        if(wglMakeCurrent(device_context, _oglctx_handle) == FALSE)
        {
            spdlog::error(std::format("[t{}] Cannot activate opengl context", std::this_thread::get_id()));
            co_return std::nullopt;
        }

        VertexBuffer vb = VertexBuffer::construct<OpenGLVertexBuffer>(std::move(indices), std::move(vertices));
        std::size_t id = vb->ID();

        if(_vertex_buffers.contains(id))co_return std::nullopt;

        _vertex_buffers.try_emplace(id, std::move(vb));

        // unbind context
        wglMakeCurrent(0, 0);
        _window.releaseDeviceContext(device_context);

        co_return id;
    }

    asio::awaitable<bool> OpenGLRenderer::eraseVertexBuffer(std::size_t id)
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));

        if(_vertex_buffers.contains(id) == false)
        {
            spdlog::warn(std::format("[t{}] Vertex buffer {} does not exist", std::this_thread::get_id(), id));
            co_return false;
        }

        // bind context
        native::device_context device_context = _window.acquireDeviceContext();
        if(wglMakeCurrent(device_context, _oglctx_handle) == FALSE)
        {
            spdlog::error(std::format("[t{}] Cannot activate opengl context", std::this_thread::get_id()));
            co_return false;
        }

        _vertex_buffers.erase(id);

        spdlog::info(std::format("[t{}] Vertex buffer {} erased", std::this_thread::get_id(), id));

        // unbind context
        wglMakeCurrent(0, 0);
        _window.releaseDeviceContext(device_context);

        co_return true;

    }
}