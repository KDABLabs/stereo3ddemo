#pragma once
#include <renderer/serenity/serenity_renderer.h>

#include <QMouseEvent>
#include <KDGui/gui_events.h>

class SerenityWindowQt;

class SerenityRendererQt : public all::serenity::SerenityRenderer
{
public:
    SerenityRendererQt(SerenityWindowQt* serenityWindow, all::StereoCamera& camera, std::function<void(std::string_view name, std::any value)> = {});

    QWindow* window() const;

    void updateMouse();

    void onMouseEvent(::QMouseEvent* event);
};
