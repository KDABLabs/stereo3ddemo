#include <QApplication>
#include <Qt3DExtras/Qt3DWindow>

#include <applications/qt/common/renderer_initializer.h>
#include <renderer/qt3d/qt3d_renderer.h>

int main(int argc, char** argv)
{
    Q_INIT_RESOURCE(resources);

    constexpr bool useRHI = false;
    if constexpr (useRHI) {
        qputenv("QT3D_RENDERER", "rhi");
        qputenv("QSG_RHI_BACKEND", "opengl"); // could also be vulkan, metal or d3d11, d3d12.
        // Note: that stereo is not supported on RHI backends, only on opengl, d3d12 and vulkan
    } else {
        qputenv("QT3D_RENDERER", "opengl");
    }

    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("Schneider Demo Qt3D OpenGL - " ALLEGIANCE_BUILD_STR));
    QApplication::setApplicationVersion(QStringLiteral(ALLEGIANCE_PROJECT_VERSION));

    all::qt::MainWindow mainWindow({ 1920, 1080 }); // Main Application Window
    auto* renderingSurface = new Qt3DExtras::Qt3DWindow(); // Rendering Surface for 3D Renderer

    // Set embedded surface that holds 3d content window in the MainWindow
    mainWindow.setEmbeddedWindow(renderingSurface); // Note: renderingSurface ownership is transferred to the MainWindow

    // Fire off Rendering
    all::qt::RendererInitializer<Qt3DExtras::Qt3DWindow, all::qt3d::Qt3DRenderer> initializer(&mainWindow, renderingSurface);

    return app.exec();
}
