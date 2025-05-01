#include "world.hpp"

namespace velora::game
{
    World::World(IRenderer &  renderer)
    :   _renderer(renderer),
        _entities(),
        _components(_entities)
    {
        _systems.emplace_back(System::construct<TransformSystem>(_components, _entities));
        _systems.emplace_back(System::construct<VisualSystem>(_components, _entities, _renderer));
        _systems.emplace_back(System::construct<HealthSystem>(_components, _entities));

        _layers = topologicalSortLayers(_systems);

        for(const auto & layer : _layers)
        {
            spdlog::debug("--- system layer ---");
            for(const auto & system : layer)
            {
                spdlog::debug(system->getName());
            }
        }
    }    
}