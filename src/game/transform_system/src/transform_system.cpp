#include "transform_system.hpp"

namespace velora::game
{
    TransformSystem::TransformSystem(ComponentManager& components, EntityManager& entities)
    :   _components(components),
        _entities(entities)
    {}

    void TransformSystem::move(Entity entity, float dx, float dy, float dz) 
    {
        auto* transform_component = _components.getComponent<TransformComponent>(entity);
        if(!transform_component) return;

        transform_component->position.x += dx;
        transform_component->position.y += dy;
        transform_component->position.z += dz;
    }
}