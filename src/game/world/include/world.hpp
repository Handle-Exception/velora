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

#include "level.hpp"

namespace velora::game
{
    class World 
    {
        public:
            World(asio::io_context & io_context);
            World(World && other) = delete;
            World& operator=(World && other) = delete;
            World(const World & other) = delete;
            World& operator=(const World & other) = delete;
            ~World() = default;

            bool constructLevel(std::string level_name);

            Level & getLevel(std::string level_name);

            const Level & getLevel(std::string level_name) const;

            void setCurrentLevel(std::string level_name);

            const Level & getCurrentLevel() const;

            Level & getCurrentLevel();

            std::vector<std::string> getLevelNames() const;

        protected:

        private:
            asio::io_context & _io_context;

            std::string _current_level;
            absl::flat_hash_map<std::string, Level> _levels;
    };
}