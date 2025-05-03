#include "transform_system.hpp"

namespace velora::game
{
    const uint32_t TransformSystem::MASK_POSITION_BIT = ComponentTypeManager::getTypeID<TransformComponent>();

    asio::awaitable<void> TransformSystem::run(ComponentManager& components, EntityManager& entities)
    {
        co_return;
    }

    const SystemState& TransformSystem::getState() const
    {
        return _state;
    }
}