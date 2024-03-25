#pragma once
#include <shared/stereo_camera.h>

namespace all::serenity {
class StereoProxyCamera : public Serenity::StereoCamera
{
public:
    using StereoCamera::StereoCamera;

public:
    void SetMatrices(const glm::mat4& left, const glm::mat4& right, const glm::mat4& center) noexcept
    {
        *(const_cast<KDBindings::Property<glm::mat4>*>(&leftEyeViewMatrix)) = left;
        *(const_cast<KDBindings::Property<glm::mat4>*>(&rightEyeViewMatrix)) = right;
        *(const_cast<KDBindings::Property<glm::mat4>*>(&centerEyeViewMatrix)) = center;
    }
};
}