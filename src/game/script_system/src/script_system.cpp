#include "script_system.hpp"

namespace velora::game
{
    const uint32_t ScriptSystem::MASK_POSITION_BIT = ComponentTypeManager::getTypeID<ScriptComponent>();

    ScriptSystem::ScriptSystem(asio::io_context& io)
    : _strand(asio::make_strand(io))
    {
        _lua.open_libraries(sol::lib::base, sol::lib::math);

        bindFunctions();

    }

    void ScriptSystem::bindFunctions()
    {
        // bind lua types
        
        //https://github.com/ThePhD/sol2/issues/547
        auto vec3_mult_overloads = sol::overload(
            [](const glm::vec3& v1, const glm::vec3& v2) -> glm::vec3 { return v1*v2; },
            [](const glm::vec3& v1, float f) -> glm::vec3 { return v1*f; },
            [](float f, const glm::vec3& v1) -> glm::vec3 { return f*v1; }
        );
        _lua.new_usertype<glm::vec3>("vec3",
	        sol::constructors<glm::vec3(), glm::vec3(float), glm::vec3(float, float, float)>(),
            sol::call_constructor, sol::constructors<glm::vec3(float, float, float)>(),
		    "x", &glm::vec3::x,
		    "y", &glm::vec3::y,
		    "z", &glm::vec3::z,
            
            sol::meta_function::multiplication, vec3_mult_overloads
	    );

        _lua.new_usertype<glm::quat>("quat",
        sol::constructors<glm::quat(), glm::quat(float, float, float, float)>(),
        sol::call_constructor, sol::constructors<glm::quat(const glm::vec3&)>(),  // overload
        "w", &glm::quat::w,
        "x", &glm::quat::x,
        "y", &glm::quat::y,
        "z", &glm::quat::z
        );

        _lua.set_function("radians", [](const glm::vec3& degrees) -> glm::vec3 {
            return glm::radians(degrees);
        });

        _lua.set_function("degrees", [](const glm::vec3& radians) -> glm::vec3 {
            return glm::degrees(radians);
        });

        _lua.set_function("eulerAngles", [](const glm::quat & quat) -> glm::vec3 {
            return glm::eulerAngles(quat);
        });

        //Namespace under glm for clarity
        _glm_namespace = _lua.create_table();
        _glm_namespace["vec3"] = _lua["vec3"];
        _glm_namespace["quat"] = _lua["quat"];
        _glm_namespace["radians"] = _lua["radians"];
        _glm_namespace["degrees"] = _lua["degrees"];
        _glm_namespace["eulerAngles"] = _lua["eulerAngles"];

        //transform component
        _lua.new_usertype<LuaTransformRef>("Transform",
            "get_x", &LuaTransformRef::get_x,
            "set_x", &LuaTransformRef::set_x,
            "get_y", &LuaTransformRef::get_y,
            "set_y", &LuaTransformRef::set_y,
            "get_z", &LuaTransformRef::get_z,
            "set_z", &LuaTransformRef::set_z,
            "set_rotation", sol::overload(
                static_cast<void(LuaTransformRef::*)(float, float, float, float)>(&LuaTransformRef::set_rotation),
                static_cast<void(LuaTransformRef::*)(const glm::quat&)>(&LuaTransformRef::set_rotation)
            ),
            "get_rotation", &LuaTransformRef::get_rotation
        );

        //input component
        _lua.new_usertype<LuaInputRef>("Input",
            "is_pressed", &LuaInputRef::is_pressed,
            "just_pressed", &LuaInputRef::just_pressed,
            "just_released", &LuaInputRef::just_released,
            "get_mouse_x", &LuaInputRef::get_mouse_x,
            "get_mouse_y", &LuaInputRef::get_mouse_y,
            "get_mouse_dx", &LuaInputRef::get_mouse_dx,
            "get_mouse_dy", &LuaInputRef::get_mouse_dy
        );

        // bind lua functions
        _lua.set_function("print", [](sol::variadic_args args) {
            std::ostringstream out;

            for (auto arg : args) {
                sol::type t = arg.get_type();

                switch (t) {
                    case sol::type::string:
                        out << arg.as<std::string>();
                        break;
                    case sol::type::number:
                        out << arg.as<double>();
                        break;
                    case sol::type::boolean:
                        out << (arg.as<bool>() ? "true" : "false");
                        break;
                    case sol::type::nil:
                        out << "nil";
                        break;
                    case sol::type::userdata:
                        out << "[userdata]";
                        break;
                    case sol::type::table:
                        out << "[table]";
                        break;
                    case sol::type::function:
                        out << "[function]";
                        break;
                    default:
                        out << "[unknown]";
                }

                out << " ";
            }

            spdlog::info("[Lua] {}", out.str());
        });


        _lua.set_function("get_transform", [](sol::this_environment te, Entity e) -> std::optional<LuaTransformRef> {
            sol::environment& env = te;

            uintptr_t addr = env["__level_ptr"];
            Level* level_ptr = reinterpret_cast<Level*>(addr);
            
            if(!level_ptr)
            {
                spdlog::error("[Lua] __level_ptr is null");
                return std::nullopt;
            }

            auto* t = level_ptr->getComponent<TransformComponent>(e);
            if (!t) return std::nullopt;
            return LuaTransformRef{t};
        });

        _lua.set_function("get_input", [](sol::this_environment te, Entity e) -> std::optional<LuaInputRef> {
            sol::environment& env = te;

            uintptr_t addr = env["__level_ptr"];
            Level* level_ptr = reinterpret_cast<Level*>(addr);

            if(!level_ptr)
            {
                spdlog::error("[Lua] __level_ptr is null");
                return std::nullopt;
            }

            auto* c = level_ptr->getComponent<InputComponent>(e);
            if (!c) return std::nullopt;
            return LuaInputRef{c};
        });
    }
    
    void ScriptSystem::loadScript(std::filesystem::path path)
    {
        std::ifstream in(path);
        if (!in.is_open()) {
            spdlog::error(std::format("Failed to open Lua script: {}", path.string()));
            return;
        }
        std::stringstream buffer;
        buffer << in.rdbuf();
        // as index use the stem path component (filename without the final extension)
        _loaded_script_sources[path.stem()] = buffer.str();
        spdlog::info(std::format("Loaded Lua script source: {}", path.string()));
    }

    asio::awaitable<void> ScriptSystem::run(ComponentManager& components, EntityManager& entities, std::chrono::duration<double> delta, Level & current_level) 
    {
        if (!_strand.running_in_this_thread())
            co_await asio::dispatch(_strand, asio::use_awaitable);
        
        sol::environment env(_lua, sol::create, _lua.globals());  // sandboxed

        // bind the level pointer to the environment
        env["__level_ptr"]      = reinterpret_cast<uintptr_t>(&current_level);

        // rebind from global state into sandbox enviroment
        env["print"]            = _lua["print"];
        env["glm"]              = _glm_namespace;
        env["get_transform"]    = _lua["get_transform"];
        env["get_input"]        = _lua["get_input"];
        env["delta"]            = delta.count();

        for (const auto& [entity, mask] : entities.getAllEntities()) {
            if (!mask.test(ComponentTypeManager::getTypeID<ScriptComponent>())) continue;

            auto* sc = components.getComponent<ScriptComponent>(entity);
            assert(sc != nullptr);

            // as index use the stem path component (filename without the final extension)
            std::string name = sc->name();
            auto it = _loaded_script_sources.find(name);
            if (it == _loaded_script_sources.end()) continue;

            sol::load_result loaded = _lua.load(it->second);
            if (!loaded .valid()) 
            {
                sol::error err = loaded;
                spdlog::error(std::format("Failed to compile script '{}': {}", name, err.what()));
                continue;
            }

            env["entity"] = entity;

            try 
            {
                sol::function f = loaded;
                env.set_on(f);
                f(); // execute inline
            }
            catch (const std::exception& e) 
            {
                spdlog::error(std::format("Script execution failed [{}]: {}", name, e.what()));
            }
        }

        co_return;
    }

    void ScriptSystem::collectGarbage() 
    {
        _lua.collect_garbage();
    }

}