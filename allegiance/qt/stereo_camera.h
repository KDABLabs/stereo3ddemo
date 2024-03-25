#pragma once
#include <QObject>
#include <shared/stereo_camera.h>

namespace all::qt {
class StereoCameraBase : public QObject
{
    Q_OBJECT
Q_SIGNALS:
    void OnViewChanged();
    void OnProjectionChanged();
};

template<typename Camera>
    requires std::is_base_of_v<all::StereoCamera, Camera>
class BasicStereoCamera : public StereoCameraBase, public Camera
{
public:
    BasicStereoCamera() noexcept = default;
    operator Camera*() noexcept
    {
        return static_cast<Camera*>(this);
    }

public:
    virtual void UpdateViewMatrix() noexcept override
    {
        Camera::UpdateViewMatrix();
        Q_EMIT OnViewChanged();
    }
    virtual void UpdateProjectionMatrix() noexcept override
    {
        Camera::UpdateProjectionMatrix();
        Q_EMIT OnProjectionChanged();
    }
};

using StereoCamera = BasicStereoCamera<all::StereoCamera>;
using OrbitalStereoCamera = BasicStereoCamera<all::OrbitalStereoCamera>;
} // namespace all::qt