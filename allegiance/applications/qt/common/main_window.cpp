#include <applications/qt/common/main_window.h>
#include <applications/qt/common/camera_control.h>

namespace all::qt {

MainWindow::MainWindow(QSize size)
{
    resize(size);
    createDockWidget();
}

void MainWindow::setEmbeddedWindow(QWindow* embeddedWindow)
{
    m_embeddedWindow = embeddedWindow;
    setCentralWidget(QWidget::createWindowContainer(m_embeddedWindow)); // Takes ownership of embeddedWindow
}

MainWindow::~MainWindow()
{
}

QWindow* MainWindow::embeddedWindow() const noexcept
{
    return m_embeddedWindow;
}

CameraControl* MainWindow::cameraControl()
{
    return m_cameraControl;
}

// Issue: unstable focus on camera control widget resizing
void MainWindow::onKeyPress(QKeyEvent* e)
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
        m_cursor = !m_cursor;
        setCursor(m_cursor ? Qt::ArrowCursor : Qt::BlankCursor);
        break;
    case Qt::Key_F1:
        m_cameraControl->reload();
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

void MainWindow::createDockWidget()
{
    QDockWidget* dock = new QDockWidget("Camera", this);
    m_cameraControl = new CameraControl(dock);
    dock->setWidget(m_cameraControl);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
}

} // namespace all::qt
