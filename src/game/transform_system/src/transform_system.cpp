#include "transform_system.hpp"

namespace velora::game
{
    const uint32_t TransformSystem::MASK_POSITION_BIT = ComponentTypeManager::getTypeID<TransformComponent>();

    TransformSystem::TransformSystem(asio::io_context & io_context)
        : _strand(asio::make_strand(io_context))
    {
    }

    asio::awaitable<void> TransformSystem::run(ComponentManager& components, EntityManager& entities)
    {
        if(!_strand.running_in_this_thread()){
            co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));
        }

        glm::quat rotation;

        glm::vec3 forward;
        glm::vec3 right;
        glm::vec3 up;

        for (const auto& [entity, mask] : entities.getAllEntities())
        {
            if (mask.test(MASK_POSITION_BIT) == false) continue;

            TransformComponent * transform_component = components.getComponent<TransformComponent>(entity);
            assert(transform_component != nullptr);
            
            _last_states[entity].CopyFrom(*transform_component);

            transform_component->mutable_prev_position()->CopyFrom(transform_component->position());
            transform_component->mutable_prev_rotation()->CopyFrom(transform_component->rotation());
            transform_component->mutable_prev_scale()->CopyFrom(transform_component->scale());

            rotation = glm::quat(transform_component->rotation().w(), transform_component->rotation().x(), transform_component->rotation().y(), transform_component->rotation().z());
            
            // calculate direction vectors
            forward = rotation * BASE_FORWARD_DIRECTION;
            forward = glm::normalize(forward);

            up = rotation * BASE_UP_DIRECTION;
            up = glm::normalize(up);

            right = glm::normalize(glm::cross(forward, up));

            transform_component->mutable_forward()->set_x(forward.x);
            transform_component->mutable_forward()->set_y(forward.y);
            transform_component->mutable_forward()->set_z(forward.z);

            transform_component->mutable_up()->set_x(up.x);
            transform_component->mutable_up()->set_y(up.y);
            transform_component->mutable_up()->set_z(up.z);

            transform_component->mutable_right()->set_x(right.x);
            transform_component->mutable_right()->set_y(right.y);
            transform_component->mutable_right()->set_z(right.z);

        }
        co_return;
    }
}