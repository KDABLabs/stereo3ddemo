#pragma once
#include "cursor.h"

#include <QApplication>
#include <QIcon>
#include <QPalette>
#include <QStyleFactory>
#include <window.h>
#include <stereo_camera.h>
#include <ui/style.h>
#include <ui/camera_controller.h>
#include <QFileDialog>
#include <spacemouse.h>

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
            event->accept();
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

class QMLNodes
{
public:
    QMLNodes()
    {
        qmlRegisterSingletonInstance("Schneider", 1, 0, "Scene", &scene);
        qmlRegisterSingletonInstance("Schneider", 1, 0, "Camera", &camera);
        qmlRegisterSingletonInstance("Schneider", 1, 0, "Cursor", &cursor);
        qmlRegisterSingletonInstance("Schneider", 1, 0, "Style", &style);
    }

public:
    SceneController scene;
    CameraController camera;
    CursorController cursor;
    AppStyle style;
};

class App
{
public:
    App(int& argc, char** argv)
        : app(argc, argv), impl(std::in_place), wnd(impl->GetWindow(), { 1920, 1080 })
    {
        // Basic setup of the application
        QApplication::setApplicationName(QStringLiteral("Schneider Demo - ") + ALLEGIANCE_BUILD_STR);
        QApplication::setApplicationVersion(QStringLiteral(ALLEGIANCE_PROJECT_VERSION));
        app.setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
        app.setWindowIcon(QIcon{ QStringLiteral(":/tlr.ico") });
        app.setPalette(qml.style.palette());
        camera.SetShear(ALLEGIANCE_SERENITY);

        auto* cc = wnd.GetCameraControl();
        auto& qwin = *impl->GetWindow();

        QObject::connect(&watcher, &WindowDestructionWatcher::OnClose,
                         [this]() {
                             impl.reset();
                             app.quit();
                         });
        QObject::connect(&watcher, &WindowDestructionWatcher::OnScroll,
                         [this](::QWheelEvent* e) {
                             camera.Zoom(e->angleDelta().y() * 0.01f);
                         });

        auto b = new QAction(QIcon{":stereo3_contrast.png"}, "Show Image", cc);
        QObject::connect(cc, &all::CameraControl::OnLoadImage, b, &QAction::trigger);
        QObject::connect(b, &QAction::triggered,
                         [this]() {
                             impl->ShowImage();
                         });
        auto a = new QAction(QIcon{ ":3D_contrast.png" }, "Load Model", cc);
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
                             static bool flipped = false;
                             switch (e->type()) {
                             case QEvent::MouseButtonPress:
                                 if (e->buttons() & Qt::MouseButton::LeftButton) {
                                     input.is_pressed = true;
                                     input.skip_first = true;
                                 } else if (e->buttons() & Qt::MouseButton::RightButton) {
                                     auto pos = all::Cursor::getInstance().getWorldPosition();
#ifdef WITH_NAVLIB
                                     qDebug() << " setting pivot " << pos;
                                     spacemouse.onSetPivotPoint({pos.x, pos.y, pos.z});
#endif
                                 }
                                 break;

                             case QEvent::MouseButtonRelease:
                                 if (e->button() == Qt::MouseButton::LeftButton) {
                                     input.is_pressed = false;
                                 }
                                 break;
                             case QEvent::MouseMove: {
                                 auto pos = e->pos();

                                 if (input.skip_first) {
                                     input.skip_first = false;
                                     input.last_pos = pos;
                                     break;
                                 }

                                 float dx = (0.f + pos.x() - input.last_pos.x()) /  100;
                                 float dy = (0.f + pos.y() - input.last_pos.y()) / 100;

                                 switch (e->buttons()) {
                                 case Qt::LeftButton:
                                     if (flipped) dy = - dy;
                                     flipped = flipped ^ camera.Rotate(dx, dy);
                                     break;
                                 case Qt::MiddleButton:
                                     camera.Translate(dx, dy);
                                     break;
                                 }

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
        QObject::connect(&qml.camera, &CameraController::OnFocusDistanceChanged, [this](float v) {
            camera.SetConvergencePlaneDistance(v);
        });
        QObject::connect(&qml.camera, &CameraController::OnEyeDistanceChanged, [this](float v) {
            camera.SetInterocularDistance(v);
        });
        QObject::connect(&qml.cursor, &CursorController::OnCursorVisibilityChanged, [this](bool checked) {
            impl->SetCursorEnabled(checked);
        });
        QObject::connect(&qml.scene, &SceneController::OpenLoadModelDialog, [this]() {
            auto fn = QFileDialog::getOpenFileName(&wnd, "Open Model", "scene", "Model Files (*.obj *.fbx *.gltf *.glb)");
            if (!fn.isEmpty())
#if ALLEGIANCE_SERENITY
                impl->LoadModel(fn.toStdString());
#else
                impl->LoadModel("file:" + fn);
#endif

        });

#ifdef WITH_NAVLIB
        try {
            spacemouse.addActions({ a, b }, "Application", "AppModi");
        } catch (const std::system_error& e) {
            qDebug() << "Could not add actions to spacemouse" << e.what();
        }
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
    Application app;
    QMLNodes qml;

    std::optional<Implementation> impl;
    all::Window wnd;
    WindowDestructionWatcher watcher{ &wnd };
    all::OrbitalStereoCamera camera;
#ifdef WITH_SPNAV
    SpacemouseSpnav spacemouse{ &camera };
#endif
#ifdef WITH_NAVLIB
    SpacemouseNavlib spacemouse{ &camera };
#endif
    all::MouseTracker input;
};
