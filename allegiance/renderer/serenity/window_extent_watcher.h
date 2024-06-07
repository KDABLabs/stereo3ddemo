#pragma once
#include <Serenity/gui/window_extent_watcher.h>

namespace all::serenity {
class SerenityWindow;

class SerenityWindowExtentWatcher : public Serenity::WindowExtentWatcher
{
public:
    explicit SerenityWindowExtentWatcher(SerenityWindow* window);

    uint32_t width() const override;
    uint32_t height() const override;

private:
    SerenityWindow* m_window = nullptr;
};

} // namespace all::serenity
