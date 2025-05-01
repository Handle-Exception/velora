#include "transform_system.hpp"

namespace velora::game
{
    const uint32_t TransformSystem::_POSITION_BIT = ComponentTypeManager::getTypeID<TransformComponent>();

    TransformSystem::TransformSystem()
    {}
}