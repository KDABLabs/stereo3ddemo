#include "mesh_loader.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <Serenity/gui/render/mesh.h>
#include <fmt/format.h>

static Serenity::VertexFormat MakeVertexFormat(const aiMaterial& material)
{
    bool has_diffuse = material.GetTextureCount(aiTextureType::aiTextureType_DIFFUSE) > 0;

    Serenity::VertexFormat vertex_format;
    vertex_format.attributes.emplace_back(KDGpu::VertexAttribute{ 0, 0, KDGpu::Format::R32G32B32_SFLOAT, 0 }); // position
    vertex_format.buffers.emplace_back(KDGpu::VertexBufferLayout{ 0, 12 });
    vertex_format.attributes.emplace_back(KDGpu::VertexAttribute{ 1, 1, KDGpu::Format::R32G32B32_SFLOAT, 0 }); // normal
    vertex_format.buffers.emplace_back(KDGpu::VertexBufferLayout{ 1, 12 });

    if (has_diffuse) {
        vertex_format.attributes.emplace_back(KDGpu::VertexAttribute{ 2, 2, KDGpu::Format::R32G32B32_SFLOAT, 0 }); // uv
        vertex_format.buffers.emplace_back(KDGpu::VertexBufferLayout{ 2, 12 });
    }

    return vertex_format;
}

std::unique_ptr<Serenity::Entity> all::serenity::MeshLoader::load(std::filesystem::path path)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path.string(),
                                             aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices | aiProcess_PreTransformVertices);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        throw std::runtime_error(fmt::format("Failed to load mesh: {}", importer.GetErrorString()));
    }

    std::unique_ptr<Serenity::Entity> pRoot{ std::make_unique<Serenity::Entity>() };

    for (size_t i = 0; i < scene->mNumMeshes; i++) {
        const auto& mesh = *scene->mMeshes[i];
        auto material = scene->mMaterials[mesh.mMaterialIndex];
        auto m = MakeMaterial(*material, path);

        auto smesh = MakeMesh(mesh, *material);
        auto renderer = pRoot->createComponent<Serenity::MeshRenderer>();
        renderer->mesh = smesh.get();
        renderer->material = m.get();

        pRoot->addChild(std::move(smesh));
        pRoot->addChild(std::move(m));

        auto bv = pRoot->createComponent<Serenity::TriangleBoundingVolume>();
        bv->meshRenderer = renderer;
        bv->cacheTriangles = true;
        bv->cullBackFaces = false;
    }

    return pRoot;
}

std::vector<uint32_t> GetIndices(const aiMesh& mesh)
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

std::unique_ptr<Serenity::Mesh> all::serenity::MeshLoader::MakeMesh(const aiMesh& mesh, const aiMaterial& material)
{
    std::unique_ptr<Serenity::Mesh> smesh = std::make_unique<Serenity::Mesh>();
    auto vertex_format = MakeVertexFormat(material);
    smesh->setVertexFormat(vertex_format);

    std::vector<Serenity::Mesh::VertexBufferData> verts;
    verts.resize(vertex_format.buffers.size());

    for (size_t i = 0; i < verts.size(); i++) {
        verts[i].resize(mesh.mNumVertices * vertex_format.buffers[i].stride);
        switch (vertex_format.attributes[i].binding) {
        case 0: // position
            std::ranges::copy_n((uint8_t*)mesh.mVertices, mesh.mNumVertices * sizeof(aiVector3D), verts[i].data());
            break;
        case 1: // normal
            std::ranges::copy_n((uint8_t*)mesh.mNormals, mesh.mNumVertices * sizeof(aiVector3D), verts[i].data());
            break;
        case 2: // uv
            std::ranges::copy_n((uint8_t*)mesh.mTextureCoords[0], mesh.mNumVertices * sizeof(aiVector3D), verts[i].data());
            break;
        case 3: // tangent
            std::ranges::copy_n((uint8_t*)mesh.mTangents, mesh.mNumVertices * sizeof(aiVector3D), verts[i].data());
            break;
        case 4: // bitangent
            std::ranges::copy_n((uint8_t*)mesh.mBitangents, mesh.mNumVertices * sizeof(aiVector3D), verts[i].data());
            break;
        default:
            break;
        }
    }

    smesh->setVertices(std::move(verts));
    smesh->setIndices(GetIndices(mesh));

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
    const auto root_path = model_path.parent_path().string() + "/";
    std::unique_ptr<Serenity::Material> m = std::make_unique<Serenity::Material>();
    std::string shader_code{ "multiview" };

    aiString tex_filename;
    bool has_diffuse = material.GetTextureCount(aiTextureType::aiTextureType_DIFFUSE) > 0;

    if (material.GetTexture(aiTextureType_DIFFUSE, 0, &tex_filename) == aiReturn_SUCCESS) {
        auto texture = m->createChild<Serenity::Texture2D>();
        texture->setPath(root_path + tex_filename.C_Str());
        m->setTexture(4, 0, texture);
        shader_code += ".dif";
    }
    //// specular
    // if (material.GetTexture(aiTextureType_SPECULAR, 0, &tex_filename) == aiReturn_SUCCESS) {
    //     auto texture = m->createChild<Serenity::Texture2D>();
    //     texture->setPath(root_path + tex_filename.C_Str());
    //     m->setTexture(5, 0, texture);
    //     shader_code += ".spc";
    // }
    //// normal
    // if (material.GetTexture(aiTextureType_NORMALS, 0, &tex_filename) == aiReturn_SUCCESS) {
    //     auto texture = m->createChild<Serenity::Texture2D>();
    //     texture->setPath(root_path + tex_filename.C_Str());
    //     m->setTexture(6, 0, texture);
    //     shader_code += ".nrm";
    // }

    auto sp = MakeShaderProgram(shader_code);
    auto spref = static_cast<Serenity::SpirVShaderProgram*>(m->addChild(std::move(sp)));
    m->shaderProgram = spref;

    aiColor3D diffuse = { 0.45f, 0.45f, 0.85f };
    material.Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);

    aiColor3D specular = { 0.18f, 0.18f, 0.18f };
    material.Get(AI_MATKEY_COLOR_SPECULAR, specular);

    float gloss = 8.0f;
    material.Get(AI_MATKEY_SHININESS, gloss);

    struct PhongData {
        glm::vec4 ambient;
        glm::vec4 diffuse;
        glm::vec4 specular;
        float shininess;
        int useTexture = true;
        float _pad[2];
    };

    const Serenity::Material::UboDataBuilder materialDataBuilder = {
        [=](uint32_t set, uint32_t binding) {
            const PhongData data{
                { 0.4f, 0.4f, 0.4f, 1.0f },
                { diffuse[0],
                  diffuse[1],
                  diffuse[2],
                  1.0f },
                { specular[0],
                  specular[1],
                  specular[2],
                  1.0f },
                gloss,
                has_diffuse,
                { 0.0f, 0.0f }
            };
            std::vector<uint8_t> rawData(sizeof(PhongData));
            std::memcpy(rawData.data(), &data, sizeof(PhongData));
            return rawData;
        },
    };

    Serenity::StaticUniformBuffer* phongUbo = m->createChild<Serenity::StaticUniformBuffer>();
    phongUbo->size = sizeof(PhongData);

    m->setUniformBuffer(3, 0, phongUbo);

    // This is how we feed Material properties
    m->setUniformBufferDataBuilder(materialDataBuilder);
    return m;
}
