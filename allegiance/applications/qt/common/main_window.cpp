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
        m_sideMenu->cursorController()->cycleDisplayMode();
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

bool MainWindow::isShiftPressed() const
{
    return m_sideMenu->isShiftPressed();
}

void MainWindow::setMouseGlobalPosition(int x, int y)
{
    m_mouseGlobalPositionX = x;
    m_mouseGlobalPositionY = y;

    m_sideMenu->sceneController()->setMousePressedX(static_cast<float>(x));
}

QPoint MainWindow::mouseGlobalPosition() const
{
    return { m_mouseGlobalPositionX, m_mouseGlobalPositionY };
}

void MainWindow::setMousePressed(bool pressed)
{
    m_mousePressed = pressed;
}

bool MainWindow::mousePressed() const
{
    return m_mousePressed;
}

void MainWindow::mouseHoverOveringOver3DView()
{
    if (
            m_sideMenu->cursorController()->displayMode() == CursorController::CursorDisplayMode::Both ||
            m_sideMenu->cursorController()->displayMode() == CursorController::CursorDisplayMode::SystemCursorOnly)
        setCursor(Qt::ArrowCursor);
    else
        setCursor(Qt::BlankCursor);
}

bool MainWindow::lockMouseInPlace() const
{
    return m_sideMenu->sceneController()->lockMouseInPlace();
}

void MainWindow::createDockWidget()
{
    QDockWidget* dock = new QDockWidget("Camera", this);
    m_sideMenu = new SideMenu(dock);
    dock->setWidget(m_sideMenu);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    connect(dock, &QDockWidget::dockLocationChanged, m_sideMenu, &SideMenu::setClearColor);
}

} // namespace all::qt
