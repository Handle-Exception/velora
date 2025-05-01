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
                if(_entities_names.contains(name))
                {
                    spdlog::error("Entity with name {} already exists in the level", name); 
                    return std::nullopt;
                }

                _entities_names.emplace(name, _entities.createEntity());

                return _entities_names.at(name);
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

            const EntityManager& getEntityManager() const { return _entities; }
            EntityManager & getEntityManager() { return _entities; }

            const ComponentManager& getComponentManager() const { return _components; }
            ComponentManager & getComponentManager() { return _components; }

            asio::awaitable<void> runSystem(ISystem & system) {co_return co_await system.run(_components, _entities); }
            
            template<class SystemType>
            asio::awaitable<void> runSystem(SystemType & system) {co_return co_await system.run(_components, _entities); }


        private:
            IRenderer & _renderer;

            EntityManager _entities;
            ComponentManager _components;

            absl::flat_hash_map<std::string, Entity> _entities_names;
    };
}