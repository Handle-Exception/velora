#include "resolution.hpp"

namespace velora
{
    Resolution::Resolution(std::size_t width, std::size_t height)
    :   _width(width), 
        _height(height)
    {
    }

    const std::size_t Resolution::getWidth() const
    {
        return _width;
    }

    const std::size_t Resolution::getHeight() const
    {
        return _height;
    }

    void Resolution::setWidth(std::size_t width) 
    {
        _width = width;
    }

    void Resolution::setHeight(std::size_t height) 
    {
        _height = height;
    }
}