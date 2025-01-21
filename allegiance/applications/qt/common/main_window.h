#pragma once
#include <QMainWindow>
#include <QMenuBar>
#include <QDockWidget>
#include <QKeyEvent>

namespace all::qt {

class SideMenu;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QSize size);
    ~MainWindow();

    QWindow* embeddedWindow() const noexcept;
    void setEmbeddedWindow(QWindow* embeddedWindow); // Takes ownership of embeddedWindow;

    SideMenu* sideMenu();

    bool onKeyPress(QKeyEvent* e);
    bool onKeyRelease(QKeyEvent* e);

    [[nodiscard]] bool isShiftPressed() const;

    void setMouseGlobalPosition(int x, int y);
    [[nodiscard]] QPoint mouseGlobalPosition() const;

    void setMousePressed(bool pressed);
    [[nodiscard]] bool mousePressed() const;

    // This is controlled from within QML to request that mouse is to be "locked in place".
    // Used for the slider incremental adjustments.
    [[nodiscard]] bool lockMouseInPlace() const;

private:
    void createDockWidget();

Q_SIGNALS:
    void onClose();
    void onScreenshot();

private:
    QWindow* m_embeddedWindow{ nullptr };
    SideMenu* m_sideMenu;
    bool m_cursor = true;

    bool m_mousePressed = false;
    int m_mouseGlobalPositionX{ 0 };
    int m_mouseGlobalPositionY{ 0 };
};
} // namespace all::qt
