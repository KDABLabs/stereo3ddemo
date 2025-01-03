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

public:
    QWindow* embeddedWindow() const noexcept;
    void setEmbeddedWindow(QWindow* embeddedWindow); // Takes ownership of embeddedWindow;

    SideMenu* sideMenu();

    void onKeyPress(QKeyEvent* e);

private:
    void createDockWidget();

Q_SIGNALS:
    void onClose();
    void onEyeSeparation(bool increase);
    void onScreenshot();

private:
    QWindow* m_embeddedWindow{ nullptr };
    SideMenu* m_sideMenu;
    bool m_cursor = true;
};
} // namespace all::qt
