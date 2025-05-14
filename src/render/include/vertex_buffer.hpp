
#pragma once

#include "type.hpp"
#include <string>
#include <utility>

namespace velora
{
    /**
     * @brief Vertex Buffer Interface
     * 
     */
    class IVertexBuffer : public type::Interface
    {
        public:
        virtual ~IVertexBuffer() = default;

        /**
         * @brief Get the ID of the vertex buffer.
         * 
         * @return The ID of the vertex buffer.
         */
        virtual std::size_t ID() const = 0;

        /**
         * @brief Check if the vertex buffer is valid.
         * 
         * @return True if the vertex buffer is valid, false otherwise.
         */
        virtual bool good() const = 0;

        /**
         * @brief Get the number of elements in the vertex buffer.
         * 
         * @return The number of elements in the vertex buffer.
         */
        virtual std::size_t numberOfElements() const = 0;

        /**
         * @brief Enable the vertex buffer for rendering.
         * 
         * @return True if the vertex buffer was successfully enabled, false otherwise.
         */
        virtual bool enable() const = 0;

        /**
         * @brief Disable the vertex buffer.
         * 
         */
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

    /**
     * @brief Compute the absolute hash value of a VertexBuffer.
     * 
     * @param h The hash object.
     * @param vb The VertexBuffer to be hashed.
     * @return The absolute hash value of the VertexBuffer.
     * 
     * @note This function is used to compute the absolute hash value of a VertexBuffer.
     * @note The hash value is computed using the absl::Hash API.
     */
    template <typename H>
    constexpr inline H AbslHashValue(H h, const VertexBuffer & vb) {
        return H::combine(std::move(h), vb->ID());
    }

    /**
     * @brief Check if two VertexBuffers are equal.
     * 
     * @param lhs The first VertexBuffer.
     * @param rhs The second VertexBuffer.
     * @return True if the IDs of the two VertexBuffers are equal, false otherwise.
     */
    constexpr inline bool operator==(const VertexBuffer & lhs, const VertexBuffer & rhs){
        return lhs->ID() == rhs->ID();
    }
}