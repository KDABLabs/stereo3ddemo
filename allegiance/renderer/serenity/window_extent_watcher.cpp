#include "window_extent_watcher.h"
#include "serenity_window.h"

namespace all::serenity {

SerenityWindowExtentWatcher::SerenityWindowExtentWatcher(SerenityWindow* window)
    : m_window(window)
{
}

uint32_t SerenityWindowExtentWatcher::width() const
{
    return m_window->GetWidth();
}

uint32_t SerenityWindowExtentWatcher::height() const
{
    return m_window->GetHeight();
}

} // namespace all
