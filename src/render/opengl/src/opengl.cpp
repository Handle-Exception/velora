#include "opengl.hpp"

namespace velora::opengl
{
    OpenGLRenderer::RenderThreadContext::RenderThreadContext(IWindow & window, native::device_context * device, native::opengl_context_handle * oglctx)
    :   _io_context(1),
        _work_guard(asio::make_work_guard(_io_context.get_executor())),
        _strand(asio::make_strand(_io_context)), 
        _window(window), 
        _device_context(device),
        _oglctx_handle(oglctx), 
        _worker_thread(&OpenGLRenderer::RenderThreadContext::workerThread, this)
    {}

    OpenGLRenderer::RenderThreadContext::~RenderThreadContext()
    {
        join();
    }

    void OpenGLRenderer::RenderThreadContext::join()
    {
        if(_worker_thread.joinable() == false)return;

        spdlog::debug(std::format("[opengl] [t:{}] Render thread waiting to finish ... ", std::this_thread::get_id()));
        
        signalClose();
        _worker_thread.join();
        
        spdlog::debug(std::format("[opengl] [t:{}] Render thread finished", std::this_thread::get_id()));
    }

    const asio::strand<asio::io_context::executor_type> & OpenGLRenderer::RenderThreadContext::getStrand() const
    {
        return _strand;
    }

    asio::awaitable<void> OpenGLRenderer::RenderThreadContext::ensureOnStrand()
    {
        if(!_strand.running_in_this_thread()) {
            co_return co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));
        }

        co_return;
    }

    void OpenGLRenderer::RenderThreadContext::signalClose()
    {
        if(_work_guard.owns_work() == false)return;
        spdlog::debug(std::format("[opengl] [t:{}] Render thread signaled to close", std::this_thread::get_id()));
        _work_guard.reset();
    }


    bool OpenGLRenderer::RenderThreadContext::running() const
    {
        if(_work_guard.owns_work() == false)return false;
        if(_worker_thread.joinable() == false)return false;
        return true;
    }

    void OpenGLRenderer::RenderThreadContext::workerThread()
    {
        #ifdef WIN32 //TODO
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
        #endif

        spdlog::debug(std::format("[opengl] [t:{}] Render thread started", std::this_thread::get_id()));
                
        *(_device_context) = _window.acquireDeviceContext();
                
        // bind context
        if(wglMakeCurrent(*(_device_context), *(_oglctx_handle)) == FALSE)
        {
            spdlog::error(std::format("[t:{}] Cannot activate OpenGL context", std::this_thread::get_id()));
            return;
        }

        glEnable(GL_DEPTH_TEST);

        try
        {
            _io_context.run();
        }
        catch(const std::exception & e)
        {
            spdlog::error("[render] OpenGL render thread exception: {}", e.what());
        }
    
        // unbind context
        wglMakeCurrent(0, 0);
        _window.releaseDeviceContext(*(_device_context));

        spdlog::debug(std::format("[opengl] [t:{}] Render thread ended", std::this_thread::get_id()));
    }


    asio::awaitable<OpenGLRenderer> OpenGLRenderer::asyncConstructor(IWindow & window, int major_version, int minor_version)
    {
        auto window_handle = window.getHandle();
        native::opengl_context_handle oglctx_handle = co_await window.getProcess().registerOGLContext(window_handle, major_version, minor_version);

        co_return OpenGLRenderer(window, oglctx_handle);
    }

    OpenGLRenderer::OpenGLRenderer(IWindow & window, native::opengl_context_handle oglctx_handle)
    :   _window(window),
        
        _oglctx_handle(std::make_unique<native::opengl_context_handle>(oglctx_handle)),
        _device_context(std::make_unique<native::device_context>(nullptr)),

        _vertex_buffers(),
        _shaders(),

        _viewport_resolution(0, 0),

        _render_context(std::make_unique<RenderThreadContext>(_window, _device_context.get(), _oglctx_handle.get()))
    {
        spdlog::info("[render] OpenGL renderer created");
    }

    OpenGLRenderer::OpenGLRenderer(OpenGLRenderer && other)
    :   _window(other._window),
        
        _render_context(std::move(other._render_context)),
        _oglctx_handle(std::move(other._oglctx_handle)),
        _device_context(std::move(other._device_context)),

        _vertex_buffers(std::move(other._vertex_buffers)),
        _shaders(std::move(other._shaders)),

        _viewport_resolution(std::move(other._viewport_resolution))
    {
        other._oglctx_handle = nullptr;
    }


    OpenGLRenderer::~OpenGLRenderer()
    {
        join();
        
        assert(_render_context == nullptr && "OpenGL context should be closed before destruction" );
        assert(_oglctx_handle == nullptr && "OpenGL context should be closed before destruction" );
        assert(_device_context == nullptr && "OpenGL context should be closed before destruction" );
    }

    void OpenGLRenderer::join()
    {
        if(_render_context == nullptr) return;

        spdlog::debug(std::format("[opengl] [t:{}] OpenGL renderer join", std::this_thread::get_id()));
        
        asio::co_spawn(_render_context->getStrand(), close(),
            // when close finishes, we know that oglcontext is unbinded and unregistered from process,
            // and device context is released
            asio::detached
        );
        
        // wait for render thread to finish
        _render_context->join();

        // destroy render thread context
        _render_context.reset();

        // destroy device context
        _device_context.reset();
    }

    asio::awaitable<void> OpenGLRenderer::close()
    {
        if(good() == false)co_return;

        co_await _render_context->ensureOnStrand();

        spdlog::debug(std::format("[opengl] [t:{}] close", std::this_thread::get_id()));

        // set render_context to closed such that we won't accept any new 
        // rendering tasks
        _render_context->signalClose();

        _vertex_buffers.clear();
        _shaders.clear();
        _shader_storage_buffers.clear();

        // unbind context before unregistering in winapi process
        wglMakeCurrent(0, 0);
        _window.releaseDeviceContext(*_device_context);

        // save handle value to be sent to winapi process for unregistration
        auto handle = *_oglctx_handle;

        // we can be certain that no other rendering task is currently running
        // since we are in the render thread strand, we can safely reset oglctx_handle
        _oglctx_handle.reset();

        co_await _window.getProcess().unregisterOGLContext(handle);
        
        co_return;
    }

    bool OpenGLRenderer::good() const
    {
        return  _oglctx_handle != nullptr && _render_context != nullptr && _device_context != nullptr &&
                _render_context != nullptr && _render_context->running();
    }

    asio::awaitable<void> OpenGLRenderer::enableVSync()
    {
        if(good() == false)co_return;

        co_await _render_context->ensureOnStrand();

        wglSwapIntervalEXT(1);
        co_return;
    }

    asio::awaitable<void> OpenGLRenderer::disableVSync()
    {
        if(good() == false)co_return;

        co_await _render_context->ensureOnStrand();

        wglSwapIntervalEXT(0);
        co_return;
    }

    asio::awaitable<std::optional<std::size_t>> OpenGLRenderer::constructVertexBuffer(std::string name,  const Mesh & mesh)
    {
        if(good() == false)co_return std::nullopt;

        co_await _render_context->ensureOnStrand();

        if(_vertex_buffer_names.contains(name))
        {
            spdlog::warn(std::format("[t:{}] Vertex buffer {} already exists", std::this_thread::get_id(), name));
            co_return std::nullopt;
        }

        VertexBuffer vb = VertexBuffer::construct<OpenGLVertexBuffer>(mesh.indices, mesh.vertices);

        std::size_t id = vb->ID();

        if(_vertex_buffers.contains(id))co_return std::nullopt;

        _vertex_buffers.try_emplace(id, std::move(vb));
        _vertex_buffer_names.try_emplace(std::move(name), id);

        co_return id;
    }

    asio::awaitable<bool> OpenGLRenderer::eraseVertexBuffer(std::size_t id)
    {
        if(good() == false)co_return false;

        co_await _render_context->ensureOnStrand();

        if(_vertex_buffers.contains(id) == false)
        {
            spdlog::warn(std::format("[t:{}] Vertex buffer {} does not exist", std::this_thread::get_id(), id));
            co_return false;
        }

        _vertex_buffers.erase(id);

        spdlog::info(std::format("[t:{}] Vertex buffer {} erased", std::this_thread::get_id(), id));

        co_return true;

    }

    std::optional<std::size_t> OpenGLRenderer::getVertexBuffer(std::string name) const
    { 
        if(good() == false)return std::nullopt;

        if(_vertex_buffer_names.contains(name) == false)
        {
            spdlog::warn(std::format("[t:{}] Vertex buffer {} does not exist", std::this_thread::get_id(), name));
            return std::nullopt;
        }
        auto id = _vertex_buffer_names.at(name);
        if(_vertex_buffers.contains(id) == false)
        {
            spdlog::warn(std::format("[t:{}] Vertex buffer {} does not exist", std::this_thread::get_id(), id));
            return std::nullopt;
        }
        return id;
    }

    asio::awaitable<std::optional<std::size_t>> OpenGLRenderer::constructShader(std::string name, std::vector<std::string> vertex_code)
    {
        if(good() == false)co_return std::nullopt;

        co_await _render_context->ensureOnStrand();

        if(_shader_names.contains(name))
        {
            spdlog::warn(std::format("[t:{}] Shader {} already exists", std::this_thread::get_id(), name));
            co_return std::nullopt;
        }

        Shader shader = Shader::construct<OpenGLShader>(std::move(vertex_code));

        std::size_t id = shader->ID();

        if(_shaders.contains(id))co_return std::nullopt;

        _shaders.try_emplace(id, std::move(shader));
        _shader_names.try_emplace(std::move(name), id);

        co_return id;
    }


    asio::awaitable<std::optional<std::size_t>> OpenGLRenderer::constructShader(std::string name, std::vector<std::string> vertex_code, std::vector<std::string> fragment_code)
    {
        if(good() == false)co_return std::nullopt;

        co_await _render_context->ensureOnStrand();

        if(_shader_names.contains(name))
        {
            spdlog::warn(std::format("[t:{}] Shader {} already exists", std::this_thread::get_id(), name));
            co_return std::nullopt;
        }

        Shader shader = Shader::construct<OpenGLShader>(std::move(vertex_code), std::move(fragment_code));

        std::size_t id = shader->ID();

        if(_shaders.contains(id))co_return std::nullopt;

        _shaders.try_emplace(id, std::move(shader));
        _shader_names.try_emplace(std::move(name), id);

        co_return id;
    }

    asio::awaitable<bool> OpenGLRenderer::eraseShader(std::size_t id)
    {
        if(good() == false)co_return false;

        co_await _render_context->ensureOnStrand();

        if(_shaders.contains(id) == false)
        {
            spdlog::warn(std::format("[t:{}] Shader {} does not exist", std::this_thread::get_id(), id));
            co_return false;
        }

        _shaders.erase(id);

        spdlog::info(std::format("[t:{}] Shader {} erased", std::this_thread::get_id(), id));

        co_return true;
    }

    std::optional<std::size_t> OpenGLRenderer::getShader(std::string name) const
    {
        if(good() == false)return std::nullopt;

        if(_shader_names.contains(name) == false)
        {
            spdlog::warn(std::format("[t:{}] Shader {} does not exist", std::this_thread::get_id(), name));
            return std::nullopt;
        }
        auto id = _shader_names.at(name);
        if(_shaders.contains(id) == false)
        {
            spdlog::warn(std::format("[t:{}] Shader {} does not exist", std::this_thread::get_id(), id));
            return std::nullopt;
        }
        return id;
    }

    asio::awaitable<std::optional<std::size_t>> OpenGLRenderer::constructShaderStorageBuffer(std::string name, const std::size_t size, const void * data)
    {
        if(good() == false)co_return std::nullopt;

        co_await _render_context->ensureOnStrand();

        if(_shader_storage_buffer_names.contains(name))
        {
            spdlog::warn(std::format("[t:{}] Shader storage buffer {} already exists", std::this_thread::get_id(), name));
            co_return std::nullopt;
        }

        ShaderStorageBuffer shader_storage_buffer = ShaderStorageBuffer::construct<OpenGLShaderStorageBuffer>(std::move(size), std::move(data));

        std::size_t id = shader_storage_buffer->ID();

        if(_shader_storage_buffers.contains(id))co_return std::nullopt;

        _shader_storage_buffers.try_emplace(id, std::move(shader_storage_buffer));
        _shader_storage_buffer_names.try_emplace(std::move(name), id);

        co_return id;
    }

    asio::awaitable<bool> OpenGLRenderer::eraseShaderStorageBuffer(std::size_t id)
    {
        if(good() == false)co_return false;

        co_await _render_context->ensureOnStrand();

        if(_shader_storage_buffers.contains(id) == false)
        {
            spdlog::warn(std::format("[t:{}] Shader storage buffer {} does not exist", std::this_thread::get_id(), id));
            co_return false;
        }

        _shader_storage_buffers.erase(id);

        spdlog::info(std::format("[t:{}] Shader storage buffer {} erased", std::this_thread::get_id(), id));

        co_return true;
    }

    std::optional<std::size_t> OpenGLRenderer::getShaderStorageBuffer(std::string name) const
    {
        if(good() == false)return std::nullopt;

        if(_shader_storage_buffer_names.contains(name) == false)
        {
            spdlog::warn(std::format("[t:{}] Shader storage buffer {} does not exist", std::this_thread::get_id(), name));
            return std::nullopt;
        }
        auto id = _shader_storage_buffer_names.at(name);
        if(_shader_storage_buffers.contains(id) == false)
        {
            spdlog::warn(std::format("[t:{}] Shader storage buffer {} does not exist", std::this_thread::get_id(), id));
            return std::nullopt;
        }
        return id;
    }
    
    asio::awaitable<bool> OpenGLRenderer::updateShaderStorageBuffer(std::size_t id, const std::size_t size, const void * data)
    {
        if(good() == false)co_return false;
        
        co_await _render_context->ensureOnStrand();
        
        if(_shader_storage_buffers.contains(id) == false)
        {
            spdlog::warn(std::format("[t:{}] Shader storage buffer {} does not exist", std::this_thread::get_id(), id));
            co_return false;
        }

        _shader_storage_buffers.at(id)->update(std::move(size), std::move(data));
        co_return true;
    }


    asio::awaitable<std::optional<std::size_t>> OpenGLRenderer::constructFrameBufferObject(std::string name)
    {
        if(good() == false)co_return std::nullopt;

        co_await _render_context->ensureOnStrand();

        if(_frame_buffer_object_names.contains(name))
        {
            spdlog::warn(std::format("[t:{}] Frame buffer object {} already exists", std::this_thread::get_id(), name));
            co_return std::nullopt;
        }

        FrameBufferObject frame_buffer_object = FrameBufferObject::construct<OpenGLFrameBufferObject>();

        std::size_t id = frame_buffer_object->ID();

        if(_frame_buffer_objects.contains(id))co_return std::nullopt;

        _frame_buffer_objects.try_emplace(id, std::move(frame_buffer_object));
        _frame_buffer_object_names.try_emplace(std::move(name), id);

        co_return id;
    }

    asio::awaitable<bool> OpenGLRenderer::eraseFrameBufferObject(std::size_t id)
    {
        if(good() == false)co_return false;

        co_await _render_context->ensureOnStrand();

        if(_frame_buffer_objects.contains(id) == false)
        {
            spdlog::warn(std::format("[t:{}] Frame buffer object {} does not exist", std::this_thread::get_id(), id));
            co_return false;
        }

        _frame_buffer_objects.erase(id);

        spdlog::info(std::format("[t:{}] Frame buffer object {} erased", std::this_thread::get_id(), id));

        co_return true;
    }

    std::optional<std::size_t> OpenGLRenderer::getFrameBufferObject(std::string name) const
    {
        if(good() == false)return std::nullopt;

        if(_frame_buffer_object_names.contains(name) == false)
        {
            spdlog::warn(std::format("[t:{}] Frame buffer object {} does not exist", std::this_thread::get_id(), name));
            return std::nullopt;
        }
        auto id = _frame_buffer_object_names.at(name);
        if(_frame_buffer_objects.contains(id) == false)
        {
            spdlog::warn(std::format("[t:{}] Frame buffer object {} does not exist", std::this_thread::get_id(), id));
            return std::nullopt;
        }
        return id;
    }


    void OpenGLRenderer::assignShaderInputs(const std::size_t & shader_ID, const ShaderInputs & shader_inputs)
    {
        for(const auto & [name, val] : shader_inputs.in_bool)
        {
            _shaders.at(shader_ID)->setUniform(name, val);
        }

        for(const auto & [name, val] : shader_inputs.in_int)
        {
            _shaders.at(shader_ID)->setUniform(name, val);
        }
        for(const auto & [name, val] : shader_inputs.in_float)
        {
            _shaders.at(shader_ID)->setUniform(name, val);
        }

        for(const auto & [name, val] : shader_inputs.in_vec2)
        {
            _shaders.at(shader_ID)->setUniform(name, val);
        }
        for(const auto & [name, val] : shader_inputs.in_vec3)
        {
            _shaders.at(shader_ID)->setUniform(name, val);
        }
        for(const auto & [name, val] : shader_inputs.in_vec4)
        {
            _shaders.at(shader_ID)->setUniform(name, val);
        }

        for(const auto & [name, val] : shader_inputs.in_mat2)
        {
            _shaders.at(shader_ID)->setUniform(name, val);
        }
        for(const auto & [name, val] : shader_inputs.in_mat3)
        {
            _shaders.at(shader_ID)->setUniform(name, val);
        }
        for(const auto & [name, val] : shader_inputs.in_mat4)
        {
            _shaders.at(shader_ID)->setUniform(name, val);
        }
    }

    asio::awaitable<void> OpenGLRenderer::clearScreen(glm::vec4 color)
    {
        if(good() == false)co_return;

        co_await _render_context->ensureOnStrand();

        glClearColor(color.r, color.g, color.b, color.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        co_return;
    }

    asio::awaitable<void> OpenGLRenderer::render(
            std::size_t vertex_buffer_ID,
            std::size_t shader_ID,
            ShaderInputs shader_inputs,
            std::optional<std::size_t> shader_storage_buffer_ID,
            RenderMode mode)
    {
        if(good() == false)co_return;

        co_await _render_context->ensureOnStrand();

        auto shader_it = _shaders.find(shader_ID);
        if(shader_it == _shaders.end()){
            spdlog::warn("Rendering: Shader not found");
            co_return;
        }

        auto vertex_buffer_it = _vertex_buffers.find(vertex_buffer_ID);
        if(vertex_buffer_it == _vertex_buffers.end()){
            spdlog::warn("Rendering: Vertex buffer not found");
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

        if(shader_storage_buffer_ID)
        {
            auto shader_storage_buffer_it = _shader_storage_buffers.find(*shader_storage_buffer_ID);
            if(shader_storage_buffer_it == _shader_storage_buffers.end()){
                spdlog::warn("Rendering: Shader storage buffer not found");
                co_return;
            }
            if(shader_storage_buffer_it->second->enable() == false)
            {
                spdlog::error("Cannot enable shader storage buffer");
                co_return;
            }
        }

        if(mode == RenderMode::Wireframe)glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glDrawElements(GL_TRIANGLES, (GLsizei)vertex_buffer_it->second->numberOfElements(), GL_UNSIGNED_INT, nullptr);
        
        if(mode == RenderMode::Wireframe)glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // restore default

        co_return;
    }

    asio::awaitable<void> OpenGLRenderer::present()
    {
        if(good() == false)co_return;

        co_await _render_context->ensureOnStrand();

        glFlush();
        SwapBuffers(*_device_context);

        co_return;
    }

    asio::awaitable<void> OpenGLRenderer::updateViewport(Resolution resolution)
    {
        if(good() == false)co_return;

        co_await _render_context->ensureOnStrand();
        
        _viewport_resolution = resolution;

        // if the OpenGL context depends on a HDC that may change
        // (e.g. due to window resize, DPI change, or re-creation), 
        // then you must re-acquire the HDC (_device_context) from the HWND before calling glViewport()
        // or any other GL function that implicitly touches the current drawable surface
        
        // release old context
        wglMakeCurrent(0, 0);
        _window.releaseDeviceContext(*_device_context);

        // acquire new context
        *_device_context = _window.acquireDeviceContext();
                
        // bind context
        if(wglMakeCurrent(*_device_context, *_oglctx_handle) == FALSE)
        {
            spdlog::error(std::format("[t:{}] Cannot activate OpenGL context", std::this_thread::get_id()));
            co_return;
        }

        glViewport(0, 0, (GLsizei)_viewport_resolution.getWidth(), (GLsizei)_viewport_resolution.getHeight());

        co_return;
    }

    Resolution OpenGLRenderer::getViewport() const
    {
        return _viewport_resolution;
    }
}