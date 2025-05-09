#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "transform_component.pb.h"

#include "ecs.hpp"

namespace velora::game
{
    constexpr const glm::vec3 BASE_FORWARD_DIRECTION = glm::vec3(0.0f, 0.0f, -1.0f);
    constexpr const glm::vec3 BASE_UP_DIRECTION = glm::vec3(0.0f, 1.0f, 0.0f);

    class TransformSystem 
    {
        public:
            static const uint32_t MASK_POSITION_BIT;
            
            constexpr static const char * NAME = "TransformSystem";
            constexpr static inline const char * getName() { return NAME; }

            constexpr static const std::initializer_list<const char *> DEPS = {};
            constexpr static inline const std::initializer_list<const char *> & getDependencies() {return DEPS;}

            TransformSystem(asio::io_context & io_context);
            TransformSystem(const TransformSystem&) = delete;
            TransformSystem(TransformSystem&&) = default;
            TransformSystem& operator=(const TransformSystem&) = delete;
            TransformSystem& operator=(TransformSystem&&) = default;
            ~TransformSystem() = default;

            asio::awaitable<void> run(ComponentManager& components, EntityManager& entities);

        private:
            asio::strand<asio::io_context::executor_type> _strand;

    };
}