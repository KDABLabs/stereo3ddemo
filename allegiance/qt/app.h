#pragma once
#include <shared/spacemouse_impl.h>
#include <shared/cursor.h>
#include <renderer_interface_qt.h>

#include "qml_nodes.h"
#include "window_watcher.h"
#include "stereo_camera.h"

#include <QStyleFactory>
#include <QFileDialog>
#include <QMouseEvent>

namespace all::qt {
struct MouseTracker {
    QPoint last_pos = {};
    bool is_pressed = false;
    bool skip_first = false;
    bool cursor_changes_focus = false;
};

class App
{
public:
    App(int& argc, char** argv)
        : app(argc, argv), impl(std::in_place, camera), wnd(impl->GetWindow(), { 1920, 1080 })
    {
        // Basic setup of the application
        QApplication::setApplicationName(QStringLiteral("Schneider Demo - " ALLEGIANCE_BUILD_STR));
        QApplication::setApplicationVersion(QStringLiteral(ALLEGIANCE_PROJECT_VERSION));
        app.setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
        app.setWindowIcon(QIcon{ QStringLiteral(":/tlr.ico") });
        app.setPalette(qml.style.palette());

        // Setup the camera
        camera.SetShear(!AUTO_STEREO_SHEAR);
        QObject::connect(&camera,
                         &all::qt::OrbitalStereoCamera::OnViewChanged,
                         [this]() {
                             impl->ViewChanged();
                         });

        QObject::connect(&camera, &all::qt::OrbitalStereoCamera::OnProjectionChanged, [this]() {
            impl->ProjectionChanged();
        });

        auto* cc = wnd.GetCameraControl();
        auto& qwin = *impl->GetWindow();

        auto nav_params = std::make_shared<all::ModelNavParameters>();
        spacemouse.emplace(&camera, nav_params);
        spacemouse->SetUseUserPivot(true);
        auto* pnav_params = nav_params.get();

        QObject::connect(&watcher, &WindowDestructionWatcher::OnClose,
                         [this]() {
                             impl.reset();
                             app.quit();
                         });
        QObject::connect(&watcher, &WindowDestructionWatcher::OnScroll,
                         [this](::QWheelEvent* e) {
                             camera.Zoom(e->angleDelta().y() / qml.scene.GetMouseSensitivity());
                         });

        auto b = new QAction(QIcon{ ":stereo3_contrast.png" }, "Show Image", cc);
        QObject::connect(cc, &all::qt::CameraControl::OnLoadImage, b, &QAction::trigger);
        QObject::connect(b, &QAction::triggered,
                         [this]() {
                             impl->ShowImage();
                         });
        auto a = new QAction(QIcon{ ":3D_contrast.png" }, "Load Model", cc);
        QObject::connect(cc, &all::qt::CameraControl::OnLoadModel, a, &QAction::trigger);
        QObject::connect(a, &QAction::triggered,
                         [this]() {
                             impl->ShowModel();
                         });
        QObject::connect(cc, &all::qt::CameraControl::OnClose,
                         [this]() {
                             app.postEvent(&wnd, new QCloseEvent);
                         });
        QObject::connect(&watcher, &WindowDestructionWatcher::OnMouseEvent,
                         [this, pnav_params](::QMouseEvent* e) {
                             static bool flipped = false;

                             switch (e->type()) {
                             case QEvent::MouseButtonPress:
                                 if (e->buttons() & Qt::MouseButton::LeftButton) {
                                     input.is_pressed = true;
                                     input.skip_first = true;
                                     if (input.cursor_changes_focus)
                                     {
                                         auto pos = impl->GetCursorWorldPosition();
                                         qml.camera.SetFocusDistance(std::clamp(glm::length(pos - camera.GetPosition()),0.5f, 100.f));
                                     }

                                 } else if (e->buttons() & Qt::MouseButton::RightButton) {
                                     auto pos = impl->GetCursorWorldPosition();
                                     qDebug() << " setting pivot " << pos.x << pos.y << pos.z;
                                     pnav_params->pivot_point = { pos.x, pos.y, pos.z };
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

                                 float dx = (0.f + pos.x() - input.last_pos.x()) / qml.scene.GetMouseSensitivity();
                                 float dy = (0.f + pos.y() - input.last_pos.y()) / qml.scene.GetMouseSensitivity();

                                 switch (e->buttons()) {
                                 case Qt::LeftButton:
                                     if (flipped)
                                         dy = -dy;
                                     flipped = flipped ^ camera.Rotate(dx, dy);
                                     break;
                                 case Qt::MiddleButton:
                                     camera.Translate(dx, dy);
                                     break;
                                 }
                                 input.last_pos = pos;
                                 impl->UpdateMouse();
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
        QObject::connect(&qml.cursor, &CursorController::OnCursorScaleChanged, [this](float scale) {
            impl->OnPropertyChanged("scale_factor", scale);
        });
        QObject::connect(&qml.cursor, &CursorController::OnCursorScalingEnableChanged, [this](bool enabled) {
            impl->OnPropertyChanged("scaling_enabled", enabled);
        });
        QObject::connect(&qml.cursor, &CursorController::OnCursorFocusChanged, [this](bool enabled) {
            input.cursor_changes_focus = enabled;
        });
        QObject::connect(&qml.scene, &SceneController::OpenLoadModelDialog, [this]() {
            auto fn = QFileDialog::getOpenFileName(&wnd, "Open Model", "scene", "Model Files (*.obj *.fbx *.gltf *.glb)");
            if (!fn.isEmpty()) {
                ResetCamera();
                impl->LoadModel(fn.toStdString());
            }
        });

        // #if !ALLEGIANCE_SERENITY
        //         QObject::connect(&impl.value(), &Qt3DImpl::OnModelExtentChanged, [this](const QVector3D& min, const QVector3D& max) {
        // #ifdef WITH_NAVLIB
        //             //spacemouse.getModel()->SetModelExtents({ { min.x(), min.y(), min.z() }, { max.x(), max.y(), max.z() } });
        // #endif
        //         });
        // #endif

        // #ifdef WITH_NAVLIB
        //         try {
        //             spacemouse->({ a, b }, "Application", "AppModi");
        //         } catch (const std::system_error& e) {
        //             qDebug() << "Could not add actions to spacemouse" << e.what();
        //         }
        // #endif

        impl->CreateAspects(nav_params, &qml.cursor);
        ResetCamera();
        wnd.show();
    }

public:
    int
    Start() noexcept
    {
        return app.exec();
    }

    void ResetCamera() noexcept
    {
        camera.SetPosition({ 0.2, 5, -10 });
        camera.SetForwardVector({ 0, -.5, 1 });
    }

private:
    Application app;
    QMLNodes qml;

    all::qt::OrbitalStereoCamera camera;
    std::optional<Renderer> impl;
    Window wnd;
    WindowDestructionWatcher watcher{ &wnd };
    std::optional<all::SpacemouseImpl> spacemouse;

    MouseTracker input;
};
} // namespace all::qt
