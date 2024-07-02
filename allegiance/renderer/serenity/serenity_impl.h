#pragma once
#include "serenity_window.h"
#include "mesh_loader.h"
#include "cursor.h"
#include <any>

// #include <ui/camera_controller.h>

namespace all {
struct ModelNavParameters;
struct StereoCamera;
} // namespace all

namespace all::serenity {
class StereoProxyCamera;
class PickingApplicationLayer;
class SerenityImpl
{
public:
    explicit SerenityImpl(std::unique_ptr<SerenityWindow> window, all::StereoCamera& camera)
        : m_window(std::move(window)), camera(camera)
    {
    }
    virtual ~SerenityImpl() = default;

public:
    void ViewChanged();
    void ProjectionChanged();
    void CreateAspects(std::shared_ptr<all::ModelNavParameters> nav_params);

    void LoadModel(std::filesystem::path file);

    void OnPropertyChanged(std::string_view name, std::any value);
    void SetCursorEnabled(bool enabled);

    void ShowModel()
    {
        setMode(Mode::Scene);
    }
    void ShowImage()
    {
        setMode(Mode::StereoImage);
    }

    glm::vec3 GetCursorWorldPosition() const;

    void OnMouseEvent(const KDFoundation::Event& event);

protected:
    std::unique_ptr<Serenity::Entity> CreateScene(Serenity::LayerManager& layers);

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

    std::unique_ptr<SerenityWindow> m_window;

    Mode m_mode{ Mode::Scene };
    Serenity::AspectEngine m_engine;
    Serenity::RenderAspect* m_renderAspect{ nullptr };
    Serenity::LayerManager* m_layerManager{ nullptr };
    Serenity::Entity* m_model{ nullptr };
    Serenity::Entity* m_scene_root{ nullptr };
    all::StereoCamera& camera;
    std::shared_ptr<all::ModelNavParameters> m_navParams;

    // Camera
    all::serenity::StereoProxyCamera* m_camera;
    all::serenity::PickingApplicationLayer* m_pickingLayer;

    std::optional<all::serenity::Cursor> m_cursor;
    float scale_factor = 1.0f;
};
} // namespace all::serenity
