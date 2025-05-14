
#pragma once

#include "type.hpp"
#include <string>
#include <utility>

namespace velora
{
    /**
     * @brief Texture Format
     * 
     */
    enum class TextureFormat
    {
        RGB_8,
        RGB_16,
        RGB_32,

        RGB_16F,
        RGB_32F,

        RGBA_8,
        RGBA_16,
        RGBA_32,

        RGBA_16F,
        RGBA_32F,

        Depth,
        Depth_32F,
        Stencil
    };
    
    /**
     * @brief Texture Interface
     * 
     */
    class ITexture : public type::Interface
    {
        public:
        virtual ~ITexture() = default;

        /**
         * @brief Get the ID of the texture.
         * 
         * @return The ID of the texture.
         */
        virtual std::size_t ID() const = 0;

        /**
         * @brief Check if the texture is valid.
         * 
         * @return True if the texture is valid, false otherwise.
         */
        virtual bool good() const = 0;

        /**
         * @brief Enable the texture for rendering.
         * 
         * @return True if the texture was successfully enabled, false otherwise.
         */
        virtual bool enable() const = 0;

        /**
         * @brief Disable the texture.
         * 
         */
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

    /**
     * @brief Compute the absolute hash value of a Texture.
     * 
     * @param h The hash object.
     * @param vb The Texture to be hashed.
     * @return The absolute hash value of the Texture.
     * 
     * @note This function is used to compute the absolute hash value of a Texture.
     * @note The hash value is computed using the absl::Hash API.
     */
    template <typename H>
    constexpr inline H AbslHashValue(H h, const Texture & vb) {
        return H::combine(std::move(h), vb->ID());
    }

    /**
     * @brief Check if two Textures are equal.
     * 
     * @param lhs The first Texture.
     * @param rhs The second Texture.
     * @return True if the IDs of the two Textures are equal, false otherwise.
     * 
     * @note This function checks if two Textures are equal by comparing their IDs.
     */
    constexpr inline bool operator==(const Texture & lhs, const Texture & rhs){
        return lhs->ID() == rhs->ID();
    }
}