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

        for (const auto& [entity, mask] : entities.getAllEntities())
        {
            if (mask.test(MASK_POSITION_BIT) == false) continue;

            TransformComponent * transform_component = components.getComponent<TransformComponent>(entity);
            assert(transform_component != nullptr);

            transform_component->mutable_prev_position()->CopyFrom(transform_component->position());
            transform_component->mutable_prev_rotation()->CopyFrom(transform_component->rotation());
            transform_component->mutable_prev_scale()->CopyFrom(transform_component->scale());
        }
        co_return;
    }
}