#pragma once

#include "transform_component.hpp"

#include "ecs.hpp"


namespace velora::game
{
    class TransformSystem 
    {
        public:
            TransformSystem(ComponentManager& components, EntityManager& entities);

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
            inline constexpr std::string_view getName() const { return "PositionSystem"; }
        
            std::ranges::ref_view<std::vector<std::string>> getDependencies() const {
                static std::vector<std::string> deps{};
                return std::views::all(deps);
            }

            asio::awaitable<void> run()
            {
                co_return;
            }

            void move(Entity entity, float dx, float dy, float dz);

    private:
        ComponentManager& _components;
        EntityManager& _entities;
    };
}