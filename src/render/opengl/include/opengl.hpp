#pragma once

#include <thread>

#include "native.hpp"
#include <asio.hpp>

#include <GL/glew.h>
#ifdef WIN32
    #include <GL/wglew.h>
#endif

#include <spdlog/spdlog.h>

#include <absl/container/flat_hash_map.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "window.hpp"
#include "process.hpp"

#include "render_mode.hpp"
#include "vertex.hpp"
#include "vertex_buffer.hpp"
#include "shader.hpp"
#include "shader_storage_buffer.hpp"
#include "frame_buffer_object.hpp"
#include "texture.hpp"

#include "opengl_debug.hpp"
#include "opengl_vertex_buffer.hpp"
#include "opengl_shader.hpp"
#include "opengl_shader_storage_buffer.hpp"
#include "opengl_frame_buffer_object.hpp"
#include "opengl_texture.hpp"

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
            asio::awaitable<void> render(std::size_t vertex_buffer_ID,
                std::size_t shader_ID,
                ShaderInputs shader_inputs,
                RenderMode mode,
                std::optional<std::size_t> frame_buffer_object_ID);
                
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

            asio::awaitable<std::optional<std::size_t>> constructShaderStorageBuffer(std::string name, const std::size_t size, const void * data);
            asio::awaitable<bool> eraseShaderStorageBuffer(std::size_t id);
            std::optional<std::size_t> getShaderStorageBuffer(std::string name) const;
            asio::awaitable<bool> updateShaderStorageBuffer(std::size_t id, const std::size_t size, const void * data);
            
            asio::awaitable<std::optional<std::size_t>> constructFrameBufferObject(std::string name, Resolution resolution, std::initializer_list<FBOAttachment> attachments);
            asio::awaitable<bool> eraseFrameBufferObject(std::size_t id);
            std::optional<std::size_t> getFrameBufferObject(std::string name) const;

            asio::awaitable<std::optional<std::size_t>> constructTexture(std::string name, Resolution resolution);
            asio::awaitable<bool> eraseTexture(std::size_t id);
            std::optional<std::size_t> getTexture(std::string name) const;

            void join();

        protected:
            OpenGLRenderer(IWindow & window, native::opengl_context_handle oglctx_handle);

            template<class ImplType, class T, class ... Args>
            asio::awaitable<std::optional<std::size_t>> constructInternalObject(
                absl::flat_hash_map<std::size_t, T> & object_map,
                absl::flat_hash_map<std::string, std::size_t> & name_map,
                std::string name,  Args && ... args)
            {
                static_assert(requires { T::template construct<ImplType>(std::forward<Args>(args)...); },
                    "T must have static template method construct<ImplType>(Args&&...)");

                if(good() == false)co_return std::nullopt;

                co_await _render_context->ensureOnStrand();

                if(name_map.contains(name))
                {
                    spdlog::warn(std::format("[t:{}] renderer object {} already exists", std::this_thread::get_id(), name));
                    co_return std::nullopt;
                }

                T obj = T::template construct<ImplType>(std::forward<Args>(args)...);

                const std::size_t id = obj->ID();

                if(object_map.contains(id))co_return std::nullopt;

                object_map.try_emplace(id, std::move(obj));
                name_map.try_emplace(std::move(name), id);

                co_return id;
            }

            template<class T>
            std::optional<std::size_t> getInternalObjectID(const absl::flat_hash_map<std::size_t, T> & object_map,
                const absl::flat_hash_map<std::string, std::size_t> & name_map,
                std::string name) const
            {
                if(good() == false)return std::nullopt;

                if(name_map.contains(name) == false)
                {
                    spdlog::warn(std::format("[t:{}] renderer object {} does not exist", std::this_thread::get_id(), name));
                    return std::nullopt;
                }
                
                const auto id = name_map.at(name);
                if(object_map.contains(id) == false)
                {
                    spdlog::warn(std::format("[t:{}] renderer object {} does not exist", std::this_thread::get_id(), id));
                    return std::nullopt;
                }
                return id;
            }

            template<class T>
            asio::awaitable<bool> eraseInternalObject(absl::flat_hash_map<std::size_t, T> & object_map, std::size_t id) const
            {
                if(good() == false)co_return false;

                co_await _render_context->ensureOnStrand();

                if(object_map.contains(id) == false)
                {
                    spdlog::warn(std::format("[t:{}] renderer object {} does not exist", std::this_thread::get_id(), id));
                    co_return false;
                }

                object_map.erase(id);

                spdlog::info(std::format("[t:{}] renderer object {} erased", std::this_thread::get_id(), id));

                co_return true;
            }


            void assignShaderInputs(const std::size_t & shader_ID, const ShaderInputs & shader_inputs);

        private:
            struct RenderThreadContext
            {
                RenderThreadContext(
                        IWindow & window,
                        native::device_context * device,
                        native::opengl_context_handle * oglctx);

                ~RenderThreadContext();
                
                void join();

                const asio::strand<asio::io_context::executor_type> & getStrand() const;

                asio::awaitable<void> ensureOnStrand();
                
                void signalClose();

                bool running() const;

                private:
                    void workerThread();


                    asio::io_context _io_context;
                    asio::executor_work_guard<asio::io_context::executor_type> _work_guard;
                    asio::strand<asio::io_context::executor_type> _strand;

                    IWindow & _window;
                    native::device_context * _device_context;
                    native::opengl_context_handle * _oglctx_handle;

                    std::thread _worker_thread;
            };

            IWindow & _window;

            std::unique_ptr<native::opengl_context_handle> _oglctx_handle;
            std::unique_ptr<native::device_context> _device_context;

            Resolution _viewport_resolution;

            absl::flat_hash_map<std::size_t, VertexBuffer> _vertex_buffers;
            absl::flat_hash_map<std::size_t, Shader> _shaders;
            absl::flat_hash_map<std::size_t, ShaderStorageBuffer> _shader_storage_buffers;
            absl::flat_hash_map<std::size_t, FrameBufferObject> _frame_buffer_objects;
            absl::flat_hash_map<std::size_t, Texture> _textures;

            absl::flat_hash_map<std::string, std::size_t> _vertex_buffer_names;
            absl::flat_hash_map<std::string, std::size_t> _shader_names;
            absl::flat_hash_map<std::string, std::size_t> _shader_storage_buffer_names;
            absl::flat_hash_map<std::string, std::size_t> _frame_buffer_object_names;
            absl::flat_hash_map<std::string, std::size_t> _textures_names;

            // Render thread initialized at the end of the constructor
            std::unique_ptr<RenderThreadContext> _render_context; // dedicated single thread
    };
}