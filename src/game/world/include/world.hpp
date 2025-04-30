#pragma once

#include "ecs.hpp"

namespace velora::game
{
    class World 
    {
        public:
            World()
            :   _entities(),
                _components(_entities)
            {}

        private:
            EntityManager _entities;
            ComponentManager _components;
    };
}