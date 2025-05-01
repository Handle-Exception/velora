#pragma once

#include "render.hpp"

#include <glm/glm.hpp>

namespace velora::game
{
    struct VisualComponent
    {
        //IVertexBuffer & vertex_buffer;
        //IShader & shader;

        glm::mat4 model_matrix{1.0f};

        bool visible = true;
    };
}