#pragma once

#include <unordered_map>
#include <typeindex>
#include <memory>

#include "entity.hpp"
#include "entity_manager.hpp"
#include "component_type.hpp"

namespace velora
{
    /**
     * @brief Interface for component storage.
     */
    class IComponentStorage
    {
        public:
            virtual ~IComponentStorage() = default;
    };

    /**
     * @brief Storage for components of a specific type.
     * 
     * This class manages the storage and retrieval of components associated with
     * entities in the ECS (Entity-Component-System) architecture.
     * 
     * @tparam Component The type of the component to store.
     */
    template<typename Component>
    class ComponentStorage : public IComponentStorage 
    {
        public:
            /**
             * @brief Adds a component to the storage.
             * 
             * @param entity The entity to which the component belongs.
             * 
             * @param component The component to add.
             */
            void add(Entity entity, Component component) {
                _components[entity] = component;
            }

            /**
             * @brief Retrieves a component from the storage.
             * 
             * @param entity The entity whose component to retrieve.
             * 
             * @return A pointer to the component, or nullptr if not found.
             */
            Component* get(Entity entity) {
                auto it = _components.find(entity);
                if (it != _components.end()) {
                    return &it->second;
                }
                return nullptr;
            }

        private:
            std::unordered_map<Entity, Component> _components;
    };

    /**
     * @brief Manages components for entities.
     * 
     * This class is responsible for adding, retrieving, and managing components
     * associated with entities in the ECS (Entity-Component-System) architecture.
     */
    class ComponentManager
    {
        public:
            ComponentManager(EntityManager& entity_manager) 
            : _entity_manager(entity_manager)
            {}

            /**
             * @brief Adds a component to an entity.
             * 
             * @tparam Component The type of the component to add.
             * 
             * @param entity The entity to which the component belongs.
             * 
             * @param component The component to add.
             */
            template<typename Component>
            void addComponent(Entity entity, Component component) {
                getOrCreateStorage<Component>()->add(entity, component);

                // Update entity mask
                uint32_t type_ID = ComponentTypeManager::getTypeID<Component>();
                assert(type_ID < MAX_COMPONENT_TYPES);
                _entity_manager.addComponentBit(entity, type_ID);
            }

            /**
             * @brief Retrieves a component from an entity.
             * 
             * @tparam Component The type of the component to retrieve.
             * 
             * @param entity The entity whose component to retrieve.
             * 
             * @return A pointer to the component, or nullptr if not found.
             */
            template<typename Component>
            Component* getComponent(Entity entity) {
                auto storage = getStorage<Component>();
                if (storage) {
                    return storage->get(entity);
                }
                return nullptr;
            }

        private:
            /**
             * @brief Retrieves the storage for a specific component type.
             * 
             * @tparam Component The type of the component.
             * 
             * @return A pointer to the storage, or nullptr if not found.
             */
            template<typename Component>
            ComponentStorage<Component>* getStorage() {
                auto it = _storages.find(std::type_index(typeid(Component)));
                if (it != _storages.end()) {
                    return static_cast<ComponentStorage<Component>*>(it->second.get());
                }
                return nullptr;
            }

            /**
             * @brief Retrieves or creates the storage for a specific component type.
             * 
             * @tparam Component The type of the component.
             * 
             * @return A pointer to the storage.
             */
            template<typename Component>
            ComponentStorage<Component>* getOrCreateStorage() {
                auto storage = getStorage<Component>();
                if (!storage) {
                    _storages[std::type_index(typeid(Component))] = std::make_unique<ComponentStorage<Component>>();
                    storage = static_cast<ComponentStorage<Component>*>(_storages[std::type_index(typeid(Component))].get());
                }
                return storage;
            }

            std::unordered_map<std::type_index, std::unique_ptr<IComponentStorage>> _storages;
            EntityManager& _entity_manager;
    };
}
