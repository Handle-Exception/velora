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

        HealthSystem(asio::io_context & io_context);
        HealthSystem(const HealthSystem&) = delete;
        HealthSystem(HealthSystem&&) = default;
        HealthSystem& operator=(const HealthSystem&) = delete;
        HealthSystem& operator=(HealthSystem&&) = default;
        ~HealthSystem() = default;

        asio::awaitable<void> run(ComponentManager& components, EntityManager& entities, std::chrono::duration<double> delta);
    private:
        asio::strand<asio::io_context::executor_type> _strand;
    };
}