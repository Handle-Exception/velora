#include "level.hpp"

namespace velora::game
{
    Level::Level(IRenderer &  renderer)
    :    _renderer(renderer),   
        _entities(),
        _components(_entities)
    {

    }
}