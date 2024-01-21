#include <serenity_impl.h>
#include <serenity_stereo_graph.h>

#include <Serenity/gui/flutter/overlay.h>
#include <Serenity/gui/forward_renderer/forward_algorithm.h>
#include <Serenity/gui/render/renderer.h>

#include <KDGpu/surface_options.h>
#include <KDGui/window.h>

#include <KDGpuKDGui/view.h>

using namespace Serenity;
using namespace KDGui;
using namespace KDGpu;

class SerenityWindowFlutter : public SerenityWindow
{
public:
    SerenityWindowFlutter(GuiApplication* app)
        : m_app{ app }
        , m_window{ std::make_unique<Window>() }
    {
        // Create window
        m_window->visible.valueChanged().connect([this](const bool& visible) {
            if (visible == false)
                m_app->quit();
        });
        m_window->title = makeBoundProperty(
                [](uint32_t w, uint32_t h) {
                    return fmt::format("Kuesa/Flutter Demo - {} x {}", w, h);
                },
                m_window->width,
                m_window->height);
        m_window->width = 1920;
        m_window->height = 1080;
        m_window->visible = true;

        // Set up KDGpu
        m_graphicsApi = std::make_unique<KDGpu::VulkanGraphicsApi>();
        m_instance = m_graphicsApi->createInstance(KDGpu::InstanceOptions{
                .applicationVersion = KDGPU_MAKE_API_VERSION(0, 1, 0, 0) });
        const KDGpu::SurfaceOptions surfaceOptions = KDGpuKDGui::View::surfaceOptions(m_window.get());
        m_surface = m_instance.createSurface(surfaceOptions);
    }

    uint32_t GetWidth() const override
    {
        return m_window->width();
    }

    uint32_t GetHeight() const override
    {
        return m_window->height();
    }

    glm::vec2 GetCursorPos() const override
    {
        return { m_window->cursorPosition().x, m_window->cursorPosition().y };
    }

    KDGpu::Instance& GetInstance() override
    {
        return m_instance;
    }

    KDGpu::Surface& GetSurface() override
    {
        return m_surface;
    }

    KDGpu::Device CreateDevice() override
    {
        // Enumerate the adapters (physical devices) and select one to use. Here we look for
        // a discrete GPU. In a real app, we could fallback to an integrated one.
        Adapter* selectedAdapter = m_instance.selectAdapter(AdapterDeviceType::Default);
        if (!selectedAdapter) {
            SPDLOG_CRITICAL("Unable to find a suitable Adapter. Aborting...");
            return {};
        }

        auto queueTypes = selectedAdapter->queueTypes();
        const bool hasGraphicsAndCompute = queueTypes[0].supportsFeature(QueueFlags(QueueFlagBits::GraphicsBit) | QueueFlags(QueueFlagBits::ComputeBit));
        SPDLOG_INFO("Queue family 0 graphics and compute support: {}", hasGraphicsAndCompute);

        // We are now able to query the adapter for swapchain properties and presentation support with the window surface
        const auto swapchainProperties = selectedAdapter->swapchainProperties(m_surface);
        SPDLOG_INFO("Supported swapchain present modes:");
        for (const auto& mode : swapchainProperties.presentModes) {
            SPDLOG_INFO("  - {}", presentModeToString(mode));
        }

        const bool supportsPresentation = selectedAdapter->supportsPresentation(m_surface, 0); // Query about the 1st queue type
        SPDLOG_INFO("Queue family 0 supports presentation: {}", supportsPresentation);

        const auto adapterExtensions = selectedAdapter->extensions();
        SPDLOG_DEBUG("Supported adapter extensions:");
        for (const auto& extension : adapterExtensions) {
            SPDLOG_DEBUG("  - {} Version {}", extension.name, extension.version);
        }

        if (!supportsPresentation || !hasGraphicsAndCompute) {
            SPDLOG_CRITICAL("Selected adapter queue family 0 does not meet requirements. Aborting.");
            return {};
        }

        // Now we can create a device from the selected adapter that we can then use to interact with the GPU.
        // We try to request 2 GraphicQueues as Flutter needs its own queue
        return selectedAdapter->createDevice(DeviceOptions{
                .queues = {
                        QueueRequest{
                                .queueTypeIndex = 0,
                                .count = std::min(queueTypes[0].availableQueues, 2U),
                                .priorities = { 0.0f, 0.0f },
                        },
                },
                .requestedFeatures = selectedAdapter->features(),
        });
    }

    Window* GetWindow()
    {
        return m_window.get();
    }

private:
    GuiApplication* m_app;
    std::unique_ptr<Window> m_window;
    std::unique_ptr<KDGpu::GraphicsApi> m_graphicsApi;
    KDGpu::Instance m_instance;
    KDGpu::Surface m_surface;
};

class SerenityImplFlutter : public SerenityImpl
{
public:
    SerenityImplFlutter(GuiApplication* app)
        : SerenityImpl{ std::make_unique<SerenityWindowFlutter>(app) }
    {
    }

    void CreateFlutterOverlay()
    {
        m_flutterOverlay = std::make_unique<Flutter::Overlay>();
        GetWindow()->registerEventReceiver(m_flutterOverlay.get());
        m_flutterOverlay->flutterBundlePath = FLUTTER_UI_ASSET_DIR "/ui/build/flutter_assets";
        m_flutterOverlay->icuDataPath = FLUTTER_UI_ASSET_DIR "/icudtl.dat";

        auto* algo = static_cast<StereoForwardAlgorithm*>(m_renderAspect->renderAlgorithm());
        algo->overlays = { m_flutterOverlay.get() };
    }

    Window* GetWindow()
    {
        return static_cast<SerenityWindowFlutter*>(m_window.get())->GetWindow();
    }

private:
    std::unique_ptr<Flutter::Overlay> m_flutterOverlay;
};

int main(int argc, const char* argv[])
{
    GuiApplication app;
    KDGpu::Device m_device;
    app.applicationName = "kuesa-flutter-demo";

    int ret = 0;
    {
        SerenityImplFlutter impl{ &app };
        all::OrbitalStereoCamera camera;
        impl.CreateAspects(&camera);
        impl.CreateFlutterOverlay();
        ret = app.exec();
    }

    return ret;
}
