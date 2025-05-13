#pragma once
#include <array>
#include <utility>
#include <string>

#include "resolution.hpp"
#include "frame_buffer_object.hpp"
#include "render.hpp"

namespace velora
{
    class PostProcessingPipeline
    {
    public:
        PostProcessingPipeline(IRenderer & renderer, Resolution resolution, std::initializer_list<FBOAttachment> attachments)
        : _renderer(renderer), _resolution(std::move(resolution)) 
        {
            initBuffers(std::move(attachments));
        }

        ~PostProcessingPipeline() = default;

        struct Pass
        {
            std::string name;
            std::size_t shader;
            bool enabled = true;
        };

    protected:
        void initBuffers(std::initializer_list<FBOAttachment> attachments)
        {
            _fbo_buffer = co_await _renderer.constructFrameBufferObject("post_process_fbo", _resolution, attachments);
        }

    private:
        IRenderer & _renderer;


        Resolution _resolution;
        std::array<std::size_t, 2> _fbo_buffer;

        

    };
}