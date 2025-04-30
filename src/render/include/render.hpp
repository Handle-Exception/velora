#pragma once

#include "type.hpp"

#include "opengl.hpp"

namespace velora
{
    class IRenderer : public type::Interface
    {
    public:
        // dtor
        virtual ~IRenderer() = default;

        //
        virtual bool good() const = 0;

        //
        virtual void close() = 0;
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
            constexpr inline void close() override { return dispatch::getImpl().close();}
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