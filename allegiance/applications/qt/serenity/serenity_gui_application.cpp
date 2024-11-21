#include "serenity_gui_application.h"

SerenityGuiApplication::SerenityGuiApplication(int& ac, char** av)
    : QApplication(ac, av)
{
    loop();
}

bool SerenityGuiApplication::event(QEvent* e)
{
    if (e->type() == SerenityUpdateEvent::eventType()) {
        m_serenityApp.processEvents();
        loop();
    }

    // We want to have the qApp call app->processEvent periodically
    return QApplication::event(e);
}

void SerenityGuiApplication::loop()
{
    postEvent(this, new SerenityUpdateEvent());
}

SerenityUpdateEvent::SerenityUpdateEvent()
    : QEvent(static_cast<QEvent::Type>(SerenityUpdateEvent::m_type))
{
}
