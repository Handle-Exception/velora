#pragma once

#include <vector>
#include <optional>

#include <spdlog/spdlog.h>
#include <absl/container/flat_hash_map.h>

#include "ecs.hpp"
#include "render.hpp"

namespace velora::game
{
    class Level 
    {
        public:
            Level(IRenderer &  renderer);

            std::optional<Entity> spawnEntity(std::string name)
            {
                if(_names_to_entities.contains(name))
                {
                    spdlog::error("Entity with name {} already exists in the level", name); 
                    return std::nullopt;
                }

                _names_to_entities.emplace(name, _entities.createEntity());
                _entities_to_names.emplace(_names_to_entities.at(name), name);

                return _names_to_entities.at(name);
            }

            template<class ComponentType>
            auto addComponent(Entity entity, ComponentType&& component)
            {
                spdlog::debug("Adding component {} to entity {}", typeid(ComponentType).name(), entity);
                return _components.addComponent<ComponentType>(entity, std::move(component));
            }

            template<class ComponentType, class ... ComponentArgs>
            auto addComponent(Entity entity, ComponentArgs&&... args)
            {
                return _components.addComponent<ComponentType>(entity, ComponentType(std::forward<ComponentArgs>(args)...));
            }

            template<typename ComponentType>
            ComponentType * getComponent(Entity entity) 
            { 
                return _components.getComponent<ComponentType>(entity);
            }

            template<typename ComponentType>
            const ComponentType * getComponent(Entity entity) const
            { 
                return _components.getComponent<ComponentType>(entity);
            }

            template<class ComponentType>
            bool hasComponent(Entity entity) const
            {
                return _entities.getComponentMask(entity).test(ComponentTypeManager::getTypeID<ComponentType>());
            }

            const EntityManager& getEntityManager() const { return _entities; }
            EntityManager & getEntityManager() { return _entities; }

            const ComponentManager& getComponentManager() const { return _components; }
            ComponentManager & getComponentManager() { return _components; }

            asio::awaitable<void> runSystem(ISystem & system) {co_return co_await system.run(_components, _entities); }
            
            template<class SystemType, class ... Args>
            asio::awaitable<void> runSystem(SystemType & system, Args&& ... args) {
                co_return co_await system.run(_components, _entities, std::forward<Args>(args)...); }

            std::optional<std::string> getName(Entity entity) const 
            {
                if(_entities_to_names.contains(entity))
                {
                    return _entities_to_names.at(entity);
                }
                else
                {
                    spdlog::error("Entity {} does not exist in the level", entity); 
                    return std::nullopt;
                }
            }

            std::optional<Entity> getEntity(std::string name) const 
            {
                if(_names_to_entities.contains(name))
                {
                    return _names_to_entities.at(name);
                }
                else
                {
                    spdlog::error("Entity with name {} does not exist in the level", name); 
                    return std::nullopt;
                }
            }

        private:
            IRenderer & _renderer;

            EntityManager _entities;
            ComponentManager _components;

            absl::flat_hash_map<std::string, Entity> _names_to_entities;
            absl::flat_hash_map<Entity, std::string> _entities_to_names;
    };
}