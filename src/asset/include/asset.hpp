#pragma once

#include <utility>
#include <functional>
#include <filesystem>
#include <fstream>
#include <string>

#include <absl/container/flat_hash_map.h>
#include <google/protobuf/util/json_util.h>

#include "native.hpp"
#include <asio.hpp>
#include "entry.hpp"
#include "render.hpp"
#include "ecs.hpp"
#include "entity_definition.pb.h"
#include "level_definition.pb.h"

#include "world.hpp"

namespace velora
{
    using ComponentLoader = std::function<void(const game::EntityDefinition &, Entity, game::Level &)>;
    using ComponentSerializer = std::function<void(const game::Level &, Entity, game::EntityDefinition &)>;

    class ComponentLoaderRegistry 
    {
    public:
        ComponentLoaderRegistry() = default;
        ComponentLoaderRegistry(ComponentLoaderRegistry && other) = default;
        ComponentLoaderRegistry& operator=(ComponentLoaderRegistry && other) = default;
        ComponentLoaderRegistry(const ComponentLoaderRegistry &) = delete;
        ComponentLoaderRegistry& operator=(const ComponentLoaderRegistry &) = delete;

        void registerLoader(const std::string& name, ComponentLoader loader);

        void loadComponents(const game::EntityDefinition & entity_def, Entity entity, game::Level & level) const;

    private:
        absl::flat_hash_map<std::string, ComponentLoader> _loaders;
    };

    class ComponentSerializerRegistry 
    {
    public:
        ComponentSerializerRegistry() = default;
        ComponentSerializerRegistry(ComponentSerializerRegistry && other) = default;
        ComponentSerializerRegistry& operator=(ComponentSerializerRegistry && other) = default;
        ComponentSerializerRegistry(const ComponentSerializerRegistry &) = delete;
        ComponentSerializerRegistry& operator=(const ComponentSerializerRegistry &) = delete;

        void registerSerializer(const std::string& name, ComponentSerializer serializer);

        game::EntityDefinition serializeEntity(const game::Level & level, Entity entity) const;

    private:
        absl::flat_hash_map<std::string, ComponentSerializer> _serializers;
    };

    ComponentLoaderRegistry constructComponentLoaderRegistry();
    ComponentSerializerRegistry constructComponentSerializerRegistry();

    asio::awaitable<std::optional<std::size_t>> loadShaderFromFile(IRenderer & renderer, std::filesystem::path shader_dir);

    asio::awaitable<std::optional<std::string>> loadLevelFromFile(game::World & world, const ComponentLoaderRegistry & registry, const std::filesystem::path& path);

    asio::awaitable<bool> saveLevelToFile(std::string level_name, game::World & world, const ComponentSerializerRegistry & registry, const std::filesystem::path& path);

    asio::awaitable<bool> saveWorldToFiles(game::World & world, const ComponentSerializerRegistry & registry, const std::filesystem::path& save_dir);

}
