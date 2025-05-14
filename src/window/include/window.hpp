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
        virtual ~IWindow() = default;

        /**
         * @brief Check if the window is still valid.
         * @return If the window is still valid and usable, @c true. Otherwise, @c false.
         */
        virtual bool good() const = 0;

        /**
         * @brief Asynchronously shows the window.
         * 
         * If the current thread is not the one associated with the strand, 
         * the operation is dispatched to the correct strand to maintain thread safety.
         */
        virtual asio::awaitable<void> show() = 0;
        
        /**
         * @brief Asynchronously hides the window
         * 
         * When the returned awaitable is awaited, the window is hidden.
         * The underlying window handle is not modified.
         * 
         * This function is thread-safe and can be called from any thread
         * since it always dispatches the work to the window's strand.
         */
        virtual asio::awaitable<void> hide() = 0;

        /**
         * Closes the window by unregistering it from the associated process.
         *
         * This function ensures that the window is closed on the correct strand. 
         * If the window handle is valid, it will unregister the window with the 
         * associated process and set the window handle to nullptr.
         *
         * @return An awaitable that resolves when the window has been successfully 
         *         unregistered and closed.
         */
        virtual asio::awaitable<void> close() = 0;

        /**
         * Returns the current resolution of the window.
         *
         * @return A Resolution object representing the current width and height of the window.
         */
        virtual const Resolution & getResolution() const = 0;

        /**
         * @brief Retrieves the native window handle.
         * 
         * @return The handle to the native window associated with this window instance.
         */
        virtual native::window_handle getHandle() const = 0;

        /**
         * Acquires a device context for the current window.
         *
         * This function retrieves a device context (DC) handle for the window
         * associated with this instance. It logs the acquisition
         * of the device context and returns the acquired handle.
         *
         * @return The acquired device context handle.
         */
        virtual native::device_context acquireDeviceContext() = 0;

        /**
         * Releases the specified device context from the window.
         *
         * @param device_context The device context to release.
         * @return True if the device context was successfully released; false otherwise.
         *
         * The function logs an error and returns false if the provided device context is null,
         * if the device context was not previously acquired, or if it does not match the
         * currently acquired device context. It also logs an error if releasing the device context fails.
         * Upon successful release, the acquired device context is set to nullopt and a success message is logged.
         */
        virtual bool releaseDeviceContext(native::device_context device_context) = 0;

        /**
         * Returns a reference to the process associated with this window.
         *
         * @return A reference to the IProcess object representing the process that owns this window.
         */
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
            constexpr inline native::window_handle getHandle() const override { return dispatch::getImpl().getHandle();}

            constexpr inline native::device_context acquireDeviceContext() override { 
                return dispatch::getImpl().acquireDeviceContext();};
            constexpr inline bool releaseDeviceContext(native::device_context device_context) override { 
                return dispatch::getImpl().releaseDeviceContext(std::move(device_context));};

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