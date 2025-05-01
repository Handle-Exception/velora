#pragma once

#include "render.hpp"
#include "ecs.hpp"

#include <vector>

#include "transform_system.hpp"
#include "visual_system.hpp"
#include "health_system.hpp"

#include <spdlog/spdlog.h>

namespace velora::game
{
    class World 
    {
        public:
            World(IRenderer &  renderer);

        private:
            IRenderer & _renderer;
            
            EntityManager _entities;
            ComponentManager _components;

            std::vector<System> _systems;

            std::vector<std::vector<ISystem*>> _layers;
    };
}