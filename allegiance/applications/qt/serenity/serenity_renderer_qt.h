#pragma once
#include <renderer/serenity/serenity_renderer.h>

#include <QMouseEvent>
#include <KDGui/gui_events.h>

class SerenityWindowQt;

class SerenityRendererQt : public all::serenity::SerenityRenderer
{
public:
    SerenityRendererQt(SerenityWindowQt* serenityWindow, all::StereoCamera& camera);

    QWindow* window() const;

    void updateMouse();

    void onMouseEvent(::QMouseEvent* event);
};
