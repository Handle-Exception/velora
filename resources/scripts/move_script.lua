local input = get_input(entity)
if input then
    local transform = get_transform(entity)
    if input:is_pressed("KEY_UP") then
        if transform then
            transform:set_z(transform:get_z() - 0.1)
        end
    end
    if input:is_pressed("KEY_DOWN") then
        if transform then
            transform:set_z(transform:get_z() + 0.1)
        end
    end
    
    if input:is_pressed("KEY_RIGHT") then
        if transform then
            transform:set_x(transform:get_x() + 0.1)
        end
    end
    if input:is_pressed("KEY_LEFT") then
        if transform then
            transform:set_x(transform:get_x() - 0.1)
        end
    end

    if input:just_released("KEY_SPACE") then
        print(input:get_mouse_x(), input:get_mouse_y())
    end
end