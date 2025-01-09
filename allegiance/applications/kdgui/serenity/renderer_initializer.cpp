#include "renderer_initializer.h"

#include <renderer/serenity/serenity_renderer.h>
#include "window_watcher.h"
#include "serenity_kdgui_window.h"

namespace all::kdgui {

constexpr float mouseSensitivity = 100.0f;

RendererInitializer::RendererInitializer(SerenityWindowKDGui* renderingSurface)
    : m_renderer(std::make_unique<all::serenity::SerenityRenderer>(renderingSurface, m_camera))
{
    KDGui::Window* window = renderingSurface->window();

    m_windowEventWatcher = window->createChild<WindowEventWatcher>();

    // Forward Window events to m_windowEventWatcher
    window->registerEventReceiver(m_windowEventWatcher);

    // Setup Camera
    m_camera.viewChanged.connect([this] { m_renderer->viewChanged(); }).release();
    m_camera.projectionChanged.connect([this] { m_renderer->projectionChanged(); }).release();

    window->width.valueChanged().connect([this, window](const uint32_t w) {
                                    m_camera.aspectRatio = (float(w) / window->height());
                                })
            .release();
    window->height.valueChanged().connect([this, window](const uint32_t h) {
                                     m_camera.aspectRatio = (float(window->width()) / h);
                                 })
            .release();

    // SpaceMouse
    auto nav_params = std::make_shared<all::ModelNavParameters>();
    m_spacemouse.emplace(&m_camera, nav_params);
    m_spacemouse->setUseUserPivot(true);
    auto* pnav_params = nav_params.get();

    // Mouse Events
    m_windowEventWatcher->mousePressEvent.connect([this, pnav_params](const KDGui::MousePressEvent* e) {
                                             if (e->buttons() & KDGui::MouseButton::LeftButton) {
                                                 m_mouseInputTracker.is_pressed = true;
                                                 m_mouseInputTracker.skip_first = true;
                                                 if (m_mouseInputTracker.cursor_changes_focus) {
                                                     auto pos = m_renderer->cursorWorldPosition();
                                                     // m_camera.setFocusDistance(std::clamp(glm::length(pos - m_camera.position()), 0.5f, 100.f));
                                                 }
                                             } else if (e->buttons() & KDGui::MouseButton::RightButton) {
                                                 auto pos = m_renderer->cursorWorldPosition();
                                                 SPDLOG_WARN(" setting pivot {},{},{}", pos.x, pos.y, pos.z);
                                                 pnav_params->pivot_point = { pos.x, pos.y, pos.z };
                                             }

                                             m_renderer->onMouseEvent(*e);
                                         })
            .release();
    m_windowEventWatcher->mouseMoveEvent.connect([this](const KDGui::MouseMoveEvent* e) {
                                            static bool flipped = false;

                                            if (m_mouseInputTracker.skip_first) {
                                                m_mouseInputTracker.skip_first = false;
                                                m_mouseInputTracker.last_x_pos = e->xPos();
                                                m_mouseInputTracker.last_y_pos = e->yPos();
                                                return;
                                            }

                                            float dx = (0.f + e->xPos() - m_mouseInputTracker.last_x_pos) / mouseSensitivity;
                                            float dy = (0.f + e->yPos() - m_mouseInputTracker.last_y_pos) / mouseSensitivity;

                                            if (e->buttons() & KDGui::MouseButton::LeftButton) {
                                                if (flipped)
                                                    dy = -dy;
                                                flipped = flipped ^ m_camera.rotate(dx, dy);
                                            }

                                            if (e->buttons() & KDGui::MouseButton::MiddleButton) {
                                                m_camera.translate(dx, dy);
                                            }

                                            m_mouseInputTracker.last_x_pos = e->xPos();
                                            m_mouseInputTracker.last_y_pos = e->yPos();

                                            m_renderer->onMouseEvent(*e);
                                        })
            .release();
    m_windowEventWatcher->mouseReleaseEvent.connect([this](const KDGui::MouseReleaseEvent* e) {
                                               if (e->button() == KDGui::MouseButton::LeftButton)
                                                   m_mouseInputTracker.is_pressed = false;

                                               m_renderer->onMouseEvent(*e);
                                           })
            .release();
    m_windowEventWatcher->mouseWheelEvent.connect([this](const KDGui::MouseWheelEvent* e) {
                                             m_camera.zoom(e->yDelta() / mouseSensitivity);

                                             m_renderer->onMouseEvent(*e);
                                         })
            .release();

    m_renderer->createAspects(nav_params);
    resetCamera();
    window->visible = true;
}

RendererInitializer::~RendererInitializer() = default;

void RendererInitializer::resetCamera() noexcept
{
    m_camera.position = { 0.2, 5, -10 };
    m_camera.setForwardVector({ 0, -.5, 1 });
}

} // namespace all::kdgui
