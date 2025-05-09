local transform = get_transform(entity)

-- Static rotation angles (persist across frames)
pitch = 45.0  -- X axis
yaw   = 90.0  -- Y axis
roll  = 30.0  -- Z axis

-- Rotation speeds (degrees per second)
local pitch_speed = 10.0
local yaw_speed   = 20.0
local roll_speed  = 30.0
local move_speed  = 0.1

-- Delta time (in seconds), assumed injected or global
local dt = delta or 0.016  -- e.g. 60 FPS fallback

if(transform) then
    local rotation_deg = glm.degrees(glm.eulerAngles(transform:get_rotation()))
    pitch = rotation_deg.x
    yaw   = rotation_deg.y
    roll  = rotation_deg.z
end

-- Update angles
pitch = pitch + pitch_speed * dt
yaw   = yaw + yaw_speed * dt
roll  = roll + roll_speed * dt

-- Compute new quaternion rotation
local euler_deg = glm.vec3(pitch, yaw, roll)
local euler_rad = glm.radians(euler_deg)
local rotation = glm.quat(euler_rad)  -- returns quat(w, x, y, z)

-- Apply to entity transform
if transform then
    transform:set_rotation(rotation.w, rotation.x, rotation.y, rotation.z)
end

local input = get_input(entity)
if input then
    if input:is_pressed("KEY_UP") then
        if transform then
            transform:set_z(transform:get_z() - move_speed)
        end
    end
    if input:is_pressed("KEY_DOWN") then
        if transform then
            transform:set_z(transform:get_z() + move_speed)
        end
    end
    
    if input:is_pressed("KEY_RIGHT") then
        if transform then
            transform:set_x(transform:get_x() + move_speed)
        end
    end
    if input:is_pressed("KEY_LEFT") then
        if transform then
            transform:set_x(transform:get_x() - move_speed)
        end
    end
end