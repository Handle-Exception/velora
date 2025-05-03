#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "transform_component.pb.h"

#include "ecs.hpp"

namespace velora::game
{
    class TransformSystem 
    {
        public:
            static const uint32_t MASK_POSITION_BIT;

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
    };
}