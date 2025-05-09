#include "level.hpp"

namespace velora::game
{
    Level::Level()
    :   _entities(),
        _components(_entities)
    {

    }

    Level::Level(Level && other)
    : _entities(std::move(other._entities)),
      _components(_entities, std::move(other._components)),
      _names_to_entities(std::move(other._names_to_entities)),
      _entities_to_names(std::move(other._entities_to_names))
    {

    }

    Level& Level::operator=(Level && other)
    {
        if(this != &other)
        {
            _entities = std::move(other._entities);
            _components = std::move(other._components);
            _names_to_entities = std::move(other._names_to_entities);
            _entities_to_names = std::move(other._entities_to_names);
        }
        return *this;
    }


    std::optional<Entity> Level::spawnEntity(std::string name)
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

    const EntityManager& Level::getEntityManager() const 
    { 
        return _entities;
    }
    
    EntityManager & Level::getEntityManager() 
    { 
        return _entities;
    }

    const ComponentManager& Level::getComponentManager() const 
    {
        return _components;
    }
    
    ComponentManager & Level::getComponentManager() 
    {
        return _components;
    }

    asio::awaitable<void> Level::runSystem(ISystem & system) 
    {
        co_return co_await system.run(_components, _entities);
    }

    std::optional<std::string> Level::getName(Entity entity) const 
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

    std::optional<Entity> Level::getEntity(std::string name) const 
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
}