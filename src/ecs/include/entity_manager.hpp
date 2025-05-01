#pragma once

#include <bitset>
#include <unordered_map>

#include "entity.hpp"

namespace velora
{
    class EntityManager 
    {
        public:
        
            /**
             * @brief Creates a new entity.
             * 
             * @return The ID of the newly created entity.
             */
            Entity createEntity() 
            {
                Entity entity = _next_entity++;
                _masks[entity] = std::bitset<MAX_COMPONENT_TYPES>();
                return entity;
            }

            /**
             * @brief Destroys an entity and its associated components.
             * 
             * @param entity The ID of the entity to destroy.
             */
            void destroyEntity(Entity entity) {
                _masks.erase(entity);
            }


            void addComponentBit(Entity entity, uint32_t component_type_ID) {
                assert(component_type_ID < MAX_COMPONENT_TYPES);
                _masks[entity].set(component_type_ID);
            }
            
            
            void removeComponentBit(Entity entity, uint32_t component_type_ID) {
                assert(component_type_ID < MAX_COMPONENT_TYPES);
                _masks[entity].reset(component_type_ID);
            }


            const std::bitset<MAX_COMPONENT_TYPES>& getComponentMask(Entity entity) const {
                return _masks.at(entity);
            }
            

            const auto& getAllEntities() const {
                return _masks;
            }

    private:
            Entity _next_entity = 1;
            std::unordered_map<Entity, std::bitset<MAX_COMPONENT_TYPES>> _masks;
    };
}