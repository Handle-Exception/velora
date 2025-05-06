#pragma once

#include "light_component.pb.h"

#include "ecs.hpp"
#include "render.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace velora::game
{
    struct GPULight {
        glm::vec4 position;     // w unused
        glm::vec4 direction;    // w = type
        glm::vec4 color;        // w = intensity
        glm::vec4 attenuation;  // x=constant, y=linear, z=quadratic, w=unused
        glm::vec2 cutoff;       // x=inner, y=outer
        uint32_t castShadows;
        uint32_t pad0;
        uint32_t pad1;
        uint32_t pad2;
    };

    class LightSystem 
    {
    public:
        static const uint32_t MASK_POSITION_BIT;

        constexpr static const char * NAME = "LightSystem";
        constexpr static inline const char * getName() { return NAME; }

        constexpr static const std::initializer_list<const char *> DEPS = {"TransformSystem"};
        constexpr static inline const std::initializer_list<const char *> & getDependencies() {return DEPS;}

        LightSystem(asio::io_context & io_context, IRenderer & renderer);
        LightSystem(const LightSystem&) = delete;
        LightSystem(LightSystem&&) = default;
        LightSystem& operator=(const LightSystem&) = delete;
        LightSystem& operator=(LightSystem&&) = default;
        ~LightSystem() = default;

        asio::awaitable<void> run(const ComponentManager& components, const EntityManager& entities);

    protected:
        void collectLights(const ComponentManager& components, const EntityManager& entities);

    private:
        asio::strand<asio::io_context::executor_type> _strand;
        IRenderer & _renderer;

        std::vector<GPULight> _gpu_lights;
    };
}