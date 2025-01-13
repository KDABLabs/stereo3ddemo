#pragma once

#include <Serenity/core/application_layer.h>
#include <kdbindings/property.h>

namespace Serenity {
class StereoCamera;
} // namespace Serenity

namespace all::serenity {
class FocusArea;
class Cursor;
class SerenityWindow;

class PickingApplicationLayer : public Serenity::ApplicationLayer
{
public:
    explicit PickingApplicationLayer(Serenity::SpatialAspect* spatialAspect);

    KDBindings::Property<bool> autoFocus{ false };
    KDBindings::Property<FocusArea*> focusArea{ nullptr };
    KDBindings::Property<Cursor*> cursor{ nullptr };
    KDBindings::Property<SerenityWindow*> window{ nullptr };
    KDBindings::Property<Serenity::StereoCamera*> camera{ nullptr };

    KDBindings::Signal<float> autoFocusDistanceChanged;

public:
    void onAfterRootEntityChanged(Serenity::Entity* oldRoot, Serenity::Entity* newRoot) override;

    void update() override;

    void setEnabled(bool en);

    glm::vec3 cursorWorldPosition() const;

private:
    void updateCursorWorldPosition();
    void handleFocusForFocusArea();

    std::vector<Serenity::Entity*> m_pickedEntities;

    Serenity::SpatialAspect* m_spatialAspect{ nullptr };
    bool m_enabled{ true };
};
} // namespace all::serenity
