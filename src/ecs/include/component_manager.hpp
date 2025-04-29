#pragma once

#include <unordered_map>
#include <typeindex>
#include <memory>

#include "entity.hpp"
#include "entity_manager.hpp"
#include "component_type.hpp"

namespace velora
{
    class IComponentStorage {
    public:
        virtual ~IComponentStorage() = default;
    };

    template<typename Component>
    class ComponentStorage : public IComponentStorage {
    public:
        void add(Entity entity, Component component) {
            _components[entity] = component;
        }

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

    class ComponentManager {
    public:
        ComponentManager(EntityManager& entity_manager) 
        : _entity_manager(entity_manager)
        {}

        template<typename Component>
        void addComponent(Entity entity, Component component) {
            getOrCreateStorage<Component>()->add(entity, component);

            // Update entity mask
            uint32_t typeID = ComponentTypeManager::getTypeID<Component>();
            _entity_manager.addComponentBit(entity, typeID);
        }

        template<typename Component>
        Component* getComponent(Entity entity) {
            auto storage = getStorage<Component>();
            if (storage) {
                return storage->get(entity);
            }
            return nullptr;
        }

    private:
        template<typename Component>
        ComponentStorage<Component>* getStorage() {
            auto it = _storages.find(std::type_index(typeid(Component)));
            if (it != _storages.end()) {
                return static_cast<ComponentStorage<Component>*>(it->second.get());
            }
            return nullptr;
        }

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
