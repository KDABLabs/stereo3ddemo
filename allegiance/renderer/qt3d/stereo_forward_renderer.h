#pragma once
#include <Qt3DRender/QRenderSurfaceSelector>
#include <shared/stereo_camera.h>

namespace Qt3DRender {
class QCameraSelector;
class QLayer;
class QNoDraw;
class QLayerFilter;
class QCamera;
} // namespace Qt3DRender

namespace all::qt3d {
class QStereoProxyCamera;

class QStereoForwardRenderer : public Qt3DRender::QRenderSurfaceSelector
{
    Q_OBJECT
public:
    enum class Mode {
        Scene,
        StereoImage
    };
    Q_ENUM(Mode)
public:
    explicit QStereoForwardRenderer(Qt3DCore::QNode* parent = nullptr);

public:
    void setCamera(QStereoProxyCamera* newCamera);
    inline QStereoProxyCamera* camera() const { return m_camera; }

    inline Qt3DRender::QCamera* frustumCamera() const { return m_frustumCamera; }
    inline Qt3DRender::QLayer* leftLayer() const { return m_leftLayer; }
    inline Qt3DRender::QLayer* rightLayer() const { return m_rightLayer; }
    inline Qt3DRender::QLayer* stereoImageLayer() const { return m_stereoImageLayer; }
    inline Qt3DRender::QLayer* sceneLayer() const { return m_sceneLayer; }
    inline Qt3DRender::QLayer* cursorLayer() const { return m_cursorLayer; }
    inline Qt3DRender::QLayer* frustumLayer() const { return m_frustumLayer; }
    inline Qt3DRender::QLayer* focusAreaLayer() const { return m_focusAreaLayer; }
    inline Qt3DRender::QLayer* focusPlaneLayer() const { return m_focusPlaneLayer; }

    void setMode(Mode mode);
    inline Mode mode() const { return m_mode; }

    void setDisplayMode(all::DisplayMode displayMode);
    inline all::DisplayMode displayMode() const { return m_displayMode; }

Q_SIGNALS:
    void cameraChanged();

private:
    Mode m_mode = Mode::Scene;
    all::DisplayMode m_displayMode = all::DisplayMode::Stereo;

    Qt3DRender::QCameraSelector* m_centerCameraSelector;
    Qt3DRender::QCameraSelector* m_leftCameraSelector;
    Qt3DRender::QCameraSelector* m_rightCameraSelector;
    Qt3DRender::QCameraSelector* m_leftFrustumCameraSelector;
    Qt3DRender::QCameraSelector* m_rightFrustumCameraSelector;

    Qt3DRender::QLayer* m_leftLayer;
    Qt3DRender::QLayer* m_rightLayer;
    Qt3DRender::QLayer* m_stereoImageLayer;
    Qt3DRender::QLayer* m_sceneLayer;
    Qt3DRender::QLayer* m_cursorLayer;
    Qt3DRender::QLayer* m_frustumLayer;
    Qt3DRender::QLayer* m_focusAreaLayer;
    Qt3DRender::QLayer* m_focusPlaneLayer;

    Qt3DRender::QNoDraw* m_sceneNoDraw;
    Qt3DRender::QNoDraw* m_stereoImageNoDraw;
    QStereoProxyCamera* m_camera;
    Qt3DRender::QCamera* m_frustumCamera;

    Qt3DRender::QLayerFilter* m_centerLayerFilter;
    Qt3DRender::QLayerFilter* m_leftLayerFilter;
    Qt3DRender::QLayerFilter* m_rightLayerFilter;
    Qt3DRender::QLayerFilter* m_frustumLayerFilter;
};
} // namespace all::qt3d
