#pragma once

#include <Serenity/core/ecs/entity.h>
#include <Serenity/gui/buffer.h>

namespace Serenity {
class StereoCamera;
class Mesh;
} // namespace Serenity

namespace all::serenity {

class SerenityWindow;

class FocusArea : public Serenity::Entity
{
public:
    FocusArea();

    KDBindings::Property<bool> enabled{ true };
    KDBindings::Property<Serenity::StereoCamera*> camera{ nullptr };
    KDBindings::Property<glm::vec3> center{}; // px
    KDBindings::Property<glm::vec3> extent{ { 100.0f, 100.0f, 0.0f } }; // px
    KDBindings::Property<SerenityWindow*> window{};

    inline bool containsMouse() const { return m_containedArea != ContainedArea::None; }

    void onMousePressed(const KDGui::MousePressEvent& mouse);
    void onMouseMoved(const KDGui::MouseMoveEvent& mouse);
    void onMouseReleased(const KDGui::MouseReleaseEvent& mouse);

private:
    void updateGeometry();
    void updateContainsMouse(const KDGui::MouseMoveEvent& mouse);

    enum class ContainedArea {
        Center,
        Resize,
        None,
    };
    ContainedArea m_containedArea = ContainedArea::None;

    enum class Operation {
        Translating,
        Scaling,
        None,
    };
    Operation m_operation = Operation::None;
    glm::vec3 m_distToCenterOnPress;
    glm::vec3 m_extentOnPress;
    glm::uvec2 m_viewSize{ 1, 1 };
    bool m_updateRequested = false;

    KDBindings::ConnectionHandle m_projectionChangedConnection;
    KDBindings::ConnectionHandle m_viewChangedConnection;
    Serenity::StaticVertexBuffer* m_vertexBuffer{ nullptr };
    std::unique_ptr<Serenity::Mesh> m_mesh;
};

} // namespace all::serenity
