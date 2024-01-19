#pragma once

#include "serenity_impl.h"

#include <QVulkanInstance>

class SerenityWindowQt : public SerenityWindow
{
public:
    SerenityWindowQt()
    {
        qApp->setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

        // Create Qt Vulkan Instance
        m_vulkanInstance.setApiVersion(QVersionNumber(1, 2, 0));
        if (!m_vulkanInstance.create()) {
            qCritical() << "Failed to create QVulkanInstance";
            throw std::runtime_error{ "Failed to create QVulkanInstance" };
        }
        m_window.setSurfaceType(QSurface::SurfaceType::VulkanSurface);
        m_window.setVulkanInstance(&m_vulkanInstance);
        m_window.create();

        VkSurfaceKHR vkSurface = QVulkanInstance::surfaceForWindow(&m_window);

        // Initialize KDGpu
        m_graphicsApi = std::make_unique<KDGpu::VulkanGraphicsApi>();

        // Create KDGpu Instance from VkInstance
        m_instance = m_graphicsApi->createInstanceFromExistingVkInstance(m_vulkanInstance.vkInstance());
        m_surface = m_graphicsApi->createSurfaceFromExistingVkSurface(m_instance, vkSurface);
    }

    ~SerenityWindowQt() override = default;

    uint32_t GetWidth() const override
    {
        return std::ceil(m_window.width() * m_window.screen()->devicePixelRatio());
    }

    uint32_t GetHeight() const override
    {
        return std::ceil(m_window.height() * m_window.screen()->devicePixelRatio());
    }

    glm::vec2 GetCursorPos() const override
    {
        const auto cursorPos = m_window.mapFromGlobal(QCursor::pos());
        const auto pixelRatio = static_cast<float>(m_window.screen()->devicePixelRatio());
        return pixelRatio * glm::vec2{ cursorPos.x(), cursorPos.y() };
    }

    KDGpu::Instance& GetInstance() override
    {
        return m_instance;
    }

    KDGpu::Surface& GetSurface() override
    {
        return m_surface;
    }

    QWindow* GetWindow()
    {
        return &m_window;
    }

private:
    QVulkanInstance m_vulkanInstance;
    QWindow m_window;
    std::unique_ptr<KDGpu::VulkanGraphicsApi> m_graphicsApi;
    KDGpu::Instance m_instance;
    KDGpu::Surface m_surface;
};

class SerenityImplQt : public SerenityImpl
{
public:
    SerenityImplQt()
        : SerenityImpl{ std::make_unique<SerenityWindowQt>() }
    {
    }

    QWindow* GetWindow()
    {
        return static_cast<SerenityWindowQt*>(m_window.get())->GetWindow();
    }
};
