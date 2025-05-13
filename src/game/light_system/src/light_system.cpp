#include "light_system.hpp"

namespace velora::game
{
    const uint32_t LightSystem::MASK_POSITION_BIT = ComponentTypeManager::getTypeID<LightComponent>();

    asio::awaitable<LightSystem> LightSystem::asyncConstructor(asio::io_context & io_context, VisualSystem & visual_system)
    {
        IRenderer & renderer = visual_system.getRenderer();

        // binding point 2 in shader
        auto light_shader_buffer = co_await renderer.constructShaderStorageBuffer(
                "LightSystem::ssbo::light", 2, 0, nullptr);
        
        if(light_shader_buffer.has_value() == false)
        {
            throw std::runtime_error("Failed to create light shader storage buffer");
        }

        auto shadow_map_fbo = co_await renderer.constructFrameBufferObject(
                "LightSystem::fbo::shadow_map", {512, 512},
                {{FBOAttachment::Type::Texture, FBOAttachment::Point::Depth, TextureFormat::Depth}});

        if(shadow_map_fbo.has_value() == false)
        {
            throw std::runtime_error("Failed to create shadow map fbo");
        }

        co_return LightSystem(io_context, renderer, visual_system, *light_shader_buffer, *shadow_map_fbo);
    }

    LightSystem::LightSystem(
            asio::io_context & io_context,
            IRenderer & renderer,
            VisualSystem & visual_system,
            std::size_t light_shader_buffer_id, 
            std::size_t shadow_map_fbo
    )
    :   _strand(asio::make_strand(io_context)),
        _renderer(renderer),
        _visual_system(visual_system),
        _light_shader_buffer_id(light_shader_buffer_id),
        _shadow_map_fbo(shadow_map_fbo)
    {
        _gpu_lights.reserve(MAX_LIGHTS);

        auto shadow_shader_result = _renderer.getShader("shadow_depth");
        if(!shadow_shader_result)
        {
            spdlog::error("Failed to get shadow pass shader");
            throw std::runtime_error("Failed to get shadow pass shader");
        }
        _shadow_pass_shader = *shadow_shader_result;

        _shadow_map_textures = _renderer.getFrameBufferObjectTextures(_shadow_map_fbo);
        assert(_shadow_map_textures.size() == 1);
    }

    std::size_t LightSystem::getLightShaderBufferID() const 
    { 
        return _light_shader_buffer_id; 
    }

    std::size_t LightSystem::getLightCount() const 
    { 
        return _gpu_lights.size();
    }

    std::size_t LightSystem::getShadowMapTexture() const
    {
        return _shadow_map_textures.at(0);
    }

    asio::awaitable<void> LightSystem::run(const ComponentManager& components, const EntityManager& entities, float alpha)
    {
        // clear shadow map fbo
        //co_await _renderer.clearScreen({0.0f, 0.0f, 0.0f, 1.0f}, _shadow_map_fbo);

        if(!_strand.running_in_this_thread()){
            co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));
        }
        
        collectLights(components, entities, alpha);

        // it can run in parallel
        co_await (
            _renderer.updateShaderStorageBuffer(_light_shader_buffer_id, sizeof(GPULight) * _gpu_lights.size(), _gpu_lights.data())
            //&& renderShadows(components, entities, alpha)
        );

        co_return;
    }

    void LightSystem::collectLights(const ComponentManager& components, const EntityManager& entities, float alpha)
    {
        _gpu_lights.clear();

        glm::vec3 position;
        glm::quat rotation;

        glm::vec3 prev_position;
        glm::quat prev_rotation;

        glm::vec3 interpolated_pos;
        glm::quat interpolated_rot;

        glm::vec3 direction;

        GPULight gpu_light{};
        uint16_t light_id = 0;
        for (const auto& [entity, mask] : entities.getAllEntities())
        {
            if(light_id >= MAX_LIGHTS)return;

            if (mask.test(MASK_POSITION_BIT) == false) continue;

            const LightComponent * light_component = components.getComponent<LightComponent>(entity);
            assert(light_component != nullptr);
            
            auto* transform = components.getComponent<TransformComponent>(entity);
            if(transform)
            {
                position = glm::vec3{transform->position().x(), transform->position().y(), transform->position().z()};
                rotation = glm::quat{transform->rotation().w(), transform->rotation().x(), transform->rotation().y(), transform->rotation().z()};
                rotation = glm::normalize(rotation);

                prev_position = glm::vec3{transform->prev_position().x(), transform->prev_position().y(), transform->prev_position().z()};

                prev_rotation = glm::quat{transform->prev_rotation().w(), transform->prev_rotation().x(), transform->prev_rotation().y(), transform->prev_rotation().z()};
                prev_rotation = glm::normalize(prev_rotation);

                interpolated_pos = glm::mix(prev_position, position, alpha);
                interpolated_rot = glm::slerp(prev_rotation, rotation, alpha);

                direction = glm::normalize(interpolated_rot * BASE_FORWARD_DIRECTION);
                
                gpu_light.position = glm::vec4(interpolated_pos.x, interpolated_pos.y, interpolated_pos.z, 1.0f);

                // w component is used to determine the type of light
                gpu_light.direction = glm::vec4(direction.x, direction.y, direction.z, 
                    static_cast<float>(light_component->type()));
            }
            else
            {
                gpu_light.position = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
                // w component is used to determine the type of light
                gpu_light.direction = glm::vec4(BASE_FORWARD_DIRECTION.x, BASE_FORWARD_DIRECTION.y, BASE_FORWARD_DIRECTION.z, 
                    static_cast<float>(light_component->type()));
            }

            gpu_light.color = glm::vec4(light_component->color_r(), light_component->color_g(), light_component->color_b(), light_component->intensity());
            gpu_light.attenuation = glm::vec4(light_component->constant(), light_component->linear(), light_component->quadratic(), 0.0f);
            gpu_light.cutoff = glm::vec2(light_component->inner_cutoff(), light_component->outer_cutoff());
            gpu_light.castShadows = glm::vec2(static_cast<uint32_t>(light_component->cast_shadows()), 0);
            
            _gpu_lights.emplace_back(std::move(gpu_light));
            light_id++;
        }
        assert(_gpu_lights.size() <= MAX_LIGHTS);
        assert(_gpu_lights.size() == light_id);
    }

    asio::awaitable<void> LightSystem::renderShadows(const ComponentManager& components, const EntityManager& entities, float alpha)
    {
        if(!_strand.running_in_this_thread()){
            co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));
        }

        glm::mat4 view_matrix;
        glm::mat4 projection_matrix;
        glm::mat4 light_space_matrix;
        glm::mat4 model_matrix;

        for (const auto& light : _gpu_lights)
        {
            if(!_strand.running_in_this_thread()){
                co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));
            }

            if(light.castShadows.x == 0)continue;
            
            view_matrix = glm::lookAt(glm::vec3(light.position), glm::vec3(light.position) + glm::vec3(light.direction), BASE_UP_DIRECTION);

            light_space_matrix = projection_matrix * view_matrix;

            // now for every light we need to render whole scene 
            // so all entities with visual component
            // using simplified shadow shader
            for(const auto & [entity, mask] : entities.getAllEntities())
            {
                if(mask.test(VisualSystem::MASK_POSITION_BIT) == false)continue;

                const VisualComponent * visual_component = components.getComponent<VisualComponent>(entity);
                assert(visual_component != nullptr);

                if(visual_component->visible() == false)continue;

                const auto vb_id = _renderer.getVertexBuffer(visual_component->vertex_buffer_name());
                if(!vb_id)continue;

                // read interpolated matrix from visual component
                model_matrix[0][0] = visual_component->model_matrix().data(0);
                model_matrix[0][1] = visual_component->model_matrix().data(1);
                model_matrix[0][2] = visual_component->model_matrix().data(2);
                model_matrix[0][3] = visual_component->model_matrix().data(3);

                model_matrix[1][0] = visual_component->model_matrix().data(4);
                model_matrix[1][1] = visual_component->model_matrix().data(5);
                model_matrix[1][2] = visual_component->model_matrix().data(6);
                model_matrix[1][3] = visual_component->model_matrix().data(7);

                model_matrix[2][0] = visual_component->model_matrix().data(8);
                model_matrix[2][1] = visual_component->model_matrix().data(9);
                model_matrix[2][2] = visual_component->model_matrix().data(10);
                model_matrix[2][3] = visual_component->model_matrix().data(11);

                model_matrix[3][0] = visual_component->model_matrix().data(12);
                model_matrix[3][1] = visual_component->model_matrix().data(13);
                model_matrix[3][2] = visual_component->model_matrix().data(14);
                model_matrix[3][3] = visual_component->model_matrix().data(15);

                
                // render depth information to shadow map fbo
                co_await _renderer.render(*vb_id, _shadow_pass_shader, 
                    ShaderInputs{
                        .in_mat4 = {
                            {"uModel", model_matrix},
                            {"uLightSpaceMatrix", light_space_matrix}
                        }
                    },
                    RenderMode::Solid,
                    _shadow_map_fbo
                );
            }

        }  
    }
}