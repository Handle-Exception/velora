#include "health_system.hpp"

namespace velora::game
{
    HealthSystem::HealthSystem(ComponentManager& components, EntityManager& entities) 
    : _components(components),  _entities(entities) 
    {}

    void HealthSystem::damage(Entity entity, int amount) 
    {
        auto* health_component = _components.getComponent<HealthComponent>(entity);
        if (health_component == nullptr)return; 
        
        health_component->health -= amount;
        if (health_component->health < 0) health_component->health = 0;
    }
}