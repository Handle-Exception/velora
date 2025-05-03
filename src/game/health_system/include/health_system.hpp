#pragma once

#include "health_component.pb.h"

#include "ecs.hpp"

namespace velora::game
{
    class HealthSystem 
    {
    public:
        static const uint32_t MASK_POSITION_BIT;

        constexpr static const char * NAME = "HealthSystem";
        constexpr static inline const char * getName() { return NAME; }

        constexpr static const std::initializer_list<const char *> DEPS = {};
        constexpr static inline const std::initializer_list<const char *> & getDependencies() {return DEPS;}

        HealthSystem() = default;
        HealthSystem(const HealthSystem&) = delete;
        HealthSystem(HealthSystem&&) = default;
        HealthSystem& operator=(const HealthSystem&) = delete;
        HealthSystem& operator=(HealthSystem&&) = default;
        ~HealthSystem() = default;

        asio::awaitable<void> run(ComponentManager& components, EntityManager& entities);

        const SystemState& getState() const;

    private:
        SystemState _state;
    };
}