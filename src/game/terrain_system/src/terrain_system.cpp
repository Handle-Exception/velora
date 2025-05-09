#include "terrain_system.hpp"

namespace velora::game
{
    const uint32_t TerrainSystem::MASK_POSITION_BIT = ComponentTypeManager::getTypeID<TerrainComponent>();

    TerrainSystem::TerrainSystem(asio::io_context & io_context, IRenderer & renderer)
        : _strand(asio::make_strand(io_context)), _renderer(renderer)
    {}

    asio::awaitable<void> TerrainSystem::run(ComponentManager& components, EntityManager& entities, std::chrono::duration<double> delta)
    {
        if(!_strand.running_in_this_thread()){
            co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));
        }

        for (const auto& [entity, mask] : entities.getAllEntities())
        {
            if (mask.test(MASK_POSITION_BIT) == false) continue;

            TerrainComponent * terrain_component = components.getComponent<TerrainComponent>(entity);
            assert(terrain_component != nullptr);

            if(_batched_meshes.contains(entity) == false)
            {
                spdlog::debug("TerrainSystem: creating terrain mesh for entity {}", entity);
                
                // Compute vertices/indices
                std::vector<Vertex> vertices;
                std::vector<uint32_t> indices;
                for (uint32_t  z = 0; z <= terrain_component->length(); ++z) {
                    for (uint32_t  x = 0; x <= terrain_component->width(); ++x) {
                        vertices.push_back(
                            Vertex{ {x * terrain_component->tile_size(), 0.0f, z * terrain_component->tile_size()}, {0, 1, 0}, {0, 0} });
                    }
                }

                for (uint32_t  z = 0; z < terrain_component->length(); ++z) {
                    for (uint32_t  x = 0; x < terrain_component->width(); ++x) {
                        uint32_t topLeft = z * (terrain_component->width() + 1) + x;
                        uint32_t topRight = topLeft + 1;
                        uint32_t bottomLeft = (z + 1) * (terrain_component->width() + 1) + x;
                        uint32_t bottomRight = bottomLeft + 1;

                        indices.insert(indices.end(), { topLeft, bottomLeft, topRight, topRight, bottomLeft, bottomRight });
                    }
                }

                _batched_meshes.try_emplace(entity, indices, vertices);
            }
        }
        co_return;
    }
}