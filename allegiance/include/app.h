#pragma once
#include <QApplication>
#include <QIcon>
#include <QPalette>
#include <QStyleFactory>
#include <window.h>
#include <stereo_camera.h>

#if ALLEGIANCE_SERENITY
#include "serenity/serenity_qt.h"
#include "serenity_impl_qt.h"
using Implementation = SerenityImplQt;
using Application = all::SerenityGuiApplication;
#else
#include "qt3d_impl.h"
using Implementation = Qt3DImpl;
using Application = QApplication;
#endif

#include "spacemouse.h"

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
            Q_EMIT OnClose();
            break;
        case QEvent::Type::KeyPress:
            m_window->OnKeyPress(static_cast<::QKeyEvent*>(event));
            break;
        case QEvent::Type::Wheel:
            if (obj == m_window->GetGraphicsWindow()) {
                Q_EMIT OnScroll(static_cast<::QWheelEvent*>(event));
            }
            break;
        case QEvent::Type::MouseMove:
        case QEvent::Type::MouseButtonPress:
        case QEvent::Type::MouseButtonRelease:
            if (obj == m_window->GetGraphicsWindow()) {
                Q_EMIT OnMouseEvent(static_cast<::QMouseEvent*>(event));
            }
            break;
        default:
            break;
        }
        return QObject::eventFilter(obj, event);
    }
Q_SIGNALS:
    void OnClose();
    void OnMouseEvent(::QMouseEvent* e);
    void OnScroll(::QWheelEvent* e);

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
        QCoreApplication::setApplicationName(QStringLiteral("Schneider Demo - " ) + ALLEGIANCE_BUILD_STR);
        QCoreApplication::setApplicationVersion(QStringLiteral("0.1.0"));
        app.setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
        app.setWindowIcon(QIcon{ QStringLiteral(":/tlr.ico") });

        camera.SetShear(ALLEGIANCE_SERENITY);

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
        auto& qwin = *impl->GetWindow();

        QObject::connect(&watcher, &WindowDestructionWatcher::OnClose,
                         [this]() {
                             impl.reset();
                             app.quit();
                         });
        QObject::connect(&watcher, &WindowDestructionWatcher::OnScroll,
                         [this](::QWheelEvent* e) {
                               camera.SetRadius(camera.GetRadius() - e->angleDelta().y() * 0.01f);
                         });
        QObject::connect(cc, &all::CameraControl::OnLoadImage,
                         [this]() {
                             impl->ShowImage();
                         });
        auto a = new QAction(QIcon{":3D_contrast.png"}, "Load Model", cc);
        QObject::connect(cc, &all::CameraControl::OnLoadModel, a, &QAction::trigger);
        QObject::connect(a, &QAction::triggered,
                         [this]() {
                             impl->ShowModel();
                         });
        QObject::connect(cc, &all::CameraControl::OnClose,
                         [this]() {
                             app.postEvent(&wnd, new QCloseEvent);
                         });
        QObject::connect(&watcher, &WindowDestructionWatcher::OnMouseEvent,
                         [this](::QMouseEvent* e) {
                             switch (e->type()) {
                             case QEvent::MouseButtonPress:
                                 if (e->buttons() & Qt::MouseButton::LeftButton) {
                                     input.is_pressed = true;
                                     input.skip_first = true;
                                 }
                                 break;

                             case QEvent::MouseButtonRelease:
                                 if (e->button() == Qt::MouseButton::LeftButton) {
                                     input.is_pressed = false;
                                 }
                                 break;
                             case QEvent::MouseMove: {
                                 if (!input.is_pressed)
                                     break;

                                 auto pos = e->pos();

                                 if (input.skip_first) {
                                     input.skip_first = false;
                                     input.last_pos = pos;
                                     break;
                                 }

                                 auto dx = pos.x() - input.last_pos.x();
                                 auto dy = pos.y() - input.last_pos.y();

                                 camera.SetPhi(camera.GetPhi() + dx * 0.01f);
                                 camera.SetTheta(camera.GetTheta() - dy * 0.01f);
                                 input.last_pos = pos;
                             } break;
                             default:
                                 break;
                             }
                         });

        QObject::connect(&qwin, &QWindow::widthChanged, [this, &qwin](int width) {
            camera.SetAspectRatio(float(qwin.width()) / qwin.height());
        });
        QObject::connect(&qwin, &QWindow::heightChanged, [this, &qwin](int height) {
            camera.SetAspectRatio(float(qwin.width()) / qwin.height());
        });
        QObject::connect(cc, &all::CameraControl::OnFocusPlaneChanged, [this](float v) {
            camera.SetConvergencePlaneDistance(v);
        });
        QObject::connect(cc, &all::CameraControl::OnEyeDisparityChanged, [this](float v) {
            camera.SetInterocularDistance(v);
        });
        QObject::connect(cc, &all::CameraControl::OnToggleCursor, [this](bool checked) {
            impl->SetCursorEnabled(checked);
        });
#ifdef WITH_NAVLIB
        spacemouse.addAction(a);
#endif

        impl->CreateAspects(&camera);
        wnd.show();
    }

public:
    int
    Start() noexcept
    {
        return app.exec();
    }

private:
    void MakeScene() { }

private:
    Application app;
    std::optional<Implementation> impl;
    all::Window wnd;
    WindowDestructionWatcher watcher{ &wnd };
    all::OrbitalStereoCamera camera;
#ifdef WITH_SPNAV
    SpacemouseSpnav spacemouse{&camera};
#endif
#ifdef WITH_NAVLIB
    SpacemouseNavlib spacemouse{&camera};
#endif
    all::MouseTracker input;
};
