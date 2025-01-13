#pragma once
#include <shared/spacemouse_impl.h>
#include <shared/cursor.h>

#include "window_event_watcher.h"

#include <applications/qt/common/window_event_watcher.h>
#include <applications/qt/common/side_menu.h>

#include <QStyleFactory>
#include <QFileDialog>
#include <QMouseEvent>
#include <QClipboard>

#include <any>

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
        , m_renderer(std::make_unique<Renderer>(rendererSurface,
                                                m_camera,
                                                std::function<void(std::string_view, std::any)>(
                                                        [this](std::string_view n, std::any v) {
                                                            propertyChanged(n, v);
                                                        })))
    {
        auto* sideMenu = m_mainWindow->sideMenu();
        {
            m_sceneController = sideMenu->sceneController();
            m_cameraController = sideMenu->cameraController();
            m_cursorController = sideMenu->cursorController();
            m_appStyle = sideMenu->appStyle();
        }

        // Basic setup of the application
        qApp->setStyle(QStyleFactory::create(QStringLiteral("Material")));
        qApp->setWindowIcon(QIcon{ QStringLiteral(":/tlr.ico") });
        qApp->setPalette(m_appStyle->palette());

        // Setup the camera
        m_camera.viewChanged.connect([this] { m_renderer->viewChanged(); }).release();
        m_camera.projectionChanged.connect([this] { m_renderer->projectionChanged(); }).release();

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
                             const glm::vec3 viewCenterBeforeZoom = m_camera.position() + m_camera.forwardVector() * m_camera.convergencePlaneDistance();

                             const float distanceToCenter = glm::length(m_camera.position() - m_renderer->sceneCenter());
                             const glm::vec3 sceneExtent = m_renderer->sceneExtent();
                             const float maxExtent = std::max(sceneExtent[0], std::max(sceneExtent[1], sceneExtent[2]));
                             const float minExtent = std::min(sceneExtent[0], std::min(sceneExtent[1], sceneExtent[2]));
                             const float zoomCameraLimit = minExtent * 0.1f;
                             // We reduce zoom as we get closer to the scene center
                             const float correctionFactor = exp(std::clamp(distanceToCenter / maxExtent, 0.0f, 1.0f) - 1.0f);
                             const float safeWheelDelta = std::clamp(float(e->angleDelta().y()), -20.0f, 20.0f); // Prevent dealing with too huge wheel deltas
                             const float zoomFactor = safeWheelDelta / m_sceneController->mouseSensitivity() * correctionFactor;
                             const float deltaLength = -1.0f * zoomFactor * distanceToCenter;

                             // Check we don't try to zoom past scene center
                             if (std::abs(distanceToCenter + deltaLength) < zoomCameraLimit)
                                 return;

                             m_camera.zoom(zoomFactor);

                             if (!m_cameraController->autoFocus()) {
                                 // Adjust convergence plane distance so that the viewCenter would remain at the same exact position after the zoom
                                 const float distToOriginalViewCenter = glm::length(viewCenterBeforeZoom - m_camera.position());
                                 const float normalizedNearFarPlaneDist = (distToOriginalViewCenter - m_camera.nearPlane()) / (m_camera.farPlane() - m_camera.nearPlane());
                                 const float focusDistancePercentage = normalizedNearFarPlaneDist * 100.0f;
                                 m_cameraController->setFocusDistance(focusDistancePercentage);
                             }
                         });
        QObject::connect(m_mainWindow, &MainWindow::onScreenshot,
                         [this]() {
                             auto x = [this](const uint8_t* data, uint32_t width, uint32_t height) {
                                 m_screenshotGrabber.CreatePixmap(data, width, height);
                             };

                             m_renderer->screenshot(x);
                         });

        auto* showImageAction = new QAction(QIcon{ ":stereo3_contrast.png" }, "Show Image", sideMenu);
        QObject::connect(sideMenu, &all::qt::SideMenu::onLoadImage, showImageAction, &QAction::trigger);
        QObject::connect(showImageAction, &QAction::triggered,
                         [this]() {
                             m_renderer->showImage();
                         });
        auto* loadModelAction = new QAction(QIcon{ ":3D_contrast.png" }, "Load Model", sideMenu);
        QObject::connect(sideMenu, &all::qt::SideMenu::onLoadModel, loadModelAction, &QAction::trigger);
        QObject::connect(loadModelAction, &QAction::triggered,
                         [this]() {
                             m_renderer->showModel();
                         });
        QObject::connect(sideMenu, &all::qt::SideMenu::onClose,
                         [this]() {
                             qApp->postEvent(m_mainWindow, new QCloseEvent);
                         });
        QObject::connect(m_windowEventWatcher.get(), &WindowEventWatcher::mouseEvent,
                         [this, pnav_params](::QMouseEvent* e) {
                             static bool flipped = false;

                             const QPointF mousePos = e->position();
                             // Let the renderer a chance to steal mouse events
                             m_renderer->onMouseEvent(e);
                             if (m_renderer->hoversFocusArea(mousePos.x(), mousePos.y())) {
                                 return;
                             }

                             switch (e->type()) {
                             case QEvent::MouseButtonPress:

                                 if (e->buttons() & Qt::MouseButton::LeftButton) {
                                     m_mouseInputTracker.is_pressed = true;
                                     m_mouseInputTracker.skip_first = true;

                                 } else if (e->buttons() & Qt::MouseButton::RightButton && !m_cameraController->autoFocus()) {
                                     auto pos = m_renderer->cursorWorldPosition();
                                     qDebug() << " setting pivot " << pos.x << pos.y << pos.z;
                                     pnav_params->pivot_point = { pos.x, pos.y, pos.z };
                                     if (m_mouseInputTracker.cursor_changes_focus && e->modifiers() & Qt::ControlModifier) {
                                         const float distanceToCamera = glm::length(pos - m_camera.position());
                                         // Convert this distance as % or near to far plane distance
                                         const float zDistanceNormalized = (distanceToCamera - m_camera.nearPlane()) / (m_camera.farPlane() - m_camera.nearPlane());
                                         m_cameraController->setFocusDistance(zDistanceNormalized * 100.0f);
                                     }
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

        QObject::connect(qWindow, &QWindow::widthChanged, [this, qWindow]() {
            m_camera.aspectRatio = (float(qWindow->width()) / qWindow->height());
        });
        QObject::connect(qWindow, &QWindow::heightChanged, [this, qWindow]() {
            m_camera.aspectRatio = (float(qWindow->width()) / qWindow->height());
        });

        auto updateFocusDistance = [this]() {
            // focusDistancePercentage is a % of the nearPlane -> farPlane distance
            const float focusDistancePercentage = m_cameraController->focusDistance();
            const float popOut = m_cameraController->popOut();
            const float focusDistanceFromNearPlane = m_camera.nearPlane() + (focusDistancePercentage * 0.01) * (m_camera.farPlane() - m_camera.nearPlane());

            // popOut == 0 -> focusDistanceFromNearPlane
            // popOut == 100 -> 1.5f * focusDistanceFromNearPlane
            // popOut == -100 -> 0.5f focusDistanceFromNearPlane
            const float popOutCorrectionFactor = (popOut * 0.01) * 0.5f + 1.0f; // [0.5, 1.5]
            m_camera.convergencePlaneDistance = popOutCorrectionFactor * focusDistanceFromNearPlane;

            if (m_cameraController->separationBasedOnFocusDistance()) {
                // Set Eye Separation to 1/30th of focus distance if enabled
                m_cameraController->setEyeDistance(m_camera.convergencePlaneDistance() / m_cameraController->separationBasedOnFocusDistanceDivider());
            }
        };

        QObject::connect(m_cameraController, &CameraController::focusDistanceChanged, updateFocusDistance);
        QObject::connect(m_cameraController, &CameraController::popOutChanged, updateFocusDistance);
        QObject::connect(m_cameraController, &CameraController::separationBasedOnFocusDistanceChanged, updateFocusDistance);
        QObject::connect(m_cameraController, &CameraController::separationBasedOnFocusDistanceDividerChanged, updateFocusDistance);

        QObject::connect(m_cameraController, &CameraController::fovChanged, [this](float v) {
            m_camera.fov = v;
        });
        QObject::connect(m_cameraController, &CameraController::flippedChanged, [this](bool v) {
            m_camera.flipped = v;
        });
        QObject::connect(m_cameraController, &CameraController::eyeDistanceChanged, [this](float v) {
            m_camera.interocularDistance = v;
        });
        QObject::connect(m_cameraController, &CameraController::displayModeChanged, [this](CameraController::DisplayMode v) {
            m_renderer->propertyChanged("display_mode", all::DisplayMode(v));
        });
        QObject::connect(m_cameraController, &CameraController::stereoModeChanged, [this](CameraController::StereoMode v) {
            m_camera.mode = all::StereoCamera::Mode(v);
        });
        QObject::connect(m_cameraController, &CameraController::frustumViewEnabledChanged, [this](bool enabled) {
            m_renderer->propertyChanged("frustum_view_enabled", enabled);
        });
        QObject::connect(m_cameraController, &CameraController::showAutoFocusAreaChanged, [this](bool enabled) {
            m_renderer->propertyChanged("show_focus_area", enabled);
        });
        QObject::connect(m_cameraController, &CameraController::showFocusPlaneChanged, [this](bool enabled) {
            m_renderer->propertyChanged("show_focus_plane", enabled);
        });
        QObject::connect(m_cameraController, &CameraController::autoFocusChanged, [this](bool enabled) {
            m_renderer->propertyChanged("auto_focus", enabled);
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
        QObject::connect(m_cameraController, &CameraController::autoFocusChanged, [this](bool afEnabled) {
            m_mouseInputTracker.cursor_changes_focus = !afEnabled;
        });
        QObject::connect(m_sceneController, &SceneController::OpenLoadModelDialog, [this]() {
            auto fn = QFileDialog::getOpenFileName(m_mainWindow, "Open Model", "scene", "Model Files (*.obj *.fbx *.gltf *.glb)");
            if (!fn.isEmpty()) {
                m_renderer->loadModel(fn.toStdString());
            }
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

        // Set Initial Values
        m_camera.aspectRatio = (float(qWindow->width()) / qWindow->height());
        m_camera.interocularDistance = m_cameraController->eyeDistance();
        m_camera.convergencePlaneDistance = m_cameraController->focusDistance();
        m_camera.fov = m_cameraController->fov();
        m_camera.mode = all::StereoCamera::Mode(m_cameraController->stereoMode());
        m_camera.flipped = m_cameraController->flipped();
        m_renderer->propertyChanged("frustum_view_enabled", m_cameraController->frustumViewEnabled());
        m_renderer->propertyChanged("show_focus_area", m_cameraController->showAutoFocusArea());
        m_renderer->propertyChanged("show_focus_plane", m_cameraController->showFocusPlane());
        m_renderer->propertyChanged("auto_focus", m_cameraController->autoFocus());
        m_renderer->propertyChanged("display_mode", all::DisplayMode(m_cameraController->displayMode()));
        m_renderer->propertyChanged("cursor_color", std::array<float, 4>{ m_cursorController->cursorTint().redF(), m_cursorController->cursorTint().greenF(), m_cursorController->cursorTint().blueF(), m_cursorController->cursorTint().alphaF() });
        m_mouseInputTracker.cursor_changes_focus = !m_cameraController->autoFocus();
    }

    ~RendererInitializer()
    {
    }

public:
    void propertyChanged(std::string_view name, std::any value)
    {
        if (name == "auto_focus_distance") {
            const float distanceToCamera = std::any_cast<float>(value);
            const float normalizedFocusDistanceOfNearFar = (distanceToCamera - m_camera.nearPlane()) / (m_camera.farPlane() - m_camera.nearPlane());
            const float focusDistanceAsPercentOfNearFar = normalizedFocusDistanceOfNearFar * 100.0f;
            m_cameraController->setFocusDistance(focusDistanceAsPercentOfNearFar);
        }
    }

private:
    all::OrbitalStereoCamera m_camera;
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
