#include <applications/qt/serenity/serenity_gui_application.h>
#include <applications/qt/serenity/serenity_window_qt.h>
#include <applications/qt/serenity/serenity_renderer_qt.h>

#include <applications/qt/common/renderer_initializer.h>

int main(int argc, char** argv)
{
    Q_INIT_RESOURCE(resources);

    qputenv("QSG_RHI_BACKEND", "vulkan");

    SerenityGuiApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("Schneider Demo Qt Serenity/Vulkan - " ALLEGIANCE_BUILD_STR));
    QApplication::setApplicationVersion(QStringLiteral(ALLEGIANCE_PROJECT_VERSION));

    all::qt::MainWindow mainWindow({ 1920, 1080 }); // Main Application Window
    SerenityWindowQt serenityRenderSurface; // Rendering Surface for 3D Renderer

    // Set embedded surface that holds 3d content window in the MainWindow
    mainWindow.setEmbeddedWindow(serenityRenderSurface.window()); // Note: window ownership is transferred to the MainWindow
    mainWindow.show();

    // Fire off Rendering
    all::qt::RendererInitializer<SerenityWindowQt, SerenityRendererQt> initializer(&mainWindow, &serenityRenderSurface);

    return app.exec();
}
