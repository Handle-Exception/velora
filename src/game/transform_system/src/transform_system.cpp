#include "transform_system.hpp"

namespace velora::game
{
    const uint32_t TransformSystem::MASK_POSITION_BIT = ComponentTypeManager::getTypeID<TransformComponent>();

    TransformSystem::TransformSystem()
    {}
}