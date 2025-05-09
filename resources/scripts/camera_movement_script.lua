local camera_input = get_input(entity)
local camera_transform = get_transform(entity)

-- Persist camera position between frames
camera_position = camera_position or {
    x = camera_transform:get_x(),
    y = camera_transform:get_y(),
    z = camera_transform:get_z()
}

local speed = 5.0
local dt = delta or 0.016  -- e.g. 60 FPS fallback

-- Horizontal movement
if camera_input:is_pressed("KEY_A") or camera_input:just_pressed("KEY_A") then
    camera_position.x = camera_position.x - (speed * dt)
end
if camera_input:is_pressed("KEY_D") or camera_input:just_pressed("KEY_D") then
    camera_position.x = camera_position.x + (speed * dt)
end

-- Forward/back
if camera_input:is_pressed("KEY_W") or camera_input:just_pressed("KEY_W") then
    camera_position.z = camera_position.z - (speed * dt)
end
if camera_input:is_pressed("KEY_S") or camera_input:just_pressed("KEY_S") then
    camera_position.z = camera_position.z + (speed * dt)
end

-- Vertical movement
if camera_input:is_pressed("KEY_Q") or camera_input:just_pressed("KEY_Q") then
    camera_position.y = camera_position.y + (speed * dt)
end
if camera_input:is_pressed("KEY_E") or camera_input:just_pressed("KEY_E") then
    camera_position.y = camera_position.y - (speed * dt)
end

-- Apply new position to ECS transform
camera_transform:set_x(camera_position.x)
camera_transform:set_y(camera_position.y)
camera_transform:set_z(camera_position.z)