#pragma once

namespace all {
class StereoCamera;
class Spacemouse
{
public:
    Spacemouse(all::StereoCamera* camera) noexcept
        : m_camera(camera)
    {
    }
    virtual ~Spacemouse() noexcept = default;

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