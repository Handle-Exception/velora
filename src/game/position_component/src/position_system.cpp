#include "position_system.hpp"

namespace velora::game
{
    PositionSystem::PositionSystem(ComponentManager& components, EntityManager& entities)
    :   _components(components),
        _entities(entities)
    {}

    void PositionSystem::move(Entity entity, float dx, float dy, float dz) 
    {
        auto* position_component = _components.getComponent<PositionComponent>(entity);
        if(!position_component) return;

        position_component->position.x += dx;
        position_component->position.y += dy;
        position_component->position.z += dz;
    }
}