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
            
            constexpr static const char * NAME = "TransformSystem";
            constexpr static inline const char * getName() { return NAME; }

            constexpr static const std::initializer_list<const char *> DEPS = {};
            constexpr static inline const std::initializer_list<const char *> & getDependencies() {return DEPS;}

            TransformSystem() = default;
            TransformSystem(const TransformSystem&) = delete;
            TransformSystem(TransformSystem&&) = default;
            TransformSystem& operator=(const TransformSystem&) = delete;
            TransformSystem& operator=(TransformSystem&&) = default;
            ~TransformSystem() = default;

            asio::awaitable<void> run(ComponentManager& components, EntityManager& entities);
            const SystemState& getState() const;

        private:    
            SystemState _state;
    };
}