#include "serenity_kdgui_window.h"

SerenityWindowKDGui::SerenityWindowKDGui()
{
    // Create and show a window
    m_window = std::make_unique<KDGui::Window>();
    m_window->width = 1920;
    m_window->height = 1080;
    m_window->visible = true;
    m_window->title = makeBinding(
            [](const std::string& appName, uint32_t w, uint32_t h) {
                return fmt::format(
                        "{} - {} x {}",
                        appName, w, h);
            },
            KDFoundation::CoreApplication::instance()->applicationName,
            m_window->width,
            m_window->height);

    // If the user closes the window, then we quit the application
    auto app = KDGui::GuiApplication::instance();
    m_window->visible.valueChanged().connect([app](const bool& visible) {
                                        if (visible == false)
                                            app->quit();
                                    })
            .release();

    // Initialize KDGpu
    m_graphicsApi = std::make_unique<KDGpu::VulkanGraphicsApi>();

    // Create Instance
    m_instance = m_graphicsApi->createInstance(KDGpu::InstanceOptions{
            .applicationVersion = KDGPU_MAKE_API_VERSION(0, 1, 0, 0) });

    // Create Surface
    const KDGpu::SurfaceOptions surfaceOptions = KDGpuKDGui::View::surfaceOptions(m_window.get());
    m_surface = m_instance.createSurface(surfaceOptions);
}

uint32_t SerenityWindowKDGui::width() const
{
    return m_window->width();
}
uint32_t SerenityWindowKDGui::height() const
{
    return m_window->height();
}

glm::vec4 SerenityWindowKDGui::viewportRect() const
{
    return glm::vec4(0.0f, 0.0f, m_window->width(), m_window->height());
}

glm::vec2 SerenityWindowKDGui::cursorPos() const
{
    const KDGui::Position pos = m_window->cursorPosition();
    return glm::vec2(pos.x, pos.y);
}

KDGpu::Instance& SerenityWindowKDGui::instance()
{
    return m_instance;
}

KDGpu::Surface& SerenityWindowKDGui::surface()
{
    return m_surface;
}

KDGpu::Device SerenityWindowKDGui::createDevice()
{
    KDGpu::AdapterAndDevice defaultDevice = m_instance.createDefaultDevice(m_surface);
    m_deviceHandle = defaultDevice.device.handle();
    return std::move(defaultDevice.device);
}

KDGui::Window* SerenityWindowKDGui::window() const
{
    return m_window.get();
}
