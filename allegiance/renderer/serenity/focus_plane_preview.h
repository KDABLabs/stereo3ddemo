#pragma once

#include <Serenity/core/ecs/entity.h>
#include <Serenity/gui/buffer.h>

namespace Serenity {
class StereoCamera;
class Mesh;
} // namespace Serenity

namespace all::serenity {

class FocusPlanePreview : public Serenity::Entity
{
public:
    FocusPlanePreview();

    KDBindings::Property<bool> enabled{ true };
    KDBindings::Property<Serenity::StereoCamera*> camera{ nullptr };

private:
    void updateGeometry();

    KDBindings::ConnectionHandle m_projectionChangedConnection;
    KDBindings::ConnectionHandle m_viewChangedConnection;
    std::unique_ptr<Serenity::Mesh> m_mesh;
    Serenity::StaticVertexBuffer* m_vertexBuffer{ nullptr };
};

} // namespace all::serenity
