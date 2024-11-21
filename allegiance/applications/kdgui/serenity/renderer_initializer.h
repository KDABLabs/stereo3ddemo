#pragma once

#include <shared/spacemouse_impl.h>
#include <shared/cursor.h>

#include "stereo_camera.h"

#include <memory>

namespace all::serenity {
class SerenityRenderer;
} // namespace all::serenity

struct MouseTracker {
    int32_t last_x_pos = {};
    int32_t last_y_pos = {};
    bool is_pressed = false;
    bool skip_first = false;
    bool cursor_changes_focus = false;
};

class SerenityWindowKDGui;

namespace all::kdgui {

class WindowEventWatcher;

class RendererInitializer
{
public:
    explicit RendererInitializer(SerenityWindowKDGui* renderingSurface);
    ~RendererInitializer();

private:
    void resetCamera() noexcept;

    all::kdgui::OrbitalStereoCamera m_camera;
    all::kdgui::WindowEventWatcher* m_windowEventWatcher{ nullptr };
    std::unique_ptr<all::serenity::SerenityRenderer> m_renderer;
    std::optional<all::SpacemouseImpl> m_spacemouse;
    MouseTracker m_mouseInputTracker;
};

} // namespace all::kdgui
