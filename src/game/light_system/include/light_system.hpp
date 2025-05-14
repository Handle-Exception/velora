#pragma once

#include <array>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "ecs.hpp"
#include "render.hpp"

#include "light_component.pb.h"
#include "transform_system.hpp"
#include "visual_system.hpp"

namespace velora::game
{
    #pragma pack(push, 1)
    struct GPULight {
        glm::vec4 position;     // w unused
        glm::vec4 direction;    // w = type
        glm::vec4 color;        // w = intensity
        glm::vec4 attenuation;  // x=constant, y=linear, z=quadratic, w=unused
        glm::vec2 cutoff;       // x=inner, y=outer
        glm::vec2 castShadows;  // x=enabled, y=shadowMapIndex
    };
    #pragma pack(pop)

    class LightSystem 
    {
    public:
        static const uint32_t MASK_POSITION_BIT;
        static constexpr const uint16_t MAX_LIGHTS = 256;
        static constexpr const uint16_t MAX_SHADOW_CASTERS = 16;

        constexpr static const char * NAME = "LightSystem";
        constexpr static inline const char * getName() { return NAME; }

        constexpr static const std::initializer_list<const char *> DEPS = {"TransformSystem", "VisualSystem"};
        constexpr static inline const std::initializer_list<const char *> & getDependencies() {return DEPS;}

        static asio::awaitable<LightSystem> asyncConstructor(asio::io_context & io_context, VisualSystem & visual_system);

        LightSystem(const LightSystem&) = delete;
        LightSystem(LightSystem&&) = default;
        LightSystem& operator=(const LightSystem&) = delete;
        LightSystem& operator=(LightSystem&&) = default;
        ~LightSystem() = default;

        asio::awaitable<void> run(const ComponentManager& components, const EntityManager& entities, float alpha);

        std::size_t getLightShaderBufferID() const;
        std::size_t getLightsCount() const;

        std::vector<std::size_t> getShadowMapTextures() const;
        std::vector<glm::mat4> getShadowMapLightSpaceMatrices() const;
        std::size_t getShadowCastersCount() const;

    protected:
        LightSystem(
            asio::io_context & io_context,
            IRenderer & renderer,
            VisualSystem & visual_system,
            std::size_t light_shader_buffer_id,
            Resolution shadow_map_resolution,
            std::vector<std::size_t> shadow_map_fbos);

        void collectLights(const ComponentManager& components, const EntityManager& entities, float alpha);
        asio::awaitable<void> renderShadows(const ComponentManager& components, const EntityManager& entities, float alpha);

    private:
        asio::strand<asio::io_context::executor_type> _strand;
        IRenderer & _renderer;
        VisualSystem & _visual_system;

        std::vector<GPULight> _gpu_lights;
        std::size_t _light_shader_buffer_id;

        std::size_t _shadow_pass_shader;
        
        Resolution _shadow_map_resolution;
        std::vector<std::size_t> _shadow_map_fbos;
        std::vector<std::size_t> _shadow_map_textures;
        std::vector<glm::mat4> _shadow_map_light_space_matrices;
        std::size_t _shadow_casters_count;
    };
}