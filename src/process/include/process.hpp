#pragma once

#include "native.hpp"
#include <asio.hpp>

#include "type.hpp"
#include "resolution.hpp"

#include "process_window_callbacks.hpp"

namespace velora
{

    class IProcess : public type::Interface
    {
    public:
        // dtor
        virtual ~IProcess() = default;
        
        //
        virtual void join() = 0;
        
        virtual asio::awaitable<void> close() = 0;
        
        virtual asio::awaitable<native::window_handle> registerWindow(std::string name, Resolution resolution) = 0;
            
        virtual asio::awaitable<bool> unregisterWindow(native::window_handle window) = 0;

        virtual asio::awaitable<bool> setWindowCallbacks(native::window_handle window, WindowCallbacks && callbacks) = 0;

        virtual asio::awaitable<native::opengl_context_handle> registerOGLContext(native::window_handle window_handle, unsigned int major_version, unsigned int minor_version) = 0;

        virtual asio::awaitable<bool> unregisterOGLContext(native::opengl_context_handle oglctx) = 0;
    };

    template<class ProcessImplType>
    class ProcessDispatcher final : public type::Dispatcher<IProcess, ProcessImplType>
    {
        public:
            using dispatch = type::Dispatcher<IProcess, ProcessImplType>;

            inline ProcessDispatcher(ProcessImplType && obj) : dispatch(std::move(obj)){}
            template<class... Args>                                                                             
            inline ProcessDispatcher(Args && ... args) : dispatch(std::forward<Args>(args)...){} 
            inline ~ProcessDispatcher() = default;

            constexpr inline void join() override { return dispatch::getImpl().join();}
            
            inline asio::awaitable<void> close() override { co_return co_await dispatch::getImpl().close();}
            
            inline asio::awaitable<native::window_handle> registerWindow(std::string name, Resolution resolution) override { 
                    co_return co_await dispatch::getImpl().registerWindow(std::move(name), std::move(resolution));}
            
            inline asio::awaitable<bool> unregisterWindow(native::window_handle window) override {
                    co_return co_await dispatch::getImpl().unregisterWindow(std::move(window));}
            
            inline asio::awaitable<bool> setWindowCallbacks(native::window_handle window, WindowCallbacks && callbacks) override {
                    co_return co_await dispatch::getImpl().setWindowCallbacks(std::move(window), std::move(callbacks));}

            inline asio::awaitable<native::opengl_context_handle> registerOGLContext(native::window_handle window_handle, unsigned int major_version, unsigned int minor_version) override{
                co_return co_await dispatch::getImpl().registerOGLContext(std::move(window_handle), major_version, minor_version);}

            inline asio::awaitable<bool> unregisterOGLContext(native::opengl_context_handle oglctx) override{
                co_return co_await dispatch::getImpl().unregisterOGLContext(std::move(oglctx));}

    };

    template<class ProcessImplType>
    ProcessDispatcher(ProcessImplType && ) -> ProcessDispatcher<ProcessImplType>;

    class Process : public type::Implementation<IProcess, ProcessDispatcher> 
    {                                                                   
        public:
            /* move ctor */
            Process(Process && other) = default;
            Process & operator=(Process && other) = default;

            /* dtor */
            virtual ~Process() = default;
        
            /* ctor */
            template<class ProcessImplType>
            Process(ProcessImplType && impl)
            : Implementation(std::move(impl))
            {}                                                          
    };
}