
#pragma once

#include "type.hpp"
#include <string>
#include <utility>

namespace velora
{
    class IShaderStorageBuffer : public type::Interface
    {
        public:
        //
        virtual ~IShaderStorageBuffer() = default;

        virtual std::size_t ID() const = 0;

        virtual bool good() const = 0;
        //
        virtual bool enable() const = 0;
        //
        virtual void disable() const = 0;
        //
        virtual void update(std::size_t size, const void * data) = 0;
    };

    template<class ShaderStorageBufferImplType>
    class ShaderStorageBufferDispatcher final : public type::Dispatcher<IShaderStorageBuffer, ShaderStorageBufferImplType>
    {
        public:
            using dispatch = type::Dispatcher<IShaderStorageBuffer, ShaderStorageBufferImplType>;

            inline ShaderStorageBufferDispatcher(ShaderStorageBufferImplType && obj) : dispatch(std::move(obj)){}
            template<class... Args>                                                                             
            inline ShaderStorageBufferDispatcher(Args && ... args) : dispatch(std::forward<Args>(args)...){} 
            inline ~ShaderStorageBufferDispatcher() = default;

            constexpr inline std::size_t ID() const override { return dispatch::getImpl().ID();}
            constexpr inline bool good() const override { return dispatch::getImpl().good();}
            constexpr inline bool enable() const override { return dispatch::getImpl().enable();}
            constexpr inline void disable() const override { return dispatch::getImpl().disable();}
            constexpr inline void update(std::size_t size, const void * data) override { return dispatch::getImpl().update(std::move(size), std::move(data));}
    };

    template<class ShaderStorageBufferImplType>
    ShaderStorageBufferDispatcher(ShaderStorageBufferImplType && ) -> ShaderStorageBufferDispatcher<ShaderStorageBufferImplType>;

    class ShaderStorageBuffer : public type::Implementation<IShaderStorageBuffer, ShaderStorageBufferDispatcher> 
    {                                                                   
        public:
            /* move ctor */
            ShaderStorageBuffer(ShaderStorageBuffer && other) = default;
            ShaderStorageBuffer & operator=(ShaderStorageBuffer && other) = default;

            /* dtor */
            virtual ~ShaderStorageBuffer() = default;
        
            /* ctor */
            template<class ShaderStorageBufferImplType>
            ShaderStorageBuffer(ShaderStorageBufferImplType && impl)
            : Implementation(std::move(impl))
            {}                                                          
    };

    template <typename H>
    constexpr inline H AbslHashValue(H h, const ShaderStorageBuffer & vb) {
        return H::combine(std::move(h), vb->ID());
    }

    constexpr inline bool operator==(const ShaderStorageBuffer & lhs, const ShaderStorageBuffer & rhs){
        return lhs->ID() == rhs->ID();
    }
}