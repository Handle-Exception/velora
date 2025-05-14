#include "light_system.hpp"

namespace velora::game
{
    const uint32_t LightSystem::MASK_POSITION_BIT = ComponentTypeManager::getTypeID<LightComponent>();

    glm::mat4 loadModelMatrixField(const VisualComponent * visual_component)
    {
        if(visual_component == nullptr) return glm::mat4(1.0f);
        if(!visual_component->has_model_matrix()) return glm::mat4(1.0f);
        glm::mat4 model_matrix;

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

        return model_matrix;
    }

    asio::awaitable<LightSystem> LightSystem::asyncConstructor(asio::io_context & io_context, VisualSystem & visual_system)
    {
        IRenderer & renderer = visual_system.getRenderer();

        // binding point 2 in shader
        auto light_shader_buffer = co_await renderer.constructShaderStorageBuffer(
                "LightSystem::ssbo::light", 2, 0, nullptr);
        
        if(!light_shader_buffer)
        {
            spdlog::error("Failed to create light shader storage buffer");
            throw std::runtime_error("Failed to create light shader storage buffer");
        }

        // create shadow map frame buffer objects
        // for all shadow casters
        const Resolution shadow_map_resolution{1024, 1024};
        std::vector<std::size_t> shadow_map_fbos;
        std::optional<std::size_t> fbo_creaton_result;
        for(unsigned int i = 0; i < MAX_SHADOW_CASTERS; ++i)
        {
            fbo_creaton_result = co_await renderer.constructFrameBufferObject(
                "LightSystem::fbo::shadow_map" + std::to_string(i), shadow_map_resolution,
                {{FBOAttachment::Type::Texture, FBOAttachment::Point::Depth, TextureFormat::Depth_32F}});
            
            if(!fbo_creaton_result)
            {
                spdlog::error("Failed to create shadow map fbo {}", i);
                throw std::runtime_error("Failed to create shadow map fbo " + std::to_string(i));
            }

            shadow_map_fbos.emplace_back(*fbo_creaton_result);
        }

        co_return LightSystem(io_context, renderer, visual_system, *light_shader_buffer, shadow_map_resolution, std::move(shadow_map_fbos));
    }

    LightSystem::LightSystem(
            asio::io_context & io_context,
            IRenderer & renderer,
            VisualSystem & visual_system,
            std::size_t light_shader_buffer_id,
            Resolution shadow_map_resolution,
            std::vector<std::size_t> shadow_map_fbos
    )
    :   _strand(asio::make_strand(io_context)),
        _renderer(renderer),
        _visual_system(visual_system),
        _light_shader_buffer_id(light_shader_buffer_id),
        _shadow_map_resolution(std::move(shadow_map_resolution)),
        _shadow_map_fbos(std::move(shadow_map_fbos))
    {
        _gpu_lights.reserve(MAX_LIGHTS);

        auto shadow_shader_result = _renderer.getShader("shadow_depth");
        if(!shadow_shader_result)
        {
            spdlog::error("Failed to get shadow pass shader");
            throw std::runtime_error("Failed to get shadow pass shader");
        }
        _shadow_pass_shader = *shadow_shader_result;

        assert(shadow_map_fbos.size() <= MAX_SHADOW_CASTERS && "Too many shadow casters! Increase MAX_SHADOW_CASTERS in light_system.hpp" );

        // fetch shadow map textures from frame buffer objects
        _shadow_map_textures.reserve(MAX_SHADOW_CASTERS);
        for(const auto & shader_map_fbo : _shadow_map_fbos)
        {
            auto tex_res = _renderer.getFrameBufferObjectTextures(shader_map_fbo);
            assert(tex_res.size() == 1 && "Shadow map fbo should only have one texture" );
            _shadow_map_textures.emplace_back(tex_res.at(0));
        }

        _shadow_map_light_space_matrices.resize(MAX_SHADOW_CASTERS);
    }

    std::size_t LightSystem::getLightShaderBufferID() const 
    { 
        return _light_shader_buffer_id; 
    }

    std::size_t LightSystem::getLightsCount() const 
    { 
        return _gpu_lights.size();
    }

    std::vector<std::size_t> LightSystem::getShadowMapTextures() const
    {
        return std::vector<std::size_t>(_shadow_map_textures.begin(), _shadow_map_textures.begin() + _shadow_casters_count);
    }

    std::vector<glm::mat4> LightSystem::getShadowMapLightSpaceMatrices() const
    {
        return std::vector<glm::mat4>(_shadow_map_light_space_matrices.begin(), _shadow_map_light_space_matrices.begin() + _shadow_casters_count);
    }

    std::size_t LightSystem::getShadowCastersCount() const
    {
        return _shadow_casters_count;
    }

    asio::awaitable<void> LightSystem::run(const ComponentManager& components, const EntityManager& entities, float alpha)
    {
        if(!_strand.running_in_this_thread()){
            co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));
        }
        
        collectLights(components, entities, alpha);

        co_await renderShadows(components, entities, alpha);

        co_await _renderer.updateShaderStorageBuffer(_light_shader_buffer_id, sizeof(GPULight) * _gpu_lights.size(), _gpu_lights.data());

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
        uint32_t light_id = 0;
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
                if(direction == glm::vec3{0.0f, 0.0f, 0.0f}) direction = BASE_FORWARD_DIRECTION;
                
                // Move the light slightly forward along its direction
                // This prevents self-shadowing collapse due to zero distance between camera and light projection centers
                gpu_light.position = glm::vec4(
                        interpolated_pos.x + (direction.x * 0.05f),
                        interpolated_pos.y + (direction.y * 0.05f),
                        interpolated_pos.z + (direction.z * 0.05f),
                        1.0f);

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
            gpu_light.castShadows.x = static_cast<uint32_t>(light_component->cast_shadows());
            
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

        const float shadow_map_aspect = (float)_shadow_map_resolution.getWidth() / (float)_shadow_map_resolution.getHeight();

        glm::mat4 view_matrix;
        glm::mat4 projection_matrix;
        glm::mat4 light_space_matrix;
        glm::mat4 model_matrix;

        // for every light
        uint32_t light_id = 0;
        for (auto& light : _gpu_lights)
        {
            if(light_id >= MAX_SHADOW_CASTERS)co_return;

            if(!_strand.running_in_this_thread()){
                co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));
            }

            if(light.castShadows.x == 0)continue;

            // store shadow caster id 
            light.castShadows.y = static_cast<float>(light_id);

            view_matrix = glm::lookAt(glm::vec3(light.position), glm::vec3(light.position) + glm::normalize(glm::vec3(light.direction)), BASE_UP_DIRECTION);
            
            if(light.direction.w == static_cast<float>(LightType::DIRECTIONAL))
            {
                // directional light
                // TODO
                projection_matrix = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, 0.1f, 100.0f);
            }
            else if(light.direction.w == static_cast<float>(LightType::POINT))
            {
                // point light
                // TODO
                projection_matrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f);
            }
            else if(light.direction.w == static_cast<float>(LightType::SPOT))
            {
                // spot light
                float outer_cutoff_cos = light.cutoff.y;
                outer_cutoff_cos = glm::clamp(outer_cutoff_cos, -1.0f, 1.0f);
                float fov = glm::degrees(2.0f * acos(glm::clamp(outer_cutoff_cos, -0.999f, 0.999f)));
                fov = glm::clamp(fov, 5.0f, 179.0f);
                const float projection_near = 0.1f;
                const float projection_far = 1024.0f;
                projection_matrix = glm::perspective(glm::radians(fov), shadow_map_aspect, projection_near, projection_far);
            }
            else
            {
                // unknown light type
                co_return;
            }

            light_space_matrix = projection_matrix * view_matrix;
            // store light space matrix
            _shadow_map_light_space_matrices[light_id] = light_space_matrix;

            // clear shadow map fbo
            co_await _renderer.clearScreen({0.0f, 0.0f, 0.0f, 1.0f}, _shadow_map_fbos.at(light_id));

            // now for every light we need to render whole scene 
            // so all entities with visual component
            // using simplified shadow shader
            for(const auto & [entity, mask] : entities.getAllEntities())
            {
                if(!_strand.running_in_this_thread()){
                    co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));
                }

                if(mask.test(VisualSystem::MASK_POSITION_BIT) == false)continue;

                const VisualComponent * visual_component = components.getComponent<VisualComponent>(entity);
                assert(visual_component != nullptr);

                if(visual_component->visible() == false)continue;

                const auto vb_id = _renderer.getVertexBuffer(visual_component->vertex_buffer_name());
                if(!vb_id)continue;

                // read interpolated matrix from visual component
                model_matrix = loadModelMatrixField(visual_component);

                // render depth information to shadow map fbo
                co_await _renderer.render(*vb_id, _shadow_pass_shader, 
                    ShaderInputs{
                        .in_mat4 = {
                            {"uModel", model_matrix},
                            {"uLightSpaceMatrix", light_space_matrix}
                        }
                    },
                    RenderOptions{
                        .mode = RenderMode::Solid,
                        .polygon_offset = PolygonOffset{.factor = 1.5f, .units = 4.0f}
                    },
                    _shadow_map_fbos.at(light_id)
                );
            }

            light_id++;
        }
        _shadow_casters_count = light_id;
    }
}