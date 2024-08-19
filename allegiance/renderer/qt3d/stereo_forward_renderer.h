#pragma once
#include <Qt3DRender/QRenderSurfaceSelector>

namespace Qt3DRender {
class QCameraSelector;
class QLayer;
class QNoDraw;
class QLayerFilter;
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
    QStereoForwardRenderer(Qt3DCore::QNode* parent = nullptr);

public:
    QStereoProxyCamera* camera() const
    {
        return m_camera;
    }

    Qt3DRender::QLayer* leftLayer() const
    {
        return m_leftLayer;
    }

    Qt3DRender::QLayer* rightLayer() const
    {
        return m_rightLayer;
    }

    Qt3DRender::QLayer* stereoImageLayer() const
    {
        return m_stereoImageLayer;
    }

    Qt3DRender::QLayer* sceneLayer() const
    {
        return m_sceneLayer;
    }

    Qt3DRender::QLayer* cursorLayer() const
    {
        return m_cursorLayer;
    }

    void setMode(Mode mode);

    void setCamera(QStereoProxyCamera* newCamera);

Q_SIGNALS:
    void cameraChanged();

private:
    Mode m_mode = Mode::Scene;
    Qt3DRender::QCameraSelector* m_leftCamera;
    Qt3DRender::QCameraSelector* m_rightCamera;
    Qt3DRender::QLayer* m_leftLayer;
    Qt3DRender::QLayer* m_rightLayer;
    Qt3DRender::QLayer* m_stereoImageLayer;
    Qt3DRender::QLayer* m_sceneLayer;
    Qt3DRender::QLayer* m_cursorLayer;
    Qt3DRender::QNoDraw* m_sceneNoDraw;
    Qt3DRender::QNoDraw* m_stereoImageNoDraw;
    QStereoProxyCamera* m_camera;

    Qt3DRender::QLayerFilter* m_leftLayerFilter;
    Qt3DRender::QLayerFilter* m_rightLayerFilter;
};
} // namespace all::qt3d
