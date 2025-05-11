#include "visual_system.hpp"

namespace velora::game
{
    const uint32_t VisualSystem::MASK_POSITION_BIT = ComponentTypeManager::getTypeID<VisualComponent>();

    asio::awaitable<VisualSystem> VisualSystem::asyncConstructor(
                asio::io_context & io_context,
                IRenderer & renderer,
                Resolution resolution,
                game::CameraSystem & camera_system,
                game::LightSystem & light_system)
    {
        auto id = co_await renderer.constructFrameBufferObject("visual_system_fbo",
            std::move(resolution), 
            {
                {FBOAttachment::Type::Texture, FBOAttachment::Point::Color},
                {FBOAttachment::Type::Texture, FBOAttachment::Point::Color},
                {FBOAttachment::Type::Texture, FBOAttachment::Point::Color},
                {FBOAttachment::Type::RenderBuffer, FBOAttachment::Point::Depth}
            }
        );

        if(id.has_value() == false)
        {
            throw std::runtime_error("Failed to create visual system fbo");
        }
        co_return VisualSystem(io_context, renderer, camera_system, light_system, *id);
    }

    VisualSystem::VisualSystem(asio::io_context & io_context,
                    IRenderer & renderer,
                    game::CameraSystem & camera_system,
                    game::LightSystem & light_system,
                    std::optional<std::size_t> fbo
        )
        :   _strand(asio::make_strand(io_context)),
            _renderer(renderer),
            _camera_system(camera_system),
            _light_system(light_system),
            _deferred_fbo(std::move(fbo))
    {
        const auto id = _renderer.getVertexBuffer("quad_prefab");
        if(!id)
        {
            spdlog::error("Failed to get quad prefab vertex buffer");
            throw std::runtime_error("Failed to get quad prefab vertex buffer");
        }

        _quad_vbo = *id;
    }

    asio::awaitable<void> VisualSystem::run(ComponentManager& components, EntityManager& entities, float alpha)
    {
        if(_renderer.good() == false)co_return;

        if(!_strand.running_in_this_thread()){
            co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));
        }

        // get camera system state
        const glm::mat4 & view_matrix = _camera_system.getView();
        const glm::mat4 & proj_matrix = _camera_system.getProjection();

        VisualComponent * visual_component = nullptr;
                
        glm::mat4 model_matrix = glm::mat4(1.0f);
        glm::vec3 position = glm::vec3(0.0f);
        glm::vec3 scale = glm::vec3(1.0f);
        glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        
        glm::vec3 prev_position = glm::vec3(0.0f);
        glm::vec3 prev_scale = glm::vec3(1.0f);
        glm::quat prev_rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

        glm::vec3 interpolated_pos;
        glm::quat interpolated_rot;
        glm::vec3 interpolated_scale;
        
        RenderMode mode;

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
            
            // check renderer before
            if(_renderer.good() == false)co_return;

            const auto vb_id = _renderer.getVertexBuffer(visual_component->vertex_buffer_name());
            const auto sh_id = _renderer.getShader(visual_component->shader_name());

            if (!vb_id || !sh_id)
            {
                // if cannot find vertex buffer or shader, skip
                continue;
            }

            // if also has a transform component
            // update transform matrix
            static const uint32_t TRANSFORM_BIT = ComponentTypeManager::getTypeID<TransformComponent>();

            if(mask.test(TRANSFORM_BIT))
            {
                auto* transform_component = components.getComponent<TransformComponent>(entity);
                assert(transform_component != nullptr);

                prev_position = glm::vec3(transform_component->prev_position().x(), 
                                    transform_component->prev_position().y(), 
                                    transform_component->prev_position().z());
                        
                prev_scale = glm::vec3(transform_component->prev_scale().x(), 
                                    transform_component->prev_scale().y(), 
                                    transform_component->prev_scale().z());
                        
                prev_rotation = glm::quat(transform_component->prev_rotation().w(), 
                                    transform_component->prev_rotation().x(), 
                                    transform_component->prev_rotation().y(), 
                                    transform_component->prev_rotation().z());
                prev_rotation = glm::normalize(prev_rotation);


                position = glm::vec3(transform_component->position().x(), 
                                    transform_component->position().y(), 
                                    transform_component->position().z());
                        
                scale = glm::vec3(transform_component->scale().x(), 
                                    transform_component->scale().y(), 
                                    transform_component->scale().z());
                        
                rotation = glm::quat(transform_component->rotation().w(), 
                                    transform_component->rotation().x(), 
                                    transform_component->rotation().y(), 
                                    transform_component->rotation().z());
                rotation = glm::normalize(rotation);

                interpolated_pos = glm::mix(prev_position, position, alpha);
                interpolated_rot = glm::slerp(prev_rotation, rotation, alpha);
                interpolated_scale = glm::mix(prev_scale, scale, alpha);

                // get model matrix from transform component
                model_matrix = glm::translate(glm::mat4(1.0f), interpolated_pos)
                                                * glm::toMat4(interpolated_rot)
                                                * glm::scale(glm::mat4(1.0f), interpolated_scale);
            }
            else 
            {
                // if no transform component, use identity matrix
                model_matrix = glm::mat4(1.0f);
            }

            mode = RenderMode::Solid;

            // render into _deferred_fbo (G Buffer)
            co_await _renderer.render(*vb_id, *sh_id, 
                        ShaderInputs{
                            .in_bool = {{"useTexture", false}},
                            .in_vec4 = {{"uColor", glm::vec4(0.5, 0.5, 0.5, 1)}},
                            .in_mat4 = {
                                {"uModel", model_matrix},
                                {"uView", view_matrix},
                                {"uProjection", proj_matrix}
                            },
                        }, 
                        mode,
                        _deferred_fbo);
        }

        // render display quad using lighting pass shader on G Buffer
        co_await _renderer.render(
            _quad_vbo, /*Tutaj lighting pass shader*/,
            ShaderInputs{
                .in_int = {{"lightCount", (int)_light_system.getLightCount()}},
                .in_samplers = {/*tutaj powinny być zbindowane textury z G buffer*/},
                .storage_buffer = _light_system.getShaderBufferID()
            } 
        );
        co_return;
    }
}