#include "world.hpp"

namespace velora::game
{
/*
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
*/

    World::World(asio::io_context & io_context)
    :   _io_context(io_context)
    {        
        // Initialize the default level
        _levels.emplace(std::move("default"), Level{});
    }


    bool World::constructLevel(std::string level_name)
    {
        auto res = _levels.emplace(std::move(level_name), Level{});
        return res.second;
    }

    Level & World::getLevel(std::string level_name)
    {
        return _levels.at(level_name);
    }

    const Level & World::getLevel(std::string level_name) const
    {
        return _levels.at(level_name);
    }

    void World::setCurrentLevel(std::string level_name)
    {
        _current_level = level_name;
    }

    const Level & World::getCurrentLevel() const
    {
        if(_current_level.empty())
        {
            spdlog::error("No current level set, returning default level");
            return _levels.at("default");
        }
        return _levels.at(_current_level);
    }

    Level & World::getCurrentLevel()
    {
        if(_current_level.empty())
        {
            spdlog::error("No current level set, returning default level");
            return _levels.at("default");
        }
        return _levels.at(_current_level);
    }

    std::vector<std::string> World::getLevelNames() const
    {
        std::vector<std::string> level_names;
        level_names.reserve(_levels.size());
        for(const auto & [name, _] : _levels)
        {
            level_names.push_back(name);
        }
        return level_names;
    }
}