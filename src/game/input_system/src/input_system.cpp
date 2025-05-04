#include "input_system.hpp"

namespace velora::game
{
    const uint32_t InputSystem::MASK_POSITION_BIT = ComponentTypeManager::getTypeID<InputComponent>();
    
    game::InputCode keyToInputCode(int key)
    {
        #ifdef WIN32 //TODO: move to native.hpp?
            switch (key)
            {
                case 87: //W
                    return game::InputCode::KEY_W;
                case 65: //A
                    return game::InputCode::KEY_A;
                case 83: //S
                    return game::InputCode::KEY_S;
                case 68: //D
                    return game::InputCode::KEY_D;
                case 81: //Q
                    return game::InputCode::KEY_Q;
                case 69: //E
                    return game::InputCode::KEY_E;

                default:
                    return game::InputCode::UNKNOWN;
            }
        #endif

        return game::InputCode::UNKNOWN;
    }

    InputSystem::InputSystem(asio::io_context & io_context)
    : _strand(asio::make_strand(io_context))
    {
    }

    asio::awaitable<void> InputSystem::recordKeyPressed(InputCode key)
    {
        if(!_strand.running_in_this_thread()) {
            co_await asio::dispatch(_strand, asio::use_awaitable);
        }
        _event_queue.push_back(InputEvent{key, InputEventType::Pressed, std::chrono::steady_clock::now()});
        co_return;
    }

    asio::awaitable<void> InputSystem::recordKeyReleased(InputCode key)
    {
        if(!_strand.running_in_this_thread()) {
            co_await asio::dispatch(_strand, asio::use_awaitable);
        }
        _event_queue.push_back(InputEvent{key, InputEventType::Released, std::chrono::steady_clock::now()});
        co_return;
    }

    asio::awaitable<void> InputSystem::run(ComponentManager& components, EntityManager& entities)
    {
        if(!_strand.running_in_this_thread()){
            co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));
        }

        std::deque<InputEvent> events;
        std::swap(events, _event_queue); // fast, avoids realloc

        absl::flat_hash_set<InputCode> just_pressed;
        absl::flat_hash_set<InputCode> just_released;

        for (const auto& event : events)
        {
            switch (event.type) 
            {
                case InputEventType::Pressed:
                    if (_held_keys.insert(event.key).second)
                    {
                        just_pressed.insert(event.key);
                    }
                    break;
                case InputEventType::Released:
                    if (_held_keys.erase(event.key) > 0)
                    {
                        just_released.insert(event.key);
                    }
                    break;
            }
        }

        for (auto& [entity, mask] : entities.getAllEntities()) {
            if (!mask.test(MASK_POSITION_BIT)) continue;
            auto* input = components.getComponent<InputComponent>(entity);
            assert(input != nullptr);

            input->clear_pressed();
            input->clear_just_pressed();
            input->clear_just_released();

            for (auto key : _held_keys) input->add_pressed(key);
            for (auto key : just_pressed) input->add_just_pressed(key);
            for (auto key : just_released) input->add_just_released(key);
        }

        co_return;
    }
}