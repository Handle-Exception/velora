
#pragma once

#include <string>
#include <utility>

#include "type.hpp"
#include "texture.hpp"

namespace velora
{
    struct FBOAttachment
    {
        enum class Type
        {
            Texture,
            RenderBuffer
        };

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

    class IFrameBufferObject : public type::Interface
    {
        public:
        //
        virtual ~IFrameBufferObject() = default;

        virtual std::size_t ID() const = 0;

        virtual bool good() const = 0;
        //
        virtual bool enable() const = 0;
        //
        virtual void disable() const = 0;

        virtual const std::vector<std::size_t> & getTextures() const = 0;
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

    template <typename H>
    constexpr inline H AbslHashValue(H h, const FrameBufferObject & vb) {
        return H::combine(std::move(h), vb->ID());
    }

    constexpr inline bool operator==(const FrameBufferObject & lhs, const FrameBufferObject & rhs){
        return lhs->ID() == rhs->ID();
    }
}