#pragma once

#include <vector>
#include <string>
#include <optional>

#include "native.hpp"
#include <asio.hpp>
#include <spdlog/spdlog.h>
#include <absl/container/flat_hash_map.h>

#include "render.hpp"

#include "transform_system.hpp"
#include "visual_system.hpp"
#include "health_system.hpp"
#include "input_system.hpp"

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

            asio::awaitable<void> update()
            {
                for(const auto & layer : _layers)
                {
                    co_await asio::co_spawn(_io_context, runLayer(layer), asio::use_awaitable);
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

        protected:
            asio::awaitable<void> runLayer(const std::vector<ISystem*> & layer)
            {
                for(const auto & system : layer)
                {
                    co_await asio::co_spawn(_io_context, getCurrentLevel().runSystem(*system), asio::use_awaitable);
                }
                co_return;
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