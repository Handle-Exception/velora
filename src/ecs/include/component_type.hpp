#pragma once

#include <typeindex>
#include <unordered_map>
#include <cstdint>
#include <stdexcept>

namespace velora
{
    class ComponentTypeManager 
    {
    public:
        /**
         * @brief Retrieves the type ID for a given component type.
         * 
         * @tparam Component The component type.
         * 
         * @return The unique type ID for the component.
         */
        template<typename Component>
        static uint32_t getTypeID() {
            static uint32_t id = _COUNTER++;
            return id;
        }

    private:
        static inline uint32_t _COUNTER = 0;
    };
}