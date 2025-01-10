#include "picking_application_layer.h"
#include "serenity_renderer.h"
#include "cursor.h"

using namespace Serenity;

namespace all::serenity {

PickingApplicationLayer::PickingApplicationLayer(Serenity::StereoCamera* camera,
                                                 SerenityWindow* window,
                                                 SpatialAspect* spatialAspect,
                                                 Cursor* cursor)
    : m_camera(camera)
    , m_window(window)
    , m_spatialAspect(spatialAspect)
    , m_cursor(cursor)
{
}

void PickingApplicationLayer::onAfterRootEntityChanged(Entity*, Entity*)
{
    m_pickedEntities.clear();
}

void PickingApplicationLayer::update()
{
    if (!m_enabled)
        return;

    updateCursorWorldPosition();

    // TODO: AF RayCasts
}

void PickingApplicationLayer::setEnabled(bool en)
{
    m_enabled = en;
}

void PickingApplicationLayer::updateCursorWorldPosition()
{
    // Perform ray cast
    const glm::vec4 viewportRect = m_window->viewportRect();
    const glm::vec2 cursorPos = m_window->cursorPos();
    const std::vector<SpatialAspect::Hit> hits = m_spatialAspect->screenCast(cursorPos, viewportRect, m_camera->viewMatrix(), m_camera->lens()->projectionMatrix());

    if (!hits.empty()) {
        // Find closest intersection
        const auto closest = std::ranges::min_element(hits, [](const SpatialAspect::Hit& a, const SpatialAspect::Hit& b) {
            return a.distance < b.distance;
        });
        assert(closest != hits.end());
        m_cursor->setPosition(closest->position);
    } else {
        const glm::vec3 unv = glm::unProject(glm::vec3(cursorPos.x, cursorPos.y, 1.0f), m_camera->viewMatrix(), m_camera->lens()->projectionMatrix(), viewportRect);
        m_cursor->setPosition(unv);
    }
}

} // namespace all::serenity
