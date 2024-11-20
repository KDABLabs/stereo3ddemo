#pragma once

#include "serenity_impl.h"
#include "serenity_kdgui.h"

namespace all::serenity {

class SerenityImplKDGui : public SerenityImpl
{
public:
    SerenityImplKDGui(all::StereoCamera& camera)
        : SerenityImpl{ std::make_unique<SerenityWindowKDGui>(), camera }
    {
    }

    KDGui::Window* window()
    {
        return static_cast<SerenityWindowKDGui*>(m_window.get())->window();
    }
};

} // namespace all::serenity
