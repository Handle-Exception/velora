#pragma once

#include "health_component.pb.h"

#include "ecs.hpp"

namespace velora::game
{
    class HealthSystem 
    {
    public:
        static const uint32_t MASK_POSITION_BIT;

        HealthSystem();

        inline constexpr std::string_view getName() const { return "HealthSystem"; }

        inline std::ranges::ref_view<std::vector<std::string>> getDependencies() const 
        { 
            static std::vector<std::string> deps{};
            return std::views::all(deps);
        }

        asio::awaitable<void> run(ComponentManager& components, EntityManager& entities)
        {
            co_return;
        }

    private:

    };
}