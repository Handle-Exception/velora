#include "asset.hpp"

namespace velora
{
    asio::awaitable<std::optional<std::size_t>> loadShader(IRenderer & renderer, std::filesystem::path shader_dir)
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
}