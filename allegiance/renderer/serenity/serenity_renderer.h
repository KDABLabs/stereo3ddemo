#pragma once
#include "serenity_stereo_graph.h"
#include "serenity_window.h"
#include "mesh_loader.h"
#include <any>

#include <shared/stereo_camera.h>

// #include <ui/camera_controller.h>

namespace all {
struct ModelNavParameters;
class StereoCamera;
} // namespace all

namespace Serenity {
class StereoCamera;
}

namespace all::serenity {
class Cursor;
class FocusArea;
class FocusPlanePreview;
class Frustum;
class FrustumRect;
class PickingApplicationLayer;

struct TopViewCameraLookAtInfo {
    glm::vec3 position;
    glm::vec3 viewCenter;
    glm::vec3 upVector;

    friend bool operator==(const TopViewCameraLookAtInfo &, const TopViewCameraLookAtInfo &) = default;
};

struct TopViewCameraProjInfo {
    float left;
    float top;
    float nearPlane;
    float farPlane;

    friend bool operator==(const TopViewCameraProjInfo &, const TopViewCameraProjInfo &) = default;
};

class SerenityRenderer
{
public:
    explicit SerenityRenderer(SerenityWindow* window, all::StereoCamera& camera, std::function<void(std::string_view name, std::any value)> = {});
    virtual ~SerenityRenderer() = default;

public:
    void viewChanged();
    void projectionChanged();
    void createAspects(std::shared_ptr<all::ModelNavParameters> nav_params);

    void loadModel(std::filesystem::path file);
    void loadImage(std::filesystem::path url);
    void viewAll();

    void propertyChanged(std::string_view name, std::any value);
    void setCursorEnabled(bool enabled);

    void showModel()
    {
        setMode(Mode::Scene);
    }
    void showImage()
    {
        setMode(Mode::StereoImage);
    }
    void screenshot(const std::function<void(const uint8_t* data, uint32_t width, uint32_t height)>& in);

    glm::vec3 cursorWorldPosition() const;
    glm::vec3 sceneCenter() const;
    glm::vec3 sceneExtent() const;
    float fieldOfView() const;
    float aspectRatio() const;

    bool hoversFocusArea(int x, int y) const;

    void onMouseEvent(const KDFoundation::Event& event);

    void completeInitialization();

protected:
    void createScene();

    enum class Mode {
        Scene,
        StereoImage
    };

    void setMode(Mode mode)
    {
        if (m_mode == mode)
            return;
        m_mode = mode;
        updateRenderPhases();
    }

    void updateRenderPhases();
    void updateDisplayMode(all::DisplayMode displayMode);

    Serenity::StereoForwardAlgorithm::RenderPhase createSkyboxPhase() const;
    Serenity::StereoForwardAlgorithm::RenderPhase createOpaquePhase() const;
    Serenity::StereoForwardAlgorithm::RenderPhase createTransparentPhase() const;
    Serenity::StereoForwardAlgorithm::RenderPhase createFocusAreaPhase() const;
    Serenity::StereoForwardAlgorithm::RenderPhase createFrustumPhase() const;
    Serenity::StereoForwardAlgorithm::RenderPhase createFocusPlanePreviewAndCursorPhase() const;
    Serenity::StereoForwardAlgorithm::RenderPhase createStereoImagePhase() const;

    SerenityWindow* m_window{ nullptr };

    Mode m_mode{ Mode::Scene };
    Serenity::AspectEngine m_engine;
    Serenity::RenderAspect* m_renderAspect{ nullptr };
    Serenity::LayerManager* m_layerManager{ nullptr };
    Serenity::Entity* m_model{ nullptr };
    Serenity::Entity* m_stereoImage{ nullptr };
    Serenity::Entity* m_sceneRoot{ nullptr };
    all::serenity::Cursor* m_cursor{ nullptr };
    FocusPlanePreview* m_focusPlanePreview{ nullptr };
    FocusArea* m_focusArea{ nullptr };
    Frustum* m_leftFrustum{ nullptr };
    Frustum* m_rightFrustum{ nullptr };
    FrustumRect *m_frustumRect { nullptr };
    all::StereoCamera& m_stereoCamera;
    bool m_supportsStereoSwapchain {false};

    std::shared_ptr<all::ModelNavParameters> m_navParams;

    // Camera
    Serenity::StereoCamera* m_camera{ nullptr };
    Serenity::StereoCamera* m_frustumAmplifiedCamera{ nullptr };
    Serenity::Camera* m_frustumTopViewCamera{ nullptr };
    all::serenity::PickingApplicationLayer* m_pickingLayer{ nullptr };
    all::serenity::StereoRenderAlgorithm* m_renderAlgorithm{ nullptr };

    KDBindings::Property<TopViewCameraLookAtInfo> m_topViewCameraLookAtInfo;
    KDBindings::Property<TopViewCameraProjInfo> m_topViewCameraProjInfo;

    float scale_factor = 1.0f;

    glm::vec3 m_sceneCenter;
    glm::vec3 m_sceneExtent;
    bool m_wireframeEnabled{ false };

    std::function<void(std::string_view, std::any)> m_propertyUpdateNofitier;
};
} // namespace all::serenity
