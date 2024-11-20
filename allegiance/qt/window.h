#pragma once
#include <QMainWindow>
#include <QMenuBar>
#include <QDockWidget>
#include <QKeyEvent>
#include "camera_control.h"

namespace all::qt {
class Window : public QMainWindow
{
    Q_OBJECT
public:
    Window(QWindow* window, QSize size) noexcept
        : window(window)
    {
        resize(size);
        createDockWidget();

        QWidget* widget = QWidget::createWindowContainer(window);
        setCentralWidget(widget);
    }

public:
    auto* graphicsWindow() const noexcept
    {
        return window;
    }
    auto* cameraControl() { return camera_control; }

    // Issue: unstable focus on camera control widget resizing
    void onKeyPress(QKeyEvent* e)
    {
        switch (e->key()) {
        case Qt::Key_F11:
            setWindowState(isFullScreen() ? Qt::WindowNoState : Qt::WindowFullScreen);
            break;
        case Qt::Key_Return:
            if (e->modifiers() & Qt::AltModifier)
                setWindowState(isFullScreen() ? Qt::WindowNoState : Qt::WindowFullScreen);
            break;
        case Qt::Key_Escape:
            if (isFullScreen())
                setWindowState(Qt::WindowNoState);
            break;
        case Qt::Key_Space:
            cursor = !cursor;
            setCursor(cursor ? Qt::ArrowCursor : Qt::BlankCursor);
            break;
        case Qt::Key_F1:
            camera_control->reload();
            break;
        case Qt::Key_F3:
            onEyeSeparation(false);
            break;
        case Qt::Key_F12:
            onEyeSeparation(true);
            break;
        case Qt::Key_F2:
            Q_EMIT onScreenshot();
            break;

        default:
            break;
        }
    }

private:
    void createDockWidget()
    {
        QDockWidget* dock = new QDockWidget("Camera", this);
        camera_control = new CameraControl(dock);
        dock->setWidget(camera_control);
        addDockWidget(Qt::RightDockWidgetArea, dock);
        dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    }

Q_SIGNALS:
    void onClose();
    void onEyeSeparation(bool increase);
    void onScreenshot();

private:
    QWindow* window;
    CameraControl* camera_control;
    bool cursor = true;
};
} // namespace all::qt
