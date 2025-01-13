#include <applications/qt/serenity/serenity_renderer_qt.h>
#include <applications/qt/serenity/serenity_window_qt.h>

#include <chrono>

using namespace all::serenity;

SerenityRendererQt::SerenityRendererQt(SerenityWindowQt* serenityWindow,
                                       all::StereoCamera& camera,
                                       std::function<void(std::string_view, std::any)> propertyUpdateNotifier)
    : SerenityRenderer{ serenityWindow, camera, propertyUpdateNotifier }
{
}

QWindow* SerenityRendererQt::window() const
{
    return static_cast<SerenityWindowQt*>(m_window)->window();
}

void SerenityRendererQt::onMouseEvent(::QMouseEvent* event)
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
        return SerenityRenderer::onMouseEvent(KDGui::MousePressEvent(timestamp, button, buttons, event->pos().x(), event->pos().y()));
    case QEvent::MouseButtonRelease:
        return SerenityRenderer::onMouseEvent(KDGui::MouseReleaseEvent(timestamp, button, buttons, event->pos().x(), event->pos().y()));
    case QEvent::MouseMove:
        return SerenityRenderer::onMouseEvent(KDGui::MouseMoveEvent(timestamp, buttons, event->pos().x(), event->pos().y()));
    default:
        break;
    }
}
