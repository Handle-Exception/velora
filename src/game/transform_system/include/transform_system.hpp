#pragma once

#include "transform_component.hpp"

#include "ecs.hpp"


namespace velora::game
{
    class TransformSystem 
    {
        public:
            TransformSystem();

            inline constexpr std::string_view getName() const { return "TransformSystem"; }
        
            std::ranges::ref_view<std::vector<std::string>> getDependencies() const {
                static std::vector<std::string> deps{};
                return std::views::all(deps);
            }

            asio::awaitable<void> run(ComponentManager& components, EntityManager& entities)
            {
                co_return;
            }

        private:
            static const uint32_t _POSITION_BIT;
    };
}