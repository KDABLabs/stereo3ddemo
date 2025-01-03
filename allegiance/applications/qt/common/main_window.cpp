#include <applications/qt/common/main_window.h>
#include <applications/qt/common/side_menu.h>

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

SideMenu* MainWindow::sideMenu()
{
    return m_sideMenu;
}

// Issue: unstable focus on camera control widget resizing
bool MainWindow::onKeyPress(QKeyEvent* e)
{
    switch (e->key()) {
    case Qt::Key_F11:
        setWindowState(isFullScreen() ? Qt::WindowNoState : Qt::WindowFullScreen);
        return true;
    case Qt::Key_Return:
        if (e->modifiers() & Qt::AltModifier)
            setWindowState(isFullScreen() ? Qt::WindowNoState : Qt::WindowFullScreen);
        return true;
    case Qt::Key_Escape:
        if (isFullScreen())
            setWindowState(Qt::WindowNoState);
        return true;
    case Qt::Key_Space:
        m_cursor = !m_cursor;
        setCursor(m_cursor ? Qt::ArrowCursor : Qt::BlankCursor);
        return true;
    case Qt::Key_F12:
        Q_EMIT onScreenshot();
        return true;
    default:
        break;
    }
    if (m_sideMenu != nullptr)
        return m_sideMenu->onKeyPress(e);
    return false;
}

bool MainWindow::onKeyRelease(QKeyEvent* e)
{
    if (m_sideMenu != nullptr)
        return m_sideMenu->onKeyRelease(e);
    return false;
}

void MainWindow::createDockWidget()
{
    QDockWidget* dock = new QDockWidget("Camera", this);
    m_sideMenu = new SideMenu(dock);
    dock->setWidget(m_sideMenu);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
}

} // namespace all::qt
