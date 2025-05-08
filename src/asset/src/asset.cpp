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

    asio::awaitable<std::optional<std::string>> loadLevelFromFile(game::World & world, const ComponentLoaderRegistry & registry, const std::filesystem::path& path, use_json_t)
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

        spdlog::debug(std::format("Loaded level: {} from {}", level_name, path.string()));

        co_return level_name;
    }

    asio::awaitable<bool> loadWorldFromFiles(game::World & world, const ComponentLoaderRegistry & registry, const std::filesystem::path& load_dir, use_json_t)
    {
        if (!std::filesystem::exists(load_dir)  || !std::filesystem::is_directory(load_dir)) {
            spdlog::error("Load directory does not exist: {}", load_dir.string());
            co_return false;
        }

        spdlog::info("Loading world from {}", load_dir.string());

        for (const std::filesystem::path & level_path : std::filesystem::directory_iterator(load_dir)) {
            auto level_name_res = co_await loadLevelFromFile(world, registry, level_path, use_json);
            if (!level_name_res) {
                spdlog::warn(std::format("Failed to load level from {}", level_path.string()) );
                continue;
            }
        }

        co_return true;
    }

    asio::awaitable<std::optional<std::string>> loadLevelFromFile(game::World & world, const ComponentLoaderRegistry & registry, const std::filesystem::path& path, use_binary_t)
    {
        std::ifstream in(path, std::ios::binary);
        if (!in.is_open()) 
        {
            spdlog::error("Failed to open level file: {}", path.string());
            co_return std::nullopt;
        }

        game::LevelDefinition level_data;
        if (!level_data.ParseFromIstream(&in)) 
        {
            spdlog::error("Failed to parse binary level file: {}", path.string());
            co_return std::nullopt;
        }

        std::string level_name = level_data.name();
        world.constructLevel(level_name);

        auto& level = world.getLevel(level_name);
        for (const auto& entity_def : level_data.entities())
        {
            auto entity = level.spawnEntity(entity_def.name());

            if (entity == std::nullopt) {
                spdlog::error("Failed to spawn entity: {}", entity_def.name());
                continue;
            }

            registry.loadComponents(entity_def, *entity, level);
        }
        
        spdlog::debug(std::format("Loaded level: {} from {}", level_name, path.string()));

        co_return level_name;
    }

    asio::awaitable<bool> loadWorldFromFiles(game::World & world, const ComponentLoaderRegistry & registry, const std::filesystem::path& load_dir, use_binary_t)
    {
        if (!std::filesystem::exists(load_dir)  || !std::filesystem::is_directory(load_dir)) {
            spdlog::error("Load directory does not exist: {}", load_dir.string());
            co_return false;
        }

        spdlog::info("Loading world from {}", load_dir.string());

        for (const std::filesystem::path & level_path : std::filesystem::directory_iterator(load_dir)) {
            auto level_name_res = co_await loadLevelFromFile(world, registry, level_path, use_binary);
            if (!level_name_res) {
                spdlog::warn(std::format("Failed to load level from {}", level_path.string()));
                continue;
            }
        }
    }


    asio::awaitable<bool> saveLevelToFile(std::string level_name, game::World & world, const ComponentSerializerRegistry & registry, const std::filesystem::path& path, use_json_t)
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

        spdlog::debug("Saved level: {} to {}", level_name, path.string());

        out << json_out;

        co_return true;
    }

    asio::awaitable<bool> saveWorldToFiles(game::World & world, const ComponentSerializerRegistry & registry, const std::filesystem::path& save_dir, use_json_t)
    {
        if (!std::filesystem::exists(save_dir)) {
            spdlog::warn("Save directory does not exist: {}. Creating one ...", save_dir.string());
            // create 
            if (!std::filesystem::create_directories(save_dir)) {
                spdlog::error("Failed to create save directory: {}", save_dir.string());
                co_return false;
            }
        }
        else
        {
            if (!std::filesystem::is_directory(save_dir)) {
                spdlog::error("Save directory is not a directory: {}", save_dir.string());
                co_return false;
            }
        }

        spdlog::debug("Saving world to {}", save_dir.string());

        bool success = true;
        for (const auto& level_name : world.getLevelNames()) {
            std::filesystem::path level_path = save_dir / (level_name + "_save.json");
            if ((co_await saveLevelToFile(level_name, world, registry, level_path, use_json)) == false) {
                spdlog::error("Failed to save level: {}", level_name);
                success = false;
                continue;
            }
        }

        co_return success;
    }

    asio::awaitable<bool> saveLevelToFile(std::string level_name, game::World & world, const ComponentSerializerRegistry & registry, const std::filesystem::path& path, use_binary_t)
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

        std::ofstream out(path, std::ios::binary);
        if (!out.is_open())
        {
            spdlog::error("Failed to open level file for writing: {}", path.string());
            co_return false;
        }

        if (!level_data.SerializeToOstream(&out)) 
        {
            spdlog::error("Failed to serialize level data to binary: {}", path.string());
            co_return false;
        }

        spdlog::debug("Saved level: {} to {}", level_name, path.string());

        co_return true;
    }

    asio::awaitable<bool> saveWorldToFiles(game::World & world, const ComponentSerializerRegistry & registry, const std::filesystem::path& save_dir, use_binary_t)
    {
        if (!std::filesystem::exists(save_dir)) {
            spdlog::warn("Save directory does not exist: {}. Creating one ...", save_dir.string());
            // create 
            if (!std::filesystem::create_directories(save_dir)) {
                spdlog::error("Failed to create save directory: {}", save_dir.string());
                co_return false;
            }
        }
        else
        {
            if (!std::filesystem::is_directory(save_dir)) {
                spdlog::error("Save directory is not a directory: {}", save_dir.string());
                co_return false;
            }
        }

        spdlog::debug("Saving world to {}", save_dir.string());

        bool success = true;
        for (const auto& level_name : world.getLevelNames()) {
            std::filesystem::path level_path = save_dir / (level_name + "_save.bin");
            if ((co_await saveLevelToFile(level_name, world, registry, level_path, use_binary)) == false) 
            {
                spdlog::error("Failed to save level: {}", level_name);
                success = false;
                continue;
            }
        }

        co_return success;
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

    template<class ComponentType>
    ComponentLoader constructComponentLoader(
            std::function<bool(const game::EntityDefinition*)> has_component,
            std::function<const ComponentType &(const game::EntityDefinition*)> get_component) 
    {
        return 
            [has_component, get_component]
            (const game::EntityDefinition & entity_def, Entity entity, game::Level & level) 
            {
                if (!has_component(&entity_def)) return;
                ComponentType c;
                c.CopyFrom(get_component(&entity_def));
                level.addComponent(entity, std::move(c));
            };
    }

    ComponentLoaderRegistry constructComponentLoaderRegistry()
    {
        ComponentLoaderRegistry components_loader_registry;

        components_loader_registry.registerLoader("TransformComponent", 
            constructComponentLoader<game::TransformComponent>(
                &game::EntityDefinition::has_transform, &game::EntityDefinition::transform)
        );

        components_loader_registry.registerLoader("VisualComponent",
            constructComponentLoader<game::VisualComponent>(
                &game::EntityDefinition::has_visual, &game::EntityDefinition::visual)
        );

        components_loader_registry.registerLoader("InputComponent",
            constructComponentLoader<game::InputComponent>(
                &game::EntityDefinition::has_input, &game::EntityDefinition::input)
        );

        components_loader_registry.registerLoader("HealthComponent", 
            constructComponentLoader<game::HealthComponent>(
                &game::EntityDefinition::has_health, &game::EntityDefinition::health)
        );

        components_loader_registry.registerLoader("CameraComponent", 
            constructComponentLoader<game::CameraComponent>(
                &game::EntityDefinition::has_camera, &game::EntityDefinition::camera)
        );

        components_loader_registry.registerLoader("TerrainComponent", 
            constructComponentLoader<game::TerrainComponent>(
                &game::EntityDefinition::has_terrain, &game::EntityDefinition::terrain)
        );

        components_loader_registry.registerLoader("LightComponent", 
            constructComponentLoader<game::LightComponent>(
                &game::EntityDefinition::has_light, &game::EntityDefinition::light)
        );

        return components_loader_registry;
    }

    template<class ComponentType>
    ComponentSerializer constructComponentSerializer(std::function<ComponentType *(game::EntityDefinition *)> mutable_component)
    {
        return 
            [mutable_component]
            (const game::Level & level, Entity entity, game::EntityDefinition & entity_def) 
            {
                if (!level.hasComponent<ComponentType>(entity)) return;
                const ComponentType * const c = level.getComponent<ComponentType>(entity);
                mutable_component(&entity_def)->CopyFrom(*c);
            };
    }

    ComponentSerializerRegistry constructComponentSerializerRegistry()
    {
        ComponentSerializerRegistry components_serializer_registry;

        components_serializer_registry.registerSerializer("TransformComponent",
            constructComponentSerializer<game::TransformComponent>(&game::EntityDefinition::mutable_transform) 
        );

        components_serializer_registry.registerSerializer("VisualComponent", 
            constructComponentSerializer<game::VisualComponent>(&game::EntityDefinition::mutable_visual)
        );

        components_serializer_registry.registerSerializer("InputComponent",
            constructComponentSerializer<game::InputComponent>(&game::EntityDefinition::mutable_input)
        );

        components_serializer_registry.registerSerializer("HealthComponent",
            constructComponentSerializer<game::HealthComponent>(&game::EntityDefinition::mutable_health)
        );

        components_serializer_registry.registerSerializer("CameraComponent", 
            constructComponentSerializer<game::CameraComponent>(&game::EntityDefinition::mutable_camera)
        );

        components_serializer_registry.registerSerializer("TerrainComponent", 
            constructComponentSerializer<game::TerrainComponent>(&game::EntityDefinition::mutable_terrain)
        );

        components_serializer_registry.registerSerializer("LightComponent", 
            constructComponentSerializer<game::LightComponent>(&game::EntityDefinition::mutable_light)
        );

        return components_serializer_registry;
    }

}