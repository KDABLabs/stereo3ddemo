#pragma once

namespace Serenity {
class StereoCamera;
} // namespace Serenity

namespace all::serenity {
class SerenityWindow;

class PickingApplicationLayer : public Serenity::ApplicationLayer
{
public:
    PickingApplicationLayer(Serenity::StereoCamera* camera, SerenityWindow* window, Serenity::SpatialAspect* spatialAspect, Serenity::SrtTransform* ctransform);

public:
    void onAfterRootEntityChanged(Serenity::Entity* oldRoot, Serenity::Entity* newRoot) override;

    void update() override;

    void setEnabled(bool en);

    void setScaleFactor(float scale_factor)
    {
        m_scale_factor = scale_factor;
    }
    void setScalingEnabled(bool enabled)
    {
        m_scaling_enabled = enabled;
    }
    void setTransform(Serenity::SrtTransform* transform)
    {
        m_ctransform = transform;
        update();
    }

private:
    Serenity::SrtTransform* m_ctransform;
    Serenity::SpatialAspect* m_spatialAspect;
    SerenityWindow* m_window;
    Serenity::StereoCamera* m_camera = nullptr;
    std::vector<Serenity::Entity*> m_pickedEntities;
    float m_scale_factor = 1.0f;
    bool m_scaling_enabled = true;
    bool enabled = true;
};
} // namespace all::serenity
