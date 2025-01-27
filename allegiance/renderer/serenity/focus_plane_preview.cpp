#include "focus_plane_preview.h"

#include <Serenity/core/ecs/camera.h>
#include <Serenity/gui/mesh.h>
#include <Serenity/gui/mesh_renderer.h>
#include <Serenity/gui/material.h>
#include <Serenity/gui/shader_program.h>

namespace all::serenity {

FocusPlanePreview::FocusPlanePreview()
{
    enabled.valueChanged().connect([this](bool isEnabled) {
                              component<Serenity::MeshRenderer>()->mesh = (isEnabled) ? m_mesh.get() : nullptr;
                              if (isEnabled)
                                  updateGeometry();
                          })
            .release();

    camera.valueChanged().connect([this](Serenity::StereoCamera* c) {
                             m_projectionChangedConnection.disconnect();
                             m_viewChangedConnection.disconnect();
                             if (c != nullptr) {
                                 m_viewChangedConnection = c->viewMatrix.valueChanged().connect([this] { updateGeometry(); });
                                 m_projectionChangedConnection = c->lens()->projectionMatrix.valueChanged().connect([this] { updateGeometry(); });
                             }
                         })
            .release();

    m_vertexBuffer = createChild<Serenity::StaticVertexBuffer>();
    m_vertexBuffer->size = 6 * sizeof(glm::vec3);

    m_mesh = std::make_unique<Serenity::Mesh>();
    m_mesh->setObjectName("FocusPlanePreviewMesh");
    m_mesh->vertexBuffers = { m_vertexBuffer };
    m_mesh->vertexFormat = Serenity::VertexFormat{
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

    auto* focusPlaneShader = createChild<Serenity::SpirVShaderProgram>();
    focusPlaneShader->vertexShader = SHADER_DIR "focus_plane.vert.spv";
    focusPlaneShader->fragmentShader = SHADER_DIR "focus_plane.frag.spv";

    auto* material = createChild<Serenity::Material>();
    material->shaderProgram = focusPlaneShader;

    auto* meshRenderer = createComponent<Serenity::MeshRenderer>();
    meshRenderer->mesh = m_mesh.get();
    meshRenderer->material = material;
    meshRenderer->computeTriangleInfo = false;
}

void FocusPlanePreview::updateGeometry()
{
    const glm::vec3 camPosition = camera()->position();
    const glm::vec3 viewVector = camera()->viewDirection();
    const glm::vec3 viewCenter = camPosition + viewVector * camera()->convergencePlaneDistance();
    const glm::vec4 viewCenterScreen = camera()->lens()->projectionMatrix() * camera()->viewMatrix() * glm::vec4(viewCenter, 1.0f);
    const float zFocus = viewCenterScreen.z / viewCenterScreen.w;

    const std::vector<glm::vec3> screenSpaceFocusPlanePreviewPoint{
        // Focus
        glm::vec3(-0.5f, 0.5f, zFocus),
        glm::vec3(0.5f, 0.5f, zFocus),
        glm::vec3(0.5f, -0.5f, zFocus),
        glm::vec3(-0.5f, -0.5f, zFocus),
    };

    std::vector<glm::vec3> worldFocusPlanePreviewPoints;
    worldFocusPlanePreviewPoints.reserve(screenSpaceFocusPlanePreviewPoint.size());

    const glm::mat4 inverseMvp = glm::inverse(camera()->lens()->projectionMatrix() * camera()->viewMatrix());
    for (const glm::vec3& p : screenSpaceFocusPlanePreviewPoint) {
        const glm::vec4 v = inverseMvp * glm::vec4(p, 1.0f);
        const float w = !std::isnan(v.w) ? v.w : 1.0f;
        worldFocusPlanePreviewPoints.push_back(glm::vec3(v) / w);
    }

    const std::vector<glm::vec3> vertices{
        worldFocusPlanePreviewPoints[0],
        worldFocusPlanePreviewPoints[3],
        worldFocusPlanePreviewPoints[2],
        worldFocusPlanePreviewPoints[2],
        worldFocusPlanePreviewPoints[1],
        worldFocusPlanePreviewPoints[0],
    };

    Serenity::Mesh::VertexBufferData vertexBufferData;
    vertexBufferData.resize(vertices.size() * sizeof(glm::vec3));
    std::memcpy(vertexBufferData.data(), vertices.data(), vertices.size() * sizeof(glm::vec3));

    m_vertexBuffer->data = vertexBufferData;
}

} // namespace all::serenity
