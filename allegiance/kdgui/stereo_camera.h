#pragma once

#include <kdbindings/signal.h>
#include <shared/stereo_camera.h>

namespace all::kdgui {

class StereoCameraBase
{
public:
    KDBindings::Signal<bool> OnViewChanged;
    KDBindings::Signal<bool> OnProjectionChanged;
};

template<typename Camera>
    requires std::is_base_of_v<all::StereoCamera, Camera>
class BasicStereoCamera : public StereoCameraBase, public Camera
{
public:
    BasicStereoCamera() noexcept
        : Camera(all::StereoCamera::API::Vulkan)
    {
    }

    operator Camera*() noexcept
    {
        return static_cast<Camera*>(this);
    }

public:
    virtual void UpdateViewMatrix() noexcept override
    {
        Camera::UpdateViewMatrix();
        OnViewChanged.emit(true);
    }
    virtual void UpdateProjectionMatrix() noexcept override
    {
        Camera::UpdateProjectionMatrix();
        OnProjectionChanged.emit(true);
    }
};

using StereoCamera = BasicStereoCamera<all::StereoCamera>;
using OrbitalStereoCamera = BasicStereoCamera<all::OrbitalStereoCamera>;

} // namespace all::kdgui
