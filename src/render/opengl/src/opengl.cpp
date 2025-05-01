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

        _vertex_buffers(),
        _shaders()

    {
        spdlog::info("[render] OpenGL renderer created");
    }

    OpenGLRenderer::OpenGLRenderer(OpenGLRenderer && other)
    :   _io_context(other._io_context),
        _window(other._window),
        _strand(std::move(other._strand)),
        _oglctx_handle(other._oglctx_handle),

        _vertex_buffers(std::move(other._vertex_buffers)),
        _shaders(std::move(other._shaders))
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

        if(_oglctx_handle == nullptr)co_return;


        native::device_context device_context = _window.acquireDeviceContext();
        // bind context
        if(wglMakeCurrent(device_context, _oglctx_handle) == FALSE)
        {
            spdlog::error(std::format("[t{}] Cannot activate opengl context", std::this_thread::get_id()));
        }

        _vertex_buffers.clear();
        _shaders.clear();

        // unbind and realease
        wglMakeCurrent(0, 0);
        _window.releaseDeviceContext(device_context);
        
        auto handle = _oglctx_handle;
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

    asio::awaitable<std::optional<std::size_t>> OpenGLRenderer::constructShader(std::vector<const char *> vertex_code)
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));
        
        // bind context
        native::device_context device_context = _window.acquireDeviceContext();
        if(wglMakeCurrent(device_context, _oglctx_handle) == FALSE)
        {
            spdlog::error(std::format("[t{}] Cannot activate opengl context", std::this_thread::get_id()));
            co_return std::nullopt;
        }

        Shader shader = Shader::construct<OpenGLShader>(std::move(vertex_code));
        std::size_t id = shader->ID();

        if(_shaders.contains(id))co_return std::nullopt;

        _shaders.try_emplace(id, std::move(shader));

        // unbind context
        wglMakeCurrent(0, 0);
        _window.releaseDeviceContext(device_context);

        co_return id;
    }

    asio::awaitable<std::optional<std::size_t>> OpenGLRenderer::constructShader(std::vector<const char *> vertex_code, std::vector<const char *> fragment_code)
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));
        
        // bind context
        native::device_context device_context = _window.acquireDeviceContext();
        if(wglMakeCurrent(device_context, _oglctx_handle) == FALSE)
        {
            spdlog::error(std::format("[t{}] Cannot activate opengl context", std::this_thread::get_id()));
            co_return std::nullopt;
        }

        Shader shader = Shader::construct<OpenGLShader>(std::move(vertex_code), std::move(fragment_code));
        std::size_t id = shader->ID();

        if(_shaders.contains(id))co_return std::nullopt;

        _shaders.try_emplace(id, std::move(shader));

        // unbind context
        wglMakeCurrent(0, 0);
        _window.releaseDeviceContext(device_context);

        co_return id;
    }

    asio::awaitable<bool> OpenGLRenderer::eraseShader(std::size_t id)
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));

        if(_shaders.contains(id) == false)
        {
            spdlog::warn(std::format("[t{}] Shader {} does not exist", std::this_thread::get_id(), id));
            co_return false;
        }

        // bind context
        native::device_context device_context = _window.acquireDeviceContext();
        if(wglMakeCurrent(device_context, _oglctx_handle) == FALSE)
        {
            spdlog::error(std::format("[t{}] Cannot activate opengl context", std::this_thread::get_id()));
            co_return false;
        }

        _shaders.erase(id);

        spdlog::info(std::format("[t{}] Shader {} erased", std::this_thread::get_id(), id));

        // unbind context
        wglMakeCurrent(0, 0);
        _window.releaseDeviceContext(device_context);

        co_return true;
    }

    void OpenGLRenderer::assignShaderInputs(const std::size_t & shader_ID, const ShaderInputs & shader_inputs)
    {
        for(const auto & [name, val] : shader_inputs.in_int)
        {
            _shaders.at(shader_ID)->setUniform(name, val);
        }
        for(const auto & [name, val] : shader_inputs.in_float)
        {
            _shaders.at(shader_ID)->setUniform(name, val);
        }

        for(const auto & [name, val] : shader_inputs.in_vec2f)
        {
            _shaders.at(shader_ID)->setUniform(name, val);
        }
        for(const auto & [name, val] : shader_inputs.in_vec3f)
        {
            _shaders.at(shader_ID)->setUniform(name, val);
        }

        for(const auto & [name, val] : shader_inputs.in_mat2f)
        {
            _shaders.at(shader_ID)->setUniform(name, val);
        }
        for(const auto & [name, val] : shader_inputs.in_mat3f)
        {
            _shaders.at(shader_ID)->setUniform(name, val);
        }
        for(const auto & [name, val] : shader_inputs.in_mat4f)
        {
            _shaders.at(shader_ID)->setUniform(name, val);
        }
    }

    asio::awaitable<void> OpenGLRenderer::clearScreen(glm::vec4 color)
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));

        // bind context
        native::device_context device_context = _window.acquireDeviceContext();
        if(wglMakeCurrent(device_context, _oglctx_handle) == FALSE)
        {
            spdlog::error(std::format("[t{}] Cannot activate opengl context", std::this_thread::get_id()));
            co_return;
        }

        glClearColor(color.r, color.g, color.b, color.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // unbind context
        wglMakeCurrent(0, 0);
        _window.releaseDeviceContext(device_context);

        co_return;
    }

    asio::awaitable<void> OpenGLRenderer::render(std::size_t vertex_buffer_ID, std::size_t shader_ID, ShaderInputs shader_inputs)
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));

        auto shader_it = _shaders.find(shader_ID);
        if(shader_it == _shaders.end()){
            spdlog::error("Shader not found");
            co_return;
        }

        auto vertex_buffer_it = _vertex_buffers.find(vertex_buffer_ID);
        if(vertex_buffer_it == _vertex_buffers.end()){
            spdlog::error("Vertex buffer not found");
            co_return;
        }

        // bind context
        native::device_context device_context = _window.acquireDeviceContext();
        if(wglMakeCurrent(device_context, _oglctx_handle) == FALSE)
        {
            spdlog::error(std::format("[t{}] Cannot activate opengl context", std::this_thread::get_id()));
            co_return;
        }

        if(shader_it->second->enable() == false){
            spdlog::error("Cannot enable shader program");
            co_return;
        }

        if(vertex_buffer_it->second->enable() == false){
            spdlog::error("Cannot enable vertex buffer");
            co_return;
        }

        assignShaderInputs(shader_ID, shader_inputs);

        glDrawElements(GL_TRIANGLES, vertex_buffer_it->second->numberOfElements(), GL_UNSIGNED_INT, nullptr);

        // unbind context
        wglMakeCurrent(0, 0);
        _window.releaseDeviceContext(device_context);

        co_return;
    }

    asio::awaitable<void> OpenGLRenderer::present()
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));
        // bind context
        native::device_context device_context = _window.acquireDeviceContext();
        if(wglMakeCurrent(device_context, _oglctx_handle) == FALSE)
        {
            spdlog::error(std::format("[t{}] Cannot activate opengl context", std::this_thread::get_id()));
            co_return;
        }

        glFinish();
        SwapBuffers(device_context);

        // unbind context
        wglMakeCurrent(0, 0);
        _window.releaseDeviceContext(device_context);

        co_return co_await _window.present();
    }

    asio::awaitable<void> OpenGLRenderer::updateViewport(Resolution resolution)
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));
        
        // bind context
        native::device_context device_context = _window.acquireDeviceContext();
        if(wglMakeCurrent(device_context, _oglctx_handle) == FALSE)
        {
            spdlog::error(std::format("[t{}] Cannot activate opengl context", std::this_thread::get_id()));
            co_return;
        }

        glViewport(0, 0, (GLsizei)resolution.getWidth(), (GLsizei)resolution.getHeight());
        
        // unbind context
        wglMakeCurrent(0, 0);
        _window.releaseDeviceContext(device_context);

        co_return;
    }

}