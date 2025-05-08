#pragma once

#include <array>
#include <vector>

#include "light_component.pb.h"
#include "transform_system.hpp"

#include "ecs.hpp"
#include "render.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace velora::game
{
    #pragma pack(push, 1)
    struct GPULight {
        glm::vec4 position;     // w unused
        glm::vec4 direction;    // w = type
        glm::vec4 color;        // w = intensity
        glm::vec4 attenuation;  // x=constant, y=linear, z=quadratic, w=unused
        glm::vec2 cutoff;       // x=inner, y=outer
        glm::vec2 castShadows;  // vec2 to have natural padding
    };
    #pragma pack(pop)

    class LightSystem 
    {
    public:
        static const uint32_t MASK_POSITION_BIT;
        static constexpr const uint16_t MAX_LIGHTS = 256;

        constexpr static const char * NAME = "LightSystem";
        constexpr static inline const char * getName() { return NAME; }

        constexpr static const std::initializer_list<const char *> DEPS = {"TransformSystem"};
        constexpr static inline const std::initializer_list<const char *> & getDependencies() {return DEPS;}

        static asio::awaitable<LightSystem> asyncConstructor(asio::io_context & io_context, IRenderer & renderer);

        LightSystem(const LightSystem&) = delete;
        LightSystem(LightSystem&&) = default;
        LightSystem& operator=(const LightSystem&) = delete;
        LightSystem& operator=(LightSystem&&) = default;
        ~LightSystem() = default;

        asio::awaitable<void> run(const ComponentManager& components, const EntityManager& entities);

        std::size_t getShaderBufferID() const;

        std::size_t getLightCount() const;

    protected:
        LightSystem(asio::io_context & io_context, IRenderer & renderer, std::size_t light_shader_buffer_id);

        void collectLights(const ComponentManager& components, const EntityManager& entities);

    private:
        asio::strand<asio::io_context::executor_type> _strand;
        IRenderer & _renderer;

        std::vector<GPULight> _gpu_lights;
        std::size_t _light_shader_buffer_id;
    };
}