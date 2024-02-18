#pragma once
#include <unordered_map>
#include <filesystem>

struct aiMesh;
struct aiMaterial;

namespace all {
class MeshLoader
{
public:
    static std::unique_ptr<Serenity::Entity> load(std::filesystem::path path);
    static std::unique_ptr<Serenity::Mesh> MakeMesh(const aiMesh& mesh, const aiMaterial& material);
    static std::unique_ptr<Serenity::Material> MakeMaterial(const aiMaterial& mesh, const std::filesystem::path& model_path);
};
}