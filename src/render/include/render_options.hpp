#pragma once

#include <optional>

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


    struct RenderOptions
    {
        RenderMode mode = RenderMode::Solid;
        std::optional<PolygonOffset> polygon_offset;
    };
}