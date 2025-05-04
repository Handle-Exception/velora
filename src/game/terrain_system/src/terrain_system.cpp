#include "terrain_system.hpp"

namespace velora::game
{
    const uint32_t TerrainSystem::MASK_POSITION_BIT = ComponentTypeManager::getTypeID<TerrainComponent>();

    TerrainSystem::TerrainSystem(IRenderer & renderer)
        : _renderer(renderer)
    {}

    const SystemState& TerrainSystem::getState() const
    {
        return _state;
    }

    asio::awaitable<void> TerrainSystem::run(ComponentManager& components, EntityManager& entities)
    {
        for (const auto& [entity, mask] : entities.getAllEntities())
        {
            if (mask.test(MASK_POSITION_BIT) == false) continue;

            TerrainComponent * terrain_component = components.getComponent<TerrainComponent>(entity);
            assert(terrain_component != nullptr);

            if(_batched_meshes.contains(terrain_component) == false)
            {
                // Compute vertices/indices
                std::vector<Vertex> vertices;
                std::vector<uint32_t> indices;
                for (int z = 0; z <= terrain_component->length(); ++z) {
                    for (int x = 0; x <= terrain_component->width(); ++x) {
                        vertices.push_back(
                            Vertex{ {x * terrain_component->tile_size(), 0.0f, z * terrain_component->tile_size()}, {0, 1, 0}, {0, 0} });
                    }
                }

                for (int z = 0; z < terrain_component->length(); ++z) {
                    for (int x = 0; x < terrain_component->width(); ++x) {
                        uint32_t topLeft = z * (terrain_component->width() + 1) + x;
                        uint32_t topRight = topLeft + 1;
                        uint32_t bottomLeft = (z + 1) * (terrain_component->width() + 1) + x;
                        uint32_t bottomRight = bottomLeft + 1;

                        indices.insert(indices.end(), { topLeft, bottomLeft, topRight, topRight, bottomLeft, bottomRight });
                    }
                }

                _batched_meshes.try_emplace(terrain_component, indices, vertices);
            }
        }
        co_return;
    }
}