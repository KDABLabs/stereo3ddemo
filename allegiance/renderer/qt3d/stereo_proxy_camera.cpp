#include "stereo_proxy_camera.h"

#include <Qt3DCore/QTransform>
#include <Qt3DRender/QCameraLens>

namespace all::qt3d {

QStereoProxyCamera::QStereoProxyCamera(Qt3DCore::QNode* parent)
    : Qt3DCore::QEntity(parent)
{
    m_leftCamera = new Qt3DRender::QCamera(this);
    m_rightCamera = new Qt3DRender::QCamera(this);
    m_centerCamera = new Qt3DRender::QCamera(this);
}

void QStereoProxyCamera::updateViewMatrices(const QVector3D& position, const QVector3D& forwardVector,
                                            const QVector3D& upVector,
                                            float convergenceDistance, float interocularDistance,
                                            all::StereoCamera::Mode mode)
{
    const QVector3D viewCenter = position + forwardVector * convergenceDistance;
    const QVector3D rightVector = QVector3D::crossProduct(forwardVector, upVector).normalized();
    const float halfInterocularDistance = interocularDistance * 0.5f;
    const QVector3D rightShift = rightVector * halfInterocularDistance;

    m_centerCamera->setPosition(position);
    m_leftCamera->setPosition(position - rightShift);
    m_rightCamera->setPosition(position + rightShift);

    m_centerCamera->setUpVector(upVector);
    m_leftCamera->setUpVector(upVector);
    m_rightCamera->setUpVector(upVector);

    m_centerCamera->setViewCenter(viewCenter);

    if (mode == all::StereoCamera::Mode::ToeIn) {
        m_leftCamera->setViewCenter(viewCenter);
        m_rightCamera->setViewCenter(viewCenter);
    } else { // all::StereoCamera::Mode::AsymmetricFrustum
        m_leftCamera->setViewCenter(viewCenter - rightShift);
        m_rightCamera->setViewCenter(viewCenter + rightShift);
    }
}

void QStereoProxyCamera::updateProjection(float nearPlane, float farPlane,
                                          float fovY, float aspectRatio,
                                          float convergenceDistance,
                                          float interocularDistance,
                                          all::StereoCamera::Mode mode)
{
    m_centerCamera->setAspectRatio(aspectRatio);
    m_leftCamera->setAspectRatio(aspectRatio);
    m_rightCamera->setAspectRatio(aspectRatio);

    m_centerCamera->setNearPlane(nearPlane);
    m_leftCamera->setNearPlane(nearPlane);
    m_rightCamera->setNearPlane(nearPlane);

    m_centerCamera->setFarPlane(farPlane);
    m_leftCamera->setFarPlane(farPlane);
    m_rightCamera->setFarPlane(farPlane);

    m_centerCamera->setProjectionType(Qt3DRender::QCameraLens::PerspectiveProjection);
    m_centerCamera->setFieldOfView(fovY);

    if (mode == all::StereoCamera::Mode::ToeIn) {
        m_leftCamera->setProjectionType(Qt3DRender::QCameraLens::PerspectiveProjection);
        m_rightCamera->setProjectionType(Qt3DRender::QCameraLens::PerspectiveProjection);

        m_leftCamera->setFieldOfView(fovY);
        m_rightCamera->setFieldOfView(fovY);
    } else { // all::StereoCamera::Mode::AsymmetricFrustum
        const float halfInterocularDistance = interocularDistance * 0.5f;
        const float halfVFov = std::tan(qDegreesToRadians(fovY) * 0.5f);
        const float halfHeightAtConvergence = halfVFov * nearPlane;
        const float halfWidthAtConvergence = aspectRatio * halfHeightAtConvergence;
        const float nearConvergenceRatio = nearPlane / convergenceDistance;
        const float frustumShift = halfInterocularDistance * nearConvergenceRatio;

        m_leftCamera->setProjectionType(Qt3DRender::QCameraLens::FrustumProjection);
        m_leftCamera->setTop(halfHeightAtConvergence);
        m_leftCamera->setBottom(-halfHeightAtConvergence);
        m_leftCamera->setLeft(-halfWidthAtConvergence + frustumShift);
        m_leftCamera->setRight(halfWidthAtConvergence + frustumShift);

        m_rightCamera->setProjectionType(Qt3DRender::QCameraLens::FrustumProjection);
        m_rightCamera->setTop(halfHeightAtConvergence);
        m_rightCamera->setBottom(-halfHeightAtConvergence);
        m_rightCamera->setLeft(-halfWidthAtConvergence - frustumShift);
        m_rightCamera->setRight(halfWidthAtConvergence - frustumShift);
    }
}

} // namespace all::qt3d
