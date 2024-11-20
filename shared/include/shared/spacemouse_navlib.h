#pragma once
#include <shared/spacemouse.h>
#include <shared/navmodel_navlib.h>

namespace all {
class SpacemouseNavlib : public Spacemouse
{
public:
    SpacemouseNavlib(all::StereoCamera* camera, std::shared_ptr<all::ModelNavParameters> nav_params);

public:
    void setUseUserPivot(bool user) override
    {
        m_model.Write("pivot.user", user);
    }

protected:
    TDx::SpaceMouse::CCommandSet m_menu{ "Default", "Ribbon" };
    CNavigationModel m_model;
};
} // namespace all
