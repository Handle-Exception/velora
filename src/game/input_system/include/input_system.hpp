#pragma once

#include <queue>

#include "native.hpp"
#include <asio.hpp>

#include "input_component.pb.h"

#include "ecs.hpp"


namespace velora::game
{
    class InputSystem
    {
        public:
            InputSystem(asio::io_context & io_context);
            InputSystem(const InputSystem&) = delete;
            InputSystem(InputSystem&&) = delete;
            InputSystem& operator=(const InputSystem&) = delete;
            InputSystem& operator=(InputSystem&&) = delete;
            ~InputSystem();

            inline constexpr std::string_view getName() const { return "InputSystem"; }
        
            std::ranges::ref_view<std::vector<std::string>> getDependencies() const {
                static std::vector<std::string> deps{};
                return std::views::all(deps);
            }

            asio::awaitable<void> run(ComponentManager& components, EntityManager& entities)
            {
                co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));
                // clear actions for entites
                for(auto & [entity, mask] : entities.getAllEntities())
                {
                    if(mask.test(_POSITION_BIT) == false) continue;

                    auto* input_component = components.getComponent<InputComponent>(entity);
                    assert(input_component != nullptr);

                    // clear action for entity
                    input_component->set_action(0);
                }

                if(_inputs.empty()) co_return;
                // apply inputs to instances of InputComponent
                int action = _inputs.front();
                _inputs.pop();
                for(auto & [entity, mask] : entities.getAllEntities())
                {
                    if(mask.test(_POSITION_BIT) == false) continue;

                    auto* input_component = components.getComponent<InputComponent>(entity);
                    assert(input_component != nullptr);
                    
                    // store current action for this system pass in component of given entity
                    // ??? tu w sumie nie jestem pewien czy to dobry plan
                    input_component->set_action(action);
                }
                co_return;
            }

            // crashes
            asio::awaitable<void> recordInput(int action)
            {
                co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));

                spdlog::debug("Recording input {}", action);
                _inputs.push(action);
                co_return;
            }

        private:
            static const uint32_t _POSITION_BIT;

            asio::strand<asio::io_context::executor_type> _strand;
            std::queue<int> _inputs;
    };
}