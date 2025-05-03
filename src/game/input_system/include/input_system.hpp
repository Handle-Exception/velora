#pragma once

#include <queue>

#include "native.hpp"
#include <asio.hpp>

#include "input_component.pb.h"

#include "ecs.hpp"

#include <absl/container/flat_hash_set.h>

namespace velora::game
{
    struct InputCallbacks
    {
        asio::any_io_executor executor;

        std::function<asio::awaitable<void>(game::InputCode)> onKeyPress;
        std::function<asio::awaitable<void>(game::InputCode)> onKeyRelease;
        std::function<asio::awaitable<void>(int, int)> onMouseMove;
        std::function<asio::awaitable<void>(int, int)> onMousePress;
        std::function<asio::awaitable<void>(int, int)> onMouseRelease;
        std::function<asio::awaitable<void>(int, int)> onMouseScroll;
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

            const SystemState& getState() const;

            asio::awaitable<void> run(ComponentManager& components, EntityManager& entities);

        private:

            asio::strand<asio::io_context::executor_type> _strand;
            absl::flat_hash_set<InputCode> _current_keys;

            SystemState _state;
    };

    template<class T>
    inline bool isInputPresent(game::InputCode code, const T & keys_set)
    {
        return std::find(keys_set.begin(), keys_set.end(), code) != keys_set.end();
    }

    game::InputCode keyToInputCode(int key);
}