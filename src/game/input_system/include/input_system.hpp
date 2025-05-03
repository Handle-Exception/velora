#pragma once

#include <queue>

#include "native.hpp"
#include <asio.hpp>

#include "input_component.pb.h"

#include "ecs.hpp"

#include <absl/container/flat_hash_set.h>

namespace velora::game
{
    class InputSystem
    {
        public:
            static const uint32_t MASK_POSITION_BIT;

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
                
                std::unordered_set<game::InputCode> last_keys;

                // clear inputs for entites
                for(auto & [entity, mask] : entities.getAllEntities())
                {
                    if(mask.test(MASK_POSITION_BIT) == false) continue;

                    auto* input_component = components.getComponent<InputComponent>(entity);
                    assert(input_component != nullptr);

                    // store old pressed keys in last_keys
                    last_keys.clear();
                    for(auto key : input_component->pressed())
                        last_keys.insert((game::InputCode)key);

                    input_component->clear_pressed();
                    input_component->clear_just_pressed();
                    input_component->clear_just_released();

                    for (InputCode key : _current_keys) 
                    {
                        // add pressed keys to input_component
                        input_component->add_pressed(key);
                        //if this key was not in last_keys, it is a new key pressed
                        if (!last_keys.contains(key)) input_component->add_just_pressed(key);
                    }

                    // iterate over last_keys and check if any key was released
                    for (InputCode key : last_keys) 
                    {
                        // if this key is not in current_keys, it was released
                        if (!_current_keys.contains(key)) input_component->add_just_released(key);
                    }

                }

                co_return;
            }

            asio::awaitable<void> recordKeyPressed(InputCode key)
            {
                co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));
                _current_keys.emplace(key);
                co_return;
            }

            asio::awaitable<void> recordKeyReleased(InputCode key)
            {
                co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));
                _current_keys.erase(key);
                co_return;
            }

            const SystemState& getState() const
            {
                return _state;
            }
        private:

            asio::strand<asio::io_context::executor_type> _strand;
            absl::flat_hash_set<InputCode> _current_keys;

            SystemState _state;
    };
}