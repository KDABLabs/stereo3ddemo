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
#include <QClipboard>

namespace all::qt {
struct MouseTracker {
    QPoint last_pos = {};
    bool is_pressed = false;
    bool skip_first = false;
    bool cursor_changes_focus = false;
};

struct ScreenshotGrabber {
    void CreatePixmap(const uint8_t* data, uint32_t width, uint32_t height)
    {
        QImage img(data, width, height, QImage::Format_RGBA8888);
        // img.save("screenshot.png");
        QPixmap pixmap = QPixmap::fromImage(img);
        Place(pixmap);
    }
    void Place(const QPixmap& pixmap)
    {
        QApplication::clipboard()->setPixmap(pixmap);
    }
};

class App
{
public:
    App(int& argc, char** argv)
        : app(argc, argv), impl(std::in_place, camera), wnd(impl->window(), { 1920, 1080 })
    {
        // Basic setup of the application
        QApplication::setApplicationName(QStringLiteral("Schneider Demo - " ALLEGIANCE_BUILD_STR));
        QApplication::setApplicationVersion(QStringLiteral(ALLEGIANCE_PROJECT_VERSION));
        app.setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
        app.setWindowIcon(QIcon{ QStringLiteral(":/tlr.ico") });
        app.setPalette(qml.style.palette());

        // Setup the camera
        camera.setShear(all::UseShearing);
        QObject::connect(&camera,
                         &all::qt::OrbitalStereoCamera::viewChanged,
                         [this]() {
                             impl->viewChanged();
                         });

        QObject::connect(&camera, &all::qt::OrbitalStereoCamera::projectionChanged, [this]() {
            impl->projectionChanged();
        });

        auto* cc = wnd.cameraControl();
        auto& qwin = *impl->window();

        auto nav_params = std::make_shared<all::ModelNavParameters>();
        spacemouse.emplace(&camera, nav_params);
        spacemouse->setUseUserPivot(true);
        auto* pnav_params = nav_params.get();

        QObject::connect(&watcher, &WindowEventWatcher::onClose,
                         [this]() {
                             impl.reset();
                             app.quit();
                         });
        QObject::connect(&wnd, &Window::onScreenshot,
                         [this]() {
                             auto x = [this](const uint8_t* data, uint32_t width, uint32_t height) {
                                 grabber.CreatePixmap(data, width, height);
                             };

                             impl->screenshot(x);
                         });
        QObject::connect(&watcher, &WindowEventWatcher::onScroll,
                         [this](::QWheelEvent* e) {
                             camera.zoom(e->angleDelta().y() / qml.scene.mouseSensitivity());
                         });

        auto b = new QAction(QIcon{ ":stereo3_contrast.png" }, "Show Image", cc);
        QObject::connect(cc, &all::qt::CameraControl::onLoadImage, b, &QAction::trigger);
        QObject::connect(b, &QAction::triggered,
                         [this]() {
                             impl->showImage();
                         });
        auto a = new QAction(QIcon{ ":3D_contrast.png" }, "Load Model", cc);
        QObject::connect(cc, &all::qt::CameraControl::onLoadModel, a, &QAction::trigger);
        QObject::connect(a, &QAction::triggered,
                         [this]() {
                             impl->showModel();
                         });
        QObject::connect(cc, &all::qt::CameraControl::onClose,
                         [this]() {
                             app.postEvent(&wnd, new QCloseEvent);
                         });
        QObject::connect(&watcher, &WindowEventWatcher::onMouseEvent,
                         [this, pnav_params](::QMouseEvent* e) {
                             static bool flipped = false;

                             switch (e->type()) {
                             case QEvent::MouseButtonPress:
                                 if (e->buttons() & Qt::MouseButton::LeftButton) {
                                     input.is_pressed = true;
                                     input.skip_first = true;
                                     if (input.cursor_changes_focus) {
                                         auto pos = impl->cursorWorldPosition();
                                         qml.camera.setFocusDistance(std::clamp(glm::length(pos - camera.position()), 0.5f, 100.f));
                                     }

                                 } else if (e->buttons() & Qt::MouseButton::RightButton) {
                                     auto pos = impl->cursorWorldPosition();
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

                                 float dx = (0.f + pos.x() - input.last_pos.x()) / qml.scene.mouseSensitivity();
                                 float dy = (0.f + pos.y() - input.last_pos.y()) / qml.scene.mouseSensitivity();

                                 switch (e->buttons()) {
                                 case Qt::LeftButton:
                                     if (flipped)
                                         dy = -dy;
                                     flipped = flipped ^ camera.rotate(dx, dy);
                                     break;
                                 case Qt::MiddleButton:
                                     camera.translate(dx, dy);
                                     break;
                                 }
                                 input.last_pos = pos;
                                 impl->updateMouse();
                             } break;
                             default:
                                 break;
                             }

// Event Forwarding for Serenity
#ifdef ALLEGIANCE_SERENITY
                             impl->onMouseEvent(e);
#endif
                         });

        QObject::connect(&qwin, &QWindow::widthChanged, [this, &qwin](int width) {
            camera.setAspectRatio(float(qwin.width()) / qwin.height());
        });
        QObject::connect(&qwin, &QWindow::heightChanged, [this, &qwin](int height) {
            camera.setAspectRatio(float(qwin.width()) / qwin.height());
        });
        QObject::connect(&qml.camera, &CameraController::focusDistanceChanged, [this](float v) {
            camera.setConvergencePlaneDistance(v);
        });
        QObject::connect(&qml.camera, &CameraController::fovChanged, [this](float v) {
            camera.setFov(v);
        });
        QObject::connect(&qml.camera, &CameraController::flippedChanged, [this](bool v) {
            camera.setFlipped(v);
        });
        QObject::connect(&qml.camera, &CameraController::eyeDistanceChanged, [this](float v) {
            camera.setInterocularDistance(v);
        });
        QObject::connect(&qml.camera, &CameraController::cameraModeChanged, [this](CameraController::CameraMode v) {
            impl->propertyChanged("camera_mode", all::CameraMode(v));
        });
        QObject::connect(&qml.cursor, &CursorController::cursorVisibilityChanged, [this](bool checked) {
            impl->setCursorEnabled(checked);
        });
        QObject::connect(&qml.cursor, &CursorController::cursorChanged, [this](CursorType type) {
            impl->propertyChanged("cursor_type", type);
        });
        QObject::connect(&qml.cursor, &CursorController::cursorScaleChanged, [this](float scale) {
            impl->propertyChanged("scale_factor", scale);
        });
        QObject::connect(&qml.cursor, &CursorController::cursorTintChanged, [this](const QColor& color) {
            std::array<float, 4> c = { color.redF(), color.greenF(), color.blueF(), color.alphaF() };
            impl->propertyChanged("cursor_color", c);
        });
        QObject::connect(&qml.cursor, &CursorController::cursorScalingEnableChanged, [this](bool enabled) {
            impl->propertyChanged("scaling_enabled", enabled);
        });
        QObject::connect(&qml.cursor, &CursorController::cursorFocusChanged, [this](bool enabled) {
            input.cursor_changes_focus = enabled;
        });
        QObject::connect(&qml.scene, &SceneController::OpenLoadModelDialog, [this]() {
            auto fn = QFileDialog::getOpenFileName(&wnd, "Open Model", "scene", "Model Files (*.obj *.fbx *.gltf *.glb)");
            if (!fn.isEmpty()) {
                resetCamera();
                impl->loadModel(fn.toStdString());
            }
        });
        QObject::connect(&wnd, &Window::onEyeSeparation, [this](bool increased) {
            float distance = qml.camera.eyeDistance() + increased ? 0.05f : -0.05f;
            qml.camera.setEyeDistance(std::clamp(distance, 0.01f, 0.5f));
        });

        // #if !ALLEGIANCE_SERENITY
        //         QObject::connect(&impl.value(), &Qt3DImpl::modelExtentChanged, [this](const QVector3D& min, const QVector3D& max) {
        // #ifdef WITH_NAVLIB
        //             //spacemouse.model()->setModelExtents({ { min.x(), min.y(), min.z() }, { max.x(), max.y(), max.z() } });
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

        impl->createAspects(nav_params);
        resetCamera();
        wnd.show();
    }

public:
    int start() noexcept
    {
        return app.exec();
    }

    void resetCamera() noexcept
    {
        camera.setPosition({ 0.2, 5, -10 });
        camera.setForwardVector({ 0, -.5, 1 });
    }

private:
    Application app;
    QMLNodes qml;

    all::qt::OrbitalStereoCamera camera;
    std::optional<Renderer> impl;
    Window wnd;
    WindowEventWatcher watcher{ &wnd };
    std::optional<all::SpacemouseImpl> spacemouse;
    ScreenshotGrabber grabber;

    MouseTracker input;
};
} // namespace all::qt
