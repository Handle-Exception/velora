#pragma once

#include "native.hpp"
#include <asio.hpp>
#include <asio/experimental/awaitable_operators.hpp>
using namespace asio::experimental::awaitable_operators;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "ecs.hpp"
#include "render.hpp"

#include "camera_system.hpp"
#include "transform_system.hpp"

#include "visual_component.pb.h"

namespace velora::game
{
    class VisualSystem
    {
        public:
            static const uint32_t MASK_POSITION_BIT;

            constexpr static const char * NAME = "VisualSystem";
            constexpr static inline const char * getName() { return NAME; }

            constexpr static const std::initializer_list<const char *> DEPS = {"TransformSystem", "CameraSystem"};
            constexpr static inline const std::initializer_list<const char *> & getDependencies() {return DEPS;}

            VisualSystem(const VisualSystem&) = delete;
            VisualSystem(VisualSystem&&) = default;
            VisualSystem& operator=(const VisualSystem&) = delete;
            VisualSystem& operator=(VisualSystem&&) = default;
            ~VisualSystem() = default;

            static asio::awaitable<VisualSystem> asyncConstructor(
                asio::io_context & io_context,
                IRenderer & renderer,
                Resolution resolution,
                game::CameraSystem & camera_system);

            // interpolated run
            asio::awaitable<void> run(ComponentManager& components, EntityManager& entities, float alpha);

            IRenderer & getRenderer() const;

            const std::vector<std::size_t> & getDeferredFBOTextures() const;

        protected:
            VisualSystem(asio::io_context & io_context,
                IRenderer & renderer,
                game::CameraSystem & camera_system,
                std::optional<std::size_t> fbo = std::nullopt);

        private:
            asio::strand<asio::io_context::executor_type> _strand;

            IRenderer & _renderer;
            game::CameraSystem & _camera_system;

            std::optional<std::size_t> _deferred_fbo;
            std::vector<std::size_t> _deferred_fbo_textures;
    };
}