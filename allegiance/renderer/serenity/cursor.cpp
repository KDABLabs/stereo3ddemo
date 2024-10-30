#include "cursor.h"
#include <ranges>
#include <algorithm>

constexpr std::array<glm::vec3, 24> cross_vertices(float width, float height, float depth)
{
    // divide cross into 3 parts
    float w = width / 2;
    float h = height / 2;
    float d = depth / 2;

    // one part
    std::array<glm::vec3, 8> part = {
        // front
        glm::vec3(-w, -h, -d),
        glm::vec3(w, -h, -d),
        glm::vec3(w, h, -d),
        glm::vec3(-w, h, -d),

        // back
        glm::vec3(-w, -h, d),
        glm::vec3(w, -h, d),
        glm::vec3(w, h, d),
        glm::vec3(-w, h, d),
    };

    // rotated part 90 degrees around y axis
    std::array<glm::vec3, 8> part_rotated = {
        // front
        glm::vec3(-d, -h, -w),
        glm::vec3(d, -h, -w),
        glm::vec3(d, h, -w),
        glm::vec3(-d, h, -w),

        // back
        glm::vec3(-d, -h, w),
        glm::vec3(d, -h, w),
        glm::vec3(d, h, w),
        glm::vec3(-d, h, w),
    };

    // rotate part 90 degrees around z axis
    std::array<glm::vec3, 8> part_rotated_z = {
        // front
        glm::vec3(-h, -w, -d),
        glm::vec3(h, -w, -d),
        glm::vec3(h, w, -d),
        glm::vec3(-h, w, -d),

        // back
        glm::vec3(-h, -w, d),
        glm::vec3(h, -w, d),
        glm::vec3(h, w, d),
        glm::vec3(-h, w, d),
    };

    // combine all parts
    std::array<glm::vec3, 24> vertices;
    std::copy(part.begin(), part.end(), vertices.begin());
    std::copy(part_rotated.begin(), part_rotated.end(), vertices.begin() + 8);
    std::copy(part_rotated_z.begin(), part_rotated_z.end(), vertices.begin() + 16);

    return vertices;
}

constexpr std::array<uint32_t, 3 * 36> cross_indices()
{
    // 3 parts, 8 vertices each parallelepiped
    // first part
    // clang-format off
    std::array indices = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        0, 4, 7, 7, 3, 0,
        1, 5, 6, 6, 2, 1,
        0, 1, 5, 5, 4, 0,
        2, 3, 7, 7, 6, 2,
    };
    // clang-format on

    // second part
    std::array indices_rotated = indices;
    for (auto& i : indices_rotated) {
        i += 8;
    }

    // third part
    std::array indices_rotated_x = indices;
    for (auto& i : indices_rotated_x) {
        i += 16;
    }

    std::array<uint32_t, 3 * 36> all_indices;
    std::copy(indices.begin(), indices.end(), all_indices.begin());
    std::copy(indices_rotated.begin(), indices_rotated.end(), all_indices.begin() + indices.size());
    std::copy(indices_rotated_x.begin(), indices_rotated_x.end(), all_indices.begin() + 2 * indices.size());

    return all_indices;
}

all::serenity::Cursor::Cursor(Serenity::LayerManager& layers)
{
    m_cursors.push_back(std::make_unique<BallCursor>(layers));
    m_cursors.push_back(std::make_unique<CrossCursor>(layers));

    m_currentCursor = (CursorBase*)m_cursors[0].get();
    SetColor(m_colorData);
}

void all::serenity::BallCursor::MakeBall(Serenity::Entity* ec, Serenity::LayerManager& layers)
{
    auto shader_ball = ec->createChild<Serenity::SpirVShaderProgram>();
    shader_ball->vertexShader = SHADER_DIR "color.vert.spv";
    shader_ball->fragmentShader = SHADER_DIR "color.frag.spv";

    m_ball_mesh = std::make_unique<Serenity::Mesh>();
    m_ball_mesh->setObjectName("Cursor Mesh");
    Serenity::MeshGenerators::sphereGenerator(m_ball_mesh.get(), 24, 24, 1.0f);

    Serenity::StaticUniformBuffer* cbuf = ec->createChild<Serenity::StaticUniformBuffer>();

    cbuf->size = sizeof(ColorData);
    m_cbuf = cbuf;

    Serenity::Material* material = ec->createChild<Serenity::Material>();
    material->shaderProgram = shader_ball;
    material->setUniformBuffer(3, 0, cbuf);

    auto cmodel = ec->createComponent<Serenity::MeshRenderer>();
    cmodel->mesh = m_ball_mesh.get();
    cmodel->material = material;
    ec->layerMask = layers.layerMask({ "Opaque" });
}

void all::serenity::BallCursor::MakeBillboard(Serenity::Entity* ec, Serenity::LayerManager& layers)
{
    auto shader_bb = ec->createChild<Serenity::SpirVShaderProgram>();
    shader_bb->vertexShader = SHADER_DIR "billboard.vert.spv";
    shader_bb->fragmentShader = SHADER_DIR "billboard.frag.spv";

    m_bb_mesh = std::make_unique<Serenity::Mesh>();
    m_bb_mesh->setObjectName("Billboard Mesh");
    Serenity::MeshGenerators::planeGenerator(m_bb_mesh.get(), 8, 8, { 2, 2 },
                                             { { 1, 0, 0, 0 },
                                               { 0, 0, -1, 0 },
                                               { 0, 1, 0, 0 },
                                               { 0, 0, 0, 1 } });

    Serenity::Material* material = ec->createChild<Serenity::Material>();
    material->shaderProgram = shader_bb;

    m_texture = std::make_unique<Serenity::Texture2D>();
    m_texture->setObjectName("Model Texture");
    m_texture->setPath("assets/cursor_billboard.png");
    material->setTexture(4, 0, m_texture.get());

    Serenity::StaticUniformBuffer* cbuf = ec->createChild<Serenity::StaticUniformBuffer>();
    cbuf->size = sizeof(ColorData);
    m_bb_cbuf = cbuf;
    material->setUniformBuffer(3, 0, cbuf);

    auto cmodel = ec->createComponent<Serenity::MeshRenderer>();
    cmodel->mesh = m_bb_mesh.get();
    cmodel->material = material;

    ec->layerMask = layers.layerMask({ "Alpha" });
}

void all::serenity::BallCursor::SetColor(const ColorData& colorData)
{
    m_cbuf->data = std::vector<uint8_t>{ (uint8_t*)&colorData, (uint8_t*)&colorData + sizeof(colorData) };
    m_bb_cbuf->data = std::vector<uint8_t>{ (uint8_t*)&colorData, (uint8_t*)&colorData + sizeof(colorData) };
}

void all::serenity::CrossCursor::SetColor(const ColorData& colorData)
{
    m_cbuf->data = std::vector<uint8_t>{ (uint8_t*)&colorData, (uint8_t*)&colorData + sizeof(colorData) };
}

void all::serenity::CrossCursor::MakeCross(Serenity::Entity* ec, Serenity::LayerManager& layers)
{
    auto shader_ball = ec->createChild<Serenity::SpirVShaderProgram>();
    shader_ball->vertexShader = SHADER_DIR "color.vert.spv";
    shader_ball->fragmentShader = SHADER_DIR "color.frag.spv";

    constexpr auto vertices = cross_vertices(6, 0.2f, 0.2f);
    constexpr auto indices = cross_indices();
    Serenity::Mesh::VertexBufferData vertexBufferData;
    vertexBufferData.resize(sizeof(vertices));
    std::memcpy(vertexBufferData.data(), vertices.data(), sizeof(vertices));

    std::vector<uint32_t> vindices;
    vindices.resize(indices.size());
    std::memcpy(vindices.data(), indices.data(), sizeof(indices));

    m_cross_mesh = std::make_unique<Serenity::Mesh>();
    m_cross_mesh->setObjectName("Cursor Cross Mesh");
    m_cross_mesh->setVertices({ vertexBufferData });
    m_cross_mesh->setIndices(std::move(vindices));
    m_cross_mesh->vertexFormat = Serenity::VertexFormat{
        .buffers = {
                KDGpu::VertexBufferLayout{
                        .binding = 0,
                        .stride = sizeof(glm::vec3),
                } },
        .attributes = { KDGpu::VertexAttribute{
                .location = 0,
                .binding = 0,
                .format = KDGpu::Format::R32G32B32_SFLOAT,
                .offset = 0,
        } },
    };

    Serenity::StaticUniformBuffer* cbuf = ec->createChild<Serenity::StaticUniformBuffer>();
    cbuf->size = sizeof(ColorData);
    m_cbuf = cbuf;

    Serenity::Material* material = ec->createChild<Serenity::Material>();
    material->shaderProgram = shader_ball;
    material->setUniformBuffer(3, 0, cbuf);

    auto cmodel = ec->createComponent<Serenity::MeshRenderer>();
    cmodel->mesh = m_cross_mesh.get();
    cmodel->material = material;
    ec->layerMask = layers.layerMask({ "Opaque" });
}
