// Header only
#pragma once
#include "serenity_impl.h"
#include "serenity_qt.h"

#include <QMouseEvent>
#include <KDGui/gui_events.h>
#include <chrono>

namespace all::serenity {
class SerenityImplQt : public SerenityImpl
{
public:
    SerenityImplQt(all::StereoCamera& camera)
        : SerenityImpl{ std::make_unique<SerenityWindowQt>(), camera }
    {
    }

    QWindow* window()
    {
        return static_cast<SerenityWindowQt*>(m_window.get())->window();
    }

    void updateMouse() { }

    void onMouseEvent(::QMouseEvent* event)
    {
        KDGui::MouseButtons buttons = KDGui::NoButton;
        KDGui::MouseButton button = KDGui::NoButton;

        if (event->buttons() & Qt::LeftButton)
            buttons |= KDGui::LeftButton;
        if (event->buttons() & Qt::RightButton)
            buttons |= KDGui::RightButton;
        if (event->buttons() & Qt::MiddleButton)
            buttons |= KDGui::MiddleButton;

        if (event->button() == Qt::LeftButton)
            button = KDGui::LeftButton;
        else if (event->button() == Qt::RightButton)
            button = KDGui::RightButton;
        else if (event->button() == Qt::MiddleButton)
            button = KDGui::MiddleButton;

        const std::time_t timestamp = std::time(nullptr);

        switch (event->type()) {
        case QEvent::MouseButtonPress:
            return SerenityImpl::onMouseEvent(KDGui::MousePressEvent(timestamp, button, buttons, event->pos().x(), event->pos().y()));
        case QEvent::MouseButtonRelease:
            return SerenityImpl::onMouseEvent(KDGui::MouseReleaseEvent(timestamp, button, buttons, event->pos().x(), event->pos().y()));
        case QEvent::MouseMove:
            return SerenityImpl::onMouseEvent(KDGui::MouseMoveEvent(timestamp, buttons, event->pos().x(), event->pos().y()));
        default:
            break;
        }
    }
};

} // namespace all::serenity
