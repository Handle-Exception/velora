
#pragma once

#include "type.hpp"
#include <string>
#include <utility>

namespace velora
{
    class IVertexBuffer : public type::Interface
    {
        public:
        //
        virtual ~IVertexBuffer() = default;

        virtual std::size_t ID() const = 0;

        virtual bool good() const = 0;
        //
        virtual std::size_t numberOfElements() const = 0;
        //
        virtual bool enable() const = 0;
        //
        virtual void disable() const = 0;
    };

    template<class VertexBufferImplType>
    class VertexBufferDispatcher final : public type::Dispatcher<IVertexBuffer, VertexBufferImplType>
    {
        public:
            using dispatch = type::Dispatcher<IVertexBuffer, VertexBufferImplType>;

            inline VertexBufferDispatcher(VertexBufferImplType && obj) : dispatch(std::move(obj)){}
            template<class... Args>                                                                             
            inline VertexBufferDispatcher(Args && ... args) : dispatch(std::forward<Args>(args)...){} 
            inline ~VertexBufferDispatcher() = default;

            constexpr inline std::size_t ID() const override { return dispatch::getImpl().ID();}
            constexpr inline bool good() const override { return dispatch::getImpl().good();}
            constexpr inline std::size_t numberOfElements() const override { return dispatch::getImpl().numberOfElements();}
            constexpr inline bool enable() const override { return dispatch::getImpl().enable();}
            constexpr inline void disable() const override { return dispatch::getImpl().disable();}
    };

    template<class VertexBufferImplType>
    VertexBufferDispatcher(VertexBufferImplType && ) -> VertexBufferDispatcher<VertexBufferImplType>;

    class VertexBuffer : public type::Implementation<IVertexBuffer, VertexBufferDispatcher> 
    {                                                                   
        public:
            /* move ctor */
            VertexBuffer(VertexBuffer && other) = default;
            VertexBuffer & operator=(VertexBuffer && other) = default;

            /* dtor */
            virtual ~VertexBuffer() = default;
        
            /* ctor */
            template<class VertexBufferImplType>
            VertexBuffer(VertexBufferImplType && impl)
            : Implementation(std::move(impl))
            {}                                                          
    };

    template <typename H>
    constexpr inline H AbslHashValue(H h, const VertexBuffer & vb) {
        return H::combine(std::move(h), vb->ID());
    }

    constexpr inline bool operator==(const VertexBuffer & lhs, const VertexBuffer & rhs){
        return lhs->ID() == rhs->ID();
    }
}