#pragma once
#include <unordered_map>
#include <filesystem>
#include <memory>

struct aiMesh;
struct aiMaterial;

namespace Serenity {
class Entity;
class Mesh;
class Material;
class LayerManager;
} // namespace Serenity

namespace all::serenity {
class MeshLoader
{
public:
    static std::unique_ptr<Serenity::Entity> load(std::filesystem::path path, Serenity::LayerManager* layerManager);
    static std::unique_ptr<Serenity::Mesh> MakeMesh(const aiMesh& mesh, const glm::mat4& transform);
    static std::unique_ptr<Serenity::Material> MakeMaterial(const aiMaterial& mesh, const std::filesystem::path& model_path);
};
} // namespace all::serenity
