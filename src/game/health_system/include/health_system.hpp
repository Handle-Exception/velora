#pragma once

#include "health_component.hpp"

#include "ecs.hpp"

namespace velora::game
{
    class HealthSystem 
    {
    public:
        HealthSystem(ComponentManager& components, EntityManager& entities);

        void damage(Entity entity, int amount);

        inline constexpr std::string_view getName() const { return "HealthSystem"; }

        inline std::ranges::ref_view<std::vector<std::string>> getDependencies() const 
        { 
            static std::vector<std::string> deps{};
            return std::views::all(deps);
        }

        asio::awaitable<void> run()
        {
            co_return;
        }

    private:

        ComponentManager& _components;
        EntityManager& _entities;
    };
}