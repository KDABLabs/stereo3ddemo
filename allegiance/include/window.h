#pragma once
#include <QMainWindow>
#include <QMenuBar>
#include <QDockWidget>
#include <QKeyEvent>
#include "camera_control.h"

namespace all {
class Window : public QMainWindow
{
    Q_OBJECT
public:
    Window(QWindow* window, QSize size) noexcept
        : window(window)
    {
        resize(size);
        CreateDockWidget();

        QWidget* widget = QWidget::createWindowContainer(window);
        setCentralWidget(widget);
    }

public:
    auto* GetGraphicsWindow() const noexcept
    {
        return window;
    }
    auto* GetCameraControl() { return camera_control; }

    // Issue: unstable focus on camera control widget resizing
    void OnKeyPress(QKeyEvent* e)
    {
        if (e->key() == Qt::Key_F11)
            setWindowState(isFullScreen() ? Qt::WindowNoState : Qt::WindowFullScreen);
        if ((e->key() == Qt::Key_Return) && (e->modifiers() & Qt::AltModifier))
            setWindowState(Qt::WindowFullScreen);
        if (e->key() == Qt::Key_Escape)
            if (isFullScreen())
                setWindowState(Qt::WindowNoState);
        if (e->key() == Qt::Key_Space) {
            cursor = !cursor;
            if (!cursor)
                setCursor(Qt::BlankCursor);
            else
                setCursor(Qt::ArrowCursor);
        }
        if (e->key() == Qt::Key_F1)
            camera_control->Reload();
    }

private:
    void CreateDockWidget()
    {
        QDockWidget* dock = new QDockWidget("Camera", this);
        camera_control = new CameraControl(dock);
        dock->setWidget(camera_control);
        addDockWidget(Qt::RightDockWidgetArea, dock);
        dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    }

Q_SIGNALS:
    void OnClose();

private:
    QWindow* window;
    CameraControl* camera_control;
    bool cursor = true;
};
} // namespace all
