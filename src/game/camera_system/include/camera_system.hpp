#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "ecs.hpp"
#include "camera_component.pb.h"
#include "transform_system.hpp"
#include "render.hpp"
#include "resolution.hpp"

namespace velora::game
{
    class CameraSystem
    {
    public:
        static const uint32_t MASK_POSITION_BIT;

        constexpr static const char * NAME = "CameraSystem";
        constexpr static inline const char * getName() { return NAME; }

        constexpr static const std::initializer_list<const char *> DEPS = {"TransformSystem"};
        constexpr static inline const std::initializer_list<const char *> & getDependencies() {return DEPS;}

        CameraSystem(asio::io_context & io_context, IRenderer & renderer);
        CameraSystem(const CameraSystem&) = delete;
        CameraSystem(CameraSystem&&) = default;
        CameraSystem& operator=(const CameraSystem&) = delete;
        CameraSystem& operator=(CameraSystem&&) = default;
        ~CameraSystem() = default;

        asio::awaitable<void> run(const ComponentManager& components, const EntityManager& entities, float alpha);

        const glm::mat4 & getView() const;
        const glm::mat4 & getProjection () const;

    private:
        asio::strand<asio::io_context::executor_type> _strand;

        IRenderer & _renderer;
        glm::mat4 _view;
        glm::mat4 _projection;
        glm::vec3 _direction;
        glm::vec3 _up;
    };
}