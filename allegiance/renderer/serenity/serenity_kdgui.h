#pragma once

#include "serenity_window.h"

#include <KDGui/window.h>
#include <KDGpuKDGui/view.h>

namespace all::serenity {

class SerenityWindowKDGui : public SerenityWindow
{
public:
    SerenityWindowKDGui()
    {
        // Create and show a window
        m_window = std::make_unique<KDGui::Window>();
        m_window->width = 1920;
        m_window->height = 1080;
        m_window->visible = true;
        m_window->title = makeBinding(
                [](const std::string& appName, uint32_t w, uint32_t h) {
                    return fmt::format(
                            "Serenity KDGui: {} - {} x {}",
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
        });

        // Initialize KDGpu
        m_graphicsApi = std::make_unique<KDGpu::VulkanGraphicsApi>();

        // Create Instance
        m_instance = m_graphicsApi->createInstance(KDGpu::InstanceOptions{
                .applicationVersion = KDGPU_MAKE_API_VERSION(0, 1, 0, 0) });

        // Create Surface
        const KDGpu::SurfaceOptions surfaceOptions = KDGpuKDGui::View::surfaceOptions(m_window.get());
        m_surface = m_instance.createSurface(surfaceOptions);
    }

    ~SerenityWindowKDGui() override = default;

    uint32_t GetWidth() const final
    {
        return m_window->width();
    }

    uint32_t GetHeight() const final
    {
        return m_window->height();
    }

    glm::vec4 GetViewportRect() const final
    {
        return glm::vec4(0.0f, 0.0f, m_window->width(), m_window->height());
    }

    glm::vec2 GetCursorPos() const final
    {
        const KDGui::Position pos = m_window->cursorPosition();
        return glm::vec2(pos.x, pos.y);
    }

    KDGpu::Instance& GetInstance() final
    {
        return m_instance;
    }

    KDGpu::Surface& GetSurface() final
    {
        return m_surface;
    }

    KDGpu::Device CreateDevice() final
    {
        KDGpu::AdapterAndDevice defaultDevice = m_instance.createDefaultDevice(m_surface);
        m_deviceHandle = defaultDevice.device.handle();
        return std::move(defaultDevice.device);
    }

    KDGui::Window* GetWindow() const
    {
        return m_window.get();
    }

private:
    std::unique_ptr<KDGui::Window> m_window;
    std::unique_ptr<KDGpu::VulkanGraphicsApi> m_graphicsApi;
    KDGpu::Instance m_instance;
    KDGpu::Surface m_surface;
    KDGpu::Handle<KDGpu::Device_t> m_deviceHandle;
};

} // namespace all::serenity
