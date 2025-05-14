#pragma once

namespace velora
{
    enum class RenderMode
    {
        Wireframe,
        Solid,
        Textured,
        Shadow,
        ShadowTextured,
        Count
    };

    struct PolygonOffset
    {
        float factor;
        float units;
    };
}