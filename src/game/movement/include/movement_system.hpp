#pragma once

#include "position_component.hpp"

#include "ecs.hpp"

namespace velora::game
{
    class MovementSystem {
    public:
        MovementSystem(ComponentManager& components, EntityManager& entities)
            : _components(components), _entities(entities)
        {}

        void moveAll(float dx, float dy) {
            uint32_t position_bit = ComponentTypeManager::getTypeID<PositionComponent>();

            for (const auto& [entity, mask] : _entities.getAllEntities()) {
                if (mask.test(position_bit)) {
                    auto* pos = _components.getComponent<PositionComponent>(entity);
                    if (pos) {
                        pos->x += dx;
                        pos->y += dy;
                    }
                }
            }
        }

    private:
        ComponentManager& _components;
        EntityManager& _entities;
    };
}