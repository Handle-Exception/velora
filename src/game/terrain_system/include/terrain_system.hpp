#pragma once

#include "ecs.hpp"
#include "render.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <asio.hpp>
#include <asio/experimental/awaitable_operators.hpp>
using namespace asio::experimental::awaitable_operators;

#include <absl/container/flat_hash_map.h>

#include "terrain_component.pb.h"

namespace velora::game
{
    class TerrainSystem
    {
        public:
            static const uint32_t MASK_POSITION_BIT;

            constexpr static const char * NAME = "TerrainSystem";
            constexpr static inline const char * getName() { return NAME; }

            constexpr static const std::initializer_list<const char *> DEPS = {};
            constexpr static inline const std::initializer_list<const char *> & getDependencies() {return DEPS;}

            TerrainSystem(asio::io_context & io_context, IRenderer & renderer);
            TerrainSystem(const TerrainSystem&) = delete;
            TerrainSystem(TerrainSystem&&) = default;
            TerrainSystem& operator=(const TerrainSystem&) = delete;
            TerrainSystem& operator=(TerrainSystem&&) = default;
            ~TerrainSystem() = default;

            asio::awaitable<void> run(ComponentManager& components, EntityManager& entities, std::chrono::duration<double> delta);

        private:
            asio::strand<asio::io_context::executor_type> _strand;

            IRenderer & _renderer;
            
            absl::flat_hash_map<Entity, Mesh> _batched_meshes;
    };
}