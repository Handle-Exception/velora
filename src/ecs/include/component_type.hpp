#pragma once

#include <typeindex>
#include <unordered_map>
#include <cstdint>
#include <stdexcept>

namespace velora
{
    class ComponentTypeManager {
    public:
        template<typename Component>
        static uint32_t getTypeID() {
            static uint32_t id = _COUNTER++;
            return id;
        }

    private:
        static inline uint32_t _COUNTER = 0;
};
}