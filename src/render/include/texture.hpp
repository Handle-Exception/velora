
#pragma once

#include "type.hpp"
#include <string>
#include <utility>

namespace velora
{
    class ITexture : public type::Interface
    {
        public:
        //
        virtual ~ITexture() = default;

        virtual std::size_t ID() const = 0;

        virtual bool good() const = 0;
        //
        virtual bool enable() const = 0;
        //
        virtual void disable() const = 0;
    };

    template<class TextureImplType>
    class TextureDispatcher final : public type::Dispatcher<ITexture, TextureImplType>
    {
        public:
            using dispatch = type::Dispatcher<ITexture, TextureImplType>;

            inline TextureDispatcher(TextureImplType && obj) : dispatch(std::move(obj)){}
            template<class... Args>                                                                             
            inline TextureDispatcher(Args && ... args) : dispatch(std::forward<Args>(args)...){} 
            inline ~TextureDispatcher() = default;

            constexpr inline std::size_t ID() const override { return dispatch::getImpl().ID();}
            constexpr inline bool good() const override { return dispatch::getImpl().good();}
            constexpr inline bool enable() const override { return dispatch::getImpl().enable();}
            constexpr inline void disable() const override { return dispatch::getImpl().disable();}
    };

    template<class TextureImplType>
    TextureDispatcher(TextureImplType && ) -> TextureDispatcher<TextureImplType>;

    class Texture : public type::Implementation<ITexture, TextureDispatcher> 
    {                                                                   
        public:
            /* move ctor */
            Texture(Texture && other) = default;
            Texture & operator=(Texture && other) = default;

            /* dtor */
            virtual ~Texture() = default;
        
            /* ctor */
            template<class TextureImplType>
            Texture(TextureImplType && impl)
            : Implementation(std::move(impl))
            {}                                                          
    };

    template <typename H>
    constexpr inline H AbslHashValue(H h, const Texture & vb) {
        return H::combine(std::move(h), vb->ID());
    }

    constexpr inline bool operator==(const Texture & lhs, const Texture & rhs){
        return lhs->ID() == rhs->ID();
    }
}