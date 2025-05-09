#pragma once

#include <bitset>
#include <absl/container/flat_hash_map.h>

#include "entity.hpp"

namespace velora
{
    class EntityManager 
    {
        public:
            EntityManager();
            EntityManager(EntityManager &&);
            EntityManager(const EntityManager &) = delete;
            EntityManager& operator=(EntityManager &&);
            EntityManager& operator=(const EntityManager &) = delete;
            ~EntityManager() = default;

            /**
             * @brief Creates a new entity.
             * 
             * @return The ID of the newly created entity.
             */
            Entity createEntity();

            /**
             * @brief Destroys an entity and its associated components.
             * 
             * @param entity The ID of the entity to destroy.
             */
            void destroyEntity(Entity entity);

            void addComponentBit(Entity entity, uint32_t component_type_ID);
            
            void removeComponentBit(Entity entity, uint32_t component_type_ID);

            const std::bitset<MAX_COMPONENT_TYPES>& getComponentMask(Entity entity) const;

            const absl::flat_hash_map<Entity, std::bitset<MAX_COMPONENT_TYPES>> & getAllEntities() const;

    private:
            Entity _next_entity;
            absl::flat_hash_map<Entity, std::bitset<MAX_COMPONENT_TYPES>> _masks;
    };
}