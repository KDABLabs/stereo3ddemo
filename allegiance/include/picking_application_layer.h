#pragma once

#include "stereo_camera.h"

class StereoProxyCamera;
class SerenityWindow;

class PickingApplicationLayer : public Serenity::ApplicationLayer
{
public:
    PickingApplicationLayer(StereoProxyCamera* camera, SerenityWindow* window, Serenity::SpatialAspect* spatialAspect, Serenity::SrtTransform* ctransform);

    void onAfterRootEntityChanged(Serenity::Entity* oldRoot, Serenity::Entity* newRoot) override;
    void update() override;

    void SetEnabled(bool en);

private:
    Serenity::SrtTransform* m_ctransform;
    Serenity::SpatialAspect* m_spatialAspect;
    SerenityWindow* m_window;
    StereoProxyCamera* m_camera = nullptr;
    std::vector<Serenity::Entity*> m_pickedEntities;
    bool enabled = true;
};
