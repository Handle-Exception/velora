#pragma once

#include <chrono>
using namespace std::chrono_literals;

#include "native.hpp"
#include <asio.hpp>
#include <spdlog/spdlog.h>

namespace velora
{

    struct FixedStepLoop
    {
        using clock = std::chrono::high_resolution_clock;

        FixedStepLoop(asio::io_context & io_context, 
                const std::chrono::duration<double> fixed_logic_step,
                std::function<bool()> condition,
                std::function<asio::awaitable<void>(std::chrono::duration<double>)> logic,
                std::function<asio::awaitable<void>(float)> priority);

        asio::awaitable<void> run();

        private:
            asio::strand<asio::any_io_executor> _strand;

            const std::chrono::duration<double> _fixed_logic_step;

            const std::function<bool()> _condition;
            const std::function<asio::awaitable<void>(std::chrono::duration<double>)> _logic;
            const std::function<asio::awaitable<void>(float)> _priority;

            constexpr static const std::chrono::duration<double> _MAX_ACCUMULATED_TIME = std::chrono::milliseconds(25); // avoid spiral of death
            uint64_t _simulation_tick = 0;

            struct TimeSpent
            {
                clock::time_point start = clock::now();
                clock::time_point end = clock::now();
                std::chrono::duration<double> duration = std::chrono::duration<double>::zero();
            };

            TimeSpent _total_time;
            TimeSpent _logic_time;
            TimeSpent _priority_time;

            std::chrono::duration<double> _lag = std::chrono::duration<double>::zero();

            constexpr static const float _ALPHA_SMOOTHING = 0.5f;
            float _raw_alpha = 0.0f;
            float _previous_alpha = 0.0f;
            float _alpha = 0.0f;
    };
}