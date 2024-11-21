#include <applications/qt/serenity/serenity_window_qt.h>

#include <QApplication>
#include <QScreen>

SerenityWindowQt::SerenityWindowQt()
    : m_window(new QWindow()) // We don't own this window, its ownership will be transferred to MainWindow's window container
{
    qApp->setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    // Create Qt Vulkan Instance
    m_vulkanInstance.setApiVersion(QVersionNumber(1, 2, 0));
    if (!m_vulkanInstance.create()) {
        qCritical() << "Failed to create QVulkanInstance";
        throw std::runtime_error{ "Failed to create QVulkanInstance" };
    }
    m_window->setSurfaceType(QSurface::SurfaceType::VulkanSurface);
    m_window->setVulkanInstance(&m_vulkanInstance);
    m_window->create();

    VkSurfaceKHR vkSurface = QVulkanInstance::surfaceForWindow(m_window);

    // Initialize KDGpu
    m_graphicsApi = std::make_unique<KDGpu::VulkanGraphicsApi>();

    // Create KDGpu Instance from VkInstance
    m_instance = m_graphicsApi->createInstanceFromExistingVkInstance(m_vulkanInstance.vkInstance());
    m_surface = m_graphicsApi->createSurfaceFromExistingVkSurface(m_instance, vkSurface);
}

uint32_t SerenityWindowQt::width() const
{
    auto c = capabilities();
    return c.currentExtent.width;
}

uint32_t SerenityWindowQt::height() const
{
    auto c = capabilities();
    return c.currentExtent.height;
}

glm::vec4 SerenityWindowQt::viewportRect() const
{
    auto v = m_window->frameGeometry();
    glm::vec4 r = { v.x(), v.y(), v.width(), v.height() };
    return r * (float)m_window->screen()->devicePixelRatio();
}

glm::vec2 SerenityWindowQt::cursorPos() const
{
    const auto cursorPos = m_window->mapFromGlobal(QCursor::pos());
    const auto pixelRatio = static_cast<float>(m_window->screen()->devicePixelRatio());
    return pixelRatio * glm::vec2{ cursorPos.x(), cursorPos.y() };
}

KDGpu::Instance& SerenityWindowQt::instance()
{
    return m_instance;
}

KDGpu::Surface& SerenityWindowQt::surface()
{
    return m_surface;
}

QWindow* SerenityWindowQt::window() const
{
    return m_window;
}

VkSurfaceCapabilitiesKHR SerenityWindowQt::capabilities() const
{
    VkSurfaceCapabilitiesKHR capabilities;
    auto vulkResMan = dynamic_cast<KDGpu::VulkanResourceManager*>(m_graphicsApi->resourceManager());
    KDGpu::VulkanSurface surface = *vulkResMan->getSurface(m_surface.handle());
    auto dev = vulkResMan->getDevice(m_deviceHandle);
    auto adapter = dev->adapterHandle;
    auto ad = vulkResMan->getAdapter(adapter);

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(ad->physicalDevice, surface.surface, &capabilities);
    return capabilities;
}

KDGpu::Device SerenityWindowQt::createDevice()
{
#ifndef FLUTTER_UI_ASSET_DIR
    KDGpu::AdapterAndDevice defaultDevice = m_instance.createDefaultDevice(m_surface);
    m_deviceHandle = defaultDevice.device.handle();
    return std::move(defaultDevice.device);
#else

    // The Flutter Embedder requires a device which has 2 graphics queues

    // Enumerate the adapters (physical devices) and select one to use.
    KDGpu::Adapter* selectedAdapter = m_instance.selectAdapter(KDGpu::AdapterDeviceType::Default);
    if (!selectedAdapter) {
        SPDLOG_CRITICAL("Unable to find a suitable Adapter. Aborting...");
        return {};
    }

    auto queueTypes = selectedAdapter->queueTypes();
    const bool hasGraphicsAndCompute = queueTypes[0].supportsFeature(KDGpu::QueueFlagBits::GraphicsBit | KDGpu::QueueFlagBits::ComputeBit);
    const bool supportsPresentation = selectedAdapter->supportsPresentation(m_surface, 0); // Query about the 1st queue type

    if (!supportsPresentation || !hasGraphicsAndCompute) {
        SPDLOG_CRITICAL("Selected adapter queue family 0 does not meet requirements. Aborting.");
        return {};
    }

    // Now we can create a device from the selected adapter that we can then use to interact with the GPU.
    // We try to request 2 GraphicQueues as Flutter needs its own queue
    KDGpu::Device device = selectedAdapter->createDevice(KDGpu::DeviceOptions{
            .queues = {
                    KDGpu::QueueRequest{
                            .queueTypeIndex = 0,
                            .count = std::min(queueTypes[0].availableQueues, 2U),
                            .priorities = { 0.0f, 0.0f },
                    },
            },
            .requestedFeatures = selectedAdapter->features(),
    });
    m_deviceHandle = device.handle();
    return device;
#endif
}
