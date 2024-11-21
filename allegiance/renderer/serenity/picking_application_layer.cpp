#include "picking_application_layer.h"
#include "serenity_renderer.h"

using namespace Serenity;

all::serenity::PickingApplicationLayer::PickingApplicationLayer(StereoProxyCamera* camera, SerenityWindow* window, SpatialAspect* spatialAspect, SrtTransform* ctransform)
    : m_camera(camera), m_window(window), m_spatialAspect(spatialAspect), m_ctransform(ctransform)
{
}

void all::serenity::PickingApplicationLayer::onAfterRootEntityChanged(Entity* oldRoot, Entity* newRoot)
{
    m_pickedEntities.clear();
}

void all::serenity::PickingApplicationLayer::update()
{
    constexpr float cursor_size = 0.3f;
    if (!enabled) {
        return;
    }

    const auto viewportRect = m_window->viewportRect();

    // Perform ray cast
    const auto cursorPos = m_window->cursorPos();
    const auto hits = m_spatialAspect->screenCast(cursorPos, viewportRect, m_camera->centerEyeViewMatrix(), m_camera->lens()->projectionMatrix());

    auto unv = glm::unProject(glm::vec3(cursorPos.x, cursorPos.y, 1.0f), m_camera->centerEyeViewMatrix(), m_camera->lens()->projectionMatrix(), viewportRect);

    if (!hits.empty()) {
        // Find closest intersection
        const auto closest = std::ranges::min_element(hits, [](const SpatialAspect::Hit& a, const SpatialAspect::Hit& b) {
            return a.distance < b.distance;
        });
        assert(closest != hits.end());
        m_ctransform->translation = closest->position;
        m_ctransform->scale = glm::vec3(m_scale_factor * (m_scaling_enabled ? std::clamp(closest->distance * 10.f, 0.01f, 1.0f) : cursor_size));
    } else {
        m_ctransform->translation = unv;
        m_ctransform->scale = m_scale_factor * glm::vec3(10.0f);
    }
}

void all::serenity::PickingApplicationLayer::setEnabled(bool en)
{
    enabled = en;
}
