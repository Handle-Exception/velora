#include "ecs.hpp"

namespace velora
{
    std::vector<std::vector<ISystem*>> topologicalSortLayers(const std::vector<System> & systems)
    {
        absl::flat_hash_map<std::string_view, ISystem*> system_map;
        absl::flat_hash_map<std::string_view, int> in_degree;
        absl::flat_hash_map<std::string_view, std::vector<std::string_view>> adj;

        for (const auto& sys : systems)
        {
            system_map[sys->getName()] = sys.get();
            in_degree[sys->getName()] = 0;
        }

        // Build adjacency list and in-degree
        for (const auto& sys : systems)
        {
            for (const auto& dep : sys->getDependencies())
            {
                if (!system_map.count(dep))
                {
                    throw std::runtime_error(std::string("Unknown dependency: ") + dep);
                }

                adj[dep].push_back(sys->getName());
                in_degree[sys->getName()]++;
            }
        }

        std::queue<std::string_view> q;
        for (const auto& [name, degree] : in_degree) 
        {
            if (degree == 0) 
            {
                q.push(name);
            }
        }

        std::vector<std::vector<ISystem*>> layers;

        while (!q.empty())
        {
            size_t layer_size = q.size();
            std::vector<ISystem*> layer;

            for (size_t i = 0; i < layer_size; ++i)
            {
                std::string_view name = q.front();
                q.pop();
                layer.push_back(system_map[name]);

                for (const auto& neighbor : adj[name])
                {
                    in_degree[neighbor]--;
                    if (in_degree[neighbor] == 0)
                    {
                        q.push(neighbor);
                    }
                }
            }

            layers.push_back(std::move(layer));
        }

        // Check for cycle
        for (const auto& [_, degree] : in_degree) {
            if (degree != 0) {
                throw std::runtime_error("Cycle detected in system dependencies.");
            }
        }

        return layers;   
    }
}