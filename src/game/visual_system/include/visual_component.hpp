#pragma once

#include "render.hpp"

#include <glm/glm.hpp>

namespace velora::game
{
    struct VisualComponent
    {
        std::size_t vertex_buffer_ID;
        std::size_t shader_ID;

        glm::mat4 model_matrix{1.0f};

        bool visible = true;
    };
}