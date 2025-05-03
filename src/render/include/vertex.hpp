#pragma once

#include <vector>

#include <glm/glm.hpp>

namespace velora
{
    //pragma pack(push, 1) to disable automatic padding
    #pragma pack(push, 1)
    struct Vertex 
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv;
    };
    #pragma pack(pop) // each vertex is now exactly 8 floats = 32 bytes, no extra padding


    struct Mesh
    {
        std::vector<unsigned int> indices;
        std::vector<Vertex> vertices;
    };

    const Mesh & getCubePrefab();
}