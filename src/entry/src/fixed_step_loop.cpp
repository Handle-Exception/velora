#include "fixed_step_loop.hpp"

namespace velora
{
    FixedStepLoop::FixedStepLoop(asio::io_context & io_context, const std::chrono::duration<double> fixed_logic_step,
                std::function<bool()> condition,
                std::function<asio::awaitable<void>(std::chrono::duration<double>)> logic,
                std::function<asio::awaitable<void>(float)> priority) 
        :   _strand(asio::make_strand(io_context)),
            _fixed_logic_step(std::move(fixed_logic_step)),
            _condition(std::move(condition)),
            _logic(std::move(logic)),
            _priority(std::move(priority)) 
    {}

    asio::awaitable<void> FixedStepLoop::run()
    {
        // make sure to start executing loop from strand associated to provided io_context
        if(_strand.running_in_this_thread() == false)
        {
            co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));
        }

        spdlog::debug(std::format("[t:{}] Fixed step loop started", std::this_thread::get_id())); 

        _previous_alpha = 0.0f;
        _raw_alpha = 0.0f;

        while (_condition()) 
        {
            _total_time.duration = _total_time.end - _total_time.start;
            _total_time.start = clock::now();

            // clamp to avoid spiral of death
            _lag += std::min(_total_time.duration, _MAX_ACCUMULATED_TIME);
                
            _logic_time.duration = _logic_time.end - _logic_time.start;
            _logic_time.start = clock::now();

            // apply fixed time steps
            while (_lag >= _fixed_logic_step) 
            {
                // fixed time update
                co_await _logic(_fixed_logic_step);

                _simulation_tick++;
                _lag -= _fixed_logic_step;
            }

            _logic_time.end = clock::now();

            _priority_time.duration = _priority_time.end - _priority_time.start;
            _priority_time.start = clock::now();
                
            _raw_alpha = (float)(_lag / _fixed_logic_step);
            _raw_alpha  = std::clamp(_raw_alpha, 0.0f, 1.0f);
            if (std::isnan(_raw_alpha)) _raw_alpha = 0.0f;

            // exponential smoothing
            _alpha = std::lerp(_previous_alpha, _raw_alpha, _ALPHA_SMOOTHING);
            // store alpha for next frame
            _previous_alpha = _alpha;

            co_await _priority(_alpha);

            _priority_time.end = clock::now();
            _total_time.end = clock::now();
        }

        // make sure to end executing loop from strand associated to provided io_context
        if(_strand.running_in_this_thread() == false)
        {
            co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));
        }

        spdlog::debug(std::format("[t:{}] Fixed step loop ended", std::this_thread::get_id())); 

        co_return;
    }
}