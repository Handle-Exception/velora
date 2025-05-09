#pragma once

#include <fstream>
#include <format>
#include <filesystem>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <native.hpp>
#include <asio.hpp>
#include <asio/experimental/awaitable_operators.hpp>
using namespace asio::experimental::awaitable_operators;

#include <absl/container/flat_hash_map.h>

#include <sol/sol.hpp>

#include "ecs.hpp"
#include "script_component.pb.h"
#include "lua_components.hpp"
#include "level.hpp"

namespace velora::game
{
    class ScriptSystem 
    {
        public:
            static const uint32_t MASK_POSITION_BIT;

            constexpr static const char * NAME = "ScriptSystem ";
            constexpr static inline const char * getName() { return NAME; }

            constexpr static const std::initializer_list<const char *> DEPS = {"TransformSystem", "InputSystem"};
            constexpr static inline const std::initializer_list<const char *> & getDependencies() {return DEPS;}

            ScriptSystem(asio::io_context & io_context);
            ScriptSystem(const ScriptSystem&) = delete;
            ScriptSystem(ScriptSystem&&) = default;
            ScriptSystem& operator=(const ScriptSystem&) = delete;
            ScriptSystem& operator=(ScriptSystem&&) = default;
            ~ScriptSystem() = default;


            asio::awaitable<void> run(ComponentManager& components, EntityManager& entities, std::chrono::duration<double> delta, Level & current_level);
            void loadScript(std::filesystem::path path);
            void collectGarbage();

        private:
            asio::strand<asio::io_context::executor_type> _strand;
            sol::state _lua;
            sol::table _glm_namespace; 

            absl::flat_hash_map<std::filesystem::path, std::string> _loaded_script_sources; // only store raw text
            absl::flat_hash_map<Entity, sol::environment> _loaded_environments;
            
            void bindFunctions();
    };
}