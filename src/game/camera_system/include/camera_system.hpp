#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "ecs.hpp"
#include "camera_component.pb.h"
#include "transform_component.pb.h"
#include "render.hpp"
#include "resolution.hpp"

namespace velora::game
{
    class CameraSystem
    {
    public:
        static const uint32_t MASK_POSITION_BIT;

        CameraSystem(IRenderer & renderer)
        : _renderer(renderer) {}

        inline std::string_view getName() const { return "CameraSystem"; }

        std::ranges::ref_view<std::vector<std::string>> getDependencies() const {
            static std::vector<std::string> deps{"TransformSystem"};
            return std::views::all(deps);
        }

        asio::awaitable<void> run(ComponentManager& components, EntityManager& entities)
        {
            const Resolution viewport = co_await _renderer.getViewport();
            if(viewport.getWidth() == 0 || viewport.getHeight() == 0) co_return;
            
            const float aspect = (float)viewport.getWidth() / (float)viewport.getHeight();

            for (const auto& [entity, mask] : entities.getAllEntities())
            {
                if (mask.test(MASK_POSITION_BIT) == false) continue;

                auto* cam = components.getComponent<CameraComponent>(entity);
                auto* transform = components.getComponent<TransformComponent>(entity);

                if (!cam || !transform) continue;
                if (!cam->is_primary()) continue;

                glm::vec3 pos{transform->position().x(), transform->position().y(), transform->position().z()};
                glm::quat rot{transform->rotation().w(), transform->rotation().x(), transform->rotation().y(), transform->rotation().z()};

                glm::mat4 view_matrix = glm::translate(glm::mat4(1.0f), -pos) * glm::toMat4(glm::conjugate(rot));
                glm::mat4 proj_matrix = glm::perspective(glm::radians(cam->fov()), aspect, cam->near_plane(), cam->far_plane());

                _state.write<glm::mat4>("uView", view_matrix);
                _state.write<glm::mat4>("uProjection", proj_matrix);

                break; // only first active camera
            }

            co_return;
        }

        const SystemState& getState() const
        {
            return _state;
        }

    private:
        IRenderer & _renderer;
        SystemState _state;
    };
}