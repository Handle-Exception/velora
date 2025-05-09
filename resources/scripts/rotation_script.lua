-- Static rotation angles (persist across frames)
pitch = pitch or 45.0  -- X axis
yaw   = yaw   or 90.0  -- Y axis
roll  = roll  or 30.0  -- Z axis

-- Rotation speeds (degrees per second)
local pitch_speed = 10.0
local yaw_speed   = 20.0
local roll_speed  = 30.0

-- Delta time (in seconds), assumed injected or global
local dt = delta or 0.016  -- e.g. 60 FPS fallback

-- Update angles
pitch = pitch + pitch_speed * dt
yaw   = yaw + yaw_speed * dt
roll  = roll + roll_speed * dt

-- Compute new quaternion rotation
local euler_deg = vec3(pitch, yaw, roll)
---local euler_rad = radians(euler_deg)
---local rotation = quat(euler_rad)  -- returns quat(w, x, y, z)

-- Apply to entity transform
local transform = get_transform(entity)
if transform then
    transform:set_rotation(rotation.w, rotation.x, rotation.y, rotation.z)
end