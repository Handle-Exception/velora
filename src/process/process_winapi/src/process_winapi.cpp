#include "process_winapi.hpp"

namespace velora::winapi
{
    PIXELFORMATDESCRIPTOR generateAdvancedPFD()
    {
	    PIXELFORMATDESCRIPTOR pfd = {0};
	    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	    pfd.nVersion = 1;
	    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	    pfd.cColorBits = 24;
	    pfd.cAlphaBits = 8;
	    pfd.cDepthBits = 24;
	    pfd.cStencilBits = 8;
	    pfd.iPixelType = PFD_TYPE_RGBA;
	    pfd.iLayerType = PFD_MAIN_PLANE;
	    return pfd;
    }

    WinapiProcess::WinapiProcess()
    :   _io_context(),
        _strand(asio::make_strand(_io_context)),
        _default_oglctx_handle(nullptr),
        _io_thread(&WinapiProcess::messageLoop, this)
    {
        spdlog::debug(std::format("[winapi] [t:{}] WinapiProcess constructor called", std::this_thread::get_id()));
    }

    WinapiProcess::~WinapiProcess()
    {
        spdlog::debug(std::format("[winapi] [t:{}] WinapiProcess destructor called", std::this_thread::get_id()));

        join();

        assert(_window_handles.empty() == true);
        assert(_registered_classes.empty() == true);
        assert(_oglctx_handles.empty() == true);
        assert(_default_oglctx_handle == nullptr);
    }
    
    void WinapiProcess::join()
    {
        if(_io_context.stopped() == false){
            asio::co_spawn(_io_context, close(), asio::detached);
        }

        if(_io_thread.joinable()){
            _io_thread.join();
        }
    }

    asio::awaitable<void> WinapiProcess::close()
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));

        spdlog::debug(std::format("[winapi] [t:{}] WinapiProcess clearing data", std::this_thread::get_id()));

        while(_oglctx_handles.empty() == false){
            co_await unregisterOGLContext(*(_oglctx_handles.begin()));
        }
        _default_oglctx_handle = nullptr;

        for(const auto & [window_handle, callbacks] : _window_handles)
        {
            PostMessage(window_handle, WM_CLOSE, 0, 0);
        }

        while(_registered_classes.empty() == false){
            unregisterClass(_registered_classes.begin()->first);
        }

        spdlog::debug(std::format("[winapi] [t:{}] WinapiProcess stopping and closing", std::this_thread::get_id()));

        PostQuitMessage(0);
    }

    bool WinapiProcess::registerClass( const WNDCLASSEX & class_structure)
    {
        std::string name = (std::stringstream{} << class_structure.lpszClassName).str();
        if(name.empty()){
            spdlog::error(std::format("[winapi-class-manager] Name of a class cannot be empty"));
            return false;
        }

        if(_registered_classes.contains(name)){
            spdlog::error(std::format("[winapi-class-manager] class {} already registered", name));
            return false;
        }

        spdlog::debug(std::format("[winapi-class-manager] registering WinApi class {}", name));

        _registered_classes.try_emplace(name, std::move(class_structure));

        if(RegisterClassEx(&_registered_classes.at(name)) == false){
            spdlog::error("winapi-class-manager] RegisterClassEx failed");
            return false;
        }
        
        return true;
    }

    bool WinapiProcess::unregisterClass(const std::string & class_name)
    {
        std::string name = (std::stringstream{} << class_name).str();
        if(name.empty()){
            spdlog::error(std::format("[winapi-class-manager] Name of a class cannot be empty"));
            return false;
        }

        if(_registered_classes.contains(name) == false){
            spdlog::error(std::format("[winapi-class-manager] class {} not registered", name));
            return false;
        }

        spdlog::debug(std::format("[winapi-class-manager] unregistering WinApi class {}", name));

        UnregisterClass(name.c_str(), GetModuleHandle(nullptr));
        _registered_classes.erase(name);

        return true;
    }

    asio::awaitable<native::window_handle> WinapiProcess::registerWindow(std::string name, Resolution resolution)
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));

        spdlog::debug(std::format("[winapi] [t:{}] registerWindow", std::this_thread::get_id()));

        const std::string class_name = "WINAPI:window";

        if(_registered_classes.contains(class_name) == false){
            spdlog::debug(std::format("[winapi] Window class not registered. Trying to register now ..."));

            const bool registration_result = registerClass(
                    defaultWindowClass<WinapiProcess *>(WinapiProcess::procedure, class_name)
            );

            if(registration_result == false){
                spdlog::error(std::format("[winapi] Cannot register Window class"));
                
                co_return nullptr;
            }
        }

        const auto class_structure = _registered_classes.at(class_name);

        spdlog::debug(std::format("[winapi] Process DPI setting awareness to DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2"));
        SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

        native::window_handle window_handle = CreateWindowExA(
            0,                                  // dwExStyle
            class_name.c_str(),                  // lpClassName
            name.c_str(),                 // lpWindowName
            WS_OVERLAPPEDWINDOW,                // dwStyle
            CW_USEDEFAULT,                      // X
            CW_USEDEFAULT,                      // Y
            (int)resolution.getWidth(),              // nWidth
            (int)resolution.getHeight(),             // nHeight
            NULL,                               //hWndParent
            NULL,                               // hMenu
            class_structure.hInstance,   // hInstance
            nullptr                             // lpParam
        );

        if(window_handle != nullptr){
            spdlog::debug(std::format("[winapi] constructing window [{}] ", window_handle));
        }else{
            spdlog::error(std::format("[winapi] constructing window failed"));
            
            co_return nullptr;
        }

        // store pointer to instance of process in winapi class
        SetWindowLongPtrW(window_handle, 0, reinterpret_cast<LONG_PTR>(this));

        native::device_context device_context = GetDC(window_handle);
        if(device_context == nullptr){
            spdlog::error(std::format("[winapi] cannot retrive device context for {}", window_handle));
            DestroyWindow(window_handle);
            co_return nullptr;
        }

        PIXELFORMATDESCRIPTOR pfd = generateAdvancedPFD();
        const int pixel_format_ID = ChoosePixelFormat(device_context, &pfd);
        spdlog::debug(std::format("[winapi] choosen pixel format ID {} ", pixel_format_ID));

        if(pixel_format_ID <= 0 || SetPixelFormat(device_context, pixel_format_ID, &pfd) == false) {
            spdlog::error(std::format( "[winapi] cannot set pixel format for {}", window_handle));
            ReleaseDC(window_handle, device_context);
            DestroyWindow(window_handle);
            co_return nullptr;
        }

        ReleaseDC(window_handle, device_context);

        _window_handles.try_emplace(window_handle, std::nullopt);
        co_return window_handle;
    }

    asio::awaitable<bool> WinapiProcess::unregisterWindow(native::window_handle window)
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));

        spdlog::debug(std::format("[winapi] [t:{}] unregisterWindow", std::this_thread::get_id()));

        if(_window_handles.contains(window) == false){
            spdlog::error(std::format("[winapi-class-manager] window {} not registered", window));
            co_return false;
        }

        spdlog::debug(std::format("[winapi-class-manager] unregistering window {}", window));

        ReleaseDC(window, GetDC(window));
        DestroyWindow(window);
        
        _window_handles.erase(window);
        co_return true;
    }

    asio::awaitable<bool> WinapiProcess::setWindowCallbacks(native::window_handle window, WindowCallbacks && callbacks)
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));

        spdlog::debug(std::format("[winapi] [t:{}] setWindowCallbacks", std::this_thread::get_id()));

        if(_window_handles.contains(window) == false){
            spdlog::error(std::format("[winapi-class-manager] window {} not registered", window));
            co_return false;
        }

        _window_handles.at(window) = std::move(callbacks);
        co_return true;
    }

    bool WinapiProcess::initOpenGL()
    {
        if(_default_oglctx_handle != nullptr){
            spdlog::debug(std::format("[winapi] default context already created"));
            return true;
        }
        spdlog::debug(std::format("[winapi] creating default context"));

        const std::string class_name = "WINAPI:oglDummyContextWindow";
        if(_registered_classes.contains(class_name) == false){
            spdlog::debug(std::format("[winapi] Window class not registered. Trying to register now ..."));

            const bool registration_result = registerClass(
                    defaultWindowClass<WinapiProcess *>(WinapiProcess::procedure, class_name)
            );

            if(registration_result == false){
                spdlog::error(std::format("[winapi] Cannot register Window class"));
                return false;
            }
        }

        const auto & class_structure = _registered_classes.at(class_name);
        
        native::window_handle dummy_window = CreateWindowExA(
            0,                                  // dwExStyle
            class_name.c_str(),                  // lpClassName
            "oglDummyContextWindow",            // lpWindowName
            WS_OVERLAPPEDWINDOW,                // dwStyle
            CW_USEDEFAULT,                      // X
            CW_USEDEFAULT,                      // Y
            1,              // nWidth
            1,             // nHeight
            NULL,                               //hWndParent
            NULL,                               // hMenu
            class_structure.hInstance,   // hInstance
            nullptr                             // lpParam
        );

        HDC device_handle = GetDC(dummy_window);
        PIXELFORMATDESCRIPTOR pfd = generateAdvancedPFD();
        int pixel_format_ID = ChoosePixelFormat(device_handle, &pfd);

        if(pixel_format_ID <= 0 || SetPixelFormat(device_handle, pixel_format_ID, &pfd) == false) {
            spdlog::error(std::format("[winapi-procedure] Cannot set pixel format to device {}" , GetLastError()));
            spdlog::debug("[winapi-procedure] removing dummy window handle...");
            ReleaseDC(dummy_window, device_handle);
            DestroyWindow(dummy_window);
            return false;
        }

        _default_oglctx_handle = wglCreateContext(device_handle);

        if(_default_oglctx_handle == nullptr){
            spdlog::error(std::format("[winapi-procedure] Cannot create default opengl context {}",  GetLastError()));
            ReleaseDC(dummy_window, device_handle);
            DestroyWindow(dummy_window);
            return false;
        }

        wglMakeCurrent(device_handle, _default_oglctx_handle);

        //init glew
        glewExperimental = true;
        if(glewInit() != GLEW_OK){
            spdlog::error("[winapi-procedure] Cannot initialize GLEW");
        }else{
            spdlog::info("[winapi-procedure] GLEW initialized properly");
        }
        
        spdlog::info(std::format("\nOpenGL\n  vendor: {}\n  renderer: {}\n  version: {}\n  shading language version: {}", 
            glGetString(GL_VENDOR), 
            glGetString(GL_RENDERER), 
            glGetString(GL_VERSION),
            glGetString(GL_SHADING_LANGUAGE_VERSION)));
        
        wglMakeCurrent(0, 0);
        ReleaseDC(dummy_window, device_handle);
        DestroyWindow(dummy_window);

        spdlog::info(std::format("[winapi-procedure] Default opengl {} created", _default_oglctx_handle));

        _oglctx_handles.emplace(_default_oglctx_handle);

        return true;
    }

    asio::awaitable<native::opengl_context_handle> WinapiProcess::registerOGLContext(native::window_handle window_handle, unsigned int major_version, unsigned int minor_version)
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));
        
        if(window_handle == nullptr){
            spdlog::error(std::format("[winapi-procedure] cannot construct OGLContext on empty window "));
            co_return nullptr;
        }

        const auto window_it = _window_handles.find(window_handle);
        if(window_it == _window_handles.end())
        {
            spdlog::error(std::format("[winapi-procedure] cannot find window {} for constructing OGLContext ", window_handle));
            co_return nullptr;
        }

        auto device_handle = GetDC(window_handle);
        if(device_handle == nullptr){
            spdlog::error(std::format("[winapi-procedure] Cannot obtain device handle for window {}, err: {}", window_handle,  GetLastError()));
            co_return nullptr;
        }

        if(_default_oglctx_handle == nullptr)
        {
            if(initOpenGL() == false || _default_oglctx_handle == nullptr)
            {
                spdlog::error(std::format("[winapi] cannot obtain default context, err: {}", GetLastError()));
                co_return nullptr;
            }
        }

        static const int CONTEXT_ATTRIBUTES[] =
        {
            WGL_CONTEXT_MAJOR_VERSION_ARB, (const int)major_version,
            WGL_CONTEXT_MINOR_VERSION_ARB, (const int)minor_version,
            WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
            0
        };

        native::opengl_context_handle oglctx  = wglCreateContextAttribsARB(device_handle, _default_oglctx_handle, CONTEXT_ATTRIBUTES);
        wglMakeCurrent(device_handle, oglctx);
        wglMakeCurrent(0, 0);
        ReleaseDC(window_handle, device_handle);

        if(oglctx == nullptr){
            spdlog::error(std::format("[winapi-procedure] Cannot create opengl context {}", GetLastError()));
            co_return nullptr;
        }

        spdlog::debug(std::format("[winapi-procedure] created opengl context {}", oglctx));
        _oglctx_handles.emplace(oglctx);
        
        co_return oglctx;
    }

    asio::awaitable<bool> WinapiProcess::unregisterOGLContext(native::opengl_context_handle oglctx)
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));
        spdlog::debug(std::format("[winapi] [t:{}] unregisterOGLContext", std::this_thread::get_id()));

        if(oglctx == nullptr)
        {
            spdlog::error(std::format("[winapi-procedure] cannot unregister OGLContext on empty context"));
            co_return false;
        }

        const auto oglctx_it = _oglctx_handles.find(oglctx);
        if(oglctx_it == _oglctx_handles.end())
        {
            spdlog::error(std::format("[winapi-procedure] cannot find OGLContext {} for unregistering", oglctx));
            co_return false;
        }

        if(wglDeleteContext(oglctx) == false)
        {
            spdlog::error(std::format("[winapi-procedure] cannot delete OGLContext {}, err: {}", oglctx, GetLastError()));
            co_return false;
        }

        _oglctx_handles.erase(oglctx);
        spdlog::debug(std::format("[winapi-procedure] deleted OGLContext {}", oglctx));

        co_return false;
    }

    LRESULT CALLBACK WinapiProcess::procedure(native::window_handle window, UINT message, WPARAM wparam, LPARAM lparam)
    {
        WinapiProcess * specyfic_process = reinterpret_cast<WinapiProcess *>(GetWindowLongPtrW(window, 0));
        // call specyfic procedure
        if ( specyfic_process != nullptr ) return specyfic_process->specyficProcedure(window, message, wparam, lparam);
        // call default procedure
        return DefWindowProc(window, message, wparam, lparam);
    }

    LRESULT CALLBACK WinapiProcess::specyficProcedure(native::window_handle window, UINT message, WPARAM wparam, LPARAM lparam)
    {
        if(window == nullptr)
        {
            spdlog::error(std::format("[winapi-procedure] window is null"));
            return DefWindowProc(window, message, wparam, lparam);
        }

        if(_window_handles.contains(window) == false)
        {
            spdlog::error(std::format("[winapi-procedure] window {} not registered", window));
            return DefWindowProc(window, message, wparam, lparam);
        }

        auto & window_callbacks_result = _window_handles.at(window);
        if(window_callbacks_result.has_value() == false)
        {
            return DefWindowProc(window, message, wparam, lparam);
        }
        auto & window_callbacks = window_callbacks_result.value();

        switch (message)
        {
        case WM_CREATE:
        {
            spdlog::debug(std::format("[winapi-procedure] WM_CREATE received for window {}", window));
            return DefWindowProc(window, message, wparam, lparam);
        }

        //WM_CLOSE is sent to the window when it is being closed - when its "X" button is clicked, 
        //or "Close" is chosen from the window's menu,
        // or Alt-F4 is pressed while the window has focus, etc.
        case WM_CLOSE:
        {
            spdlog::debug(std::format("[winapi-procedure] WM_CLOSE received, for window {}", window));
            if(window_callbacks.onDestroy != nullptr)
            {
                // spawn onDestroy callback on window_callbacks executor
                // it will handle the window destruction
                asio::co_spawn(window_callbacks.executor, window_callbacks.onDestroy(), asio::detached);
            }
            else
            {
                // if no callback is set, destroy window synchronously
                _window_handles.at(window) = std::nullopt;
                return DefWindowProc(window, message, wparam, lparam); 
            } 

            return 0;
        }

        case WM_PAINT :
        {
            return DefWindowProc(window, message, wparam, lparam);
        }

        case WM_SIZE:
        {
            spdlog::debug(std::format("[winapi-procedure] WM_SIZE received, for window {}", window));
            auto handling_result = DefWindowProc(window, message, wparam, lparam);
            if(window_callbacks.onResize != nullptr)
            {
                asio::co_spawn(window_callbacks.executor, window_callbacks.onResize((int)LOWORD(lparam), (int)HIWORD(lparam)), asio::detached);
            }
            return handling_result;
        }

        case WM_KEYDOWN: 
        {
            int key = (int)wparam;

            auto handling_result = DefWindowProc(window, message, wparam, lparam);
            if(window_callbacks.onKeyPress != nullptr)
            {
                asio::co_spawn(window_callbacks.executor, window_callbacks.onKeyPress(key), asio::detached);
            }
            return handling_result;
        }

        case WM_KEYUP : 
        {
            int key = (int)wparam;

            auto handling_result = DefWindowProc(window, message, wparam, lparam);
            if(window_callbacks.onKeyRelease != nullptr)
            {
                asio::co_spawn(window_callbacks.executor, window_callbacks.onKeyRelease(key), asio::detached);
            }
            return handling_result;
        }

        default:
            return DefWindowProc(window, message, wparam, lparam);
        }
    }

    void WinapiProcess::messageLoop() 
    { 
        spdlog::debug(std::format("[winapi-procedure] [t:{}] Message loop started", std::this_thread::get_id()));

        MSG msgcontainer;
        bool working = true;

        while(working)
        {     
            // perform implementation part of messages from the queue
            // first call to this function will create message queue for this thread in winapi
            if(PeekMessage(&msgcontainer, NULL, 0, 0, PM_REMOVE))
            {
                switch(msgcontainer.message)
                {
                    case WM_QUIT: 
                    {
                        spdlog::debug(std::format("[winapi-procedure] [t:{}]Input thread received WM_QUIT", std::this_thread::get_id()));

                        // handle rest of messages
                        while(PeekMessage(&msgcontainer, 0, 0, 0, PM_REMOVE) != 0)
                        {
                            TranslateMessage(&msgcontainer);
                            //will call SpecyficProcedure of give n WM_ message in current thread
                            DispatchMessage(&msgcontainer);
                            // handle other tasks
                            _io_context.poll();
                        }
                        // handle rest of tasks
                        _io_context.run();
                        // all msg handled
                        _io_context.stop();

                        working = false;
                        break;
                    }

                    default:
                    {
                        TranslateMessage(&msgcontainer);
                        //will call SpecyficProcedure of give n WM_ message in current thread
                        DispatchMessage(&msgcontainer);
                        break;
                    }
                }
            }

            // handle other tasks
            _io_context.poll();
        }

        spdlog::debug(std::format("[winapi-procedure] Input thread ended"));
    }
}