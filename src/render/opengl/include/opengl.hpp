#pragma once

#include "native.hpp"
#include "window.hpp"
#include "process.hpp"

#include "vertex.hpp"
#include "vertex_buffer.hpp"
#include "shader.hpp"

#include <GL/glew.h>
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

            static asio::awaitable<OpenGLRenderer> asyncConstructor(asio::io_context & io_context, IWindow & window, int major_version, int minor_version);

            bool good() const;

            asio::awaitable<void> close();

            void render(IVertexBuffer &, IShader &, const glm::mat4 &)
            {

            }

            asio::awaitable<std::optional<std::size_t>> constructVertexBuffer(std::vector<unsigned int> indices, std::vector<Vertex> vertices);
            
            asio::awaitable<bool> eraseVertexBuffer(std::size_t id);
        protected:
            OpenGLRenderer(asio::io_context & io_context, IWindow & window, native::opengl_context_handle oglctx_handle);


        private:
            IWindow & _window;
            asio::io_context & _io_context;
            asio::strand<asio::io_context::executor_type> _strand;
            native::opengl_context_handle _oglctx_handle;

            absl::flat_hash_map<std::size_t, VertexBuffer> _vertex_buffers;
            //absl::flat_hash_set<Shader> _shaders;
    };
}