#pragma once
#include <shared/spacemouse.h>
#include <atomic>
#include <thread>

namespace all {
struct ModelNavParameters;
class StereoCamera;

class SpacemouseSpnav : public Spacemouse
{
public:
    SpacemouseSpnav(all::StereoCamera* camera, std::shared_ptr<all::ModelNavParameters>);
    ~SpacemouseSpnav();

public:
    void setUseUserPivot(bool) override
    {
    }

    float m_rotFactor = 0.0001;
    float m_translFactor = 0.001;

protected:
    void poll();

    std::thread m_poller;
    std::atomic_bool stop{ false };
};
} // namespace all
