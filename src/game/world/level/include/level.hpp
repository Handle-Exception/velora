#pragma once

#include <vector>
#include <optional>

#include <spdlog/spdlog.h>
#include <absl/container/flat_hash_map.h>

#include "ecs.hpp"

namespace velora::game
{
    class Level 
    {
        public:
            Level();
            Level(Level &&);
            Level& operator=(Level &&);
            Level(const Level &) = delete;
            Level& operator=(const Level &) = delete;
            ~Level() = default;


            template<class ComponentType>
            auto addComponent(Entity entity, ComponentType&& component)
            {
                spdlog::debug("Adding component {} to entity {}", typeid(ComponentType).name(), entity);
                return _components.addComponent<ComponentType>(entity, std::move(component));
            }

            template<class ComponentType, class ... ComponentArgs>
            auto addComponent(Entity entity, ComponentArgs&&... args)
            {
                spdlog::debug("Adding component {} to entity {}", typeid(ComponentType).name(), entity);
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

            template<class SystemType, class ... Args>
            asio::awaitable<void> runSystem(SystemType & system, Args&& ... args)
            {
                co_return co_await system.run(_components, _entities, std::forward<Args>(args)...);
            }

            std::optional<Entity> spawnEntity(std::string name);

            const EntityManager& getEntityManager() const;

            EntityManager & getEntityManager();

            const ComponentManager& getComponentManager() const;

            ComponentManager & getComponentManager();

            asio::awaitable<void> runSystem(ISystem & system);
            
            std::optional<std::string> getName(Entity entity) const;

            std::optional<Entity> getEntity(std::string name) const;

        private:
            EntityManager _entities;
            ComponentManager _components;

            absl::flat_hash_map<std::string, Entity> _names_to_entities;
            absl::flat_hash_map<Entity, std::string> _entities_to_names;
    };
}