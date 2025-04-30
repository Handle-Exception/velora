#pragma once

#include "type.hpp"

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
         * @brief Destroy renderer
         * called when process is already notified
         * 
         * @return asio::awaitable<void> 
         */
        virtual asio::awaitable<void> destroy() = 0;
        
        /**
         * @brief Close the renderer
         * notifies process
         * 
         * @return asio::awaitable<void> 
         */
        virtual asio::awaitable<void> close() = 0;

        virtual void render(IVertexBuffer &, IShader &, const glm::mat4 &) = 0;
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
            inline asio::awaitable<void> destroy() override { co_return co_await dispatch::getImpl().destroy();}

            inline void render(IVertexBuffer & vb, IShader & s, const glm::mat4 & m) { 
                return dispatch::getImpl().render(vb, s, std::move(m));};

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