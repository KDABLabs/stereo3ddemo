#pragma once
#include <shared/spacemouse_impl.h>
#include <shared/cursor.h>

#include "stereo_camera.h"
#include "window_event_watcher.h"

#include <applications/qt/common/window_event_watcher.h>
#include <applications/qt/common/camera_control.h>
#include <applications/qt/common/qml/Schneider/camera_controller.h>
#include <applications/qt/common/qml/Schneider/style.h>

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
    } // namespace all::qt
    void Place(const QPixmap& pixmap)
    {
        QApplication::clipboard()->setPixmap(pixmap);
    }
};

template<typename RendererSurface, typename Renderer>
class RendererInitializer
{
public:
    RendererInitializer(MainWindow* mainWindow, RendererSurface* rendererSurface)
        : m_mainWindow(mainWindow)
        , m_windowEventWatcher(std::make_unique<WindowEventWatcher>(m_mainWindow))
        , m_renderer(std::make_unique<Renderer>(rendererSurface, m_camera))
    {
        // Retrieve QML Singleton instances
        auto* cameraControlPanel = m_mainWindow->cameraControl();

        QQmlEngine* qmlEngine = cameraControlPanel->qmlEngine();
        m_sceneController = qmlEngine->singletonInstance<SceneController*>("Schneider", "Scene");
        m_cameraController = qmlEngine->singletonInstance<CameraController*>("Schneider", "Camera");
        m_cursorController = qmlEngine->singletonInstance<CursorController*>("Schneider", "Cursor");
        m_appStyle = qmlEngine->singletonInstance<AppStyle*>("Schneider", "Style");

        // Basic setup of the application
        qApp->setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
        qApp->setWindowIcon(QIcon{ QStringLiteral(":/tlr.ico") });
        qApp->setPalette(m_appStyle->palette());

        // Setup the camera
        m_camera.setShear(true);
        QObject::connect(&m_camera,
                         &all::qt::OrbitalStereoCamera::viewChanged,
                         [this]() {
                             m_renderer->viewChanged();
                         });

        QObject::connect(&m_camera, &all::qt::OrbitalStereoCamera::projectionChanged, [this]() {
            m_renderer->projectionChanged();
        });

        auto nav_params = std::make_shared<all::ModelNavParameters>();
        m_spacemouse.emplace(&m_camera, nav_params);
        m_spacemouse->setUseUserPivot(true);
        auto* pnav_params = nav_params.get();

        QObject::connect(m_windowEventWatcher.get(), &WindowEventWatcher::close,
                         [this]() {
                             m_renderer.reset();
                             qApp->quit();
                         });
        QObject::connect(m_windowEventWatcher.get(), &WindowEventWatcher::scrollEvent,
                         [this](::QWheelEvent* e) {
                             m_camera.zoom(e->angleDelta().y() / m_sceneController->mouseSensitivity());
                         });
        QObject::connect(m_mainWindow, &MainWindow::onScreenshot,
                         [this]() {
                             auto x = [this](const uint8_t* data, uint32_t width, uint32_t height) {
                                 m_screenshotGrabber.CreatePixmap(data, width, height);
                             };

                             m_renderer->screenshot(x);
                         });

        auto b = new QAction(QIcon{ ":stereo3_contrast.png" }, "Show Image", cameraControlPanel);
        QObject::connect(cameraControlPanel, &all::qt::CameraControl::onLoadImage, b, &QAction::trigger);
        QObject::connect(b, &QAction::triggered,
                         [this]() {
                             m_renderer->showImage();
                         });
        auto a = new QAction(QIcon{ ":3D_contrast.png" }, "Load Model", cameraControlPanel);
        QObject::connect(cameraControlPanel, &all::qt::CameraControl::onLoadModel, a, &QAction::trigger);
        QObject::connect(a, &QAction::triggered,
                         [this]() {
                             m_renderer->showModel();
                         });
        QObject::connect(cameraControlPanel, &all::qt::CameraControl::onClose,
                         [this]() {
                             qApp->postEvent(m_mainWindow, new QCloseEvent);
                         });
        QObject::connect(m_windowEventWatcher.get(), &WindowEventWatcher::mouseEvent,
                         [this, pnav_params](::QMouseEvent* e) {
                             static bool flipped = false;

                             switch (e->type()) {
                             case QEvent::MouseButtonPress:
                                 if (e->buttons() & Qt::MouseButton::LeftButton) {
                                     m_mouseInputTracker.is_pressed = true;
                                     m_mouseInputTracker.skip_first = true;
                                     if (m_mouseInputTracker.cursor_changes_focus) {
                                         auto pos = m_renderer->cursorWorldPosition();
                                         m_cameraController->setFocusDistance(std::clamp(glm::length(pos - m_camera.position()), 0.5f, 100.f));
                                     }

                                 } else if (e->buttons() & Qt::MouseButton::RightButton) {
                                     auto pos = m_renderer->cursorWorldPosition();
                                     qDebug() << " setting pivot " << pos.x << pos.y << pos.z;
                                     pnav_params->pivot_point = { pos.x, pos.y, pos.z };
                                 }
                                 break;

                             case QEvent::MouseButtonRelease:
                                 if (e->button() == Qt::MouseButton::LeftButton) {
                                     m_mouseInputTracker.is_pressed = false;
                                 }
                                 break;
                             case QEvent::MouseMove: {
                                 auto pos = e->pos();

                                 if (m_mouseInputTracker.skip_first) {
                                     m_mouseInputTracker.skip_first = false;
                                     m_mouseInputTracker.last_pos = pos;
                                     break;
                                 }

                                 float dx = (0.f + pos.x() - m_mouseInputTracker.last_pos.x()) / m_sceneController->mouseSensitivity();
                                 float dy = (0.f + pos.y() - m_mouseInputTracker.last_pos.y()) / m_sceneController->mouseSensitivity();

                                 switch (e->buttons()) {
                                 case Qt::LeftButton:
                                     if (flipped)
                                         dy = -dy;
                                     flipped = flipped ^ m_camera.rotate(dx, dy);
                                     break;
                                 case Qt::MiddleButton:
                                     m_camera.translate(dx, dy);
                                     break;
                                 }
                                 m_mouseInputTracker.last_pos = pos;
                                 m_renderer->updateMouse();
                             } break;
                             default:
                                 break;
                             }

// Event Forwarding for Serenity
#ifdef ALLEGIANCE_SERENITY
                             m_renderer->onMouseEvent(e);
#endif
                         });

        QWindow* qWindow = m_renderer->window();

        QObject::connect(qWindow, &QWindow::widthChanged, [this, qWindow](int width) {
            m_camera.setAspectRatio(float(qWindow->width()) / qWindow->height());
        });
        QObject::connect(qWindow, &QWindow::heightChanged, [this, qWindow](int height) {
            m_camera.setAspectRatio(float(qWindow->width()) / qWindow->height());
        });
        QObject::connect(m_cameraController, &CameraController::focusDistanceChanged, [this](float v) {
            m_camera.setConvergencePlaneDistance(v);
        });
        QObject::connect(m_cameraController, &CameraController::fovChanged, [this](float v) {
            m_camera.setFov(v);
        });
        QObject::connect(m_cameraController, &CameraController::flippedChanged, [this](bool v) {
            m_camera.setFlipped(v);
        });
        QObject::connect(m_cameraController, &CameraController::eyeDistanceChanged, [this](float v) {
            m_camera.setInterocularDistance(v);
        });
        QObject::connect(m_cameraController, &CameraController::cameraModeChanged, [this](CameraController::CameraMode v) {
            m_renderer->propertyChanged("camera_mode", all::CameraMode(v));
        });
        QObject::connect(m_cursorController, &CursorController::cursorVisibilityChanged, [this](bool checked) {
            m_renderer->setCursorEnabled(checked);
        });
        QObject::connect(m_cursorController, &CursorController::cursorChanged, [this](CursorType type) {
            m_renderer->propertyChanged("cursor_type", type);
        });
        QObject::connect(m_cursorController, &CursorController::cursorScaleChanged, [this](float scale) {
            m_renderer->propertyChanged("scale_factor", scale);
        });
        QObject::connect(m_cursorController, &CursorController::cursorTintChanged, [this](const QColor& color) {
            std::array<float, 4> c = { color.redF(), color.greenF(), color.blueF(), color.alphaF() };
            m_renderer->propertyChanged("cursor_color", c);
        });
        QObject::connect(m_cursorController, &CursorController::cursorScalingEnableChanged, [this](bool enabled) {
            m_renderer->propertyChanged("scaling_enabled", enabled);
        });
        QObject::connect(m_cursorController, &CursorController::cursorFocusChanged, [this](bool enabled) {
            m_mouseInputTracker.cursor_changes_focus = enabled;
        });
        QObject::connect(m_sceneController, &SceneController::OpenLoadModelDialog, [this]() {
            auto fn = QFileDialog::getOpenFileName(m_mainWindow, "Open Model", "scene", "Model Files (*.obj *.fbx *.gltf *.glb)");
            if (!fn.isEmpty()) {
                resetCamera();
                m_renderer->loadModel(fn.toStdString());
            }
        });
        QObject::connect(m_mainWindow, &MainWindow::onEyeSeparation, [this](bool increased) {
            float distance = m_cameraController->eyeDistance() + increased ? 0.05f : -0.05f;
            m_cameraController->setEyeDistance(std::clamp(distance, 0.01f, 0.5f));
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

        m_renderer->createAspects(nav_params);
        resetCamera();
    }

    ~RendererInitializer()
    {
    }

public:
    void resetCamera() noexcept
    {
        m_camera.setPosition({ 0.2, 5, -10 });
        m_camera.setForwardVector({ 0, -.5, 1 });
    }

private:
    all::qt::OrbitalStereoCamera m_camera;
    MainWindow* m_mainWindow{ nullptr };
    std::unique_ptr<WindowEventWatcher> m_windowEventWatcher;
    std::unique_ptr<Renderer> m_renderer;
    std::optional<all::SpacemouseImpl> m_spacemouse;
    ScreenshotGrabber m_screenshotGrabber;

    SceneController* m_sceneController{ nullptr };
    CameraController* m_cameraController{ nullptr };
    CursorController* m_cursorController{ nullptr };
    AppStyle* m_appStyle{ nullptr };

    MouseTracker m_mouseInputTracker;
};
} // namespace all::qt
