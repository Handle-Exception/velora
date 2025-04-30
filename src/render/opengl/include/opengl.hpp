#pragma once

#include "native.hpp"
#include "window.hpp"

#include <GL/glew.h>
#include <spdlog/spdlog.h>
#include <asio.hpp>

namespace velora::opengl
{
    bool logOpenGLState();

    class OpenGLRenderer
    {
        public:
            ~OpenGLRenderer();
            OpenGLRenderer(OpenGLRenderer && other);



            static asio::awaitable<OpenGLRenderer> asyncConstructor(asio::io_context & io_context, IWindow & window, int major_version, int minor_version)
            {
                auto window_handle = window.getHandle();
                auto & process = window.getProcess();

                native::opengl_context_handle oglctx_handle = co_await process.registerOGLContext(window_handle, major_version, minor_version);

                co_return OpenGLRenderer(io_context, window, oglctx_handle);
            }

            bool good() const
            {
                return _oglctx_handle != nullptr;
            }

            //
            void close()
            {

            }

        protected:
            OpenGLRenderer(asio::io_context & io_context, IWindow & window, native::opengl_context_handle oglctx_handle);


        private:
            asio::io_context & _io_context;
            asio::strand<asio::io_context::executor_type> _strand;
            native::opengl_context_handle _oglctx_handle;
            IWindow & _window;
    };
}