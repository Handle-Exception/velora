#pragma once

#include "ecs.hpp"
#include "render.hpp"

#include "visual_component.pb.h"
#include "transform_component.pb.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <asio.hpp>
#include <asio/experimental/awaitable_operators.hpp>
using namespace asio::experimental::awaitable_operators;

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
                
                glm::mat4 model_matrix = glm::mat4(1.0f);
                glm::vec3 position = glm::vec3(0.0f);
                glm::vec3 scale = glm::vec3(1.0f);
                glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

                for (const auto& [entity, mask] : entities.getAllEntities())
                {
                    if (mask.test(_POSITION_BIT) == false) continue;

                    visual_component = components.getComponent<VisualComponent>(entity);
                    assert(visual_component != nullptr);
                    
                    // if not visible, skip
                    if(visual_component->visible() == false) continue;

                    const auto [vb_id, sh_id] = co_await (
                            _renderer.getVertexBuffer(visual_component->vertex_buffer_name()) && 
                            _renderer.getShader(visual_component->shader_name()));

                    if (!vb_id || !sh_id)
                    {
                        // if cannot find vertex buffer or shader, skip
                        continue;
                    }

                    // if also has a transform component
                    // update transform matrix
                    static const uint32_t TRANSFORM_BIT = ComponentTypeManager::getTypeID<TransformComponent>();

                    if(mask.test(TRANSFORM_BIT))
                    {
                        auto* transform_component = components.getComponent<TransformComponent>(entity);
                        assert(transform_component != nullptr);
                        
                        position = glm::vec3(transform_component->position().x(), 
                                           transform_component->position().y(), 
                                           transform_component->position().z());
                        
                        scale = glm::vec3(transform_component->scale().x(), 
                                         transform_component->scale().y(), 
                                         transform_component->scale().z());
                        
                        rotation = glm::quat(transform_component->rotation().w(), 
                                            transform_component->rotation().x(), 
                                            transform_component->rotation().y(), 
                                            transform_component->rotation().z());

                        // get model matrix from transform component
                        model_matrix = glm::translate(glm::mat4(1.0f), position)
                                                        * glm::toMat4(rotation)
                                                        * glm::scale(glm::mat4(1.0f), scale);
                    }
                    else 
                    {
                        // if no transform component, use identity matrix
                        model_matrix = glm::mat4(1.0f);
                    }

                    // render
                    co_await _renderer.render(*vb_id, *sh_id, 
                        ShaderInputs{
                            .in_mat4f = {{"uModel", model_matrix}}
                        });
                }
                co_return;
            }

            inline constexpr std::string_view getName() const { return "VisualSystem"; }
        
            std::ranges::ref_view<std::vector<std::string>> getDependencies() const 
            {
                static std::vector<std::string> deps{"TransformSystem"};
                return std::views::all(deps);
            }

        private:
            static const uint32_t _POSITION_BIT;

            IRenderer & _renderer;
    };
}