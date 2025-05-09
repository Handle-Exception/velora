#pragma once

#include <vector>
#include <string>
#include <optional>
#include <cstring>
#include <tuple>

#include "native.hpp"
#include <asio.hpp>
#include <spdlog/spdlog.h>
#include <absl/container/flat_hash_map.h>

#include "render.hpp"

#include "transform_system.hpp"
#include "visual_system.hpp"
#include "health_system.hpp"
#include "input_system.hpp"
#include "camera_system.hpp"
#include "terrain_system.hpp"
#include "light_system.hpp"
#include "script_system.hpp"

#include "level.hpp"

namespace velora::game
{
    class World 
    {
        public:
            World(asio::io_context & io_context, IRenderer &  renderer);

            bool constructLevel(std::string level_name)
            {
                auto res = _levels.try_emplace(level_name , _renderer);
                return res.second;
            }

            Level & getLevel(std::string level_name)
            {
                return _levels.at(level_name);
            }

            const Level & getLevel(std::string level_name) const
            {
                return _levels.at(level_name);
            }

            void setCurrentLevel(std::string level_name)
            {
                _current_level = level_name;
            }

            const Level & getCurrentLevel() const
            {
                return _levels.at(_current_level);
            }

            Level & getCurrentLevel()
            {
                if(_current_level.empty())
                {
                    spdlog::error("No current level set, returning default level");
                    return _levels.at("default");
                }
                return _levels.at(_current_level);
            }

            // Run all systems layer by layer
            // in each layer, all systems are run in parallel
            // the order of the layers is guaranteed
            // the order of the systems in a layer is not guaranteed
            // systems in layer must be independent of each other
            asio::awaitable<void> update(std::chrono::duration<double> delta)
            {
                for(const auto & layer : _layers)
                {
                    co_await asio::co_spawn(_io_context, runLayer(delta, layer), asio::use_awaitable);
                }
                co_return;
            }

            std::vector<std::string> getLevelNames() const
            {
                std::vector<std::string> level_names;
                level_names.reserve(_levels.size());
                for(const auto & [name, _] : _levels)
                {
                    level_names.push_back(name);
                }
                return level_names;
            }

            ISystem & getSystem(const char * name)
            {
                for(auto & system : _systems)
                {
                    if(std::strcmp(system->getName(), name) == 0)
                    {
                        return *system;
                    }
                }
                throw std::runtime_error("System not found: " + std::string(name));
            }

            const ISystem & getSystem(const char * name) const
            {
                for(auto & system : _systems)
                {
                    if(std::strcmp(system->getName(), name) == 0)
                    {
                        return *system;
                    }
                }
                throw std::runtime_error("System not found: " + std::string(name));
            }

        protected:
            // Run a layer of systems
            // all systems in a layer are run in parallel
            // the order of the systems in the layer is not guaranteed
            // systems in layer must be independent of each other      
            asio::awaitable<void> runLayer(const std::chrono::duration<double> & delta, const std::vector<ISystem*> & layer)
            {
                std::vector<asio::awaitable<void>> joiners;
                joiners.reserve(layer.size());

                for (auto* system : layer) 
                {
                    auto joiner = asio::co_spawn(_io_context, getCurrentLevel().runSystem(*system), asio::use_awaitable);
                    joiners.emplace_back(std::move(joiner));
                }

                // Await all joiners (manually wait each)
                for (asio::awaitable<void> & joiner : joiners) 
                {
                    co_await std::move(joiner);
                }
            }

        private:
            asio::io_context & _io_context;
            IRenderer & _renderer;

            std::vector<System> _systems;
            std::vector<std::vector<ISystem*>> _layers;

            std::string _current_level;
            absl::flat_hash_map<std::string, Level> _levels;
    };
}