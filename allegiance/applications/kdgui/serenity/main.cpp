#include <KDGui/gui_application.h>
#include "serenity_kdgui_window.h"
#include "renderer_initializer.h"

int main(int argc, char** argv)
{
    KDGui::GuiApplication app;
    SerenityWindowKDGui window;

    app.applicationName = "Schneider Demo KDGui Serenity/Vulkan : ";
    all::kdgui::RendererInitializer initializer(&window);

    return app.exec();
}
