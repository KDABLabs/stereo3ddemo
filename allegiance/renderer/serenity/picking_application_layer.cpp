#include "picking_application_layer.h"
#include "serenity_renderer.h"
#include "cursor.h"
#include "focus_area.h"

using namespace Serenity;

namespace all::serenity {

PickingApplicationLayer::PickingApplicationLayer(SpatialAspect* spatialAspect)
    : m_spatialAspect(spatialAspect)
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

    if (window() == nullptr || camera() == nullptr)
        return;

    if (cursor() != nullptr)
        updateCursorWorldPosition();

    // AF RayCasts
    if (autoFocus() && focusArea() != nullptr)
        handleFocusForFocusArea();
}

void PickingApplicationLayer::setEnabled(bool en)
{
    m_enabled = en;
}

void PickingApplicationLayer::updateCursorWorldPosition()
{
    if (cursor()->locked())
        return;

    // Perform ray cast
    const glm::vec4 viewportRect = window()->viewportRect();
    const glm::vec2 cursorPos = window()->cursorPos();
    const std::vector<SpatialAspect::Hit> hits = m_spatialAspect->screenCast(cursorPos, viewportRect, camera()->viewMatrix(), camera()->lens()->projectionMatrix());

    if (!hits.empty()) {
        // Find closest intersection
        const auto closest = std::ranges::min_element(hits, [](const SpatialAspect::Hit& a, const SpatialAspect::Hit& b) {
            return a.distance < b.distance;
        });
        assert(closest != hits.end());
        cursor()->setPosition(closest->position);
    } else {
        const glm::vec3 viewCenter = camera()->position() + camera()->viewDirection() * camera()->convergencePlaneDistance();
        const glm::vec4 viewCenterScreen = camera()->lens()->projectionMatrix() * camera()->viewMatrix() * glm::vec4(viewCenter, 1.0f);
        const float zFocus = viewCenterScreen.z / viewCenterScreen.w;

        const glm::vec3 unv = glm::unProject(glm::vec3(cursorPos.x, cursorPos.y, zFocus), camera()->viewMatrix(), camera()->lens()->projectionMatrix(), viewportRect);
        cursor()->setPosition(unv);
    }
}

void PickingApplicationLayer::handleFocusForFocusArea()
{
    const glm::vec3 center = focusArea()->center();
    const glm::vec3 extent = focusArea()->extent();

    constexpr size_t AFSamplesY = 2;
    constexpr size_t AFSamplesX = 2;
    float averagedDistanceFromCamera = 0.0f;
    size_t validHits = 0;

    for (size_t y = 0; y < AFSamplesY; ++y) {
        const float yPos = center.y + ((float(y) / AFSamplesY) - 1.0f) * (extent.y * 0.5f);
        for (size_t x = 0; x < AFSamplesX; ++x) {
            const float xPos = center.x + ((float(x) / AFSamplesX) - 1.0f) * (extent.x * 0.5f);

            const std::vector<SpatialAspect::Hit> hits = m_spatialAspect->screenCast(glm::vec2(xPos, yPos),
                                                                                     window()->viewportRect(),
                                                                                     camera()->viewMatrix(),
                                                                                     camera()->lens()->projectionMatrix());

            if (!hits.empty()) {
                const auto closest = std::ranges::min_element(hits, [](const SpatialAspect::Hit& a, const SpatialAspect::Hit& b) {
                    return a.distance < b.distance;
                });
                averagedDistanceFromCamera += glm::length(closest->position - camera()->position());
                ++validHits;
            }
        }
    }

    if (validHits > 0) {
        averagedDistanceFromCamera /= float(validHits);
        autoFocusDistanceChanged.emit(averagedDistanceFromCamera);
    }
}

} // namespace all::serenity
