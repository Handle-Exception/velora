#pragma once

#include "type.hpp"
#include "component_manager.hpp"
#include "entity_manager.hpp"

#include <ranges>
#include <vector>
#include <string>
#include <queue>
#include <unordered_map>

namespace velora
{

    class ISystem
    {
        public:
            virtual ~ISystem() = default;

            virtual const char * getName() const = 0;
            virtual const std::initializer_list<const char *> & getDependencies() const = 0;

            virtual asio::awaitable<void> run(ComponentManager& components, EntityManager& entities) = 0;
    };

    template<class SystemImplType>
    class SystemDispatcher final : public type::Dispatcher<ISystem, SystemImplType>
    {
        public:
            using dispatch = type::Dispatcher<ISystem, SystemImplType>;

            inline SystemDispatcher(SystemImplType && obj) : dispatch(std::move(obj)){}
            template<class... Args>                                                                             
            inline SystemDispatcher(Args && ... args) : dispatch(std::forward<Args>(args)...){} 
            inline ~SystemDispatcher() = default;

            constexpr inline const char * getName() const override {
                return dispatch::getImpl().getName();}

            constexpr inline const std::initializer_list<const char *> & getDependencies() const override { 
                return dispatch::getImpl().getDependencies();}
                
            inline asio::awaitable<void> run(ComponentManager& components, EntityManager& entities) override { 
                co_return co_await dispatch::getImpl().run(components, entities);}
    };

    template<class SystemImplType>
    SystemDispatcher(SystemImplType && ) -> SystemDispatcher<SystemImplType>;

    class System : public type::Implementation<ISystem, SystemDispatcher> 
    {                                                                   
        public:
            /* move ctor */
            System(System && other) = default;
            System & operator=(System && other) = default;

            /* dtor */
            virtual ~System() = default;
        
            /* ctor */
            template<class SystemImplType>
            System(SystemImplType && impl)
            : Implementation(std::move(impl))
            {}                                                          
    };


    /**
     * @brief Performs a layered topological sort 
     * where each layer contains systems whose dependencies were satisfied in previous layers. 
     * This lets us safely execute each layer in parallel
     */
    std::vector<std::vector<ISystem*>> topologicalSortLayers(const std::vector<System> & systems);
}