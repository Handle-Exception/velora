#pragma once

#include <cstdint>

namespace velora
{
    using Entity = std::uint32_t;

    constexpr Entity INVALID_ENTITY = 0;
    constexpr std::size_t MAX_COMPONENTS = 256;
}