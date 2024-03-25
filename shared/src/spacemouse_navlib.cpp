#include <shared/spacemouse_navlib.h>

using namespace all;

SpacemouseNavlib::SpacemouseNavlib(all::StereoCamera* camera, std::shared_ptr<all::ModelNavParameters> nav_params)
    : Spacemouse(camera), m_model(std::move(nav_params), camera)
{
    try {
        m_model.EnableNavigation(true);
    } catch (const std::system_error& e) {
        std::cerr << "Could not enable Spacemouse, are the drivers installed? :" << e.what();
    }
    Update();
}
