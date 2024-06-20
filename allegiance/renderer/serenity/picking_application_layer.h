#pragma once
#include "stereo_proxy_camera.h"

namespace all::serenity {
class StereoProxyCamera;
class SerenityWindow;

class PickingApplicationLayer : public Serenity::ApplicationLayer
{
public:
    PickingApplicationLayer(StereoProxyCamera* camera, SerenityWindow* window, Serenity::SpatialAspect* spatialAspect, Serenity::SrtTransform* ctransform);

public:
    void onAfterRootEntityChanged(Serenity::Entity* oldRoot, Serenity::Entity* newRoot) override;

    void update() override;

    void SetEnabled(bool en);

    void SetScaleFactor(float scale_factor)
    {
        m_scale_factor = scale_factor;
    }
    void SetScalingEnabled(bool enabled)
    {
        m_scaling_enabled = enabled;
    }

private:
    Serenity::SrtTransform* m_ctransform;
    Serenity::SpatialAspect* m_spatialAspect;
    SerenityWindow* m_window;
    StereoProxyCamera* m_camera = nullptr;
    std::vector<Serenity::Entity*> m_pickedEntities;
    float m_scale_factor = 1.0f;
    bool m_scaling_enabled = true;
    bool enabled = true;
};
} // namespace all::serenity