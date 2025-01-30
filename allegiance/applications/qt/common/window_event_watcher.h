#pragma once
#include <QQuickWidget>
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
        case QEvent::Type::KeyRelease:
            if (m_window->onKeyRelease(static_cast<::QKeyEvent*>(event)))
                return true;
            break;
        case QEvent::Type::Wheel:
            if (obj == m_window->embeddedWindow()) {
                Q_EMIT scrollEvent(static_cast<::QWheelEvent*>(event));
            }
            break;

        case QEvent::Type::MouseButtonPress:
            if (obj == m_window->embeddedWindow()) {
                Q_EMIT mouseEvent(dynamic_cast<::QMouseEvent*>(event));
            }

            // record position when press occurs.
            // This is used by Sliders to have a reference position to compare to for the fine tuning mode
            if (qobject_cast<QQuickWidget*>(obj)) {
                auto e = dynamic_cast<::QMouseEvent*>(event);
                auto globalPos = e->globalPosition();
                m_window->setMousePressed(true);
                m_window->setMouseGlobalPosition(static_cast<size_t>(globalPos.x()), static_cast<size_t>(globalPos.y()));
            }
            break;

        case QEvent::Type::MouseButtonRelease:
            m_window->setMousePressed(false);
            if (obj == m_window->embeddedWindow())
                Q_EMIT mouseEvent(dynamic_cast<::QMouseEvent*>(event));
            break;

        case QEvent::Type::MouseMove:
            if (obj == m_window->embeddedWindow()) {
                auto e = dynamic_cast<::QMouseEvent*>(event);
                Q_EMIT mouseEvent(e);
                m_window->mouseHoverOveringOver3DView();
            }

            // when a slider requests the mouse to be locked in place for gradual adjustments,
            // reposition mouse cursor back to the position it was clicked
            if (qobject_cast<QQuickWidget*>(obj)) {
                if (m_window->mousePressed() && m_window->lockMouseInPlace()) {
                    QCursor::setPos(m_window->mouseGlobalPosition());
                }
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
