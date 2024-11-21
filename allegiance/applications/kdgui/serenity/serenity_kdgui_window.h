#pragma once

#include <renderer/serenity/serenity_window.h>

#include <KDGui/window.h>
#include <KDGpuKDGui/view.h>

class SerenityWindowKDGui : public all::serenity::SerenityWindow
{
public:
    SerenityWindowKDGui();

    ~SerenityWindowKDGui() override = default;

    uint32_t width() const final;
    uint32_t height() const final;
    glm::vec4 viewportRect() const final;

    glm::vec2 cursorPos() const final;

    KDGpu::Instance& instance() final;
    KDGpu::Surface& surface() final;
    KDGpu::Device createDevice() final;
    KDGui::Window* window() const;

private:
    std::unique_ptr<KDGui::Window> m_window;
    std::unique_ptr<KDGpu::VulkanGraphicsApi> m_graphicsApi;
    KDGpu::Instance m_instance;
    KDGpu::Surface m_surface;
    KDGpu::Handle<KDGpu::Device_t> m_deviceHandle;
};
