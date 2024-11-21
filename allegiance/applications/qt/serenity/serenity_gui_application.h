#include <QApplication>
#include <KDGui/gui_application.h>

class SerenityGuiApplication : public QApplication
{
public:
    explicit SerenityGuiApplication(int& ac, char** av);

protected:
    bool event(QEvent* e) override;
    void loop();

private:
    KDGui::GuiApplication m_serenityApp;
};

class SerenityUpdateEvent : public QEvent
{
public:
    SerenityUpdateEvent();
    static int eventType() { return m_type; }

private:
    static inline const int m_type = QEvent::registerEventType();
};
