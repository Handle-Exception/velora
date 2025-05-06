#include "light_system.hpp"

namespace velora::game
{
    const uint32_t LightSystem::MASK_POSITION_BIT = ComponentTypeManager::getTypeID<LightComponent>();

    LightSystem::LightSystem(asio::io_context & io_context, IRenderer & renderer)
    : _strand(asio::make_strand(io_context)), _renderer(renderer)
    {
    }


    asio::awaitable<void> LightSystem::run(const ComponentManager& components, const EntityManager& entities)
    {
        if(!_strand.running_in_this_thread()){
            co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));
        }
        
        collectLights(components, entities);
        co_return;
    }

    void LightSystem::collectLights(const ComponentManager& components, const EntityManager& entities)
    {
        _gpu_lights.clear();

        GPULight gpu_light{};
        for (const auto& [entity, mask] : entities.getAllEntities())
        {
            if (mask.test(MASK_POSITION_BIT) == false) continue;

            const LightComponent * light_component = components.getComponent<LightComponent>(entity);
            assert(light_component != nullptr);
            
            // w component is used to determine the type of light
            gpu_light.direction = glm::vec4(light_component->direction_x(), light_component->direction_y(), light_component->direction_z(), 
                static_cast<float>(light_component->type()));

            gpu_light.color = glm::vec4(light_component->color_r(), light_component->color_g(), light_component->color_b(), light_component->intensity());
            gpu_light.attenuation = glm::vec4(light_component->constant(), light_component->linear(), light_component->quadratic(), 0.0f);
            gpu_light.cutoff = glm::vec2(light_component->inner_cutoff(), light_component->outer_cutoff());
            gpu_light.castShadows = static_cast<uint32_t>(light_component->cast_shadows());

            // padding
            gpu_light.pad0 = gpu_light.pad1 = gpu_light.pad2 = 0;
            
            _gpu_lights.emplace_back(std::move(gpu_light));

        }

    }
}