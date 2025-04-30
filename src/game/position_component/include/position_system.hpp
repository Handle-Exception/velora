#pragma once

#include "position_component.hpp"

#include "ecs.hpp"

namespace velora::game
{
    class PositionSystem 
    {
        public:
            PositionSystem(ComponentManager& components, EntityManager& entities);

        // void moveAll(float dx, float dy) {
        //     uint32_t position_bit = ComponentTypeManager::getTypeID<PositionComponent>();

        //     for (const auto& [entity, mask] : _entities.getAllEntities()) {
        //         if (mask.test(position_bit)) {
        //             auto* pos = _components.getComponent<PositionComponent>(entity);
        //             if (pos) {
        //                 pos->x += dx;
        //                 pos->y += dy;
        //             }
        //         }
        //     }
        // }

            void move(Entity entity, float dx, float dy, float dz);

    private:
        ComponentManager& _components;
        EntityManager& _entities;
    };
}