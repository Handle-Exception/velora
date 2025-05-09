#include "vertex.hpp"

namespace velora
{
    namespace detail
    {
        struct VertexHasher {
            size_t operator()(const std::pair<uint32_t, uint32_t>& p) const noexcept {
                return std::hash<uint64_t>()((uint64_t)p.first << 32 | p.second);
            }
        };
    }

    const Mesh & getCubePrefab()
    {
        static const Mesh cube{
            {
                // Front (+Z)
                0, 1, 2,  3, 4, 5,
                // Back (-Z)
                6, 7, 8,  9,10,11,
                // Left (-X)
               12,13,14, 15,16,17,
                // Right (+X)
               18,19,20, 21,22,23,
                // Top (+Y)
               24,25,26, 27,28,29,
                // Bottom (-Y)
               30,31,32, 33,34,35
            },
            {
                // Front (+Z)
                {{-0.5f, -0.5f,  0.5f}, {0, 0, 1}, {0.0f, 0.0f}}, // 0
                {{ 0.5f, -0.5f,  0.5f}, {0, 0, 1}, {1.0f, 0.0f}}, // 1
                {{ 0.5f,  0.5f,  0.5f}, {0, 0, 1}, {0.5f, 1.0f}}, // 2
                {{-0.5f, -0.5f,  0.5f}, {0, 0, 1}, {0.0f, 0.0f}}, // 3
                {{ 0.5f,  0.5f,  0.5f}, {0, 0, 1}, {1.0f, 0.0f}}, // 4
                {{-0.5f,  0.5f,  0.5f}, {0, 0, 1}, {0.5f, 1.0f}}, // 5

                // Back (-Z)
                {{ 0.5f, -0.5f, -0.5f}, {0, 0, -1}, {0.0f, 0.0f}}, // 6
                {{-0.5f, -0.5f, -0.5f}, {0, 0, -1}, {1.0f, 0.0f}}, // 7
                {{-0.5f,  0.5f, -0.5f}, {0, 0, -1}, {0.5f, 1.0f}}, // 8
                {{ 0.5f, -0.5f, -0.5f}, {0, 0, -1}, {0.0f, 0.0f}}, // 9
                {{-0.5f,  0.5f, -0.5f}, {0, 0, -1}, {1.0f, 0.0f}}, //10
                {{ 0.5f,  0.5f, -0.5f}, {0, 0, -1}, {0.5f, 1.0f}}, //11

                // Left (-X)
                {{-0.5f, -0.5f, -0.5f}, {-1, 0, 0}, {0.0f, 0.0f}}, //12
                {{-0.5f, -0.5f,  0.5f}, {-1, 0, 0}, {1.0f, 0.0f}}, //13
                {{-0.5f,  0.5f,  0.5f}, {-1, 0, 0}, {0.5f, 1.0f}}, //14
                {{-0.5f, -0.5f, -0.5f}, {-1, 0, 0}, {0.0f, 0.0f}}, //15
                {{-0.5f,  0.5f,  0.5f}, {-1, 0, 0}, {1.0f, 0.0f}}, //16
                {{-0.5f,  0.5f, -0.5f}, {-1, 0, 0}, {0.5f, 1.0f}}, //17

                // Right (+X)
                {{ 0.5f, -0.5f,  0.5f}, {1, 0, 0}, {0.0f, 0.0f}},  //18
                {{ 0.5f, -0.5f, -0.5f}, {1, 0, 0}, {1.0f, 0.0f}},  //19
                {{ 0.5f,  0.5f, -0.5f}, {1, 0, 0}, {0.5f, 1.0f}},  //20
                {{ 0.5f, -0.5f,  0.5f}, {1, 0, 0}, {0.0f, 0.0f}},  //21
                {{ 0.5f,  0.5f, -0.5f}, {1, 0, 0}, {1.0f, 0.0f}},  //22
                {{ 0.5f,  0.5f,  0.5f}, {1, 0, 0}, {0.5f, 1.0f}},  //23

                // Top (+Y)
                {{-0.5f, 0.5f,  0.5f}, {0, 1, 0}, {0.0f, 0.0f}},   //24
                {{ 0.5f, 0.5f,  0.5f}, {0, 1, 0}, {1.0f, 0.0f}},   //25
                {{ 0.5f, 0.5f, -0.5f}, {0, 1, 0}, {0.5f, 1.0f}},   //26
                {{-0.5f, 0.5f,  0.5f}, {0, 1, 0}, {0.0f, 0.0f}},   //27
                {{ 0.5f, 0.5f, -0.5f}, {0, 1, 0}, {1.0f, 0.0f}},   //28
                {{-0.5f, 0.5f, -0.5f}, {0, 1, 0}, {0.5f, 1.0f}},   //29

                // Bottom (-Y)
                {{-0.5f, -0.5f, -0.5f}, {0, -1, 0}, {0.0f, 0.0f}}, //30
                {{ 0.5f, -0.5f, -0.5f}, {0, -1, 0}, {1.0f, 0.0f}}, //31
                {{ 0.5f, -0.5f,  0.5f}, {0, -1, 0}, {0.5f, 1.0f}}, //32
                {{-0.5f, -0.5f, -0.5f}, {0, -1, 0}, {0.0f, 0.0f}}, //33
                {{ 0.5f, -0.5f,  0.5f}, {0, -1, 0}, {1.0f, 0.0f}}, //34
                {{-0.5f, -0.5f,  0.5f}, {0, -1, 0}, {0.5f, 1.0f}}, //35
            }
        };

        return cube;
    }

    const Mesh& getConePrefab(unsigned int segments)
    {
        static Mesh cone;
        if (!cone.vertices.empty()) return cone;

        const float radius = 0.5f;
        const float height = 1.0f;
        const glm::vec3 apex = {0.0f, height * 0.5f, 0.0f};
        const glm::vec3 base_center = {0.0f, -height * 0.5f, 0.0f};

        cone.vertices.push_back({base_center, {0, -1, 0}, {0.5f, 0.5f}}); // center vertex at base
        unsigned int center_index = 0;

        // Base circle
        for (unsigned int i = 0; i <= segments; ++i) {
            float angle = (float)i / segments * glm::two_pi<float>();
            float x = cos(angle) * radius;
            float z = sin(angle) * radius;
            glm::vec2 uv = {x * 0.5f + 0.5f, z * 0.5f + 0.5f};
            cone.vertices.push_back({{x, base_center.y, z}, {0, -1, 0}, uv});
        }

        // Base indices (triangle fan)
        for (unsigned int i = 1; i <= segments; ++i) {
            cone.indices.push_back(center_index);
            cone.indices.push_back(i);
            cone.indices.push_back(i % segments + 1);
        }

        // Side faces
        unsigned int apex_index = cone.vertices.size();
        cone.vertices.push_back({apex, {0, 1, 0}, {0.5f, 1.0f}}); // top point

        for (unsigned int i = 1; i <= segments; ++i) {
            glm::vec3 p1 = cone.vertices[i].position;
            glm::vec3 p2 = cone.vertices[i % segments + 1].position;
            glm::vec3 n = glm::normalize(glm::cross(p2 - apex, p1 - apex));
            unsigned int i1 = cone.vertices.size();
            unsigned int i2 = i1 + 1;
            cone.vertices.push_back({p1, n, {0, 0}});
            cone.vertices.push_back({p2, n, {1, 0}});
            cone.indices.insert(cone.indices.end(), {i1, i2, apex_index});
        }

        return cone;
    }

    const Mesh& getCylinderPrefab(unsigned int segments)
    {
        static Mesh cyl;
        if (!cyl.vertices.empty()) return cyl;

        float radius = 0.5f, height = 1.0f;
        float y_top = +height * 0.5f, y_bottom = -height * 0.5f;

        // Top center
        unsigned int top_center_idx = cyl.vertices.size();
        cyl.vertices.push_back({{0, y_top, 0}, {0, 1, 0}, {0.5f, 0.5f}});

        // Top ring
        for (unsigned int i = 0; i <= segments; ++i) {
            float angle = (float)i / segments * glm::two_pi<float>();
            float x = cos(angle) * radius;
            float z = sin(angle) * radius;
            glm::vec2 uv = {x * 0.5f + 0.5f, z * 0.5f + 0.5f};
            cyl.vertices.push_back({{x, y_top, z}, {0, 1, 0}, uv});
        }

        for (unsigned int i = 1; i <= segments; ++i) {
            cyl.indices.insert(cyl.indices.end(), {
                top_center_idx, top_center_idx + i, top_center_idx + (i % segments + 1)
            });
        }

        // Bottom center
        unsigned int bottom_center_idx = cyl.vertices.size();
        cyl.vertices.push_back({{0, y_bottom, 0}, {0, -1, 0}, {0.5f, 0.5f}});

        // Bottom ring
        unsigned int start_idx = cyl.vertices.size();
        for (unsigned int i = 0; i <= segments; ++i) {
            float angle = (float)i / segments * glm::two_pi<float>();
            float x = cos(angle) * radius;
            float z = sin(angle) * radius;
            glm::vec2 uv = {x * 0.5f + 0.5f, z * 0.5f + 0.5f};
            cyl.vertices.push_back({{x, y_bottom, z}, {0, -1, 0}, uv});
        }

        for (unsigned int i = 1; i <= segments; ++i) {
            cyl.indices.insert(cyl.indices.end(), {
                bottom_center_idx, start_idx + (i % segments + 1), start_idx + i
            });
        }

        // Side walls
        unsigned int base_idx = cyl.vertices.size();
        for (unsigned int i = 0; i <= segments; ++i) {
            float angle = (float)i / segments * glm::two_pi<float>();
            float x = cos(angle) * radius;
            float z = sin(angle) * radius;
            glm::vec3 normal = glm::normalize(glm::vec3(x, 0, z));
            cyl.vertices.push_back({{x, y_top, z}, normal, {float(i) / segments, 0}});
            cyl.vertices.push_back({{x, y_bottom, z}, normal, {float(i) / segments, 1}});
        }

        for (unsigned int i = 0; i < segments; ++i) {
            unsigned int top1 = base_idx + i * 2;
            unsigned int bot1 = base_idx + i * 2 + 1;
            unsigned int top2 = base_idx + (i + 1) * 2;
            unsigned int bot2 = base_idx + (i + 1) * 2 + 1;
            cyl.indices.insert(cyl.indices.end(), {
                top1, bot1, top2,
                top2, bot1, bot2
            });
        }

        return cyl;
    }

    const Mesh& getIcoSpherePrefab(unsigned int subdivisions)
    {
        static std::vector<Mesh> cache;
        if (cache.size() <= subdivisions)cache.resize(subdivisions + 1);
        if (!cache[subdivisions].vertices.empty())return cache[subdivisions];

        auto& mesh = cache[subdivisions];

        // Golden ratio
        const float t = (1.0f + glm::sqrt(5.0f)) / 2.0f;

        std::vector<glm::vec3> positions = {
            {-1,  t,  0}, { 1,  t,  0}, {-1, -t,  0}, { 1, -t,  0},
            { 0, -1,  t}, { 0,  1,  t}, { 0, -1, -t}, { 0,  1, -t},
            { t,  0, -1}, { t,  0,  1}, {-t,  0, -1}, {-t,  0,  1}
        };

        for (auto& p : positions)
            p = glm::normalize(p);

        std::vector<std::array<uint32_t, 3>> faces = {
            {0, 11, 5}, {0, 5, 1}, {0, 1, 7}, {0, 7, 10}, {0, 10, 11},
            {1, 5, 9}, {5, 11, 4}, {11, 10, 2}, {10, 7, 6}, {7, 1, 8},
            {3, 9, 4}, {3, 4, 2}, {3, 2, 6}, {3, 6, 8}, {3, 8, 9},
            {4, 9, 5}, {2, 4, 11}, {6, 2, 10}, {8, 6, 7}, {9, 8, 1}
        };

        std::unordered_map<std::pair<uint32_t, uint32_t>, uint32_t, detail::VertexHasher> midpoint_cache;

        auto getMiddlePoint = [&](const uint32_t a, const uint32_t b) -> uint32_t {
            auto key = std::minmax(a, b);
            auto it = midpoint_cache.find(key);
            if (it != midpoint_cache.end())
                return it->second;

            glm::vec3 midpoint = glm::normalize((positions[a] + positions[b]) * 0.5f);
            positions.push_back(midpoint);
            uint32_t idx = static_cast<uint32_t>(positions.size() - 1);
            midpoint_cache[key] = idx;
            return idx;
        };

        for (int i = 0; i < subdivisions; ++i) {
            std::vector<std::array<uint32_t, 3>> subdivided;
            for (const auto& tri : faces) {
                const uint32_t a = getMiddlePoint(tri[0], tri[1]);
                uint32_t b = getMiddlePoint(tri[1], tri[2]);
                uint32_t c = getMiddlePoint(tri[2], tri[0]);
                subdivided.push_back({tri[0], a, c});
                subdivided.push_back({tri[1], b, a});
                subdivided.push_back({tri[2], c, b});
                subdivided.push_back({a, b, c});
            }
            faces = std::move(subdivided);
        }

        // Deduplicate vertices
        std::vector<uint32_t> remap(positions.size(), -1);
        for (const auto& tri : faces) {
            for (uint32_t idx : tri) {
                if (remap[idx] == uint32_t(-1)) {
                    remap[idx] = static_cast<uint32_t>(mesh.vertices.size());
                    mesh.vertices.push_back({positions[idx], positions[idx], {0, 0}});
                }
                mesh.indices.push_back(remap[idx]);
            }
        }

        return mesh;
    }

}