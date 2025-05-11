
#pragma once

#include "type.hpp"
#include <string>
#include <utility>

namespace velora
{
    class IRenderBuffer : public type::Interface
    {
        public:
        //
        virtual ~IRenderBuffer() = default;

        virtual std::size_t ID() const = 0;

        virtual bool good() const = 0;
        //
        virtual bool enable() const = 0;
        //
        virtual void disable() const = 0;
    };

    template<class RenderBufferImplType>
    class RenderBufferDispatcher final : public type::Dispatcher<IRenderBuffer, RenderBufferImplType>
    {
        public:
            using dispatch = type::Dispatcher<IRenderBuffer, RenderBufferImplType>;

            inline RenderBufferDispatcher(RenderBufferImplType && obj) : dispatch(std::move(obj)){}
            template<class... Args>                                                                             
            inline RenderBufferDispatcher(Args && ... args) : dispatch(std::forward<Args>(args)...){} 
            inline ~RenderBufferDispatcher() = default;

            constexpr inline std::size_t ID() const override { return dispatch::getImpl().ID();}
            constexpr inline bool good() const override { return dispatch::getImpl().good();}
            constexpr inline bool enable() const override { return dispatch::getImpl().enable();}
            constexpr inline void disable() const override { return dispatch::getImpl().disable();}
    };

    template<class RenderBufferImplType>
    RenderBufferDispatcher(RenderBufferImplType && ) -> RenderBufferDispatcher<RenderBufferImplType>;

    class RenderBuffer : public type::Implementation<IRenderBuffer, RenderBufferDispatcher> 
    {                                                                   
        public:
            /* move ctor */
            RenderBuffer(RenderBuffer && other) = default;
            RenderBuffer & operator=(RenderBuffer && other) = default;

            /* dtor */
            virtual ~RenderBuffer() = default;
        
            /* ctor */
            template<class RenderBufferImplType>
            RenderBuffer(RenderBufferImplType && impl)
            : Implementation(std::move(impl))
            {}                                                          
    };

    template <typename H>
    constexpr inline H AbslHashValue(H h, const RenderBuffer & vb) {
        return H::combine(std::move(h), vb->ID());
    }

    constexpr inline bool operator==(const RenderBuffer & lhs, const RenderBuffer & rhs){
        return lhs->ID() == rhs->ID();
    }
}