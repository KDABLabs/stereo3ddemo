#pragma once
#include <QMainWindow>
#include <QMenuBar>
#include <QDockWidget>
#include <QKeyEvent>
#include "camera_control.h"

namespace all
{
    class Window : public QMainWindow
    {
        Q_OBJECT
    public:
        Window(QWindow* window, QSize size)noexcept
            :window(window)
        {
            resize(size);
            CreateDockWidget();

            QWidget* widget = QWidget::createWindowContainer(window);
            setCentralWidget(widget);

            QMenu* fileMenu = menuBar()->addMenu("File");

            fileMenu->addAction("Load Model", [this]() {OnLoadModel(); });
            fileMenu->addAction("Load Image", [this]() {OnLoadImage(); });
            fileMenu->addAction("Exit", [this]() {close(); });
        }
    public:
        auto* GetCameraControl() { return camera_control; }
    private:
        void CreateDockWidget()
        {
            QDockWidget* dock = new QDockWidget("Camera", this);
            camera_control = new CameraControl(dock);
            dock->setWidget(camera_control);
            addDockWidget(Qt::RightDockWidgetArea, dock);
            dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
        }
        void keyPressEvent(QKeyEvent* e) override
        {
            if ((e->key() == Qt::Key_Return) && (e->modifiers() & Qt::AltModifier))
                setWindowState(Qt::WindowFullScreen);
            if (e->key() == Qt::Key_Escape)
                if (isFullScreen())
                    setWindowState(Qt::WindowNoState);
        }
    signals:
        void OnClose();
        void OnLoadModel();
        void OnLoadImage();
    private:
        QWindow* window;
        CameraControl* camera_control;
    };
}
