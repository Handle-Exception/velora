
#pragma once

#include "type.hpp"
#include <string>
#include <utility>

#include <glm/glm.hpp>
#include <absl/container/flat_hash_map.h>


namespace velora
{
    struct ShaderInputs
    {
        absl::flat_hash_map<std::string, bool> in_bool;

        absl::flat_hash_map<std::string, int> in_int;
        absl::flat_hash_map<std::string, float> in_float;

        absl::flat_hash_map<std::string, glm::vec2> in_vec2;
        absl::flat_hash_map<std::string, glm::vec3> in_vec3;
        absl::flat_hash_map<std::string, glm::vec4> in_vec4;

        absl::flat_hash_map<std::string, glm::mat2> in_mat2;
        absl::flat_hash_map<std::string, glm::mat3> in_mat3;
        absl::flat_hash_map<std::string, glm::mat4> in_mat4;
    };


    class IShader : public type::Interface
    {
        public:
        //
        virtual ~IShader() {};

        virtual std::size_t ID() const = 0;
        //
        virtual bool enable() = 0;
        //
        virtual bool disable() = 0;

        virtual void setUniform(const std::string & name, bool value) = 0;

        virtual void setUniform(const std::string & name, int value) = 0;
        virtual void setUniform(const std::string & name, float value)= 0;

        virtual void setUniform(const std::string & name, glm::vec2 value)= 0;
        virtual void setUniform(const std::string & name, glm::vec3 value)= 0;
        virtual void setUniform(const std::string & name, glm::vec4 value)= 0;

        virtual void setUniform(const std::string & name, glm::mat2 value)= 0;
        virtual void setUniform(const std::string & name, glm::mat3 value)= 0;
        virtual void setUniform(const std::string & name, glm::mat4 value)= 0;
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

            constexpr inline std::size_t ID() const override { return dispatch::getImpl().ID();}

            constexpr inline bool enable() override { return dispatch::getImpl().enable();}
            constexpr inline bool disable() override { return dispatch::getImpl().disable();}

            constexpr inline void setUniform(const std::string & name, bool value) override { return dispatch::getImpl().setUniform(name, std::move(value));}

            constexpr inline void setUniform(const std::string & name, int value) override { return dispatch::getImpl().setUniform(name, std::move(value));}
            constexpr inline void setUniform(const std::string & name, float value) override { return dispatch::getImpl().setUniform(name, std::move(value));}

            constexpr inline void setUniform(const std::string & name, glm::vec2 value) override { return dispatch::getImpl().setUniform(name, std::move(value));}
            constexpr inline void setUniform(const std::string & name, glm::vec3 value) override { return dispatch::getImpl().setUniform(name, std::move(value));}
            constexpr inline void setUniform(const std::string & name, glm::vec4 value) override { return dispatch::getImpl().setUniform(name, std::move(value));}

            constexpr inline void setUniform(const std::string & name, glm::mat2 value) override { return dispatch::getImpl().setUniform(name, std::move(value));}
            constexpr inline void setUniform(const std::string & name, glm::mat3 value) override { return dispatch::getImpl().setUniform(name, std::move(value));}
            constexpr inline void setUniform(const std::string & name, glm::mat4 value) override { return dispatch::getImpl().setUniform(name, std::move(value));}
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

    template <typename H>
    constexpr inline H AbslHashValue(H h, const Shader & shader) {
        return H::combine(std::move(h), shader->ID());
    }

    constexpr inline bool operator==(const Shader & lhs, const Shader & rhs){
        return lhs->ID() == rhs->ID();
    }
}