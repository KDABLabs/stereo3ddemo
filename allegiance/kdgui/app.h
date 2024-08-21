#pragma once

#include <shared/spacemouse_impl.h>
#include <shared/cursor.h>

#include <serenity_impl_kdgui.h>
#include "window_watcher.h"
#include "stereo_camera.h"

namespace all {
constexpr bool UseShearing = true;
using Renderer = serenity::SerenityImplKDGui;
using Application = ::KDGui::GuiApplication;
} // namespace all

namespace all::kdgui {

struct MouseTracker {
    int32_t last_x_pos = {};
    int32_t last_y_pos = {};
    bool is_pressed = false;
    bool skip_first = false;
    bool cursor_changes_focus = false;
};

constexpr float mouseSensitivity = 100.0f;

class App
{
public:
    App(int&, char**)
        : app()
        , impl(std::in_place, camera)
    {
        KDGui::Window* window = impl->GetWindow();
        windowEventWatcher = window->createChild<WindowEventWatcher>();

        // Forward Window events to WindowEventWatcher
        window->registerEventReceiver(windowEventWatcher);

        // Setup Camera
        camera.SetShear(all::UseShearing);

        camera.OnViewChanged.connect([this] { impl->ViewChanged(); });
        camera.OnProjectionChanged.connect([this] { impl->ProjectionChanged(); });

        window->width.valueChanged().connect([this, window](const uint32_t w) {
            camera.SetAspectRatio(float(w) / window->height());
        });
        window->height.valueChanged().connect([this, window](const uint32_t h) {
            camera.SetAspectRatio(float(window->width()) / h);
        });

        // SpaceMouse
        auto nav_params = std::make_shared<all::ModelNavParameters>();
        spacemouse.emplace(&camera, nav_params);
        spacemouse->SetUseUserPivot(true);
        auto* pnav_params = nav_params.get();

        // Mouse Events
        windowEventWatcher->mousePressEvent.connect([this, pnav_params](const KDGui::MousePressEvent* e) {
            if (e->buttons() & KDGui::MouseButton::LeftButton) {
                input.is_pressed = true;
                input.skip_first = true;
                if (input.cursor_changes_focus) {
                    auto pos = impl->GetCursorWorldPosition();
                    // camera.SetFocusDistance(std::clamp(glm::length(pos - camera.GetPosition()), 0.5f, 100.f));
                }
            } else if (e->buttons() & KDGui::MouseButton::RightButton) {
                auto pos = impl->GetCursorWorldPosition();
                SPDLOG_WARN(" setting pivot {},{},{}", pos.x, pos.y, pos.z);
                pnav_params->pivot_point = { pos.x, pos.y, pos.z };
            }

            impl->OnMouseEvent(*e);
        });
        windowEventWatcher->mouseMoveEvent.connect([this](const KDGui::MouseMoveEvent* e) {
            static bool flipped = false;

            if (input.skip_first) {
                input.skip_first = false;
                input.last_x_pos = e->xPos();
                input.last_y_pos = e->yPos();
                return;
            }

            float dx = (0.f + e->xPos() - input.last_x_pos) / mouseSensitivity;
            float dy = (0.f + e->yPos() - input.last_y_pos) / mouseSensitivity;

            if (e->buttons() & KDGui::MouseButton::LeftButton) {
                if (flipped)
                    dy = -dy;
                flipped = flipped ^ camera.Rotate(dx, dy);
            }

            if (e->buttons() & KDGui::MouseButton::MiddleButton) {
                camera.Translate(dx, dy);
            }

            input.last_x_pos = e->xPos();
            input.last_y_pos = e->yPos();

            impl->OnMouseEvent(*e);
        });
        windowEventWatcher->mouseReleaseEvent.connect([this](const KDGui::MouseReleaseEvent* e) {
            if (e->button() == KDGui::MouseButton::LeftButton)
                input.is_pressed = false;

            impl->OnMouseEvent(*e);
        });
        windowEventWatcher->mouseWheelEvent.connect([this](const KDGui::MouseWheelEvent* e) {
            camera.Zoom(e->yDelta() / mouseSensitivity);

            impl->OnMouseEvent(*e);
        });

        // TODO: CameraController

        // TODO: Cursor Controller

        // TODO: SceneController

        impl->CreateAspects(nav_params);
        ResetCamera();
        window->visible = true;
    }

    int Start() noexcept
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
    std::optional<Renderer> impl;
    all::kdgui::OrbitalStereoCamera camera;
    std::optional<all::SpacemouseImpl> spacemouse;
    WindowEventWatcher* windowEventWatcher{ nullptr };
    MouseTracker input;
};

} // namespace all::kdgui
