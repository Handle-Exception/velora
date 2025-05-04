#pragma once

#include <queue>
#include <deque>

#include "native.hpp"
#include <asio.hpp>

#include "input_component.pb.h"

#include "ecs.hpp"

#include <absl/container/flat_hash_set.h>

namespace velora::game
{
    enum class InputEventType 
    {
        Pressed,
        Released
    };

    struct InputEvent 
    {
        InputCode key;
        InputEventType type;
        std::chrono::steady_clock::time_point timestamp;
    };

    class InputSystem
    {
        public:
            static const uint32_t MASK_POSITION_BIT;

            constexpr static const char * NAME = "InputSystem";
            constexpr static inline const char * getName() { return NAME; }

            constexpr static const std::initializer_list<const char *> DEPS = {};
            constexpr static inline const std::initializer_list<const char *> & getDependencies() {return DEPS;}

            InputSystem(asio::io_context & io_context);
            InputSystem(const InputSystem&) = delete;
            InputSystem(InputSystem&&) = delete;
            InputSystem& operator=(const InputSystem&) = delete;
            InputSystem& operator=(InputSystem&&) = delete;
            ~InputSystem() = default;

            asio::awaitable<void> recordKeyPressed(InputCode key);

            asio::awaitable<void> recordKeyReleased(InputCode key);

            asio::awaitable<void> run(ComponentManager& components, EntityManager& entities);

        private:

            asio::strand<asio::io_context::executor_type> _strand;

            std::deque<InputEvent> _event_queue;
            absl::flat_hash_set<InputCode> _held_keys; // tracks current state
    };

    template<class T>
    inline bool isInputPresent(game::InputCode code, const T & keys_set)
    {
        return std::find(keys_set.begin(), keys_set.end(), code) != keys_set.end();
    }

    game::InputCode keyToInputCode(int key);
}