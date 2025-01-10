#pragma once

namespace Serenity {
class StereoCamera;
} // namespace Serenity

namespace all::serenity {
class SerenityWindow;
class Cursor;

class PickingApplicationLayer : public Serenity::ApplicationLayer
{
public:
    PickingApplicationLayer(Serenity::StereoCamera* camera,
                            SerenityWindow* window,
                            Serenity::SpatialAspect* spatialAspect,
                            Cursor* cursor);

public:
    void onAfterRootEntityChanged(Serenity::Entity* oldRoot, Serenity::Entity* newRoot) override;

    void update() override;

    void setEnabled(bool en);

    glm::vec3 cursorWorldPosition() const;

private:
    void updateCursorWorldPosition();

    std::vector<Serenity::Entity*> m_pickedEntities;

    Serenity::StereoCamera* m_camera{ nullptr };
    SerenityWindow* m_window{ nullptr };
    Serenity::SpatialAspect* m_spatialAspect{ nullptr };
    Cursor* m_cursor{ nullptr };
    bool m_enabled{ true };
};
} // namespace all::serenity
