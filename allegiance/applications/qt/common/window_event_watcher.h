#pragma once
#include <applications/qt/common/main_window.h>
#include <QWindow>

namespace all::qt {
class WindowEventWatcher : public QObject
{
    Q_OBJECT
public:
    explicit WindowEventWatcher(MainWindow* window)
        : m_window(window)
    {
        qApp->installEventFilter(this);
    }

    bool eventFilter(QObject* obj, QEvent* event) override
    {
        switch (event->type()) {
        case QEvent::Type::Close:
            if (obj != m_window)
                return false;
            qApp->removeEventFilter(this);
            Q_EMIT close();
            return true;
        case QEvent::Type::KeyPress:
            if (m_window->onKeyPress(static_cast<::QKeyEvent*>(event)))
                return true;
            break;
        case QEvent::Type::Wheel:
            if (obj == m_window->embeddedWindow()) {
                Q_EMIT scrollEvent(static_cast<::QWheelEvent*>(event));
            }
            break;
        case QEvent::Type::MouseMove:
        case QEvent::Type::MouseButtonPress:
        case QEvent::Type::MouseButtonRelease:
            if (obj == m_window->embeddedWindow()) {
                Q_EMIT mouseEvent(static_cast<::QMouseEvent*>(event));
            }
            break;
        default:
            break;
        }
        return QObject::eventFilter(obj, event);
    }
Q_SIGNALS:
    void close();
    void mouseEvent(::QMouseEvent* e);
    void scrollEvent(::QWheelEvent* e);

private:
    MainWindow* m_window;
};
} // namespace all::qt
