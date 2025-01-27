#include "focus_area.h"
#include "serenity_window.h"

#include <Serenity/core/ecs/camera.h>
#include <Serenity/gui/mesh.h>
#include <Serenity/gui/mesh_renderer.h>
#include <Serenity/gui/material.h>
#include <Serenity/gui/shader_program.h>
#include <Serenity/core/math/view_projection.h>

namespace all::serenity {

FocusArea::FocusArea()
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

    center.valueChanged().connect([this] { updateGeometry(); }).release();
    extent.valueChanged().connect([this] { updateGeometry(); }).release();
    window.valueChanged().connect([this] { updateGeometry(); }).release();

    m_vertexBuffer = createChild<Serenity::StaticVertexBuffer>();
    m_vertexBuffer->size = 14 * 2 * sizeof(glm::vec3);

    m_mesh = std::make_unique<Serenity::Mesh>();
    m_mesh->setObjectName("FocusArea");
    m_mesh->vertexBuffers = { m_vertexBuffer };
    m_mesh->vertexFormat = Serenity::VertexFormat{
        .buffers = {
                KDGpu::VertexBufferLayout{
                        .binding = 0,
                        .stride = 2 * sizeof(glm::vec3),
                },
        },
        .attributes = {
                KDGpu::VertexAttribute{
                        // Pos
                        .location = 0,
                        .binding = 0,
                        .format = KDGpu::Format::R32G32B32_SFLOAT,
                        .offset = 0,
                },
                KDGpu::VertexAttribute{
                        // Color
                        .location = 1,
                        .binding = 0,
                        .format = KDGpu::Format::R32G32B32_SFLOAT,
                        .offset = sizeof(glm::vec3),
                },
        },
    };

    auto* focusAreaShader = createChild<Serenity::SpirVShaderProgram>();
    focusAreaShader->vertexShader = SHADER_DIR "focus_area.vert.spv";
    focusAreaShader->fragmentShader = SHADER_DIR "focus_area.frag.spv";

    auto* material = createChild<Serenity::Material>();
    material->shaderProgram = focusAreaShader;

    Serenity::RenderStateSet lineRenderState;
    lineRenderState.primitiveRasterizerState().topology = KDGpu::PrimitiveTopology::LineList;

    auto* meshRenderer = createComponent<Serenity::MeshRenderer>();
    meshRenderer->renderState = lineRenderState;
    meshRenderer->mesh = m_mesh.get();
    meshRenderer->material = material;
    meshRenderer->computeTriangleInfo = false;
}

void FocusArea::updateGeometry()
{
    if (window() == nullptr || camera() == nullptr)
        return;

    const glm::uvec2 viewSize = { window()->width(), window()->height() };
    if (m_viewSize != viewSize) {
        m_viewSize = viewSize;
        center = { m_viewSize.x * 0.5f, m_viewSize.y * 0.5f, 0.0f };
        return; // Since center changing will trigger an update
    }

    const glm::vec3 camPosition = camera()->position();
    const glm::vec3 viewVector = camera()->viewDirection();
    const glm::vec3 viewCenter = camPosition + viewVector * camera()->convergencePlaneDistance();
    const glm::vec3 viewCenterScreen = Serenity::ViewProjection::project(viewCenter, camera()->viewMatrix(), camera()->lens()->projectionMatrix(), window()->viewportRect());
    const float focusPlaneScreenCoords = viewCenterScreen.z;

    auto screenPosToWorldCoords = [&](glm::vec3 screenPos) {
        screenPos.z = focusPlaneScreenCoords;
        return Serenity::ViewProjection::unproject(screenPos,
                                                   camera()->viewMatrix(),
                                                   camera()->lens()->projectionMatrix(),
                                                   window()->viewportRect());
    };

    const glm::vec3 white = glm::vec3(1.0f, 1.0f, 1.0f);
    const glm::vec3 red = glm::vec3(1.0f, 0.0f, 0.0f);
    const glm::vec3 centerColor = m_containedArea == ContainedArea::Center ? red : white;
    const glm::vec3 resizeColor = m_containedArea == ContainedArea::Resize ? red : white;

    const std::vector<glm::vec3> vertices{
        // Rect
        screenPosToWorldCoords(center() + glm::vec3(-0.5f, 0.5f, 0.0f) * extent()),
        white,
        screenPosToWorldCoords(center() + glm::vec3(-0.5f, -0.5f, 0.0f) * extent()),
        white,
        screenPosToWorldCoords(center() + glm::vec3(-0.5f, -0.5f, 0.0f) * extent()),
        white,
        screenPosToWorldCoords(center() + glm::vec3(0.5f, -0.5f, 0.0f) * extent()),
        white,
        screenPosToWorldCoords(center() + glm::vec3(0.5f, -0.5f, 0.0f) * extent()),
        white,
        screenPosToWorldCoords(center() + glm::vec3(0.5f, 0.5f, 0.0f) * extent()),
        white,
        screenPosToWorldCoords(center() + glm::vec3(0.5f, 0.5f, 0.0f) * extent()),
        white,
        screenPosToWorldCoords(center() + glm::vec3(-0.5f, 0.5f, 0.0f) * extent()),
        white,

        // Center
        screenPosToWorldCoords(center() + glm::vec3(0.0f, 1.0f, 0.0f) * 20.0f),
        centerColor,
        screenPosToWorldCoords(center() + glm::vec3(0.0f, -1.0f, 0.0f) * 20.0f),
        centerColor,
        screenPosToWorldCoords(center() + glm::vec3(-1.0f, 0.0f, 0.0f) * 20.0f),
        centerColor,
        screenPosToWorldCoords(center() + glm::vec3(1.0f, 0.0f, 0.0f) * 20.0f),
        centerColor,

        // Resize
        screenPosToWorldCoords(center() + glm::vec3(0.5f, 0.3f, 0.0f) * extent()),
        resizeColor,
        screenPosToWorldCoords(center() + glm::vec3(0.3f, 0.5f, 0.0f) * extent()),
        resizeColor,
    };

    Serenity::Mesh::VertexBufferData vertexBufferData;
    vertexBufferData.resize(vertices.size() * sizeof(glm::vec3));
    std::memcpy(vertexBufferData.data(), vertices.data(), vertices.size() * sizeof(glm::vec3));

    m_vertexBuffer->data = vertexBufferData;
}

void FocusArea::updateContainsMouse(const KDGui::MouseMoveEvent& mouse)
{
    if (!enabled())
        return;

    const ContainedArea oldContainedArea = m_containedArea;
    m_containedArea = ContainedArea::None;

    if (glm::length2(glm::vec3(mouse.xPos(), mouse.yPos(), 0.0f) - center()) < (20.0f * 20.0f)) {
        // Within the Center Cross
        m_containedArea = ContainedArea::Center;
    } else {
        // Within the Resize Handle
        const glm::vec3 a = center() + glm::vec3(0.5f, 0.3f, 0.0f) * extent();
        const glm::vec3 b = center() + glm::vec3(0.3f, 0.5f, 0.0f) * extent();
        const glm::vec3 c = center() + glm::vec3(0.5f, 0.5f, 0.0f) * extent();

        const glm::vec3 p = glm::vec3(mouse.xPos(), mouse.yPos(), 0.0f);

        const glm::vec3 ap = p - a;
        const glm::vec3 ab = b - a;
        const bool abXapPositive = (ab.x * ap.y - ab.y * ap.x) > 0.0f;

        const glm::vec3 bc = c - b;
        const glm::vec3 bp = p - b;
        const bool bcXbpPositive = (bc.x * bp.y - bc.y * bp.x) > 0.0f;

        const glm::vec3 ca = a - c;
        const glm::vec3 cp = p - c;
        const bool caXcpPositive = (ca.x * cp.y - ca.y * cp.x) > 0.0f;

        if (abXapPositive == bcXbpPositive && bcXbpPositive == caXcpPositive)
            m_containedArea = ContainedArea::Resize;
    }

    if (oldContainedArea != m_containedArea) {
        updateGeometry();
    }
}

void FocusArea::onMousePressed(const KDGui::MousePressEvent& mouse)
{
    m_distToCenterOnPress = center() - glm::vec3(mouse.xPos(), mouse.yPos(), 0.0f);
    m_extentOnPress = extent();
    if (m_containedArea == ContainedArea::Center) {
        m_operation = Operation::Translating;
    } else if (m_containedArea == ContainedArea::Resize) {
        m_operation = Operation::Scaling;
    }
}

void FocusArea::onMouseMoved(const KDGui::MouseMoveEvent& mouse)
{
    if (m_operation == Operation::None) {
        updateContainsMouse(mouse);
    } else if (m_operation == Operation::Translating) {
        center = m_distToCenterOnPress + glm::vec3(mouse.xPos(), mouse.yPos(), 0.0f);
    } else if (m_operation == Operation::Scaling) {
        const glm::vec3 distToCenter = center() - glm::vec3(mouse.xPos(), mouse.yPos(), 0.0f);
        extent = m_extentOnPress * (glm::length(distToCenter) / glm::length(m_distToCenterOnPress));
    }
}

void FocusArea::onMouseReleased(const KDGui::MouseReleaseEvent&)
{
    m_operation = Operation::None;
}

} // namespace all::serenity
