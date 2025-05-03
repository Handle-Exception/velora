#include "asset.hpp"

namespace velora
{
    asio::awaitable<std::optional<std::size_t>> loadShaderFromFile(IRenderer & renderer, std::filesystem::path shader_dir)
    {
        shader_dir = getResourcesPath() / shader_dir;

        if(!std::filesystem::exists(shader_dir)) {
            spdlog::error("Shader directory does not exist: {}", shader_dir.string());
            co_return std::nullopt;
        }

        std::string line;
        std::vector<std::string> vs_lines;
        std::vector<std::string> frag_lines;
        std::ifstream shader_file;

        std::filesystem::path vs_path = shader_dir / "vertex.glsl";
        if (!std::filesystem::exists(vs_path)) {
            spdlog::error("Vertex shader file does not exist: {}", vs_path.string());
            co_return std::nullopt;
        }
        else
        {
            shader_file.open(vs_path);

            if (!shader_file.is_open()) {
                spdlog::error("Failed to open shader file: {}", vs_path.string());
                co_return std::nullopt;
            }

            while (std::getline(shader_file, line)) {
                vs_lines.push_back(line + "\n");
            }
            
            shader_file.close();
        }


        std::filesystem::path fs_path = shader_dir / "fragment.glsl";
        if (!std::filesystem::exists(fs_path)) {
            spdlog::warn("Fragment shader file does not exist: {}", fs_path.string());
        }
        else
        {
            shader_file.open(fs_path);
            
            if (!shader_file.is_open()) {
                spdlog::error("Failed to open shader file: {}", fs_path.string());
                co_return std::nullopt;
            }

            while (std::getline(shader_file, line)) {
                frag_lines.push_back(line + "\n");
            }
            
            shader_file.close();
        }

        assert(shader_file.is_open() == false && "Shader file is still open after closing it.");

        const std::string shader_name = shader_dir.stem().string();

        if(vs_lines.empty() == false)
        {
            if(frag_lines.empty())
            {
                spdlog::debug("Loading shader: {} (vertex only)", shader_name);
                co_return co_await renderer.constructShader(std::move(shader_name), std::move(vs_lines));
            }
            else
            {
                spdlog::debug("Loading shader: {} (vertex + fragment)", shader_name);
                co_return co_await renderer.constructShader(std::move(shader_name), std::move(vs_lines), std::move(frag_lines));
            }
        }
        else
        {
            spdlog::error("Vertex shader file is empty: {}", vs_path.string());
            co_return std::nullopt;
        }
    }

    asio::awaitable<std::optional<std::string>> loadLevelFromFile(game::World & world, const ComponentLoaderRegistry & registry, const std::filesystem::path& path)
    {
        std::ifstream in(path);
        if (!in.is_open()) 
        {
            spdlog::error("Failed to open level file: {}", path.string());
            co_return std::nullopt;
        }

        std::stringstream buffer;
        buffer << in.rdbuf();
        in.close();

        game::LevelDefinition level_data;
        auto status = google::protobuf::util::JsonStringToMessage(buffer.str(), &level_data);
        if (!status.ok()) 
        {
            spdlog::error("Failed to parse level file: {}", status.ToString());
            co_return std::nullopt;
        }

        std::string level_name = level_data.name();
        world.constructLevel(level_name);

        auto& level = world.getLevel(level_name);
        for (const auto& entity_def : level_data.entities())
        {
            auto entity = level.spawnEntity(entity_def.name());

            if(entity == std::nullopt) {
                spdlog::error("Failed to spawn entity: {}", entity_def.name());
                continue;
            }

            registry.loadComponents(entity_def, *entity, level);
        }

        co_return level_name;
    }

    asio::awaitable<bool> saveLevelToFile(std::string level_name, game::World & world, const ComponentSerializerRegistry & registry, const std::filesystem::path& path)
    {
        game::LevelDefinition level_data;
        level_data.set_name(level_name);

        auto& level = world.getLevel(level_data.name());
        for (const auto& [entity, mask] : level.getEntityManager().getAllEntities())
        {
            auto name_res = level.getName(entity);
            if (name_res == std::nullopt) {
                spdlog::error("Failed to get entity name for serialization");
                continue;
            }
            
            auto entity_def = registry.serializeEntity(level, entity);
            entity_def.set_name(*name_res);

            *level_data.add_entities() = entity_def;
        }

        std::string json_out;
        google::protobuf::util::JsonPrintOptions print_options;
        print_options.add_whitespace = true;  // pretty-print
        print_options.always_print_fields_with_no_presence = true; // ensures e.g. `health: 0` shows
        print_options.preserve_proto_field_names = true; // ensures field names are not converted to camelCase

        auto status = google::protobuf::util::MessageToJsonString(level_data, &json_out, print_options);
        if (!status.ok()) {
            spdlog::error("Failed to serialize level to JSON: {}", status.ToString());
            co_return false;
        }

        std::ofstream out(path);
        if (!out.is_open())
        {
            spdlog::error("Failed to open level file: {}", path.string());
            co_return false;
        }

        out << json_out;

        co_return true;
    }

    asio::awaitable<bool> saveWorldToFiles(game::World & world, const ComponentSerializerRegistry & registry, const std::filesystem::path& save_dir)
    {
        if (!std::filesystem::exists(save_dir)) {
            spdlog::warn("Save directory does not exist: {}. Creating one ...", save_dir.string());
            // create 
            if (!std::filesystem::create_directories(save_dir)) {
                spdlog::error("Failed to create save directory: {}", save_dir.string());
                co_return false;
            }
        }

        for (const auto& level_name : world.getLevelNames()) {
            std::filesystem::path level_path = save_dir / (level_name + "_save.json");
            bool success = co_await saveLevelToFile(level_name, world, registry, level_path);
            if (!success) {
                spdlog::error("Failed to save level: {}", level_name);
                co_return false;
            }
        }

        co_return true;
    }

    void ComponentLoaderRegistry::registerLoader(const std::string& name, ComponentLoader loader) 
    {
        _loaders[name] = std::move(loader);
    }

    void ComponentLoaderRegistry::loadComponents(const game::EntityDefinition & entity_def, Entity entity, game::Level & level) const 
    {
        for (const auto& [name, loader] : _loaders)
        {
            loader(entity_def, entity, level); // only loads if data is present
        }
    }

    void ComponentSerializerRegistry::registerSerializer(const std::string& name, ComponentSerializer serializer) {
        _serializers[name] = std::move(serializer);
    }

    game::EntityDefinition ComponentSerializerRegistry::serializeEntity(const game::Level & level, Entity entity) const {
        game::EntityDefinition entity_def;
        for (const auto& [name, serializer] : _serializers) {
            serializer(level, entity, entity_def);
        }
        return entity_def;
    }

    ComponentLoaderRegistry constructComponentLoaderRegistry()
    {
        ComponentLoaderRegistry components_loader_registry;

        components_loader_registry.registerLoader("TransformComponent", 
        [](const game::EntityDefinition & entity_def, Entity entity, game::Level & level) 
            {
                // add transform component to level from entity_def
                if (!entity_def.has_transform()) return;
                game::TransformComponent t;
                t.CopyFrom(entity_def.transform());
                level.addComponent(entity, std::move(t));
            }
        );

        components_loader_registry.registerLoader("VisualComponent", 
        [](const game::EntityDefinition & entity_def, Entity entity, game::Level & level) 
            {
                if (!entity_def.has_visual()) return;
                game::VisualComponent v;
                v.CopyFrom(entity_def.visual());
                level.addComponent(entity, std::move(v));
            }
        );

        components_loader_registry.registerLoader("InputComponent", 
        [](const game::EntityDefinition & entity_def, Entity entity, game::Level & level) 
            {
                if (!entity_def.has_input()) return;
                game::InputComponent i;
                i.CopyFrom(entity_def.input());
                level.addComponent(entity, std::move(i));
            }
        );

        components_loader_registry.registerLoader("HealthComponent", 
        [](const game::EntityDefinition & entity_def, Entity entity, game::Level & level) 
            {
                if (!entity_def.has_health()) return;
                game::HealthComponent h;
                h.CopyFrom(entity_def.health());
                level.addComponent(entity, std::move(h));
            }
        );

        components_loader_registry.registerLoader("CameraComponent", 
    [](const game::EntityDefinition & entity_def, Entity entity, game::Level & level) 
            {
                if (!entity_def.has_camera()) return;
                game::CameraComponent cam;
                cam.CopyFrom(entity_def.camera());
                level.addComponent(entity, std::move(cam));
            }
        );

        return components_loader_registry;
    }

    ComponentSerializerRegistry constructComponentSerializerRegistry()
    {
        ComponentSerializerRegistry components_serializer_registry;

        components_serializer_registry.registerSerializer("TransformComponent", 
        [](const game::Level & level, Entity entity, game::EntityDefinition & entity_def) 
            {
                // serialize transform component to entity_def
                if (!level.hasComponent<game::TransformComponent>(entity)) return;
                const game::TransformComponent * const t = level.getComponent<game::TransformComponent>(entity);
                entity_def.mutable_transform()->CopyFrom(*t);
            }
        );

        components_serializer_registry.registerSerializer("VisualComponent", 
        [](const game::Level & level, Entity entity, game::EntityDefinition & entity_def) 
            {
                if (!level.hasComponent<game::VisualComponent>(entity)) return;
                const game::VisualComponent * const v = level.getComponent<game::VisualComponent>(entity);
                entity_def.mutable_visual()->CopyFrom(*v);
            }
        );

        components_serializer_registry.registerSerializer("InputComponent", 
        [](const game::Level & level, Entity entity, game::EntityDefinition & entity_def) 
            {
                if (!level.hasComponent<game::InputComponent>(entity)) return;
                const game::InputComponent * const  i = level.getComponent<game::InputComponent>(entity);
                entity_def.mutable_input()->CopyFrom(*i);
            }
        );

        components_serializer_registry.registerSerializer("HealthComponent", 
        [](const game::Level & level, Entity entity, game::EntityDefinition & entity_def) 
            {
                if (!level.hasComponent<game::HealthComponent>(entity)) return;
                const game::HealthComponent * const h = level.getComponent<game::HealthComponent>(entity);
                entity_def.mutable_health()->CopyFrom(*h);
            }
        );

        components_serializer_registry.registerSerializer("CameraComponent", 
            [](const game::Level & level, Entity entity, game::EntityDefinition & entity_def)
            {
                if (!level.hasComponent<game::CameraComponent>(entity)) return;
                const game::CameraComponent * const cam = level.getComponent<game::CameraComponent>(entity);
                entity_def.mutable_camera()->CopyFrom(*cam);
            }
        );

        return components_serializer_registry;
    }

}