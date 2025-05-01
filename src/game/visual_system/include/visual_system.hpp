#pragma once

#include "ecs.hpp"

#include "visual_component.hpp"

#include "transform_component.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace velora::game
{
    class VisualSystem
    {
        public:
            VisualSystem(IRenderer & renderer)
            : _renderer(renderer)
            {}

            asio::awaitable<void> run(ComponentManager& components, EntityManager& entities)
            {
                VisualComponent * visual_component = nullptr;

                for (const auto& [entity, mask] : entities.getAllEntities())
                {
                    if (mask.test(_POSITION_BIT) == false) continue;
                    visual_component = components.getComponent<VisualComponent>(entity);
                    assert(visual_component != nullptr);
                    
                    // if not visible, skip
                    if(visual_component->visible == false) continue;

                    // if also has a transform component
                    // update transform matrix
                    static const uint32_t TRANSFORM_BIT = ComponentTypeManager::getTypeID<TransformComponent>();

                    if(mask.test(TRANSFORM_BIT))
                    {
                        auto* transform_component = components.getComponent<TransformComponent>(entity);
                        assert(transform_component != nullptr);
                        visual_component->model_matrix = glm::translate(glm::mat4(1.0f), transform_component->position)
                                                        * glm::toMat4(transform_component->rotation)
                                                        * glm::scale(glm::mat4(1.0f), transform_component->scale);
                    }

                    // render
                    //co_await_renderer.render(*visual_component);
                }
                co_return;
            }

            inline constexpr std::string_view getName() const { return "VisualSystem"; }
        
            std::ranges::ref_view<std::vector<std::string>> getDependencies() const 
            {
                static std::vector<std::string> deps{"PositionSystem"};
                return std::views::all(deps);
            }

        private:
            static const uint32_t _POSITION_BIT;

            IRenderer & _renderer;
    };
}