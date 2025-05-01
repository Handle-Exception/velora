#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace velora::game
{
    struct TransformComponent 
    {
        glm::vec3 position;
        glm::quat rotation;
        glm::vec3 scale;
    };
}