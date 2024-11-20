#pragma once
#include <Qt3DCore/QEntity>

namespace Qt3DCore {
class QTransform;
} // namespace Qt3DCore

namespace Qt3DRender {
class QCameraLens;
} // namespace Qt3DRender

namespace all::qt3d {
class QStereoProxyCamera : Qt3DCore::QEntity
{
public:
    QStereoProxyCamera(Qt3DCore::QNode* parent = nullptr);

public:
    Qt3DCore::QEntity* leftCamera() const
    {
        return m_leftCamera;
    }
    Qt3DCore::QEntity* rightCamera() const
    {
        return m_rightCamera;
    }

    void setPositionAndForward(const QVector3D& position, const QQuaternion& rotation);

    void setMatrices(const QMatrix4x4& left, const QMatrix4x4& right);
    void setProjection(const QMatrix4x4& proj, qreal skew, bool same = false);

private:
    Qt3DCore::QEntity* m_leftCamera;
    Qt3DCore::QEntity* m_rightCamera;
    Qt3DCore::QTransform* m_leftTransform;
    Qt3DCore::QTransform* m_rightTransform;
    Qt3DRender::QCameraLens* m_leftCameraLens;
    Qt3DRender::QCameraLens* m_rightCameraLens;
};

} // namespace all::qt3d