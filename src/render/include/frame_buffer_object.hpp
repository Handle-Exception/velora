
#pragma once

#include <string>
#include <utility>

#include "type.hpp"
#include "texture.hpp"
#include "resolution.hpp"

namespace velora
{
    // Represents information about attachment to a Frame Buffer Object (FBO)
    struct FBOAttachment
    {
        // Attachment type
        enum class Type
        {
            Texture,
            RenderBuffer
        };

        // Attachment point
        enum class Point
        {
            Color,
            Depth,
            Stencil
        };

        Type type;
        Point point;
        TextureFormat format;
    };

    /**
     * @brief Interface definition for a Frame Buffer Object (FBO)
    */
    class IFrameBufferObject : public type::Interface
    {
        public:
        //
        virtual ~IFrameBufferObject() = default;
        
        /**
         * @brief Get the ID of the frame buffer object.
         * 
         * @return The ID of the frame buffer object.
         */
        virtual std::size_t ID() const = 0;

        /**
         * @brief Check if the frame buffer object is valid.
         * 
         * @return True if the frame buffer object is valid, false otherwise.
         */
        virtual bool good() const = 0;

        /**
         * @brief Enable the frame buffer object for rendering.
         * 
         * @return True if the frame buffer object was successfully enabled, false otherwise.
         */
        virtual bool enable() const = 0;

        /**
         * @brief Disable the frame buffer object.
         * 
         */
        virtual void disable() const = 0;

        /**
         * @brief Get the textures attached to the frame buffer object.
         * 
         * @return A vector of texture IDs.
         */
        virtual const std::vector<std::size_t> & getTextures() const = 0;
        
        /**
         * @brief Get the resolution of the frame buffer object.
         * 
         * @return The resolution of the frame buffer object.
         */
        virtual const Resolution & getResolution() const = 0;
    };

    template<class FrameBufferObjectImplType>
    class FrameBufferObjectDispatcher final : public type::Dispatcher<IFrameBufferObject, FrameBufferObjectImplType>
    {
        public:
            using dispatch = type::Dispatcher<IFrameBufferObject, FrameBufferObjectImplType>;

            inline FrameBufferObjectDispatcher(FrameBufferObjectImplType && obj) : dispatch(std::move(obj)){}
            template<class... Args>                                                                             
            inline FrameBufferObjectDispatcher(Args && ... args) : dispatch(std::forward<Args>(args)...){} 
            inline ~FrameBufferObjectDispatcher() = default;

            constexpr inline std::size_t ID() const override { return dispatch::getImpl().ID();}
            constexpr inline bool good() const override { return dispatch::getImpl().good();}
            constexpr inline bool enable() const override { return dispatch::getImpl().enable();}
            constexpr inline void disable() const override { return dispatch::getImpl().disable();}
            constexpr inline const std::vector<std::size_t> & getTextures() const override { return dispatch::getImpl().getTextures();}
            constexpr inline const Resolution & getResolution() const override { return dispatch::getImpl().getResolution();}
    };

    template<class FrameBufferObjectImplType>
    FrameBufferObjectDispatcher(FrameBufferObjectImplType && ) -> FrameBufferObjectDispatcher<FrameBufferObjectImplType>;

    class FrameBufferObject : public type::Implementation<IFrameBufferObject, FrameBufferObjectDispatcher> 
    {                                                                   
        public:
            /* move ctor */
            FrameBufferObject(FrameBufferObject && other) = default;
            FrameBufferObject & operator=(FrameBufferObject && other) = default;

            /* dtor */
            virtual ~FrameBufferObject() = default;
        
            /* ctor */
            template<class FrameBufferObjectImplType>
            FrameBufferObject(FrameBufferObjectImplType && impl)
            : Implementation(std::move(impl))
            {}                                                          
    };

    /**
     * @brief Compute the absolute hash value of a FrameBufferObject.
     * 
     * @param h The hash object.
     * @param vb The FrameBufferObject to be hashed.
     * @return The absolute hash value of the FrameBufferObject.
     */
    template <typename H>
    constexpr inline H AbslHashValue(H h, const FrameBufferObject & vb) {
        return H::combine(std::move(h), vb->ID());
    }

    /**
     * @brief Check if two FrameBufferObjects are equal.
     * 
     * @param lhs The first FrameBufferObject.
     * @param rhs The second FrameBufferObject.
     * @return True if the IDs of the two FrameBufferObjects are equal, false otherwise.
     */
    constexpr inline bool operator==(const FrameBufferObject & lhs, const FrameBufferObject & rhs){
        return lhs->ID() == rhs->ID();
    }
}