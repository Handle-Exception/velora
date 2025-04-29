#pragma once

#include "health_component.hpp"

#include "ecs.hpp"

namespace velora::game
{

    class HealthSystem {
    public:
        HealthSystem(ComponentManager& components, EntityManager& entities) 
        : _components(components),  _entities(entities) 
        {}

        void damage(Entity entity, int amount) {
            auto* health = _components.getComponent<HealthComponent>(entity);
            if (health) {
                health->health -= amount;
                if (health->health < 0) health->health = 0;
            }
        }

    private:
        ComponentManager& _components;
        EntityManager& _entities;
    };
}