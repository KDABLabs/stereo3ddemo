#pragma once

#include <kdbindings/signal.h>
#include <shared/stereo_camera.h>

namespace all::kdgui {

class StereoCameraBase
{
public:
    KDBindings::Signal<bool> viewChanged;
    KDBindings::Signal<bool> projectionChanged;
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
    virtual void updateViewMatrix() noexcept override
    {
        Camera::updateViewMatrix();
        viewChanged.emit(true);
    }
    virtual void updateProjectionMatrix() noexcept override
    {
        Camera::updateProjectionMatrix();
        projectionChanged.emit(true);
    }
};

using StereoCamera = BasicStereoCamera<all::StereoCamera>;
using OrbitalStereoCamera = BasicStereoCamera<all::OrbitalStereoCamera>;

} // namespace all::kdgui
