#include "world.hpp"

namespace velora::game
{
    World::World(asio::io_context & io_context, IRenderer &  renderer)
    :   _io_context(io_context),
        _renderer(renderer)
    {
        velora::ISystem * transform_system = _systems.emplace_back(System::construct<TransformSystem>(_io_context)).get();
        assert(transform_system != nullptr && "TransformSystem construction failed" );
        velora::ISystem * health_system = _systems.emplace_back(System::construct<HealthSystem>(_io_context)).get();
        assert(health_system != nullptr && "HealthSystem construction failed" );
        velora::ISystem * terrain_system = _systems.emplace_back(System::construct<TerrainSystem>(io_context, _renderer)).get();
        assert(terrain_system != nullptr && "TerrainSystem construction failed" );

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