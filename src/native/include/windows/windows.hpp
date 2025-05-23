#pragma once

#ifndef _WIN32_WINNT
#define _WIN32_WINNT	0xa00
#endif
#ifdef UNICODE
  #undef UNICODE
#endif

#define NOMINMAX

#include <winsock2.h>
#include <windows.h>
#include <windowsx.h>

#include <format>
#include <spdlog/spdlog.h>

namespace velora::native
{
    using opengl_context_handle = HGLRC;
    using device_context = HDC;
    using console_handle = HANDLE;
    using window_handle = HWND;
    using process_handle = HANDLE;
}

template <>
struct std::formatter<velora::native::window_handle> : std::formatter<std::string> {
  auto format(velora::native::window_handle handle, format_context& ctx) const {
    return formatter<string>::format(
      std::format("window_handle[{}]", (std::stringstream{} << handle).str()), ctx);
  }
};

template <>
struct std::formatter<velora::native::opengl_context_handle> : std::formatter<std::string> {
  auto format(velora::native::opengl_context_handle handle, format_context& ctx) const {
    return formatter<string>::format(
      std::format("opengl_context[{}]", (std::stringstream{} << handle).str()), ctx);
  }
};

template <>
struct std::formatter<velora::native::device_context> : std::formatter<std::string> {
  auto format(velora::native::device_context dc, format_context& ctx) const {
    return formatter<string>::format(
      std::format("device_context[{}]", (std::stringstream{} << dc).str()), ctx);
  }
};