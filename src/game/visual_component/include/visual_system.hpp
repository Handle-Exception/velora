#pragma once

#include "ecs.hpp"

#include "visual_component.hpp"

namespace velora::game
{
    class VisualSystem
    {
        public:
            VisualSystem(ComponentManager& components, EntityManager& entities, Renderer & renderer)
            :   _components(components),
                _entities(entities),
                _renderer(renderer)
            {}

            void render(Entity entity) 
            {
                auto* visual_component = _components.getComponent<VisualComponent>(entity);
                if (!visual_component) return;

                //_renderer.render(*visual_component);
            }

            void renderAll()
            {
                uint32_t position_bit = ComponentTypeManager::getTypeID<VisualComponent>();
                VisualComponent * visual_component = nullptr;
                
                for (const auto& [entity, mask] : _entities.getAllEntities())
                {
                    if (mask.test(position_bit) == false) continue;
                    visual_component = _components.getComponent<VisualComponent>(entity);
                    assert(visual_component != nullptr);

                }
            }
        private:
            ComponentManager& _components;
            EntityManager& _entities;
            Renderer & _renderer;
    };
}