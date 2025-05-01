
#pragma once

#include "type.hpp"
#include <string>
#include <utility>

namespace velora
{
    class IShader : public type::Interface
    {
        public:
        //
        virtual ~IShader() {};
        //
        virtual bool enable() const = 0;
        //
        virtual bool disable() const = 0;
    };

    template<class ShaderImplType>
    class ShaderDispatcher final : public type::Dispatcher<IShader, ShaderImplType>
    {
        public:
            using dispatch = type::Dispatcher<IShader, ShaderImplType>;

            inline ShaderDispatcher(ShaderImplType && obj) : dispatch(std::move(obj)){}
            template<class... Args>                                                                             
            inline ShaderDispatcher(Args && ... args) : dispatch(std::forward<Args>(args)...){} 
            inline ~ShaderDispatcher() = default;

            constexpr inline bool enable() const override { return dispatch::getImpl().enable();}
            constexpr inline bool disable() const override { return dispatch::getImpl().disable();}
    };

    template<class ShaderImplType>
    ShaderDispatcher(ShaderImplType && ) -> ShaderDispatcher<ShaderImplType>;

    class Shader : public type::Implementation<IShader, ShaderDispatcher> 
    {                                                                   
        public:
            /* move ctor */
            Shader(Shader && other) = default;
            Shader & operator=(Shader && other) = default;

            /* dtor */
            virtual ~Shader() = default;
        
            /* ctor */
            template<class ShaderImplType>
            Shader(ShaderImplType && impl)
            : Implementation(std::move(impl))
            {}                                                          
    };
}