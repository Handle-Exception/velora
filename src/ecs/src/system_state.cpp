#include "system_state.hpp"

namespace velora
{
    std::size_t SystemState::align_up(std::size_t offset, std::size_t alignment) 
    {
        return (offset + alignment - 1) & ~(alignment - 1);
    }
}