#pragma once
#include <Qt3DExtras/Qt3DWindow>
#include <glm/mat4x4.hpp>
#include "frustum_rect.h"
#include "stereo_forward_renderer.h"
#include <QVector3D>
#include <QVector2D>
#include <QUrl>
#include <shared/stereo_camera.h>

#include <filesystem>
#include <any>

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
class Frustum;
class FrustumRect;

class Qt3DRenderer
{
public:
    explicit Qt3DRenderer(Qt3DExtras::Qt3DWindow* view, all::StereoCamera& camera);
    ~Qt3DRenderer();

public:
    void viewChanged();
    void projectionChanged();
    void createAspects(std::shared_ptr<all::ModelNavParameters> nav_params);

    void loadModel(std::filesystem::path path = "assets/motorbike.fbx");
    void setCursorEnabled(bool /* enabled */);

    QWindow* window() { return m_view; }

    void updateMouse();
    void showImage();
    void showModel();
    void screenshot(const std::function<void(const uint8_t* data, uint32_t width, uint32_t height)>& in)
    { /*tbd*/
    }

    void propertyChanged(std::string_view name, std::any value);
    glm::vec3 cursorWorldPosition() const;

private:
    static void addDirectionalLight(Qt3DCore::QNode* node, QVector3D position);

    void createScene(Qt3DCore::QEntity* root);
    void loadImage(QUrl path = QUrl::fromLocalFile(":/13_3840x2160_sbs.jpg"));

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
    void modelExtentChanged(const QVector3D& min, const QVector3D& max);

private:
    Qt3DExtras::Qt3DWindow* m_view{ nullptr };
    std::unique_ptr<Qt3DCore::QEntity> m_rootEntity;
    Qt3DCore::QEntity* m_sceneEntity = nullptr;
    Qt3DCore::QEntity* m_userEntity = nullptr;
    Qt3DCore::QEntity* m_skyBox = nullptr;
    Qt3DRender::QRayCaster* m_raycaster;

    std::unordered_map<QString, Qt3DRender::QMaterial*> m_materials;
    QStereoForwardRenderer* m_renderer;
    QStereoProxyCamera* m_camera;
    CursorEntity* m_cursor;
    float cursor_scale = 1.0f;
    all::StereoCamera* m_stereoCamera;
    all::CameraMode m_cameraMode = all::CameraMode::Stereo;

    Picker* m_picker;
    std::shared_ptr<all::ModelNavParameters> m_nav_params;

    FrustumRect* m_frustumRect{ nullptr };
    Frustum* m_centerFrustum{ nullptr };
    Frustum* m_leftFrustum{ nullptr };
    Frustum* m_rightFrustum{ nullptr };
};
} // namespace all::qt3d
