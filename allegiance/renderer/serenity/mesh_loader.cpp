#include "mesh_loader.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <Serenity/gui/render/mesh.h>
#include <fmt/format.h>
#include <glm/gtc/matrix_access.hpp>
#include <algorithm>

namespace {

glm::mat4 toMatrix4x4(const aiMatrix4x4& matrix)
{
    return glm::mat4(
            matrix.a1, matrix.b1, matrix.c1, matrix.d1,
            matrix.a2, matrix.b2, matrix.c2, matrix.d2,
            matrix.a3, matrix.b3, matrix.c3, matrix.d3,
            matrix.a4, matrix.b4, matrix.c4, matrix.d4);
}

[[nodiscard]] std::string toLowerCase(std::string str)
{
    std::transform(str.begin(),
                   str.end(),
                   str.begin(), [](unsigned char c) -> unsigned char { return std::tolower(c); });
    return str;
}

} // namespace

static Serenity::VertexFormat MakeVertexFormat()
{
    Serenity::VertexFormat vertex_format;
    vertex_format.attributes.emplace_back(KDGpu::VertexAttribute{ 0, 0, KDGpu::Format::R32G32B32_SFLOAT, 0 }); // position
    vertex_format.buffers.emplace_back(KDGpu::VertexBufferLayout{ 0, 12 });
    vertex_format.attributes.emplace_back(KDGpu::VertexAttribute{ 1, 1, KDGpu::Format::R32G32B32_SFLOAT, 0 }); // normal
    vertex_format.buffers.emplace_back(KDGpu::VertexBufferLayout{ 1, 12 });
    vertex_format.attributes.emplace_back(KDGpu::VertexAttribute{ 2, 2, KDGpu::Format::R32G32B32_SFLOAT, 0 }); // uv
    vertex_format.buffers.emplace_back(KDGpu::VertexBufferLayout{ 2, 12 });

    return vertex_format;
}

std::unique_ptr<Serenity::Entity> all::serenity::MeshLoader::load(std::filesystem::path path, Serenity::LayerManager* layerManager)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path.string(),
                                             aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        throw std::runtime_error(fmt::format("Failed to load mesh: {}", importer.GetErrorString()));
    }

    std::unique_ptr<Serenity::Entity> pRoot{ std::make_unique<Serenity::Entity>() };

    std::function<void(const aiNode*, Serenity::Entity*, const glm::mat4&)> processMeshesForNode =
            [&](const aiNode* node, Serenity::Entity* root, const glm::mat4& transform) {
                const size_t nodeMeshCount = node->mNumMeshes;
                const glm::mat4 worldTransform = transform * toMatrix4x4(node->mTransformation);

                if (nodeMeshCount > 0) {
                    Serenity::Entity* e = root->createChildEntity<Serenity::Entity>();

                    for (size_t i = 0; i < nodeMeshCount; i++) {
                        const auto& mesh = *scene->mMeshes[node->mMeshes[i]];
                        const auto material = scene->mMaterials[mesh.mMaterialIndex];

                        auto m = MakeMaterial(*material, path);
                        auto smesh = MakeMesh(mesh, worldTransform);
                        auto renderer = e->createComponent<Serenity::MeshRenderer>();
                        renderer->mesh = smesh.get();
                        renderer->material = m.get();

                        pRoot->addChild(std::move(smesh));
                        pRoot->addChild(std::move(m));

                        auto bv = e->createComponent<Serenity::TriangleBoundingVolume>();
                        bv->meshRenderer = renderer;
                        bv->cacheTriangles = true;
                        bv->cullBackFaces = false;

                        float opacity = 1.0f;
                        material->Get(AI_MATKEY_OPACITY, opacity);
                        if (opacity < 1.0f) {
                            e->layerMask = layerManager->layerMask({ "Alpha" });
                        } else {
                            e->layerMask = layerManager->layerMask({ "Opaque" });
                        }
                    }
                }

                for (std::size_t i = 0; i < node->mNumChildren; ++i) {
                    const aiNode* childNode = node->mChildren[i];
                    processMeshesForNode(childNode, root, worldTransform);
                }
            };

    processMeshesForNode(scene->mRootNode, pRoot.get(), glm::mat4(1.0f));

    return pRoot;
}

std::vector<uint32_t> indices(const aiMesh& mesh)
{
    std::vector<uint32_t> indices;
    indices.reserve(mesh.mNumFaces * 3);
    for (size_t i = 0; i < mesh.mNumFaces; i++) {
        const auto& face = mesh.mFaces[i];
        for (size_t j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }
    return indices;
}

std::unique_ptr<Serenity::Mesh> all::serenity::MeshLoader::MakeMesh(const aiMesh& mesh, const glm::mat4& transform)
{
    std::unique_ptr<Serenity::Mesh> smesh = std::make_unique<Serenity::Mesh>();
    auto vertex_format = MakeVertexFormat();
    smesh->vertexFormat = vertex_format;

    std::vector<Serenity::Mesh::VertexBufferData> verts;
    verts.resize(vertex_format.buffers.size());

    for (size_t i = 0; i < verts.size(); i++) {
        verts[i].resize(mesh.mNumVertices * vertex_format.buffers[i].stride);
        switch (vertex_format.attributes[i].binding) {
        case 0: // position
        {
            glm::vec3* pos = reinterpret_cast<glm::vec3*>(verts[i].data());
            for (size_t i = 0, mP = mesh.mNumVertices; i < mP; ++i) {
                const aiVector3D v = mesh.mVertices[i];
                const glm::vec3 transformed = transform * glm::vec4(v[0], v[1], v[2], 1.0f);
                pos[i] = glm::vec3(transformed.x, transformed.y, transformed.z);
            }
            break;
        }
        case 1: // normal
        {
            const glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(transform)));
            glm::vec3* norms = reinterpret_cast<glm::vec3*>(verts[i].data());
            for (size_t i = 0, mN = mesh.mNumVertices; i < mN; ++i) {
                const aiVector3D n = mesh.mNormals[i];
                norms[i] = normalMatrix * glm::vec3(n[0], n[1], n[2]);
            }
            break;
        }
        case 2: // uv
        {
            if (mesh.HasTextureCoords(0))
                std::ranges::copy_n((uint8_t*)mesh.mTextureCoords[0], mesh.mNumVertices * sizeof(aiVector3D), verts[i].data());
            break;
        }
        case 3: // tangent
        {
            std::ranges::copy_n((uint8_t*)mesh.mTangents, mesh.mNumVertices * sizeof(aiVector3D), verts[i].data());
            break;
        }
        case 4: // bitangent
        {
            std::ranges::copy_n((uint8_t*)mesh.mBitangents, mesh.mNumVertices * sizeof(aiVector3D), verts[i].data());
            break;
        }
        default:
            break;
        }
    }

    smesh->setVertices(std::move(verts));
    smesh->setIndices(indices(mesh));

    return smesh;
}

std::unique_ptr<Serenity::SpirVShaderProgram> MakeShaderProgram(std::string shader_code)
{
    std::unique_ptr<Serenity::SpirVShaderProgram> program = std::make_unique<Serenity::SpirVShaderProgram>();
    program->vertexShader = fmt::format(SHADER_DIR "{}.vert.spv", shader_code);
    program->fragmentShader = fmt::format(SHADER_DIR "{}.frag.spv", shader_code);
    return program;
}

std::unique_ptr<Serenity::Material> all::serenity::MeshLoader::MakeMaterial(const aiMaterial& material, const std::filesystem::path& model_path)
{
    std::unique_ptr<Serenity::Material> m = std::make_unique<Serenity::Material>();

    const bool hasDiffuseTexture = material.GetTextureCount(aiTextureType::aiTextureType_DIFFUSE) > 0;

    aiString tex_filename;
    std::string shader_code{ "multiview." };
    if (hasDiffuseTexture && material.GetTexture(aiTextureType_DIFFUSE, 0, &tex_filename) == aiReturn_SUCCESS) {
        shader_code += "phong.texture";

        auto texture = m->createChild<Serenity::Texture2D>();
        const auto root_path = model_path.parent_path().string() + "/";
        texture->setPath(root_path + tex_filename.C_Str());
        m->setTexture(4, 0, texture);
    } else {
        shader_code += "phong";
    }

    auto sp = MakeShaderProgram(shader_code);
    auto spref = static_cast<Serenity::SpirVShaderProgram*>(m->addChild(std::move(sp)));
    m->shaderProgram = spref;

    aiColor3D ambient = { 0.05f, 0.05f, 0.05f };
    material.Get(AI_MATKEY_COLOR_AMBIENT, ambient);
    ambient = ambient * 0.05f; // Limit ambient contributions

    aiColor3D diffuse = { 0.45f, 0.45f, 0.85f };
    material.Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);

    aiColor3D specular = { 0.18f, 0.18f, 0.18f };
    material.Get(AI_MATKEY_COLOR_SPECULAR, specular);

    float shininess = 0.2f;
    material.Get(AI_MATKEY_SHININESS, shininess);

    struct PhongData {
        glm::vec4 ambient;
        glm::vec4 diffuse;
        glm::vec4 specular;
        float shininess;
        int useTexture = true;
        float _pad[2];
    };

    const PhongData data{
        { ambient[0], ambient[1], ambient[2], 1.0f },
        { diffuse[0],
          diffuse[1],
          diffuse[2],
          1.0f },
        { specular[0],
          specular[1],
          specular[2],
          1.0f },
        shininess,
        hasDiffuseTexture,
        { 0.0f, 0.0f }
    };

    Serenity::StaticUniformBuffer* phongUbo = m->createChild<Serenity::StaticUniformBuffer>();
    phongUbo->size = sizeof(PhongData);

    std::vector<uint8_t> rawData(sizeof(PhongData));
    std::memcpy(rawData.data(), &data, sizeof(PhongData));
    phongUbo->data = rawData;

    m->setUniformBuffer(3, 0, phongUbo);

    return m;
}
