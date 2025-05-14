
#pragma once

#include "type.hpp"
#include <string>
#include <utility>

namespace velora
{
    class IRenderBuffer : public type::Interface
    {
        public:
        virtual ~IRenderBuffer() = default;

        /**
         *  @brief Get the ID of the render buffer.
         *  
         *  @return The ID of the render buffer.
         */
        virtual std::size_t ID() const = 0;

        /**
         *  @brief Check if the render buffer is valid.
         *  
         *  @return True if the render buffer is valid, false otherwise.
         */
        virtual bool good() const = 0;

        /**
         *  @brief Enable the render buffer for rendering.
         *  
         *  @return True if the render buffer was successfully enabled, false otherwise.
         */
        virtual bool enable() const = 0;

        /**
         *  @brief Disable the render buffer.
         *  
         */
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

    /**
     *  @brief Compute the absolute hash value of a RenderBuffer.
     *  
     *  @param h The hash object.
     *  @param vb The RenderBuffer to be hashed.
     *  @return The absolute hash value of the RenderBuffer.
     *  
     *  @note This function is used to compute the absolute hash value of a RenderBuffer.
     *  @note The hash value is computed using the absl::Hash API.
     */
    template <typename H>
    constexpr inline H AbslHashValue(H h, const RenderBuffer & vb) {
        return H::combine(std::move(h), vb->ID());
    }

    /**
     * @brief Check if two RenderBuffers are equal.
     * 
     * @param lhs The first RenderBuffer.
     * @param rhs The second RenderBuffer.
     * @return True if the IDs of the two RenderBuffers are equal, false otherwise.
     */
    constexpr inline bool operator==(const RenderBuffer & lhs, const RenderBuffer & rhs){
        return lhs->ID() == rhs->ID();
    }
}