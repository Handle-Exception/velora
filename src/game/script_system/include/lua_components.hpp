#pragma once

#include "transform_system.hpp"
#include "input_system.hpp"

namespace velora::game
{
    struct LuaTransformRef 
    {
        TransformComponent* ref;

        float get_x() const { return ref->position().x(); }
        void set_x(float x) { ref->mutable_position()->set_x(x); }

        float get_y() const { return ref->position().y(); }
        void set_y(float y) { ref->mutable_position()->set_y(y); }

        float get_z() const { return ref->position().z(); }
        void set_z(float z) { ref->mutable_position()->set_z(z);}
    };

    struct LuaInputRef 
    {
        const InputComponent* ref;

        bool is_pressed(const std::string& key) const 
        {
            return isInputPresent(keyToInputCode(key), ref->pressed());
        }

        bool just_pressed(const std::string& key) const 
        {
            return isInputPresent(keyToInputCode(key), ref->just_pressed());
        }

        bool just_released(const std::string& key) const 
        {
            return isInputPresent(keyToInputCode(key), ref->just_released());
        }

        float get_mouse_x() const { return ref->mouse_x(); }
        float get_mouse_y() const { return ref->mouse_y(); }

        float get_mouse_dx() const { return ref->mouse_dx(); }
        float get_mouse_dy() const { return ref->mouse_dy(); }
    };
}