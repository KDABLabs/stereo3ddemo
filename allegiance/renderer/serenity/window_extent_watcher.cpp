#include "window_extent_watcher.h"
#include "serenity_window.h"

namespace all::serenity {

SerenityWindowExtentWatcher::SerenityWindowExtentWatcher(SerenityWindow* window)
    : m_window(window)
{
}

uint32_t SerenityWindowExtentWatcher::width() const
{
    return m_window->width();
}

uint32_t SerenityWindowExtentWatcher::height() const
{
    return m_window->height();
}

} // namespace all::serenity
