#include <renderer/serenity/serenity_window.h>
#include <QVulkanInstance>
#include <QWindow>

class SerenityWindowQt : public all::serenity::SerenityWindow
{
public:
    SerenityWindowQt();
    ~SerenityWindowQt() override = default;

    uint32_t width() const override;
    uint32_t height() const override;
    glm::vec4 viewportRect() const override;

    glm::vec2 cursorPos() const override;

    KDGpu::Instance& instance() override;
    KDGpu::Surface& surface() override;
    KDGpu::Device createDevice() override;
    VkSurfaceCapabilitiesKHR capabilities() const;

    QWindow* window() const;

private:
    QVulkanInstance m_vulkanInstance;
    QWindow* m_window{ nullptr };
    std::unique_ptr<KDGpu::VulkanGraphicsApi> m_graphicsApi;
    KDGpu::Instance m_instance;
    KDGpu::Surface m_surface;
    KDGpu::Handle<KDGpu::Device_t> m_deviceHandle;
};
