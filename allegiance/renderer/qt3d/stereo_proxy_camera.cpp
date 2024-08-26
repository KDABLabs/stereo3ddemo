#include "stereo_proxy_camera.h"

#include <Qt3DCore/QTransform>
#include <Qt3DRender/QCameraLens>

all::qt3d::QStereoProxyCamera::QStereoProxyCamera(Qt3DCore::QNode* parent)
    : Qt3DCore::QEntity(parent)
{
    m_leftCamera = new Qt3DCore::QEntity(this);
    m_rightCamera = new Qt3DCore::QEntity(this);

    m_leftTransform = new Qt3DCore::QTransform;
    m_leftTransform->setObjectName("Left Camera Transform");
    m_leftCamera->addComponent(m_leftTransform);

    m_rightTransform = new Qt3DCore::QTransform;
    m_rightTransform->setObjectName("Right Camera Transform");
    m_rightCamera->addComponent(m_rightTransform);

    m_leftCameraLens = new Qt3DRender::QCameraLens;
    m_leftCameraLens->setObjectName("Left Camera Lens");
    m_leftCamera->addComponent(m_leftCameraLens);

    m_rightCameraLens = new Qt3DRender::QCameraLens;
    m_rightCameraLens->setObjectName("Right Camera Lens");
    m_rightCamera->addComponent(m_rightCameraLens);
}

void all::qt3d::QStereoProxyCamera::SetPositionAndForward(const QVector3D& position, const QQuaternion& rotation)
{
    m_leftTransform->setTranslation(position);
    m_leftTransform->setRotation(rotation);
    m_rightTransform->setTranslation(position);
    m_rightTransform->setRotation(rotation);
}

void all::qt3d::QStereoProxyCamera::SetMatrices(const QMatrix4x4& left, const QMatrix4x4& right)
{
    m_leftTransform->setMatrix(left.inverted());
    m_rightTransform->setMatrix(right.inverted());
}

void all::qt3d::QStereoProxyCamera::SetProjection(const QMatrix4x4& proj, qreal skew, bool same)
{
    QMatrix4x4 m;
    m(0, 2) = same ? skew : -skew;
    m_leftCameraLens->setProjectionMatrix(m * proj);
    m(0, 2) = skew;
    m_rightCameraLens->setProjectionMatrix(m * proj);
}
