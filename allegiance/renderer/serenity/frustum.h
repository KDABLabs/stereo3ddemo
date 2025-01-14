#pragma once

#include <Serenity/core/ecs/entity.h>
#include <Serenity/gui/buffer.h>

namespace Serenity {
class MeshRenderer;
class StereoCamera;
} // namespace Serenity

namespace all::serenity {


class Frustum : public Serenity::Entity
{
public:
    Frustum();

    KDBindings::Property<bool> enabled{ true };
    KDBindings::Property<Serenity::Camera*> topViewCamera{ nullptr };
    KDBindings::Property<glm::mat4> viewMatrix;
    KDBindings::Property<glm::mat4> projectionMatrix;
    KDBindings::Property<float> convergence;
    KDBindings::Property<glm::vec4> color;

private:
    void updateGeometry();
    void updateColor();
    void updateTopViewCamera();

    KDBindings::ConnectionHandle m_projectionChangedConnection;
    KDBindings::ConnectionHandle m_viewChangedConnection;

    Serenity::StaticVertexBuffer* m_linesVertexBuffer{ nullptr };
    Serenity::StaticVertexBuffer* m_trianglesVertexBuffer{ nullptr };
    Serenity::StaticUniformBuffer* m_colorUBO{ nullptr };
    Serenity::StaticUniformBuffer* m_topCameraUBO{ nullptr };
    Serenity::MeshRenderer* m_lineRenderer{ nullptr };
    Serenity::MeshRenderer* m_triangleRenderer{ nullptr };
    std::unique_ptr<Serenity::Mesh> m_lineMesh;
    std::unique_ptr<Serenity::Mesh> m_triangleMesh;
};

} // namespace all::serenity
