#pragma once

#include "health_component.hpp"

#include "ecs.hpp"

namespace velora::game
{
    class HealthSystem 
    {
    public:
        HealthSystem(ComponentManager& components, EntityManager& entities);

        void damage(Entity entity, int amount);

    private:
        ComponentManager& _components;
        EntityManager& _entities;
    };
}