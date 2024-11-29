#pragma once
#include <Qt3DRender/QCamera>
#include <shared/stereo_camera.h>

namespace all::qt3d {
class QStereoProxyCamera : Qt3DCore::QEntity
{
public:
    QStereoProxyCamera(Qt3DCore::QNode* parent = nullptr);

public:
    inline Qt3DRender::QCamera* leftCamera() const { return m_leftCamera; }
    inline Qt3DRender::QCamera* rightCamera() const { return m_rightCamera; }
    inline Qt3DRender::QCamera* centerCamera() const { return m_centerCamera; }

    void updateViewMatrices(const QVector3D& position, const QVector3D& forwardVector,
                            const QVector3D& upVector,
                            float convergenceDistance, float interocularDistance,
                            all::StereoCamera::Mode mode);
    void updateProjection(float nearPlane, float farPlane,
                          float fovY, float aspectRatio,
                          float convergenceDistance, float interocularDistance,
                          all::StereoCamera::Mode mode);

private:
    Qt3DRender::QCamera* m_leftCamera{ nullptr };
    Qt3DRender::QCamera* m_rightCamera{ nullptr };
    Qt3DRender::QCamera* m_centerCamera{ nullptr };
};

} // namespace all::qt3d
