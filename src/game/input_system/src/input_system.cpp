#include "input_system.hpp"

namespace velora::game
{
    const uint32_t InputSystem::MASK_POSITION_BIT = ComponentTypeManager::getTypeID<InputComponent>();

    InputSystem::InputSystem(asio::io_context & io_context)
    : _strand(asio::make_strand(io_context))
    {
        spdlog::debug(std::format("[t:{}] InputSystem constructed", std::this_thread::get_id()));
    }

    InputSystem::~InputSystem()
    {
        spdlog::debug(std::format("[t:{}] InputSystem destroyed", std::this_thread::get_id()));
    }
}