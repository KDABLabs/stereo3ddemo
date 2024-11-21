#pragma once
#include <QObject>
#include <shared/stereo_camera.h>

namespace all::qt {
class StereoCameraBase : public QObject
{
    Q_OBJECT
Q_SIGNALS:
    void viewChanged();
    void projectionChanged();
};

template<typename Camera>
    requires std::is_base_of_v<all::StereoCamera, Camera>
class BasicStereoCamera : public StereoCameraBase, public Camera
{
public:
    BasicStereoCamera() noexcept
        : Camera(all::StereoCamera::API::OpenGL)
    {
    }

    operator Camera*() noexcept
    {
        return static_cast<Camera*>(this);
    }

public:
    virtual void updateViewMatrix() noexcept override
    {
        Camera::updateViewMatrix();
        Q_EMIT viewChanged();
    }
    virtual void updateProjectionMatrix() noexcept override
    {
        Camera::updateProjectionMatrix();
        Q_EMIT projectionChanged();
    }
};

using StereoCamera = BasicStereoCamera<all::StereoCamera>;
using OrbitalStereoCamera = BasicStereoCamera<all::OrbitalStereoCamera>;
} // namespace all::qt
