#include "transform_system.hpp"

namespace velora::game
{
    const uint32_t TransformSystem::MASK_POSITION_BIT = ComponentTypeManager::getTypeID<TransformComponent>();

    glm::mat4 calculateInterpolatedTransformMatrix(const TransformComponent & transform_component, float alpha)
    {
        const glm::vec3 prev_position = glm::vec3(transform_component.prev_position().x(), 
                                    transform_component.prev_position().y(), 
                                    transform_component.prev_position().z());
                        
        const glm::vec3 prev_scale = glm::vec3(transform_component.prev_scale().x(), 
                                    transform_component.prev_scale().y(), 
                                    transform_component.prev_scale().z());
                        
        const glm::quat prev_rotation = glm::normalize(glm::quat(transform_component.prev_rotation().w(), 
                                    transform_component.prev_rotation().x(), 
                                    transform_component.prev_rotation().y(), 
                                    transform_component.prev_rotation().z()));


        const glm::vec3 position = glm::vec3(transform_component.position().x(), 
                                    transform_component.position().y(), 
                                    transform_component.position().z());
                        
        const glm::vec3 scale = glm::vec3(transform_component.scale().x(), 
                                    transform_component.scale().y(), 
                                    transform_component.scale().z());
                        
        const glm::quat rotation = glm::normalize(glm::quat(transform_component.rotation().w(), 
                                    transform_component.rotation().x(), 
                                    transform_component.rotation().y(), 
                                    transform_component.rotation().z()));

        const glm::vec3 interpolated_pos = glm::mix(prev_position, position, alpha);
        const glm::quat interpolated_rot = glm::slerp(prev_rotation, rotation, alpha);
        const glm::vec3 interpolated_scale = glm::mix(prev_scale, scale, alpha);

                // get model matrix from transform component
        return glm::translate(glm::mat4(1.0f), interpolated_pos)
                                                * glm::toMat4(interpolated_rot)
                                                * glm::scale(glm::mat4(1.0f), interpolated_scale);
    }


    TransformSystem::TransformSystem(asio::io_context & io_context)
        : _strand(asio::make_strand(io_context))
    {
    }

    asio::awaitable<void> TransformSystem::run(ComponentManager& components, EntityManager& entities, std::chrono::duration<double> delta)
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