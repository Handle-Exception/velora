#pragma once

#include "type.hpp"

#include "resolution.hpp"

#include "render_mode.hpp"
#include "vertex.hpp"
#include "vertex_buffer.hpp"
#include "shader_storage_buffer.hpp"
#include "frame_buffer_object.hpp"
#include "texture.hpp"

#include "shader.hpp"

#include "fps_counter.hpp"

namespace velora
{
    class IRenderer : public type::Interface
    {
    public:
        virtual ~IRenderer() = default;

        /**
         * @brief Check if renderer is properly initialized
         * 
         * @return `true` if renderer is properly initialized and can render, `false` if renderer is closed or not initialized
         */
        virtual bool good() const = 0;
        
        /**
         * @brief Close the renderer
         * notifies process
         * 
         * @return asio::awaitable<void> 
         */
        virtual asio::awaitable<void> close() = 0;

        /**
         * @brief Wait for the renderer to finish rendering and close
         * 
         */
        virtual void join() = 0;

        /**
         * @brief Clear the screen with a color
         * 
         * @param color Color to clear the screen with
         * @return asio::awaitable<void> 
         */
        virtual asio::awaitable<void> clearScreen(glm::vec4 color, std::optional<std::size_t> fbo = std::nullopt) = 0;
        
        /**
         * @brief Render a vertex buffer with a shader
         * 
         * @param vertex_buffer_ID ID of the vertex buffer to render
         * @param shader_ID ID of the shader to use
         * @param shader_inputs Inputs for the shader
         * @return asio::awaitable<void> 
         */
        virtual asio::awaitable<void> render(
                std::size_t vertex_buffer_ID,
                std::size_t shader_ID,
                ShaderInputs shader_inputs = ShaderInputs{},
                RenderMode mode = RenderMode::Solid,
                std::optional<std::size_t> fbo = std::nullopt,
                std::optional<PolygonOffset> offset = std::nullopt) = 0;
        
        /**
         * @brief Present the rendered frame
         * 
         * @return asio::awaitable<void> 
         */
        virtual asio::awaitable<void> present() = 0;

        /**
         * @brief Update the viewport
         * 
         * @param resolution Resolution of the viewport
         * @return asio::awaitable<void> 
         */
        virtual asio::awaitable<void> updateViewport(Resolution resolution) = 0;

        /**
         * @brief Get the viewport resolution
         * 
         * @return Resolution Resolution of the viewport
         */
        virtual Resolution getViewport() const = 0;
        
        virtual asio::awaitable<void> enableVSync() = 0;
        virtual asio::awaitable<void> disableVSync() = 0;

        virtual asio::awaitable<std::optional<std::size_t>> constructVertexBuffer(std::string name, const Mesh & mesh) = 0;
        virtual asio::awaitable<bool> eraseVertexBuffer(std::size_t id) = 0;
        virtual std::optional<std::size_t> getVertexBuffer(std::string name) const = 0;

        virtual asio::awaitable<std::optional<std::size_t>> constructShader(std::string name, std::vector<std::string> vertex_code) = 0;
        virtual asio::awaitable<std::optional<std::size_t>> constructShader(std::string name, std::vector<std::string> vertex_code, std::vector<std::string> fragment_code) = 0;
        virtual asio::awaitable<bool> eraseShader(std::size_t id) = 0;
        virtual std::optional<std::size_t> getShader(std::string name) const = 0;
        
        virtual asio::awaitable<std::optional<std::size_t>> constructShaderStorageBuffer(std::string name, unsigned int binding_point, const std::size_t size, const void * data) = 0;
        virtual asio::awaitable<bool> updateShaderStorageBuffer(std::size_t id, const std::size_t size, const void * data) = 0;
        virtual asio::awaitable<bool> eraseShaderStorageBuffer(std::size_t id) = 0;
        virtual std::optional<std::size_t> getShaderStorageBuffer(std::string name) const = 0;

        virtual asio::awaitable<std::optional<std::size_t>> constructFrameBufferObject(std::string name, Resolution resolution, std::initializer_list<FBOAttachment> attachments) = 0;
        virtual asio::awaitable<bool> eraseFrameBufferObject(std::size_t id) = 0;
        virtual std::optional<std::size_t> getFrameBufferObject(std::string name) const = 0;
        virtual std::vector<std::size_t> getFrameBufferObjectTextures(std::size_t id) const = 0;
    };

    template<class RendererImplType>
    class RendererDispatcher final : public type::Dispatcher<IRenderer, RendererImplType>
    {
        public:
            using dispatch = type::Dispatcher<IRenderer, RendererImplType>;

            inline RendererDispatcher(RendererImplType && obj) : dispatch(std::move(obj)){}
            template<class... Args>                                                                             
            inline RendererDispatcher(Args && ... args) : dispatch(std::forward<Args>(args)...){} 
            inline ~RendererDispatcher() = default;

            constexpr inline bool good() const override { return dispatch::getImpl().good();}
            inline asio::awaitable<void> close() override { co_return co_await dispatch::getImpl().close();}
            constexpr inline void join() override { return dispatch::getImpl().join();}

            inline asio::awaitable<void> clearScreen(glm::vec4 color, std::optional<std::size_t> fbo) override { 
                co_return co_await dispatch::getImpl().clearScreen(std::move(color), std::move(fbo));
            }

            inline asio::awaitable<void> render(std::size_t vertex_buffer_ID, std::size_t shader_ID, 
                ShaderInputs shader_inputs, RenderMode mode,
                std::optional<std::size_t> fbo,
                std::optional<PolygonOffset> offset) override { 
                co_return co_await dispatch::getImpl().render(std::move(vertex_buffer_ID), std::move(shader_ID),
                    std::move(shader_inputs), mode, 
                    std::move(fbo),
                    std::move(offset));
            }

            inline asio::awaitable<void> present() override { 
                co_return co_await dispatch::getImpl().present();
            }

            inline asio::awaitable<void> updateViewport(Resolution resolution) override { 
                co_return co_await dispatch::getImpl().updateViewport(std::move(resolution));
            }

            constexpr inline Resolution getViewport() const override { 
                return dispatch::getImpl().getViewport();
            }

            inline asio::awaitable<void> enableVSync() override { 
                co_return co_await dispatch::getImpl().enableVSync();
            }

            inline asio::awaitable<void> disableVSync() override { 
                co_return co_await dispatch::getImpl().disableVSync();
            }

            inline asio::awaitable<std::optional<std::size_t>> constructVertexBuffer(std::string name, const Mesh & mesh) override{ 
                co_return co_await dispatch::getImpl().constructVertexBuffer(std::move(name), mesh);
            }
        
            inline asio::awaitable<bool> eraseVertexBuffer(std::size_t id) override {
                co_return co_await dispatch::getImpl().eraseVertexBuffer(std::move(id));
            }

            constexpr inline std::optional<std::size_t> getVertexBuffer(std::string name) const override{
                return dispatch::getImpl().getVertexBuffer(std::move(name));
            }

            inline asio::awaitable<std::optional<std::size_t>> constructShader(std::string name, std::vector<std::string> vertex_code) override { 
                co_return co_await dispatch::getImpl().constructShader(std::move(name), std::move(vertex_code));
            }

            inline asio::awaitable<std::optional<std::size_t>> constructShader(std::string name, std::vector<std::string> vertex_code, std::vector<std::string> fragment_code) override { 
                co_return co_await dispatch::getImpl().constructShader(std::move(name), std::move(vertex_code), std::move(fragment_code));
            }

            inline asio::awaitable<bool> eraseShader(std::size_t id) override {
                co_return co_await dispatch::getImpl().eraseShader(std::move(id));
            }
            
            constexpr inline std::optional<std::size_t> getShader(std::string name) const override{
                return  dispatch::getImpl().getShader(std::move(name));
            }

            inline asio::awaitable<std::optional<std::size_t>> constructShaderStorageBuffer(std::string name, unsigned int binding_point, const std::size_t size, const void * data) override{ 
                co_return co_await dispatch::getImpl().constructShaderStorageBuffer(std::move(name), binding_point, std::move(size), std::move(data));
            }

            inline asio::awaitable<bool> updateShaderStorageBuffer(std::size_t id, const std::size_t size, const void * data) override {
                co_return co_await dispatch::getImpl().updateShaderStorageBuffer(std::move(id), std::move(size), std::move(data));
            }

            inline asio::awaitable<bool> eraseShaderStorageBuffer(std::size_t id) override {
                co_return co_await dispatch::getImpl().eraseShaderStorageBuffer(std::move(id));
            }

            constexpr inline std::optional<std::size_t> getShaderStorageBuffer(std::string name) const override{
                return dispatch::getImpl().getShaderStorageBuffer(std::move(name));
            }

            inline asio::awaitable<std::optional<std::size_t>> constructFrameBufferObject(std::string name, Resolution resolution, std::initializer_list<FBOAttachment> attachments) override{ 
                co_return co_await dispatch::getImpl().constructFrameBufferObject(std::move(name), std::move(resolution), std::move(attachments));
            }

            inline asio::awaitable<bool> eraseFrameBufferObject(std::size_t id) override {
                co_return co_await dispatch::getImpl().eraseFrameBufferObject(std::move(id));
            }

            inline std::optional<std::size_t> getFrameBufferObject(std::string name) const override{
                return dispatch::getImpl().getFrameBufferObject(std::move(name));
            }

            inline std::vector<std::size_t> getFrameBufferObjectTextures(std::size_t id) const override {
                return dispatch::getImpl().getFrameBufferObjectTextures(std::move(id));
            }
    };

    template<class RendererImplType>
    RendererDispatcher(RendererImplType && ) -> RendererDispatcher<RendererImplType>;

    class Renderer : public type::Implementation<IRenderer, RendererDispatcher> 
    {                                                                   
        public:
            /* move ctor */
            Renderer(Renderer && other) = default;
            Renderer & operator=(Renderer && other) = default;

            /* dtor */
            virtual ~Renderer() = default;
        
            /* ctor */
            template<class RendererImplType>
            Renderer(RendererImplType && impl)
            : Implementation(std::move(impl))
            {}                                                          
    };
}