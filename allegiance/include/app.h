#pragma once
#include <QApplication>
#include <QIcon>
#include <QPalette>
#include <QStyleFactory>
#include <window.h>

#if ALLEGIANCE_SERENITY
#include "serenity_impl.h"
using Implementation = SerenityImpl;
using Application = all::SerenityGuiApplication;
#else
#include "qt3d_impl.h"
using Implementation = Qt3DImpl;
using Application = QApplication;
#endif

class WindowDestructionWatcher : public QObject
{
    Q_OBJECT
public:
    explicit WindowDestructionWatcher(all::Window* window)
        : m_window(window)
    {
        qApp->installEventFilter(this);
    }

    bool eventFilter(QObject* obj, QEvent* event) override
    {
        switch (event->type()) {
        case QEvent::Type::Close:
            if (obj != m_window)
                break;
            qApp->removeEventFilter(this);
            emit OnClose();
            break;
        case QEvent::Type::KeyPress:
            m_window->OnKeyPress(static_cast<::QKeyEvent*>(event));
            break;
        case QEvent::Type::MouseMove:
        case QEvent::Type::MouseButtonPress:
        case QEvent::Type::MouseButtonRelease:
            if (obj == m_window->GetGraphicsWindow()) {
                emit OnMouseEvent(static_cast<::QMouseEvent*>(event));
            }
            break;
        default:
            break;
        }
        return QObject::eventFilter(obj, event);
    }
signals:
    void OnClose();
    void OnMouseEvent(::QMouseEvent* e);

private:
    all::Window* m_window;
};

class App
{
public:
    App(int& argc, char** argv)
        : app(argc, argv), impl(std::in_place), wnd(impl->GetWindow(), { 1920, 1080 })
    {
        // Basic setup of the application
        QCoreApplication::setApplicationName(QStringLiteral("Schneider Demo"));
        QCoreApplication::setApplicationVersion(QStringLiteral("0.1.0"));
        app.setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
        app.setWindowIcon(QIcon{ QStringLiteral(":/tlr.ico") });

        // Style definition (darkmode)
        QPalette darkPalette;
        darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::WindowText, Qt::white);
        darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
        darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
        darkPalette.setColor(QPalette::ToolTipText, Qt::white);
        darkPalette.setColor(QPalette::Text, Qt::white);
        darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::ButtonText, Qt::white);
        darkPalette.setColor(QPalette::BrightText, Qt::red);
        darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::HighlightedText, Qt::black);
        app.setPalette(darkPalette);

        auto* cc = wnd.GetCameraControl();

        QObject::connect(&watcher, &WindowDestructionWatcher::OnClose,
                         [this]() {
                             impl.reset();
                             app.quit();
                         });
        QObject::connect(cc, &all::CameraControl::OnClose,
                         [this]() {
                             app.postEvent(&wnd, new QCloseEvent);
                         });
#if ALLEGIANCE_SERENITY
        QObject::connect(&watcher, &WindowDestructionWatcher::OnMouseEvent,
                         [this](::QMouseEvent* e) {
                             impl->OnMouseEvent(e);
                         });
#endif
        impl->CreateAspects(cc);
        wnd.show();
    }

public:
    int Start() noexcept { return app.exec(); }

private:
    void MakeScene() { }

private:
    Application app;
    std::optional<Implementation> impl;
    all::Window wnd;
    WindowDestructionWatcher watcher{ &wnd };
};
