#include "visual_system.hpp"

namespace velora::game
{
    const uint32_t VisualSystem::MASK_POSITION_BIT = ComponentTypeManager::getTypeID<VisualComponent>();

    void updateModelMatrixField(VisualComponent * visual_component, glm::mat4 model_matrix)
    {
        if(visual_component->has_model_matrix())
        {
            // save interpolated matrix in visual component
            visual_component->mutable_model_matrix()->set_data(0, model_matrix[0][0]);
            visual_component->mutable_model_matrix()->set_data(1, model_matrix[0][1]);
            visual_component->mutable_model_matrix()->set_data(2, model_matrix[0][2]);
            visual_component->mutable_model_matrix()->set_data(3, model_matrix[0][3]);

            visual_component->mutable_model_matrix()->set_data(4, model_matrix[1][0]);
            visual_component->mutable_model_matrix()->set_data(5, model_matrix[1][1]);
            visual_component->mutable_model_matrix()->set_data(6, model_matrix[1][2]);
            visual_component->mutable_model_matrix()->set_data(7, model_matrix[1][3]);     

            visual_component->mutable_model_matrix()->set_data(8, model_matrix[2][0]);
            visual_component->mutable_model_matrix()->set_data(9, model_matrix[2][1]);
            visual_component->mutable_model_matrix()->set_data(10, model_matrix[2][2]);
            visual_component->mutable_model_matrix()->set_data(11, model_matrix[2][3]);  

            visual_component->mutable_model_matrix()->set_data(12, model_matrix[3][0]);
            visual_component->mutable_model_matrix()->set_data(13, model_matrix[3][1]);
            visual_component->mutable_model_matrix()->set_data(14, model_matrix[3][2]);
            visual_component->mutable_model_matrix()->set_data(15, model_matrix[3][3]);
        }
        else
        {
            visual_component->mutable_model_matrix()->add_data( model_matrix[0][0]);
            visual_component->mutable_model_matrix()->add_data(model_matrix[0][1]);
            visual_component->mutable_model_matrix()->add_data(model_matrix[0][2]);
            visual_component->mutable_model_matrix()->add_data(model_matrix[0][3]);

            visual_component->mutable_model_matrix()->add_data(model_matrix[1][0]);
            visual_component->mutable_model_matrix()->add_data(model_matrix[1][1]);
            visual_component->mutable_model_matrix()->add_data(model_matrix[1][2]);
            visual_component->mutable_model_matrix()->add_data(model_matrix[1][3]);     

            visual_component->mutable_model_matrix()->add_data(model_matrix[2][0]);
            visual_component->mutable_model_matrix()->add_data(model_matrix[2][1]);
            visual_component->mutable_model_matrix()->add_data(model_matrix[2][2]);
            visual_component->mutable_model_matrix()->add_data(model_matrix[2][3]);  

            visual_component->mutable_model_matrix()->add_data(model_matrix[3][0]);
            visual_component->mutable_model_matrix()->add_data(model_matrix[3][1]);
            visual_component->mutable_model_matrix()->add_data(model_matrix[3][2]);
            visual_component->mutable_model_matrix()->add_data(model_matrix[3][3]);  
        }
    }

    asio::awaitable<VisualSystem> VisualSystem::asyncConstructor(
                asio::io_context & io_context,
                IRenderer & renderer,
                Resolution resolution,
                game::CameraSystem & camera_system)
    {
        auto fbo = co_await renderer.constructFrameBufferObject("VisualSystem::fbo",
            std::move(resolution), 
            {
                {FBOAttachment::Type::Texture, FBOAttachment::Point::Color, TextureFormat::RGB_16F}, // gPosition
                {FBOAttachment::Type::Texture, FBOAttachment::Point::Color, TextureFormat::RGB_16F}, // gNormal
                {FBOAttachment::Type::Texture, FBOAttachment::Point::Color, TextureFormat::RGBA_16F}, // gAlbedo
                {FBOAttachment::Type::RenderBuffer, FBOAttachment::Point::Depth, TextureFormat::Depth}
            }
        );

        if(fbo.has_value() == false)
        {
            throw std::runtime_error("Failed to create visual system fbo");
        }
        co_return VisualSystem(io_context, renderer, camera_system, *fbo);
    }

    VisualSystem::VisualSystem(asio::io_context & io_context,
                    IRenderer & renderer,
                    game::CameraSystem & camera_system,
                    std::optional<std::size_t> fbo
        )
        :   _strand(asio::make_strand(io_context)),
            _renderer(renderer),
            _camera_system(camera_system),
            _deferred_fbo(std::move(fbo))
    {
        _deferred_fbo_textures = _renderer.getFrameBufferObjectTextures(*_deferred_fbo);
    }

    IRenderer & VisualSystem::getRenderer() const 
    {
        return _renderer;
    }

    const std::vector<std::size_t> & VisualSystem::getDeferredFBOTextures() const 
    { 
        return _deferred_fbo_textures;
    }

    asio::awaitable<void> VisualSystem::run(ComponentManager& components, EntityManager& entities, float alpha)
    {
        if(_renderer.good() == false)co_return;

        // clear deferred_fbo (G Buffer)
        co_await _renderer.clearScreen({0.0f, 0.0f, 0.0f, 1.0f}, _deferred_fbo);

        if(!_strand.running_in_this_thread()){
            co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));
        }

        // get camera system state
        const glm::vec3 & view_position = _camera_system.getPosition();
        const glm::mat4 & view_matrix = _camera_system.getView();
        const glm::mat4 & proj_matrix = _camera_system.getProjection();

        VisualComponent * visual_component = nullptr;
                
        glm::mat4 model_matrix = glm::mat4(1.0f);
        glm::vec4 color = glm::vec4(0.5, 0.5, 0.5, 1);
        for (const auto& [entity, mask] : entities.getAllEntities())
        {
            if(!_strand.running_in_this_thread()){
                co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));
            }

            if (mask.test(MASK_POSITION_BIT) == false) continue;

            visual_component = components.getComponent<VisualComponent>(entity);
            assert(visual_component != nullptr);
                    
            // if not visible, skip
            if(visual_component->visible() == false) continue;
            
            const auto vb_id = _renderer.getVertexBuffer(visual_component->vertex_buffer_name());
            const auto sh_id = _renderer.getShader(visual_component->shader_name());

            if (!vb_id || !sh_id)
            {
                // if cannot find vertex buffer or shader, skip
                continue;
            }

            // if also has a transform component
            // update transform matrix
            if(mask.test(TransformSystem::MASK_POSITION_BIT))
            {
                auto* transform_component = components.getComponent<TransformComponent>(entity);
                assert(transform_component != nullptr);

                // get model matrix from transform component
                model_matrix = calculateInterpolatedTransformMatrix(*transform_component, alpha); 
            }
            else 
            {
                // if no transform component, use identity matrix
                model_matrix = glm::mat4(1.0f);
            }

            updateModelMatrixField(visual_component, model_matrix);

            // get color from visual component
            if(visual_component->has_color())
            {
                color = glm::vec4(visual_component->color().x(), 
                                  visual_component->color().y(), 
                                  visual_component->color().z(), 
                                  visual_component->color().w());
            }

            // render into deferred_fbo (G Buffer)
            co_await _renderer.render(*vb_id, *sh_id, 
                        ShaderInputs{
                            .in_bool = {{"useTexture", false}},
                            .in_vec4 = {{"uColor", color}},
                            .in_mat4 = {
                                {"uModel", model_matrix},
                                {"uView", view_matrix},
                                {"uProjection", proj_matrix}
                            },
                        },
                        RenderOptions{
                            .mode = RenderMode::Solid
                        },
                        _deferred_fbo);
        }
        
        co_return;
    }
}