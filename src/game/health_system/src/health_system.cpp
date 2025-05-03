#include "health_system.hpp"

namespace velora::game
{
    const uint32_t HealthSystem::MASK_POSITION_BIT = ComponentTypeManager::getTypeID<HealthSystem>();

    asio::awaitable<void> HealthSystem::run(ComponentManager& components, EntityManager& entities)
    {
        co_return;
    }

    const SystemState& HealthSystem::getState() const
    {
        return _state;
    }
}