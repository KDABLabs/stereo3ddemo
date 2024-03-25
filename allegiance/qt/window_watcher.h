#pragma once
#include "window.h"
#include <QWindow>

namespace all::qt {
class WindowDestructionWatcher : public QObject
{
    Q_OBJECT
public:
    explicit WindowDestructionWatcher(Window* window)
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
            Q_EMIT OnClose();
            break;
        case QEvent::Type::KeyPress:
            m_window->OnKeyPress(static_cast<::QKeyEvent*>(event));
            event->accept();
            break;
        case QEvent::Type::Wheel:
            if (obj == m_window->GetGraphicsWindow()) {
                Q_EMIT OnScroll(static_cast<::QWheelEvent*>(event));
            }
            break;
        case QEvent::Type::MouseMove:
        case QEvent::Type::MouseButtonPress:
        case QEvent::Type::MouseButtonRelease:
            if (obj == m_window->GetGraphicsWindow()) {
                Q_EMIT OnMouseEvent(static_cast<::QMouseEvent*>(event));
            }
            break;
        default:
            break;
        }
        return QObject::eventFilter(obj, event);
    }
Q_SIGNALS:
    void OnClose();
    void OnMouseEvent(::QMouseEvent* e);
    void OnScroll(::QWheelEvent* e);

private:
    Window* m_window;
};
} // namespace all::qt
