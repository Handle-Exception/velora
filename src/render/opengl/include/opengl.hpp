#pragma once

#include <thread>

#include "native.hpp"
#include "window.hpp"
#include "process.hpp"

#include "vertex.hpp"
#include "vertex_buffer.hpp"
#include "shader.hpp"

#include <GL/glew.h>
#ifdef WIN32
    #include <GL/wglew.h>
#endif

#include <spdlog/spdlog.h>
#include <asio.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <absl/container/flat_hash_map.h>

#include "opengl_debug.hpp"
#include "opengl_vertex_buffer.hpp"
#include "opengl_shader.hpp"

namespace velora::opengl
{
    class OpenGLRenderer
    {
        public:
            ~OpenGLRenderer();
            OpenGLRenderer(OpenGLRenderer && other);

            static asio::awaitable<OpenGLRenderer> asyncConstructor(IWindow & window, int major_version, int minor_version);

            bool good() const;

            asio::awaitable<void> close();

            asio::awaitable<void> clearScreen(glm::vec4 color);
            asio::awaitable<void> render(std::size_t vertex_buffer_ID, std::size_t shader_ID, ShaderInputs shader_inputs);
            asio::awaitable<void> present();
            asio::awaitable<void> updateViewport(Resolution resolution);
            Resolution getViewport() const;
            asio::awaitable<void> enableVSync();
            asio::awaitable<void> disableVSync();

            asio::awaitable<std::optional<std::size_t>> constructVertexBuffer(std::string name, const Mesh & mesh);
            
            asio::awaitable<bool> eraseVertexBuffer(std::size_t id);
            std::optional<std::size_t> getVertexBuffer(std::string name) const;

            asio::awaitable<std::optional<std::size_t>> constructShader(std::string name, std::vector<std::string> vertex_code);
            asio::awaitable<std::optional<std::size_t>> constructShader(std::string name, std::vector<std::string> vertex_code, std::vector<std::string> fragment_code);

            asio::awaitable<bool> eraseShader(std::size_t id);
            std::optional<std::size_t> getShader(std::string name) const;


            void join();

        protected:
            OpenGLRenderer(IWindow & window, native::opengl_context_handle oglctx_handle);

            void assignShaderInputs(const std::size_t & shader_ID, const ShaderInputs & shader_inputs);

        private:
            IWindow & _window;

            std::unique_ptr<asio::io_context> _render_context; // dedicated single thread
            asio::executor_work_guard<asio::io_context::executor_type> _render_context_work_guard;
            asio::strand<asio::io_context::executor_type> _render_context_strand;

            std::unique_ptr<native::opengl_context_handle> _oglctx_handle;
            std::unique_ptr<native::device_context> _device_context;

            Resolution _viewport_resolution;

            absl::flat_hash_map<std::size_t, VertexBuffer> _vertex_buffers;
            absl::flat_hash_map<std::size_t, Shader> _shaders;

            absl::flat_hash_map<std::string, std::size_t> _vertex_buffer_names;
            absl::flat_hash_map<std::string, std::size_t> _shader_names;

            // Render thread initialized at the end of the constructor
            std::thread _render_thread;
    };
}