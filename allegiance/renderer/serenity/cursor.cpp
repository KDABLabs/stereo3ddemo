#include "cursor.h"
#include <ranges>
#include <algorithm>
#include "serenity_window.h"

#include <Serenity/core/ecs/camera.h>
#include <Serenity/core/ecs/layer_manager.h>

namespace all::serenity {

namespace {

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

} // namespace

Cursor::Cursor(const Serenity::LayerManager* layers, SerenityWindow* window)
    : Serenity::Entity()
    , m_window(window)
{
    m_transform = createComponent<Serenity::SrtTransform>();

    m_cross = createChildEntity<CrossCursor>(layers);
    m_sphere = createChildEntity<BallCursor>(layers);
    m_billboard = createChildEntity<BillboardCursor>(layers);

    camera.valueChanged().connect([this](Serenity::StereoCamera* c) {
                             m_projectionChangedConnection.disconnect();
                             m_viewChangedConnection.disconnect();
                             if (c != nullptr) {
                                 m_viewChangedConnection = c->viewMatrix.valueChanged().connect([this] { updateSize(); });
                                 m_projectionChangedConnection = c->lens()->projectionMatrix.valueChanged().connect([this] { updateSize(); });
                             }
                         })
            .release();

    type.valueChanged().connect([this](all::CursorType type) { applyType(type); }).release();
    color.valueChanged().connect([this](const ColorData& colorData) { applyColor(colorData); }).release();
    scalingEnabled.valueChanged().connect([this] { updateSize(); }).release();
    scaleFactor.valueChanged().connect([this] { updateSize(); }).release();

    applyColor(color());
    applyType(type());
}

void Cursor::setPosition(const glm::vec3& worldPosition)
{
    if (locked.get())
        return;

    m_transform->translation = worldPosition;
    updateSize();
}

glm::vec3 Cursor::position() const
{
    return m_transform->translation();
}

void Cursor::applyType(all::CursorType type)
{
    switch (type) {
    default:
    case CursorType::Ball:
        m_billboard->texture = BillboardCursor::CursorTexture::Default;
        m_billboard->enabled = true;
        m_sphere->enabled = true;
        m_cross->enabled = false;
        break;
    case CursorType::Cross:
        m_billboard->enabled = false;
        m_sphere->enabled = false;
        m_cross->enabled = true;
        break;
    case CursorType::CrossHair:
        m_billboard->texture = BillboardCursor::CursorTexture::CrossHair;
        m_billboard->enabled = true;
        m_sphere->enabled = false;
        m_cross->enabled = false;
        break;
    case CursorType::Dot:
        m_billboard->texture = BillboardCursor::CursorTexture::Dot;
        m_billboard->enabled = true;
        m_sphere->enabled = false;
        m_cross->enabled = false;
        break;
    }
}

void Cursor::applyColor(const ColorData& colorData)
{
    m_sphere->setColor(colorData);
    m_cross->setColor(colorData);
    m_billboard->setColor(colorData);
}

void Cursor::updateSize()
{
    constexpr float cursor_size = 0.06f;
    constexpr int targetSize = 20;
    glm::vec3 cameraPosition = camera()->position();

    const float distanceToCamera = (cameraPosition - position()).length();
    const float vfov = glm::radians(camera()->lens()->verticalFieldOfView());
    const float pixelsToAngle = vfov / m_window->height();
    const float radius = distanceToCamera * tan(targetSize * pixelsToAngle / 2.0);

    // Set the scale based on the calculated radius
    m_transform->scale = glm::vec3(scaleFactor() * (scalingEnabled() ? radius : cursor_size));
}

void CursorBase::setColor(const ColorData& colorData)
{
    m_cbuf->data = std::vector<uint8_t>{ (uint8_t*)&colorData, (uint8_t*)&colorData + sizeof(colorData) };
}

CursorBase::CursorBase()
    : Serenity::Entity()
{

    m_cbuf = createChild<Serenity::StaticUniformBuffer>();
    m_cbuf->size = sizeof(ColorData);

    enabled.valueChanged().connect([this](bool isEnabled) {
                              component<Serenity::MeshRenderer>()->mesh = (isEnabled) ? m_mesh.get() : nullptr;
                          })
            .release();
}

BallCursor::BallCursor(const Serenity::LayerManager* layers)
    : CursorBase()
{
    makeBall(this, layers);
}

void BallCursor::makeBall(Serenity::Entity* ec, const Serenity::LayerManager* layers)
{
    auto shader_ball = ec->createChild<Serenity::SpirVShaderProgram>();
    shader_ball->vertexShader = SHADER_DIR "color.vert.spv";
    shader_ball->fragmentShader = SHADER_DIR "color.frag.spv";

    m_mesh = std::make_unique<Serenity::Mesh>();
    m_mesh->setObjectName("Cursor Mesh");
    Serenity::MeshGenerators::sphereGenerator(m_mesh.get(), 24, 24, 1.0f);

    Serenity::Material* material = ec->createChild<Serenity::Material>();
    material->shaderProgram = shader_ball;
    material->setUniformBuffer(3, 0, m_cbuf);

    auto cmodel = ec->createComponent<Serenity::MeshRenderer>();
    cmodel->mesh = m_mesh.get();
    cmodel->material = material;
    ec->layerMask = layers->layerMask({ "Opaque" });
}

BillboardCursor::BillboardCursor(const Serenity::LayerManager* layers)
    : CursorBase()
{
    makeBillboard(this, layers);
    texture.valueChanged().connect([this](BillboardCursor::CursorTexture texture) {
                              switch (texture) {
                              case CursorTexture::Default:
                                  m_texture->setPath("assets/cursor_billboard.png");
                                  break;
                              case CursorTexture::CrossHair:
                                  m_texture->setPath("assets/cursor_billboard_crosshair.png");
                                  break;
                              case CursorTexture::Dot:
                                  m_texture->setPath("assets/cursor_billboard_dot.png");
                                  break;
                              }
                          })
            .release();
}

void BillboardCursor::makeBillboard(Serenity::Entity* ec, const Serenity::LayerManager* layers)
{
    // Note: Billboarding is done at the shader level
    auto shader_bb = ec->createChild<Serenity::SpirVShaderProgram>();
    shader_bb->vertexShader = SHADER_DIR "billboard.vert.spv";
    shader_bb->fragmentShader = SHADER_DIR "billboard.frag.spv";

    m_mesh = std::make_unique<Serenity::Mesh>();
    m_mesh->setObjectName("Billboard Mesh");
    Serenity::MeshGenerators::planeGenerator(m_mesh.get(), 12, 12, { 2, 2 },
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
    material->setUniformBuffer(3, 0, m_cbuf);

    auto cmodel = ec->createComponent<Serenity::MeshRenderer>();
    cmodel->mesh = m_mesh.get();
    cmodel->material = material;

    ec->layerMask = layers->layerMask({ "Alpha" });
}

CrossCursor::CrossCursor(const Serenity::LayerManager* layers)
    : CursorBase()
{
    makeCross(this, layers);
}

void CrossCursor::makeCross(Serenity::Entity* ec, const Serenity::LayerManager* layers)
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

    m_mesh = std::make_unique<Serenity::Mesh>();
    m_mesh->setObjectName("Cursor Cross Mesh");
    m_mesh->setVertices({ vertexBufferData });
    m_mesh->setIndices(std::move(vindices));
    m_mesh->vertexFormat = Serenity::VertexFormat{
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

    Serenity::Material* material = ec->createChild<Serenity::Material>();
    material->shaderProgram = shader_ball;
    material->setUniformBuffer(3, 0, m_cbuf);

    auto cmodel = ec->createComponent<Serenity::MeshRenderer>();
    cmodel->mesh = m_mesh.get();
    cmodel->material = material;
    ec->layerMask = layers->layerMask({ "Opaque" });
}

} // namespace all::serenity
