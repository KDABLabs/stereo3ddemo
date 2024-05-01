// Header only
#pragma once
#include "serenity_impl.h"
#include "serenity_qt.h"

namespace all::serenity {
class SerenityImplQt : public SerenityImpl
{
public:
    SerenityImplQt(all::StereoCamera& camera)
        : SerenityImpl{ std::make_unique<SerenityWindowQt>(), camera }
    {
    }

    QWindow* GetWindow()
    {
        return static_cast<SerenityWindowQt*>(m_window.get())->GetWindow();
    }

    void UpdateMouse() {}
};
} // namespace all::serenity