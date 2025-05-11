#pragma once

#include <vector>
#include <algorithm>
#include <array>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <absl/container/flat_hash_map.h>

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

    const Mesh & getNormalizedDeviceCoordinatesQuadPrefab();

    const Mesh & getQuadPrefab();

    const Mesh & getCubePrefab();

    const Mesh& getIcoSpherePrefab(unsigned int subdivisions = 1);

    const Mesh& getConePrefab(unsigned int segments = 32);

    const Mesh& getCylinderPrefab(unsigned int segments = 32);
}