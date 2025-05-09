#include "input_system.hpp"

namespace velora::game
{
    const uint32_t InputSystem::MASK_POSITION_BIT = ComponentTypeManager::getTypeID<InputComponent>();
    
    //TODO
    game::InputCode keyToInputCode(int key)
    {
        #ifdef WIN32 //TODO: move to native.hpp?
            switch (key)
            { 
                // Letters
                case 0x41 : return InputCode::KEY_A;
                case 0x42 : return InputCode::KEY_B;
                case 0x43 : return InputCode::KEY_C;
                case 0x44 : return InputCode::KEY_D;
                case 0x45 : return InputCode::KEY_E;
                case 0x46 : return InputCode::KEY_F;
                case 0x47 : return InputCode::KEY_G;
                case 0x48 : return InputCode::KEY_H;
                case 0x49 : return InputCode::KEY_I;
                case 0x4A : return InputCode::KEY_J;
                case 0x4B : return InputCode::KEY_K;
                case 0x4C : return InputCode::KEY_L;
                case 0x4D : return InputCode::KEY_M;
                case 0x4E : return InputCode::KEY_N;
                case 0x4F : return InputCode::KEY_O;
                case 0x50 : return InputCode::KEY_P;
                case 0x51 : return InputCode::KEY_Q;
                case 0x52 : return InputCode::KEY_R;
                case 0x53 : return InputCode::KEY_S;
                case 0x54 : return InputCode::KEY_T;
                case 0x55 : return InputCode::KEY_U;
                case 0x56 : return InputCode::KEY_V;
                case 0x57 : return InputCode::KEY_W;
                case 0x58 : return InputCode::KEY_X;
                case 0x59 : return InputCode::KEY_Y;
                case 0x5A : return InputCode::KEY_Z;

                // Numbers (Top row)
                case 0x30 : return InputCode::KEY_0;
                case 0x31 : return InputCode::KEY_1;
                case 0x32 : return InputCode::KEY_2;
                case 0x33 : return InputCode::KEY_3;
                case 0x34 : return InputCode::KEY_4;
                case 0x35 : return InputCode::KEY_5;
                case 0x36 : return InputCode::KEY_6;
                case 0x37 : return InputCode::KEY_7;
                case 0x38 : return InputCode::KEY_8;
                case 0x39 : return InputCode::KEY_9;
                
                // Function keys
                case VK_F1 : return InputCode::KEY_F1;
                case VK_F2 : return InputCode::KEY_F2;
                case VK_F3 : return InputCode::KEY_F3;
                case VK_F4 : return InputCode::KEY_F4;
                case VK_F5 : return InputCode::KEY_F5;
                case VK_F6 : return InputCode::KEY_F6;
                case VK_F7 : return InputCode::KEY_F7;
                case VK_F8 : return InputCode::KEY_F8;
                case VK_F9 : return InputCode::KEY_F9;
                case VK_F10 : return InputCode::KEY_F10;
                case VK_F11 : return InputCode::KEY_F11;
                case VK_F12 : return InputCode::KEY_F12;
                
                // Arrows
                case VK_UP : return InputCode::KEY_UP;
                case VK_DOWN : return InputCode::KEY_DOWN;
                case VK_LEFT : return InputCode::KEY_LEFT;
                case VK_RIGHT : return InputCode::KEY_RIGHT;
    
                // Modifiers - with L/R distinction
                case VK_SHIFT : return InputCode::KEY_LSHIFT; // TODO
                //case 0x41 : return InputCode::KEY_RSHIFT; TODO
                case VK_CONTROL : return InputCode::KEY_LCTRL;
                //case 0x41 : return InputCode::KEY_RCTRL; TODO
                case VK_MENU : return InputCode::KEY_LALT; // TODO
                //case 0x41 : return InputCode::KEY_RALT; //TODO

                case VK_SPACE : return InputCode::KEY_SPACE;
                case VK_TAB : return InputCode::KEY_TAB;
                case VK_RETURN : return InputCode::KEY_ENTER;
                case VK_ESCAPE : return InputCode::KEY_ESCAPE;
                case VK_BACK : return InputCode::KEY_BACKSPACE;
                
                // Special
                case VK_CAPITAL : return InputCode::KEY_CAPSLOCK;
                case VK_INSERT : return InputCode::KEY_INSERT;
                case VK_DELETE : return InputCode::KEY_DELETE;
                case VK_HOME : return InputCode::KEY_HOME;
                case VK_END : return InputCode::KEY_END;
                case VK_PRIOR : return InputCode::KEY_PAGEUP;
                case VK_NEXT : return InputCode::KEY_PAGEDOWN;
                
                // Numpad
                case VK_NUMPAD0 : return InputCode::KEY_NUMPAD_0;
                case VK_NUMPAD1 : return InputCode::KEY_NUMPAD_1;
                case VK_NUMPAD2 : return InputCode::KEY_NUMPAD_2;
                case VK_NUMPAD3 : return InputCode::KEY_NUMPAD_3;
                case VK_NUMPAD4 : return InputCode::KEY_NUMPAD_4;
                case VK_NUMPAD5 : return InputCode::KEY_NUMPAD_5;
                case VK_NUMPAD6 : return InputCode::KEY_NUMPAD_6;
                case VK_NUMPAD7 : return InputCode::KEY_NUMPAD_7;
                case VK_NUMPAD8 : return InputCode::KEY_NUMPAD_8;
                case VK_NUMPAD9 : return InputCode::KEY_NUMPAD_9;
                case VK_ADD : return InputCode::KEY_NUMPAD_ADD;
                case VK_SUBTRACT : return InputCode::KEY_NUMPAD_SUB;
                case VK_MULTIPLY : return InputCode::KEY_NUMPAD_MUL;
                case VK_DIVIDE : return InputCode::KEY_NUMPAD_DIV;
                //case 0x41 : return InputCode::KEY_NUMPAD_ENTER; TODO
                
                // Mouse buttons
                case VK_LBUTTON : return InputCode::MOUSE_LEFT;
                case VK_RBUTTON : return InputCode::MOUSE_RIGHT;
                case VK_MBUTTON : return InputCode::MOUSE_MIDDLE;
                case VK_XBUTTON1 : return InputCode::MOUSE_X1;
                case VK_XBUTTON2 : return InputCode::MOUSE_X2;


                default:
                    return game::InputCode::UNKNOWN_InputCode;
            }
        #endif

        return game::InputCode::UNKNOWN_InputCode;
    }
    
    //TODO
    game::InputCode keyToInputCode(const std::string & key_name)
    {
        // Letters
        if(key_name == "KEY_A") return game::InputCode::KEY_A;
        if(key_name == "KEY_B") return game::InputCode::KEY_B;
        if(key_name == "KEY_C") return game::InputCode::KEY_C;
        if(key_name == "KEY_D") return game::InputCode::KEY_D;
        if(key_name == "KEY_E") return game::InputCode::KEY_E;
        if(key_name == "KEY_F") return game::InputCode::KEY_F;
        if(key_name == "KEY_G") return game::InputCode::KEY_G;
        if(key_name == "KEY_H") return game::InputCode::KEY_H;
        if(key_name == "KEY_I") return game::InputCode::KEY_I;
        if(key_name == "KEY_J") return game::InputCode::KEY_J;
        if(key_name == "KEY_K") return game::InputCode::KEY_K;
        if(key_name == "KEY_L") return game::InputCode::KEY_L;
        if(key_name == "KEY_M") return game::InputCode::KEY_M;
        if(key_name == "KEY_N") return game::InputCode::KEY_N;
        if(key_name == "KEY_O") return game::InputCode::KEY_O;
        if(key_name == "KEY_P") return game::InputCode::KEY_P;
        if(key_name == "KEY_Q") return game::InputCode::KEY_Q;
        if(key_name == "KEY_R") return game::InputCode::KEY_R;
        if(key_name == "KEY_S") return game::InputCode::KEY_S;
        if(key_name == "KEY_T") return game::InputCode::KEY_T;
        if(key_name == "KEY_U") return game::InputCode::KEY_U;
        if(key_name == "KEY_V") return game::InputCode::KEY_V;
        if(key_name == "KEY_W") return game::InputCode::KEY_W;
        if(key_name == "KEY_X") return game::InputCode::KEY_X;
        if(key_name == "KEY_Y") return game::InputCode::KEY_Y;
        if(key_name == "KEY_Z") return game::InputCode::KEY_Z;
        
        // Numbers (Top row)
        if(key_name == "KEY_0") return game::InputCode::KEY_0;
        if(key_name == "KEY_1") return game::InputCode::KEY_1;
        if(key_name == "KEY_2") return game::InputCode::KEY_2;
        if(key_name == "KEY_3") return game::InputCode::KEY_3;
        if(key_name == "KEY_4") return game::InputCode::KEY_4;
        if(key_name == "KEY_5") return game::InputCode::KEY_5;
        if(key_name == "KEY_6") return game::InputCode::KEY_6;
        if(key_name == "KEY_7") return game::InputCode::KEY_7;
        if(key_name == "KEY_8") return game::InputCode::KEY_8;
        if(key_name == "KEY_9") return game::InputCode::KEY_9;
        
        // Function keys
        if(key_name == "KEY_F1") return game::InputCode::KEY_F1;
        if(key_name == "KEY_F2") return game::InputCode::KEY_F2;
        if(key_name == "KEY_F3") return game::InputCode::KEY_F3;
        if(key_name == "KEY_F4") return game::InputCode::KEY_F4;
        if(key_name == "KEY_F5") return game::InputCode::KEY_F5;
        if(key_name == "KEY_F6") return game::InputCode::KEY_F6;
        if(key_name == "KEY_F7") return game::InputCode::KEY_F7;
        if(key_name == "KEY_F8") return game::InputCode::KEY_F8;
        if(key_name == "KEY_F9") return game::InputCode::KEY_F9;
        if(key_name == "KEY_F10") return game::InputCode::KEY_F10;
        if(key_name == "KEY_F11") return game::InputCode::KEY_F11;
        if(key_name == "KEY_F12") return game::InputCode::KEY_F12;
        
        // Arrows
        if(key_name == "KEY_UP") return game::InputCode::KEY_UP;
        if(key_name == "KEY_DOWN") return game::InputCode::KEY_DOWN;
        if(key_name == "KEY_LEFT") return game::InputCode::KEY_LEFT;
        if(key_name == "KEY_RIGHT") return game::InputCode::KEY_RIGHT;
        
        // Modifiers - with L/R distinction
        if(key_name == "KEY_LSHIFT") return game::InputCode::KEY_LSHIFT;
        if(key_name == "KEY_RSHIFT") return game::InputCode::KEY_RSHIFT;
        if(key_name == "KEY_LCTRL") return game::InputCode::KEY_LCTRL;
        if(key_name == "KEY_RCTRL") return game::InputCode::KEY_RCTRL;
        if(key_name == "KEY_LALT") return game::InputCode::KEY_LALT;
        if(key_name == "KEY_RALT") return game::InputCode::KEY_RALT;

        if(key_name == "KEY_SPACE") return game::InputCode::KEY_SPACE;
        if(key_name == "KEY_TAB") return game::InputCode::KEY_TAB;
        if(key_name == "KEY_ENTER") return game::InputCode::KEY_ENTER;
        if(key_name == "KEY_ESCAPE") return game::InputCode::KEY_ESCAPE;
        if(key_name == "KEY_BACKSPACE") return game::InputCode::KEY_BACKSPACE;
        
        // Special
        if(key_name == "KEY_CAPSLOCK") return game::InputCode::KEY_CAPSLOCK;
        if(key_name == "KEY_INSERT") return game::InputCode::KEY_INSERT;
        if(key_name == "KEY_DELETE") return game::InputCode::KEY_DELETE;
        if(key_name == "KEY_HOME") return game::InputCode::KEY_HOME;
        if(key_name == "KEY_END") return game::InputCode::KEY_END;
        if(key_name == "KEY_PAGEUP") return game::InputCode::KEY_PAGEUP;
        if(key_name == "KEY_PAGEDOWN") return game::InputCode::KEY_PAGEDOWN;
        
        // Numpad
        if(key_name == "KEY_NUMPAD_0") return game::InputCode::KEY_NUMPAD_0;
        if(key_name == "KEY_NUMPAD_1") return game::InputCode::KEY_NUMPAD_1;
        if(key_name == "KEY_NUMPAD_2") return game::InputCode::KEY_NUMPAD_2;
        if(key_name == "KEY_NUMPAD_3") return game::InputCode::KEY_NUMPAD_3;
        if(key_name == "KEY_NUMPAD_4") return game::InputCode::KEY_NUMPAD_4;
        if(key_name == "KEY_NUMPAD_5") return game::InputCode::KEY_NUMPAD_5;
        if(key_name == "KEY_NUMPAD_6") return game::InputCode::KEY_NUMPAD_6;
        if(key_name == "KEY_NUMPAD_7") return game::InputCode::KEY_NUMPAD_7;
        if(key_name == "KEY_NUMPAD_8") return game::InputCode::KEY_NUMPAD_8;
        if(key_name == "KEY_NUMPAD_9") return game::InputCode::KEY_NUMPAD_9;
        if(key_name == "KEY_NUMPAD_ADD") return game::InputCode::KEY_NUMPAD_ADD;
        if(key_name == "KEY_NUMPAD_SUB") return game::InputCode::KEY_NUMPAD_SUB;
        if(key_name == "KEY_NUMPAD_MUL") return game::InputCode::KEY_NUMPAD_MUL;
        if(key_name == "KEY_NUMPAD_DIV") return game::InputCode::KEY_NUMPAD_DIV;
        if(key_name == "KEY_NUMPAD_ENTER") return game::InputCode::KEY_NUMPAD_ENTER;
            
        // Mouse buttons
        if(key_name == "MOUSE_LEFT") return game::InputCode::MOUSE_LEFT;
        if(key_name == "MOUSE_RIGHT") return game::InputCode::MOUSE_RIGHT;
        if(key_name == "MOUSE_MIDDLE") return game::InputCode::MOUSE_MIDDLE;
        if(key_name == "MOUSE_X1") return game::InputCode::MOUSE_X1;
        if(key_name == "MOUSE_X2") return game::InputCode::MOUSE_X2;

        return game::InputCode::UNKNOWN_InputCode;   
    }

    InputSystem::InputSystem(asio::io_context & io_context)
    : _strand(asio::make_strand(io_context))
    {
    }

    asio::awaitable<void> InputSystem::recordKeyPressed(InputCode key)
    {
        if(!_strand.running_in_this_thread()) {
            co_await asio::dispatch(_strand, asio::use_awaitable);
        }
        _event_queue.push_back(InputEvent{InputEventType::Pressed, key, 0, 0, 0, 0});
        co_return;
    }

    asio::awaitable<void> InputSystem::recordKeyReleased(InputCode key)
    {
        if(!_strand.running_in_this_thread()) {
            co_await asio::dispatch(_strand, asio::use_awaitable);
        }
        _event_queue.push_back(InputEvent{InputEventType::Released, key, 0, 0, 0, 0});
        co_return;
    }

    asio::awaitable<void> InputSystem::recordMouseMove(float x, float y, float dx, float dy)
    {
        if(!_strand.running_in_this_thread()) {
            co_await asio::dispatch(_strand, asio::use_awaitable);
        }
        _event_queue.push_back(InputEvent{InputEventType::MouseMove, InputCode::UNKNOWN_InputCode, x, y, dx, dy});
        co_return;
    }

    asio::awaitable<void> InputSystem::run(ComponentManager& components, EntityManager& entities)
    {
        if(!_strand.running_in_this_thread()){
            co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));
        }

        std::deque<InputEvent> events;
        std::swap(events, _event_queue); // fast, avoids realloc

        absl::flat_hash_set<InputCode> just_pressed;
        absl::flat_hash_set<InputCode> just_released;

        bool mouse_moved = false;
        float mouse_x = 0.0f;
        float mouse_y = 0.0f;
        float mouse_dx = 0.0f;
        float mouse_dy = 0.0f;

        for (const auto& event : events)
        {
            switch (event.type) 
            {
                case InputEventType::Pressed:
                    if(event.key == InputCode::UNKNOWN_InputCode) break;
                    if (_held_keys.insert(event.key).second)
                    {
                        just_pressed.insert(event.key);
                    }
                    break;
                case InputEventType::Released:
                    if(event.key == InputCode::UNKNOWN_InputCode) break;
                    if (_held_keys.erase(event.key) > 0)
                    {
                        just_released.insert(event.key);
                    }
                    break;
                case InputEventType::MouseMove:
                    mouse_moved = true;
                    mouse_x = event.mouse_x;
                    mouse_y = event.mouse_y;
                    mouse_dx = event.mouse_dx;
                    mouse_dy = event.mouse_dy;
                    break;
            }
        }

        for (auto& [entity, mask] : entities.getAllEntities()) {
            if (!mask.test(MASK_POSITION_BIT)) continue;
            auto* input = components.getComponent<InputComponent>(entity);
            assert(input != nullptr);

            input->clear_pressed();
            input->clear_just_pressed();
            input->clear_just_released();

            for (auto key : _held_keys) input->add_pressed(key);
            for (auto key : just_pressed) input->add_just_pressed(key);
            for (auto key : just_released) input->add_just_released(key);

            if(mouse_moved)
            {
                input->set_mouse_x(mouse_x);
                input->set_mouse_y(mouse_y);
                input->set_mouse_dx(mouse_dx);
                input->set_mouse_dy(mouse_dy);
            }
        }

        co_return;
    }
}