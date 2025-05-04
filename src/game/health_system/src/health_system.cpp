#include "health_system.hpp"

namespace velora::game
{
    const uint32_t HealthSystem::MASK_POSITION_BIT = ComponentTypeManager::getTypeID<HealthSystem>();

    HealthSystem::HealthSystem(asio::io_context & io_context)
    : _strand(asio::make_strand(io_context))
    {
    }


    asio::awaitable<void> HealthSystem::run(ComponentManager& components, EntityManager& entities)
    {
        if(!_strand.running_in_this_thread()){
            co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));
        }
        
        for (const auto& [entity, mask] : entities.getAllEntities())
        {
            if (mask.test(MASK_POSITION_BIT) == false) continue;

            HealthComponent * health_component = components.getComponent<HealthComponent>(entity);
            assert(health_component != nullptr);
            
            // Update health component logic here
        }
        co_return;
    }
}