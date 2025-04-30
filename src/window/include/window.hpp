#pragma once

#include "native.hpp"
#include "type.hpp"
#include "resolution.hpp"
#include "process.hpp"

namespace velora
{
    class IWindow : public type::Interface
    {
    public:
        // dtor
        virtual ~IWindow() = default;
        //
        virtual bool good() const = 0;
        //
        virtual asio::awaitable<void> show() = 0;
        //
        virtual asio::awaitable<void> hide() = 0;
        //
        virtual asio::awaitable<void> close() = 0;
        //
        virtual asio::awaitable<void> destroy() = 0;
        // 
        virtual void present() = 0;
        //
        virtual const Resolution & getResolution() const = 0;
        //
        virtual native::window_handle getHandle() const = 0;
        //
        virtual native::device_context getDeviceContext() const = 0;

        virtual IProcess & getProcess() = 0;
    };

    template<class WindowImplType>
    class WindowDispatcher final : public type::Dispatcher<IWindow, WindowImplType>
    {
        public:
            using dispatch = type::Dispatcher<IWindow, WindowImplType>;

            inline WindowDispatcher(WindowImplType && obj) : dispatch(std::move(obj)){}
            template<class... Args>                                                                             
            inline WindowDispatcher(Args && ... args) : dispatch(std::forward<Args>(args)...){} 
            inline ~WindowDispatcher() = default;

            constexpr inline bool good() const override { return dispatch::getImpl().good();}
            inline asio::awaitable<void> show() override {co_return co_await dispatch::getImpl().show();}
            inline asio::awaitable<void> hide() override {co_return co_await dispatch::getImpl().hide();}
            inline asio::awaitable<void> close() override {co_return co_await dispatch::getImpl().close();}
            inline asio::awaitable<void> destroy() override {co_return co_await dispatch::getImpl().destroy();}
            constexpr inline void present() override {return dispatch::getImpl().present();}
            constexpr inline native::window_handle getHandle() const override { return dispatch::getImpl().getHandle();}
            constexpr inline native::device_context getDeviceContext() const override { return dispatch::getImpl().getDeviceContext();};
            constexpr inline const Resolution & getResolution() const override {return dispatch::getImpl().getResolution();}
            constexpr inline IProcess & getProcess() override {return dispatch::getImpl().getProcess();}

    };

    template<class WindowImplType>
    WindowDispatcher(WindowImplType && ) -> WindowDispatcher<WindowImplType>;

    class Window : public type::Implementation<IWindow, WindowDispatcher> 
    {                                                                   
        public:
            /* move ctor */
            Window(Window && other) = default;
            Window & operator=(Window && other) = default;

            /* dtor */
            virtual ~Window() {}
        
            /* ctor */
            template<class WindowImplType>
            Window(WindowImplType && impl)
            : Implementation(std::move(impl))
            {}                                                          
    };
}