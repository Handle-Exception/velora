#include "light_system.hpp"

namespace velora::game
{
    const uint32_t LightSystem::MASK_POSITION_BIT = ComponentTypeManager::getTypeID<LightComponent>();

    asio::awaitable<LightSystem> LightSystem::asyncConstructor(asio::io_context & io_context, IRenderer & renderer)
    {
        auto id = co_await renderer.constructShaderStorageBuffer("lights", 0, nullptr);
        if(id.has_value() == false)
        {
            throw std::runtime_error("Failed to create light buffer");
        }
        co_return LightSystem(io_context, renderer, *id);
    }

    LightSystem::LightSystem(asio::io_context & io_context, IRenderer & renderer, std::size_t light_shader_buffer_id)
    : _strand(asio::make_strand(io_context)), _renderer(renderer), _light_shader_buffer_id(light_shader_buffer_id)
    {
        _gpu_lights.reserve(MAX_LIGHTS);
    }

    std::size_t LightSystem::getShaderBufferID() const 
    { 
        return _light_shader_buffer_id; 
    }

    std::size_t LightSystem::getLightCount() const 
    { 
        return _gpu_lights.size();
    }

    asio::awaitable<void> LightSystem::run(const ComponentManager& components, const EntityManager& entities)
    {
        if(!_strand.running_in_this_thread()){
            co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));
        }
        
        collectLights(components, entities);

        co_await _renderer.updateShaderStorageBuffer(_light_shader_buffer_id, sizeof(GPULight) * _gpu_lights.size(), _gpu_lights.data());
        co_return;
    }

    void LightSystem::collectLights(const ComponentManager& components, const EntityManager& entities)
    {
        _gpu_lights.clear();

        GPULight gpu_light{};
        uint16_t light_id = 0;
        glm::vec3 direction;
        glm::quat rotation;
        for (const auto& [entity, mask] : entities.getAllEntities())
        {
            if(light_id >= MAX_LIGHTS)return;

            if (mask.test(MASK_POSITION_BIT) == false) continue;

            const LightComponent * light_component = components.getComponent<LightComponent>(entity);
            assert(light_component != nullptr);
            
            auto* transform = components.getComponent<TransformComponent>(entity);
            if(transform)
            {
                gpu_light.position = glm::vec4(transform->position().x(), transform->position().y(), transform->position().z(), 1.0f);
                rotation = glm::quat(transform->rotation().w(), transform->rotation().x(), transform->rotation().y(), transform->rotation().z());
                rotation = glm::normalize(rotation);
            }
            else
            {
                gpu_light.position = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
                rotation = glm::quat(1, 0, 0, 0); // Identity quaternion
            }
            
            direction = rotation * BASE_FORWARD_DIRECTION;
            direction = glm::normalize(direction);

            // w component is used to determine the type of light
            gpu_light.direction = glm::vec4(direction.x, direction.y, direction.z, 
                static_cast<float>(light_component->type()));

            gpu_light.color = glm::vec4(light_component->color_r(), light_component->color_g(), light_component->color_b(), light_component->intensity());
            gpu_light.attenuation = glm::vec4(light_component->constant(), light_component->linear(), light_component->quadratic(), 0.0f);
            gpu_light.cutoff = glm::vec2(light_component->inner_cutoff(), light_component->outer_cutoff());
            gpu_light.castShadows = glm::vec2(static_cast<uint32_t>(light_component->cast_shadows()), 0);
            
            _gpu_lights.emplace_back(std::move(gpu_light));
            light_id++;
        }
        assert(_gpu_lights.size() <= MAX_LIGHTS);
        assert(_gpu_lights.size() == light_id);
    }
}