#pragma once

#include <utility>

namespace velora
{
    class Resolution
    {
        public:
            Resolution(std::size_t width, std::size_t height);
            const std::size_t getWidth() const;
            const std::size_t getHeight() const;
            void setWidth(std::size_t width);
            void setHeight(std::size_t height);
        private:
            std::size_t _width;
            std::size_t _height;
    };
}