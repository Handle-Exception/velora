#include "camera_system.hpp"

namespace velora::game
{
    const uint32_t CameraSystem::MASK_POSITION_BIT = ComponentTypeManager::getTypeID<CameraComponent>();
    
    CameraSystem::CameraSystem(asio::io_context & io_context, IRenderer & renderer)
    : _strand(asio::make_strand(io_context)), _renderer(renderer) 
    {

    }

    const glm::mat4 & CameraSystem::getView() const 
    { 
        return _view;
    }
    
    const glm::mat4 & CameraSystem::getProjection () const 
    { 
        return _projection;
    }

    asio::awaitable<void> CameraSystem::run(const ComponentManager& components, const EntityManager& entities, float alpha)
    {
        if(!_strand.running_in_this_thread()){
            co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));
        }
        
        const Resolution & viewport = _renderer.getViewport();
        if(viewport.getWidth() == 0 || viewport.getHeight() == 0) co_return;

        const float aspect = (float)viewport.getWidth() / (float)viewport.getHeight();

        glm::vec3 position;
        glm::quat rotation;
        
        float eased_alpha = glm::smoothstep(0.0f, 1.0f, alpha);

        glm::vec3 prev_position;
        glm::quat prev_rotation;

        glm::vec3 interpolated_pos;
        glm::quat interpolated_rot;

        for (const auto& [entity, mask] : entities.getAllEntities())
        {
            if (mask.test(MASK_POSITION_BIT) == false) continue;

            auto* cam = components.getComponent<CameraComponent>(entity);
            auto* transform = components.getComponent<TransformComponent>(entity);

            if (!cam || !transform) continue;
            if (!cam->is_primary()) continue;

            position = glm::vec3{transform->position().x(), transform->position().y(), transform->position().z()};
            rotation = glm::quat{transform->rotation().w(), transform->rotation().x(), transform->rotation().y(), transform->rotation().z()};
            rotation = glm::normalize(rotation);

            prev_position = glm::vec3{transform->prev_position().x(), transform->prev_position().y(), transform->prev_position().z()};

            prev_rotation = glm::quat{transform->prev_rotation().w(), transform->prev_rotation().x(), transform->prev_rotation().y(), transform->prev_rotation().z()};
            prev_rotation = glm::normalize(prev_rotation);

            interpolated_pos = glm::mix(prev_position, position, eased_alpha);
            interpolated_rot = glm::slerp(prev_rotation, rotation, eased_alpha);

            _direction = interpolated_rot * BASE_FORWARD_DIRECTION;
            _up = interpolated_rot * BASE_UP_DIRECTION;

            _view = glm::lookAt(interpolated_pos, interpolated_pos + _direction, _up);
            _projection = glm::perspective(glm::radians(cam->fov()), aspect, cam->near_plane(), cam->far_plane());

            break; // only first active camera
        }

        co_return;
    }
}