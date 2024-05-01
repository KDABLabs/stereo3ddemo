#pragma once
#include <Qt3DExtras/Qt3DWindow>
#include <glm/mat4x4.hpp>
#include "stereo_forward_renderer.h"
#include <QVector3D>
#include <QVector2D>
#include <QUrl>
#include <ui/camera_controller.h>

#include <filesystem>

namespace Qt3DRender {
class QRayCaster;
class QMaterial;
} // namespace Qt3DRender

namespace all {
struct ModelNavParameters;
struct StereoCamera;
} // namespace all

namespace all::qt3d {
class CursorEntity;
class Picker;

class Qt3DImpl
{
public:
    Qt3DImpl(all::StereoCamera& camera);
    ~Qt3DImpl();

public:
    void ViewChanged();
    void ProjectionChanged();
    void CreateAspects(std::shared_ptr<all::ModelNavParameters> nav_params, CursorController* cursorController);

    void LoadModel(std::filesystem::path path = "assets/gltf/showroom2303.gltf");
    void SetCursorEnabled(bool /* enabled */)
    {
        // TODO
    }

    QWindow* GetWindow() { return &m_view; }

    void UpdateMouse();
    void ShowImage();
    void ShowModel();

    glm::vec3 GetCursorWorldPosition() const;

private:
    static void AddDirectionalLight(Qt3DCore::QNode* node, QVector3D position);

    void CreateScene(Qt3DCore::QEntity* root, CursorController* cursorController);
    void LoadImage(QUrl path = QUrl::fromLocalFile(":/13_3840x2160_sbs.jpg"));

    QVector2D calculateSceneDimensions(Qt3DCore::QEntity* scene) const;

    struct SceneExtent {
        QVector3D min, max;
    };
    SceneExtent calculateSceneExtent(Qt3DCore::QNode* node)
    {
        SceneExtent e;
        _calculateSceneDimensions(node, e.min, e.max);
        return e;
    }

protected:
    void _calculateSceneDimensions(Qt3DCore::QNode* node, QVector3D& minBounds, QVector3D& maxBounds) const;
    void OnModelExtentChanged(const QVector3D& min, const QVector3D& max);

private:
    Qt3DExtras::Qt3DWindow m_view;
    std::unique_ptr<Qt3DCore::QEntity> m_rootEntity;
    Qt3DCore::QEntity* m_sceneEntity = nullptr;
    Qt3DCore::QEntity* m_userEntity = nullptr;
    Qt3DCore::QEntity* m_skyBox = nullptr;
    Qt3DRender::QRayCaster* m_raycaster;

    std::unordered_map<QString, Qt3DRender::QMaterial*> m_materials;
    QStereoForwardRenderer* m_renderer;
    QStereoProxyCamera* m_camera;
    CursorEntity* m_cursor;
    all::StereoCamera* m_stereoCamera;

    Picker* m_picker;
    std::shared_ptr<all::ModelNavParameters> m_nav_params;
};
} // namespace all::qt3d
