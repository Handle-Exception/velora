#pragma once

#include <thread>
#include <optional>

#include "native.hpp"
#include "resolution.hpp"
#include "events.hpp"

#include <asio.hpp>
#include <spdlog/spdlog.h>
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <GL/glew.h>
#include <GL/wglew.h>

namespace velora::winapi
{
    template<class Extra, class ProcedureFunc>
    inline WNDCLASSEX defaultWindowClass(ProcedureFunc && proc, const std::string & class_name)
    {
        WNDCLASSEX wcex = {0};
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.cbClsExtra = 0;
        wcex.hInstance = GetModuleHandle(nullptr);
        wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
        wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
        wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
        wcex.lpfnWndProc = proc;
        wcex.cbWndExtra  = sizeof(Extra);
        wcex.lpszClassName = class_name.c_str();
        wcex.lpszMenuName = NULL;
        wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        return wcex;
    }

    class WinapiProcess
    {
        public:
            WinapiProcess();
            WinapiProcess(const WinapiProcess &) = delete;
            WinapiProcess(WinapiProcess &&);
            WinapiProcess & operator=(const WinapiProcess &) = delete;
            WinapiProcess & operator=(WinapiProcess &&);
            ~WinapiProcess();
            //
            static LRESULT CALLBACK procedure(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
            //
            LRESULT CALLBACK specyficProcedure(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
        
            asio::awaitable<void> close();

            asio::awaitable<native::window_handle> registerWindow(std::string name, Resolution resolution);

            asio::awaitable<bool> unregisterWindow(native::window_handle window);

            asio::awaitable<bool> setWindowCallbacks(native::window_handle window, WindowCallbacks && callbacks);

            asio::awaitable<native::opengl_context_handle> registerOGLContext(native::window_handle window_handle, unsigned int major_version, unsigned int minor_version);

            asio::awaitable<bool> unregisterOGLContext(native::opengl_context_handle oglctx);

            void join();

        protected:
            void messageLoop();
            bool registerClass(const WNDCLASSEX & class_structure);
            bool unregisterClass(const std::string & class_name);

            bool initOpenGL();

        private:
            asio::io_context _io_context;
            asio::strand<asio::io_context::executor_type> _strand;

            absl::flat_hash_map<std::string, const WNDCLASSEX> _registered_classes;
            absl::flat_hash_map<native::window_handle, std::optional<WindowCallbacks>> _window_handles;

            native::opengl_context_handle _default_oglctx_handle;
            absl::flat_hash_set<native::opengl_context_handle> _oglctx_handles;

            // IO thread initialized at the end of the constructor
            std::thread _io_thread;

    };
}

template <>
struct std::formatter<const GLubyte *> : std::formatter<std::string> {
  auto format(const GLubyte * ogl_str, format_context& ctx) const {
    return formatter<string>::format(
      std::format("{}", (std::stringstream{} << ogl_str).str()), ctx);
  }
};