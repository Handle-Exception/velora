#pragma once

#include "type.hpp"

#include "resolution.hpp"

#include "vertex.hpp"
#include "vertex_buffer.hpp"

#include "shader.hpp"

namespace velora
{
    class IRenderer : public type::Interface
    {
    public:
        virtual ~IRenderer() = default;

        /**
         * @brief Check if renderer is properly initialized
         * 
         * @return `true` if renderer is properly initialized
         */
        virtual bool good() const = 0;
        
        /**
         * @brief Close the renderer
         * notifies process
         * 
         * @return asio::awaitable<void> 
         */
        virtual asio::awaitable<void> close() = 0;

        virtual asio::awaitable<void> clearScreen(glm::vec4 color) = 0;
        virtual asio::awaitable<void> render(std::size_t vertex_buffer_ID, std::size_t shader_ID, ShaderInputs shader_inputs = ShaderInputs{}) = 0;
        virtual asio::awaitable<void> present() = 0;
        virtual asio::awaitable<void> updateViewport(Resolution resolution) = 0;

        virtual asio::awaitable<std::optional<std::size_t>> constructVertexBuffer(std::string name, std::vector<unsigned int> indices, std::vector<Vertex> vertices) = 0;
        
        virtual asio::awaitable<bool> eraseVertexBuffer(std::size_t id) = 0;

        virtual asio::awaitable<std::optional<std::size_t>> getVertexBuffer(std::string name) = 0;

        virtual asio::awaitable<std::optional<std::size_t>> constructShader(std::string name, std::vector<std::string> vertex_code) = 0;
        virtual asio::awaitable<std::optional<std::size_t>> constructShader(std::string name, std::vector<std::string> vertex_code, std::vector<std::string> fragment_code) = 0;

        virtual asio::awaitable<bool> eraseShader(std::size_t id) = 0;
        
        virtual asio::awaitable<std::optional<std::size_t>> getShader(std::string name) = 0;

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

            inline asio::awaitable<void> clearScreen(glm::vec4 color) override { 
                co_return co_await dispatch::getImpl().clearScreen(std::move(color));
            };

            inline asio::awaitable<void> render(std::size_t vertex_buffer_ID, std::size_t shader_ID, ShaderInputs shader_inputs) override { 
                co_return co_await dispatch::getImpl().render(std::move(vertex_buffer_ID), std::move(shader_ID), std::move(shader_inputs));
            };

            inline asio::awaitable<void> present() override { 
                co_return co_await dispatch::getImpl().present();
            };

            inline asio::awaitable<void> updateViewport(Resolution resolution) override { 
                co_return co_await dispatch::getImpl().updateViewport(std::move(resolution));
            };

            inline asio::awaitable<std::optional<std::size_t>> constructVertexBuffer(std::string name, std::vector<unsigned int> indices, std::vector<Vertex> vertices) override{ 
                co_return co_await dispatch::getImpl().constructVertexBuffer(std::move(name), std::move(indices), std::move(vertices));
            };
        
            inline asio::awaitable<bool> eraseVertexBuffer(std::size_t id) override {
                co_return co_await dispatch::getImpl().eraseVertexBuffer(std::move(id));
            }

            inline asio::awaitable<std::optional<std::size_t>> getVertexBuffer(std::string name) override{
                co_return co_await dispatch::getImpl().getVertexBuffer(std::move(name));
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
            
            inline asio::awaitable<std::optional<std::size_t>> getShader(std::string name) override{
                co_return co_await dispatch::getImpl().getShader(std::move(name));
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