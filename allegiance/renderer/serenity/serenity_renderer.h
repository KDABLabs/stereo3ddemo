#pragma once
#include "serenity_stereo_graph.h"
#include "serenity_window.h"
#include "mesh_loader.h"
#include "cursor.h"
#include <any>

#include <shared/stereo_camera.h>

// #include <ui/camera_controller.h>

namespace all {
struct ModelNavParameters;
struct StereoCamera;
} // namespace all

namespace all::serenity {
class StereoProxyCamera;
class PickingApplicationLayer;
class SerenityRenderer
{
public:
    explicit SerenityRenderer(SerenityWindow* window, all::StereoCamera& camera, std::function<void(std::string_view name, std::any value)> = {})
        : m_window(window), camera(camera)
    {
    }
    virtual ~SerenityRenderer() = default;

public:
    void viewChanged();
    void projectionChanged();
    void createAspects(std::shared_ptr<all::ModelNavParameters> nav_params);

    void loadModel(std::filesystem::path file);

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

    bool hoversFocusArea(int x, int y) const;

    void onMouseEvent(const KDFoundation::Event& event);

protected:
    std::unique_ptr<Serenity::Entity> createScene(Serenity::LayerManager& layers);

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

    Serenity::StereoForwardAlgorithm::RenderPhase createOpaquePhase() const;
    Serenity::StereoForwardAlgorithm::RenderPhase createTransparentPhase() const;
    Serenity::StereoForwardAlgorithm::RenderPhase createStereoImagePhase() const;

    SerenityWindow* m_window{ nullptr };

    Mode m_mode{ Mode::Scene };
    Serenity::AspectEngine m_engine;
    Serenity::RenderAspect* m_renderAspect{ nullptr };
    Serenity::LayerManager* m_layerManager{ nullptr };
    Serenity::Entity* m_model{ nullptr };
    Serenity::Entity* m_scene_root{ nullptr };
    all::StereoCamera& camera;
    all::CameraMode m_cameraMode{ all::CameraMode::Stereo };

    std::shared_ptr<all::ModelNavParameters> m_navParams;

    // Camera
    all::serenity::StereoProxyCamera* m_camera{ nullptr };
    all::serenity::PickingApplicationLayer* m_pickingLayer{ nullptr };
    all::serenity::StereoRenderAlgorithm* m_renderAlgorithm{ nullptr };

    std::optional<all::serenity::Cursor> m_cursor;
    float scale_factor = 1.0f;
};
} // namespace all::serenity
