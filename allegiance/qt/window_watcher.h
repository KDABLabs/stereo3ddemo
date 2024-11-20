#pragma once
#include "window.h"
#include <QWindow>

namespace all::qt {
class WindowEventWatcher : public QObject
{
    Q_OBJECT
public:
    explicit WindowEventWatcher(Window* window)
        : m_window(window)
    {
        qApp->installEventFilter(this);
    }

    bool eventFilter(QObject* obj, QEvent* event) override
    {
        switch (event->type()) {
        case QEvent::Type::Close:
            if (obj != m_window)
                break;
            qApp->removeEventFilter(this);
            Q_EMIT onClose();
            break;
        case QEvent::Type::KeyPress:
            m_window->onKeyPress(static_cast<::QKeyEvent*>(event));
            event->accept();
            break;
        case QEvent::Type::Wheel:
            if (obj == m_window->graphicsWindow()) {
                Q_EMIT onScroll(static_cast<::QWheelEvent*>(event));
            }
            break;
        case QEvent::Type::MouseMove:
        case QEvent::Type::MouseButtonPress:
        case QEvent::Type::MouseButtonRelease:
            if (obj == m_window->graphicsWindow()) {
                Q_EMIT onMouseEvent(static_cast<::QMouseEvent*>(event));
            }
            break;
        default:
            break;
        }
        return QObject::eventFilter(obj, event);
    }
Q_SIGNALS:
    void onClose();
    void onMouseEvent(::QMouseEvent* e);
    void onScroll(::QWheelEvent* e);

private:
    Window* m_window;
};
} // namespace all::qt
