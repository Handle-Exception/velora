#include "world.hpp"

namespace velora::game
{
    World::World(asio::io_context & io_context, IRenderer &  renderer)
    :   _io_context(io_context),
        _renderer(renderer)
    {
        _systems.emplace_back(System::construct<TransformSystem>());
        _systems.emplace_back(System::construct<VisualSystem>( _renderer));
        _systems.emplace_back(System::construct<HealthSystem>());

        _layers = topologicalSortLayers(_systems);

        for(const auto & layer : _layers)
        {
            spdlog::debug("--- system layer ---");
            for(const auto & system : layer)
            {
                spdlog::debug(system->getName());
            }
        }

        _levels.emplace("default", _renderer);
    }    
}