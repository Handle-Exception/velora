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

            static asio::awaitable<WinapiWindow> asyncConstructor(asio::io_context & io_context,
                IProcess & process,
                std::string name,
                Resolution resolution
                )
            {
                auto window_handle = co_await process.registerWindow(std::move(name), std::move(resolution));
                co_return WinapiWindow(io_context, process, window_handle);
            }

            bool good() const;
            //
            asio::awaitable<void> show();
            //
            asio::awaitable<void> hide();
            //
            asio::awaitable<void> close();
            
            // 
            asio::awaitable<void> present();
        //
        const Resolution & getResolution() const
        {
            static Resolution res(0, 0);
            return res;
        }
        //
        native::window_handle getHandle() const;
        
        //
        native::device_context acquireDeviceContext();

        bool releaseDeviceContext(native::device_context device_context);

        IProcess & getProcess();
        
        protected:

            WinapiWindow(asio::io_context & io_context, IProcess & process, native::window_handle window_handle);


        private:
            asio::io_context & _io_context;
            asio::strand<asio::io_context::executor_type> _strand;
            native::window_handle _window_handle;
            IProcess & _process;
        
    };
}