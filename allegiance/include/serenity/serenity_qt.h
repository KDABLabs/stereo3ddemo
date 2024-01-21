#pragma once

#include <QApplication>
#include <QEvent>
#include <QWindow>
#include <QScreen>

namespace all {
class SerenityUpdateEvent : public QEvent
{
public:
    SerenityUpdateEvent()
        : QEvent(static_cast<QEvent::Type>(SerenityUpdateEvent::m_type))
    {
    }

    static int eventType() { return m_type; }

private:
    static inline const int m_type = QEvent::registerEventType();
};

class SerenityGuiApplication : public QApplication
{
public:
    explicit SerenityGuiApplication(int& ac, char** av)
        : QApplication(ac, av)
    {
        loop();
    }

protected:
    bool event(QEvent* e) override
    {
        if (e->type() == SerenityUpdateEvent::eventType()) {
            m_serenityApp.processEvents();
            loop();
        }

        // We want to have the qApp call app->processEvent periodically
        return QApplication::event(e);
    }

    void loop()
    {
        postEvent(this, new SerenityUpdateEvent());
    }

private:
    KDGui::GuiApplication m_serenityApp;
};
} // namespace all
