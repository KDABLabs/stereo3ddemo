#include "frustum.h"

#include <Serenity/core/ecs/camera.h>
#include <Serenity/gui/mesh.h>
#include <Serenity/gui/mesh_renderer.h>
#include <Serenity/gui/material.h>
#include <Serenity/gui/shader_program.h>


namespace all::serenity {

Frustum::Frustum()
{
    enabled.valueChanged().connect([this](bool isEnabled) {
                              m_lineRenderer->mesh = (isEnabled) ? m_lineMesh.get() : nullptr;
                              m_triangleRenderer->mesh = (isEnabled) ? m_triangleMesh.get() : nullptr;
                          })
            .release();

    viewMatrix.valueChanged().connect([this] { updateGeometry(); }).release();
    projectionMatrix.valueChanged().connect([this] { updateGeometry(); }).release();
    convergence.valueChanged().connect([this] { updateGeometry(); }).release();
    color.valueChanged().connect([this] { updateColor(); }).release();

    topViewCamera.valueChanged().connect([this](Serenity::Camera* c) {
                                    m_projectionChangedConnection.disconnect();
                                    m_viewChangedConnection.disconnect();
                                    if (c != nullptr) {
                                        m_viewChangedConnection = c->viewMatrix.valueChanged().connect([this] { updateTopViewCamera(); });
                                        m_projectionChangedConnection = c->lens()->projectionMatrix.valueChanged().connect([this] { updateTopViewCamera(); });
                                    }
                                })
            .release();

    m_topCameraUBO = createChild<Serenity::StaticUniformBuffer>();
    m_topCameraUBO->size = sizeof(glm::mat4);

    {
        m_linesVertexBuffer = createChild<Serenity::StaticVertexBuffer>();
        m_linesVertexBuffer->size = 12 * sizeof(glm::vec3);

        m_lineMesh = std::make_unique<Serenity::Mesh>();
        m_lineMesh->setObjectName("FrustumLines");
        m_lineMesh->vertexBuffers = { m_linesVertexBuffer };
        m_lineMesh->vertexFormat = Serenity::VertexFormat{
            .buffers = {
                    KDGpu::VertexBufferLayout{
                            .binding = 0,
                            .stride = sizeof(glm::vec3),
                    },
            },
            .attributes = {
                    KDGpu::VertexAttribute{
                            .location = 0,
                            .binding = 0,
                            .format = KDGpu::Format::R32G32B32_SFLOAT,
                            .offset = 0,
                    },
            },
        };

        auto* frustumLineShader = createChild<Serenity::SpirVShaderProgram>();
        frustumLineShader->vertexShader = SHADER_DIR "frustum_lines.vert.spv";
        frustumLineShader->fragmentShader = SHADER_DIR "frustum_lines.frag.spv";

        auto* material = createChild<Serenity::Material>();
        material->shaderProgram = frustumLineShader;
        material->setUniformBuffer(4, 0, m_topCameraUBO);

        Serenity::RenderStateSet lineRenderState;
        lineRenderState.primitiveRasterizerState().topology = KDGpu::PrimitiveTopology::LineList;
        lineRenderState.primitiveRasterizerState().lineWidth = 2.0f;

        m_lineRenderer = createComponent<Serenity::MeshRenderer>();
        m_lineRenderer->renderState = lineRenderState;
        m_lineRenderer->mesh = m_lineMesh.get();
        m_lineRenderer->material = material;
        m_lineRenderer->computeTriangleInfo = false;
    }

    {
        m_trianglesVertexBuffer = createChild<Serenity::StaticVertexBuffer>();
        m_trianglesVertexBuffer->size = 6 * sizeof(glm::vec3);

        m_triangleMesh = std::make_unique<Serenity::Mesh>();
        m_triangleMesh->setObjectName("FrustumTriangles");
        m_triangleMesh->vertexBuffers = { m_trianglesVertexBuffer };
        m_triangleMesh->vertexFormat = Serenity::VertexFormat{
            .buffers = {
                    KDGpu::VertexBufferLayout{
                            .binding = 0,
                            .stride = sizeof(glm::vec3),
                    },
            },
            .attributes = {
                    KDGpu::VertexAttribute{
                            .location = 0,
                            .binding = 0,
                            .format = KDGpu::Format::R32G32B32_SFLOAT,
                            .offset = 0,
                    },
            },
        };

        auto* frustumTriShader = createChild<Serenity::SpirVShaderProgram>();
        frustumTriShader->vertexShader = SHADER_DIR "frustum_tris.vert.spv";
        frustumTriShader->fragmentShader = SHADER_DIR "frustum_tris.frag.spv";

        auto* material = createChild<Serenity::Material>();
        material->shaderProgram = frustumTriShader;

        m_colorUBO = createChild<Serenity::StaticUniformBuffer>();
        m_colorUBO->size = sizeof(glm::vec4);
        material->setUniformBuffer(3, 0, m_colorUBO);
        material->setUniformBuffer(4, 0, m_topCameraUBO);

        m_triangleRenderer = createComponent<Serenity::MeshRenderer>();
        m_triangleRenderer->mesh = m_triangleMesh.get();
        m_triangleRenderer->material = material;
        m_triangleRenderer->computeTriangleInfo = false;
    }
}

void Frustum::updateGeometry()
{
    const glm::mat4 invViewMatrix = glm::inverse(viewMatrix());
    const glm::vec3 camPosition = glm::vec3(invViewMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0));
    const glm::vec3 viewVector = glm::normalize(glm::vec3(invViewMatrix * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)));
    const glm::vec3 viewCenter = camPosition + viewVector * convergence();
    const glm::vec4 viewCenterScreen = projectionMatrix() * viewMatrix() * glm::vec4(viewCenter, 1.0f);
    const float zFocus = viewCenterScreen.z / viewCenterScreen.w;

    const std::vector<glm::vec3> screenSpaceFrustumPoint{
        // Near
        glm::vec3(-1.0f, 0.5f, -1.0f),
        glm::vec3(1.0f, 0.5f, -1.0f),
        // Focus
        glm::vec3(-1.0f, 0.5f, zFocus),
        glm::vec3(1.0f, 0.5f, zFocus),
        // Far
        glm::vec3(-1.0f, 0.5f, 1.0f),
        glm::vec3(1.0f, 0.5f, 1.0f),
    };

    std::vector<glm::vec3> worldFrustumPoints;
    worldFrustumPoints.reserve(screenSpaceFrustumPoint.size());

    const glm::mat4 inverseMvp = glm::inverse(projectionMatrix() * viewMatrix());
    for (const glm::vec3& p : screenSpaceFrustumPoint) {
        const glm::vec4 v = inverseMvp * glm::vec4(p, 1.0f);
        const float w = !std::isnan(v.w) ? v.w : 1.0f;
        worldFrustumPoints.push_back(glm::vec3(v) / w);
    }

    const std::vector<glm::vec3> frustumLineVertices{
        worldFrustumPoints[0],
        worldFrustumPoints[1],

        worldFrustumPoints[2],
        worldFrustumPoints[3],

        worldFrustumPoints[4],
        worldFrustumPoints[5],

        worldFrustumPoints[0],
        worldFrustumPoints[4],

        worldFrustumPoints[1],
        worldFrustumPoints[5],

        camPosition,
        viewCenter,
    };

    Serenity::Mesh::VertexBufferData lineVertexBufferData;
    lineVertexBufferData.resize(frustumLineVertices.size() * sizeof(glm::vec3));
    std::memcpy(lineVertexBufferData.data(), frustumLineVertices.data(), frustumLineVertices.size() * sizeof(glm::vec3));
    m_linesVertexBuffer->data = lineVertexBufferData;

    const std::vector<glm::vec3> frustumTriangleVertices{
        worldFrustumPoints[0],
        worldFrustumPoints[1],
        worldFrustumPoints[4],

        worldFrustumPoints[4],
        worldFrustumPoints[1],
        worldFrustumPoints[5],
    };

    Serenity::Mesh::VertexBufferData triangleVertexBufferData;
    triangleVertexBufferData.resize(frustumTriangleVertices.size() * sizeof(glm::vec3));
    std::memcpy(triangleVertexBufferData.data(), frustumTriangleVertices.data(), frustumTriangleVertices.size() * sizeof(glm::vec3));
    m_trianglesVertexBuffer->data = triangleVertexBufferData;

    updateTopViewCamera();
}

void Frustum::updateColor()
{
    const glm::vec4& c = color();
    std::vector<uint8_t> rawData(sizeof(glm::vec4));
    std::memcpy(rawData.data(), &c, sizeof(glm::vec4));
    m_colorUBO->data = rawData;
}

void Frustum::updateTopViewCamera()
{
    const glm::mat4 vp = topViewCamera()->lens()->projectionMatrix() * topViewCamera()->viewMatrix();
    std::vector<uint8_t> rawData(sizeof(glm::mat4));
    std::memcpy(rawData.data(), &vp, sizeof(glm::mat4));
    m_topCameraUBO->data = rawData;
}

} // namespace all::serenity
