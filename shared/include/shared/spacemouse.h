#pragma once

#include <memory>

namespace all {
class StereoCamera;
struct ModelNavParameters;

class Spacemouse
{
public:
    Spacemouse(all::StereoCamera* camera, std::shared_ptr<all::ModelNavParameters>) noexcept
        : m_camera(camera)
    {
    }
    virtual ~Spacemouse() noexcept = default;

    virtual void SetUseUserPivot(bool)
    {
    }

protected:
    all::StereoCamera* Camera() const noexcept
    {
        return m_camera;
    }

    virtual void Update()
    {
    }

protected:
    all::StereoCamera* m_camera;
};
} // namespace all
