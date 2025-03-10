#pragma once
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DRender/QScreenRayCaster>

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
class Frustum;
class FrustumRect;
class FocusArea;
class FocusPlanePreview;

class Qt3DRenderer : public QObject
{
    Q_OBJECT
public:
    explicit Qt3DRenderer(Qt3DExtras::Qt3DWindow* view,
                          all::StereoCamera& camera,
                          std::function<void(std::string_view, std::any)> propertyUpdateNotifier);
    ~Qt3DRenderer();

public:
    void viewChanged();
    void projectionChanged();
    void createAspects(std::shared_ptr<all::ModelNavParameters> nav_params);

    void loadModel(std::filesystem::path path = "assets/motorbike.obj");
    void viewAll();
    void setCursorEnabled(bool /* enabled */);

    QWindow* window() { return m_view; }

    bool hoversFocusArea(int x, int y) const;
    void showImage();
    void showModel();
    void screenshot(const std::function<void(const uint8_t* data, uint32_t width, uint32_t height)>& in)
    { /*tbd*/
    }

    void onMouseEvent(::QMouseEvent* event);

    void propertyChanged(std::string_view name, std::any value);
    glm::vec3 cursorWorldPosition() const;
    glm::vec3 sceneCenter() const;
    glm::vec3 sceneExtent() const;
    float fieldOfView() const;
    float aspectRatio() const;

    void completeInitialization();

private:
    static void addDirectionalLight(Qt3DCore::QNode* node, QVector3D position);

    void createScene(Qt3DCore::QEntity* root);
    void loadImage(QUrl path = QUrl::fromLocalFile(":/13_3840x2160_sbs.jpg"));

    struct SceneExtent {
        QVector3D min, max;
    };
    SceneExtent calculateSceneExtent(Qt3DCore::QEntity* entity) const;

protected:
    void _calculateSceneDimensions(Qt3DCore::QEntity* entity, QVector3D& minBounds, QVector3D& maxBounds) const;
    void modelExtentChanged(const QVector3D& min, const QVector3D& max);
    void setupCameraBasedOnSceneExtent();
    void requestFocusForFocusArea();
    void handleFocusForFocusArea();

private:
    void afRaycasterHitResult(size_t idx, const Qt3DRender::QAbstractRayCaster::Hits& hits);
    void cursorHitResult(const Qt3DRender::QAbstractRayCaster::Hits& hits);

    Qt3DExtras::Qt3DWindow* m_view{ nullptr };
    std::unique_ptr<Qt3DCore::QEntity> m_rootEntity;
    Qt3DCore::QEntity* m_sceneEntity = nullptr;
    Qt3DCore::QEntity* m_userEntity = nullptr;
    Qt3DRender::QScreenRayCaster* m_cursorRaycaster;

    QStereoForwardRenderer* m_renderer;
    QStereoProxyCamera* m_camera;
    QStereoProxyCamera* m_frustumAmplifiedCamera;
    CursorEntity* m_cursor;

    float cursor_scale = 1.0f;
    QVector3D m_sceneCenter;
    QVector3D m_sceneExtent;
    all::StereoCamera* m_stereoCamera;
    bool m_autoFocus{ false };

    std::shared_ptr<all::ModelNavParameters> m_nav_params;

    FrustumRect* m_frustumRect{ nullptr };
    Frustum* m_leftFrustum{ nullptr };
    Frustum* m_rightFrustum{ nullptr };

    FocusArea* m_focusArea{ nullptr };
    FocusPlanePreview* m_focusPlanePreview{ nullptr };

    static constexpr size_t AFSamplesY = 2;
    static constexpr size_t AFSamplesX = 2;
    static constexpr size_t AFSamples = AFSamplesY * AFSamplesX;

    std::array<Qt3DRender::QScreenRayCaster*, AFSamples> m_afRayCasters{ nullptr };
    std::array<float, AFSamples> m_lastAfHitDistances{};
    bool m_afResultUpdateRequested{ false };
    std::function<void(std::string_view, std::any)> m_propertyUpdateNofitier;
    bool m_focusUpdateRequested{ false };
};
} // namespace all::qt3d
