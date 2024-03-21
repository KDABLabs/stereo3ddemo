#include <picking_application_layer.h>

#include <serenity_impl.h>

using namespace Serenity;

PickingApplicationLayer::PickingApplicationLayer(StereoProxyCamera* camera, SerenityWindow* window, SpatialAspect* spatialAspect, SrtTransform* ctransform)
    : m_camera(camera), m_window(window), m_spatialAspect(spatialAspect), m_ctransform(ctransform)
{
}

void PickingApplicationLayer::onAfterRootEntityChanged(Entity* oldRoot, Entity* newRoot)
{
    m_pickedEntities.clear();
}

void PickingApplicationLayer::update()
{
    if (!enabled) {
        return;
    }

    const auto viewportRect = m_window->GetViewportRect();

    // Perform ray cast
    const auto cursorPos = m_window->GetCursorPos();
    const auto hits = m_spatialAspect->screenCast(cursorPos, viewportRect, m_camera->centerEyeViewMatrix(), m_camera->lens()->projectionMatrix());

    auto unv = glm::unProject(glm::vec3(cursorPos.x, cursorPos.y, 1.0f), m_camera->centerEyeViewMatrix(), m_camera->lens()->projectionMatrix(), viewportRect);

    if (!hits.empty()) {
        // Find closest intersection
        const auto closest = std::ranges::min_element(hits, [](const SpatialAspect::Hit& a, const SpatialAspect::Hit& b) {
            return a.distance < b.distance;
        });
        assert(closest != hits.end());
        m_ctransform->translation = closest->worldIntersection;
        m_ctransform->scale = glm::vec3(std::clamp(closest->distance * 10.f, 0.01f, 1.0f));
    } else {
        m_ctransform->translation = unv;
        m_ctransform->scale = glm::vec3(10.0f);
    }
}

void PickingApplicationLayer::SetEnabled(bool en)
{
    enabled = en;
}
