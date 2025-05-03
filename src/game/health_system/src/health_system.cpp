#include "health_system.hpp"

namespace velora::game
{
    const uint32_t HealthSystem::MASK_POSITION_BIT = ComponentTypeManager::getTypeID<HealthSystem>();
    
    HealthSystem::HealthSystem() 
    {}
}