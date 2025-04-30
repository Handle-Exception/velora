#pragma once

#include "native.hpp"
#include "type.hpp"
#include "resolution.hpp"
#include "process.hpp"

#ifdef WIN32
#include "window_winapi.hpp"
#endif

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
        virtual void show() = 0;
        //
        virtual void hide() = 0;
        //
        virtual void close() = 0;
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
            constexpr inline void show() override {return dispatch::getImpl().show();}
            constexpr inline void hide() override {return dispatch::getImpl().hide();}
            constexpr inline void close() override {return dispatch::getImpl().close();}
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