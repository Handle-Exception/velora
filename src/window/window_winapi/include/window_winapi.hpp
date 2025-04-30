#pragma once

#include "native.hpp"
#include "resolution.hpp"
#include "process.hpp"

#include <asio.hpp>

namespace velora::winapi
{
    class WinapiWindow
    {
        public:
            ~WinapiWindow();

            WinapiWindow(WinapiWindow && other);

            static asio::awaitable<WinapiWindow> asyncConstructor(asio::io_context & io_context, IProcess & process, std::string name, Resolution resolution)
            {
                auto window_handle = co_await process.registerWindow(std::move(name), std::move(resolution));
                co_return WinapiWindow(io_context, process, window_handle);
            }

        bool good() const{
            return _window_handle != nullptr;
        }
        //
        void show()
        {

        }
        //
        void hide()
        {

        }
        //
        void close()
        {

        }
        // 
        void present()
        {

        }
        //
        const Resolution & getResolution() const
        {
            static Resolution res(0, 0);
            return res;
        }
        //
        native::window_handle getHandle() const
        {
            return _window_handle;
        }
        //
        native::device_context getDeviceContext() const
        {
            return native::device_context{};
        }

        IProcess & getProcess()
        {
            return _process;
        }
        
        protected:

            WinapiWindow(asio::io_context & io_context, IProcess & process, native::window_handle window_handle);


        private:
            asio::io_context & _io_context;
            asio::strand<asio::io_context::executor_type> _strand;
            native::window_handle _window_handle;
            IProcess & _process;
        
    };
}