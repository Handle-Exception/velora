
#pragma once

#include "type.hpp"
#include <string>
#include <utility>

namespace velora
{
    /**
     * @brief Interface for Shader Storage Buffer Objects (SSBO)
     * 
     * @note This interface is used to create and manage Shader Storage Buffer Objects (SSBO).
     * 
     */
    class IShaderStorageBuffer : public type::Interface
    {
        public:
        virtual ~IShaderStorageBuffer() = default;

        /**
         * @brief Get the ID of the shader storage buffer.
         * 
         * @return The ID of the shader storage buffer.
         * 
         * @note This function must be called on the render thread strand.
         */
        virtual std::size_t ID() const = 0;
        
        /**
         * @brief Check if the shader storage buffer is valid.
         * 
         * @return True if the shader storage buffer is valid, false otherwise.
         * 
         * @note This function must be called on the render thread strand.
         */
        virtual bool good() const = 0;

        /**
         * @brief Enable the shader storage buffer.
         * 
         * @return True if the shader storage buffer was successfully enabled, false otherwise.
         * 
         * @note This function must be called on the render thread strand.
         */
        virtual bool enable() const = 0;

        /**
         * @brief Disable the shader storage buffer.
         * 
         * @note This function must be called on the render thread strand.
         */
        virtual void disable() const = 0;

        /**
         * @brief Update the data of the shader storage buffer.
         * 
         * @param size The size of the data to update.
         * @param data The data to update.
         * 
         * @note This function must be called on the render thread strand.
         */
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