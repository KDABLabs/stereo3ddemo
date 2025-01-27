#include "serenity_renderer.h"
#include "window_extent_watcher.h"
#include "picking_application_layer.h"
#include "shared/cursor.h"
#include "cursor.h"
#include "focus_plane_preview.h"
#include "focus_area.h"
#include "frustum.h"
#include "frustum_rect.h"

#include <Serenity/gui/imgui/overlay.h>
#ifdef FLUTTER_UI_ASSET_DIR
#include <Serenity/gui/flutter/overlay.h>
#endif
#include <Serenity/gui/render/renderer.h>
#include <imgui.h>

using namespace Serenity;

namespace all::serenity {

namespace {

Serenity::ImGui::Overlay* createImGuiOverlay(SerenityWindow* w, AspectEngine* engine, StereoForwardAlgorithm* algo)
{
    auto renderOverlay = [&w, engine, algo](ImGuiContext* ctx) {
        ::ImGui::SetCurrentContext(ctx);
        ::ImGui::SetNextWindowPos(ImVec2(10, 10));
        ::ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_FirstUseEver);
        ::ImGui::Begin(
                "Schneider Serenity",
                nullptr,
                ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        const auto* dev = algo->renderer()->device();
        ::ImGui::Text("GPU: %s", dev->adapter()->properties().deviceName.c_str());
        const auto fps = engine->fps.get();
        ::ImGui::Text("%.2f ms/frame (%.1f fps)", (1000.0f / fps), fps);

        ::ImGui::End();
    };

    // TODO:
    // Set Serenity::ImGui::Overlay on the window to
    // - send input events to the overlay
    auto overlay = algo->createChild<Serenity::ImGui::Overlay>(renderOverlay);
    //    w->registerEventReceiver(overlay);
    return overlay;
}

#ifdef FLUTTER_UI_ASSET_DIR

Serenity::Flutter::Overlay* createFlutterOverlay(SerenityWindow* w, StereoForwardAlgorithm* algo)
{
    auto flutterOverlay = algo->createChild<Serenity::Flutter::Overlay>();

    // TODO:
    // Set Serenity::Flutter::Overlay on the window to
    // - send input events to the overlay
    // w->registerEventReceiver(flutterOverlay.get());

    flutterOverlay->flutterBundlePath = FLUTTER_UI_ASSET_DIR "/build/flutter_assets";
    flutterOverlay->icuDataPath = FLUTTER_UI_ASSET_DIR "/icudtl.dat";
    return flutterOverlay;
}

#endif

TopViewCameraLookAtInfo updateFrustumTopViewLookAtInfo(const glm::vec3& position,
                                                       const glm::vec3& viewDirection,
                                                       const glm::vec3& up,
                                                       float nearPlane, float farPlane)
{
    const float centerPlaneDist = nearPlane + (farPlane - nearPlane) * 0.5f;
    const glm::vec3 viewCenter = position + viewDirection * centerPlaneDist;
    glm::vec3 viewVector = viewCenter - position;
    glm::vec3 upVector = up;

    const glm::vec3 rotationAxis = glm::normalize(glm::cross(upVector, viewDirection));
    const glm::mat4 rotationMat = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), rotationAxis);

    upVector = rotationMat * glm::vec4(upVector, 0.0f);
    viewVector = rotationMat * glm::vec4(viewVector, 0.0f);

    return TopViewCameraLookAtInfo{
        .position = viewCenter - viewVector,
        .viewCenter = viewCenter,
        .upVector = glm::normalize(upVector),
    };
}
KDBINDINGS_DECLARE_FUNCTION(updateFrustumTopViewLookAtInfoBinding, updateFrustumTopViewLookAtInfo)

TopViewCameraProjInfo updateFrustumTopViewProjInfo(float nearPlane, float farPlane,
                                                   float verticalFieldOfView,
                                                   float aspectRatio,
                                                   float interocularDistance,
                                                   bool toeIn)
{
    const float frustumLength = (farPlane - nearPlane);
    const float vFov = glm::radians(verticalFieldOfView);
    const float hFov = 2.0f * std::atan(aspectRatio * tan(vFov * 0.5f));

    float frustumShiftAtFarPlane = 0.0f;
    if (!toeIn) {
        frustumShiftAtFarPlane = 2.0f * std::fabs(interocularDistance);
    }

    const float frustumHalfWidth = frustumLength * std::tan(hFov * 0.5f) + frustumShiftAtFarPlane;
    const float frustumMaxHalfSize = std::max(frustumHalfWidth, frustumLength * 0.5f);

    return TopViewCameraProjInfo{
        .left = -frustumMaxHalfSize,
        .top = frustumMaxHalfSize * aspectRatio,
        .nearPlane = nearPlane,
        .farPlane = farPlane * 3.0f,
    };
}
KDBINDINGS_DECLARE_FUNCTION(updateFrustumTopViewProjInfoBinding, updateFrustumTopViewProjInfo)

} // namespace

SerenityRenderer::SerenityRenderer(SerenityWindow* window,
                                   StereoCamera& camera,
                                   std::function<void(std::string_view, std::any)> propertyUpdateNotifier)
    : m_window(window), m_stereoCamera(camera), m_propertyUpdateNofitier(propertyUpdateNotifier)
{
}

void SerenityRenderer::loadModel(std::filesystem::path file)
{
    setMode(Mode::Scene);
    if (m_sceneRoot == nullptr)
        return;

    // Release previous scene model
    m_sceneRoot->takeEntity(m_model);

    // Load Mesh
    std::unique_ptr<Entity> entity = MeshLoader::load(file);
    m_model = entity.get();
    if (m_model == nullptr)
        return;

    // Set Layers
    m_model->layerMask = m_layerManager->layerMask({ "Opaque" });
    auto* bv = m_model->createComponent<TriangleBoundingVolume>();
    bv->meshRenderer = entity->component<MeshRenderer>();
    bv->cacheTriangles = true; // Generate Octree for faster ray casting checks

    m_sceneRoot->addChildEntity(std::move(entity));

    viewAll();
}

void SerenityRenderer::viewAll()
{
    if (m_model == nullptr)
        return;
    auto* bv = m_model->component<TriangleBoundingVolume>();
    if (bv == nullptr)
        return;
    auto bb = bv->worldAxisAlignedBoundingBox.get();
    m_navParams->max_extent = bb.max;
    m_navParams->min_extent = bb.min;
    m_sceneCenter = (bb.max + bb.min) * 0.5f;
    m_sceneExtent = bb.max - bb.min;

    m_propertyUpdateNofitier("scene_loaded", {});
}

void SerenityRenderer::propertyChanged(std::string_view name, std::any value)
{
    if (name == "scale_factor") {
        m_cursor->scaleFactor = std::any_cast<float>(value);
        return;
    }

    if (name == "scaling_enabled") {
        m_cursor->scalingEnabled = std::any_cast<bool>(value);
        return;
    }

    if (name == "cursor_type") {
        m_cursor->type = std::any_cast<CursorType>(value);
        return;
    }

    if (name == "cursor_locked") {
        m_cursor->locked = std::any_cast<bool>(value);
        return;
    }

    if (name == "cursor_color") {
        auto color = std::any_cast<std::array<float, 4>>(value);
        m_cursor->color = ColorData{
            .ambient = { color[0], color[1], color[2], color[3] },
        };
        return;
    }

    if (name == "display_mode") {
        updateDisplayMode(std::any_cast<DisplayMode>(value));
        return;
    }


    if (name == "frustum_view_enabled") {
        const bool frustumEnabled = std::any_cast<bool>(value);
        m_frustumRect->enabled = frustumEnabled;
        m_leftFrustum->enabled = frustumEnabled;
        m_rightFrustum->enabled = frustumEnabled;
        return;
    }

    if (name == "show_focus_area") {
        const bool showFocusArea = std::any_cast<bool>(value);
        m_focusArea->enabled = showFocusArea;
        return;
    }

    if (name == "auto_focus") {
        const bool useAF = std::any_cast<bool>(value);
        m_pickingLayer->autoFocus = useAF;
        return;
    }

    if (name == "show_focus_plane") {
        const bool focusPlanePreviewEnabled = std::any_cast<bool>(value);
        m_focusPlanePreview->enabled = focusPlanePreviewEnabled;
    }
}

void SerenityRenderer::setCursorEnabled(bool enabled)
{
    if (enabled) {
        m_cursor->scaleFactor = scale_factor;
    } else {
        scale_factor = m_cursor->scaleFactor();
        m_cursor->scaleFactor = 0.0f;
    }
}

void SerenityRenderer::screenshot(const std::function<void(const uint8_t* data, uint32_t width, uint32_t height)>& in)
{
    assert(m_renderAlgorithm != nullptr);
    m_renderAlgorithm->Screenshot(in);
}

glm::vec3 SerenityRenderer::cursorWorldPosition() const
{
    return m_cursor->position();
}

glm::vec3 SerenityRenderer::sceneCenter() const
{
    return m_sceneCenter;
}

glm::vec3 SerenityRenderer::sceneExtent() const
{
    return m_sceneExtent;
}

float SerenityRenderer::fieldOfView() const
{
    return m_camera->lens()->verticalFieldOfView();
}

float SerenityRenderer::aspectRatio() const
{
    return m_camera->lens()->aspectRatio();
}

bool SerenityRenderer::hoversFocusArea(int x, int y) const
{
    return m_focusArea->containsMouse();
}

void SerenityRenderer::viewChanged()
{
    const float flippedCorrection = m_stereoCamera.flipped() ? -1.0f : 1.0f;
    const float interocularDistance = flippedCorrection * m_stereoCamera.interocularDistance();
    m_camera->interocularDistance = interocularDistance;
    m_camera->convergencePlaneDistance = m_stereoCamera.convergencePlaneDistance();
    m_camera->toeIn = m_stereoCamera.mode() == all::StereoCamera::Mode::ToeIn;

    m_camera->lookAt(m_stereoCamera.position(), m_stereoCamera.position() + m_stereoCamera.forwardVector(), m_stereoCamera.upVector());
}

void SerenityRenderer::projectionChanged()
{
    m_camera->lens()->setPerspectiveProjection(m_stereoCamera.fov(),
                                               m_stereoCamera.aspectRatio(),
                                               m_stereoCamera.nearPlane(),
                                               m_stereoCamera.farPlane());
}

void SerenityRenderer::createAspects(std::shared_ptr<all::ModelNavParameters> nav_params)
{
    m_navParams = std::move(nav_params);
    KDGpu::Device device = m_window->createDevice();

    m_layerManager = m_engine.createChild<Serenity::LayerManager>();
    for (auto&& layerName : { "Alpha", "Opaque", "StereoImage", "FocusArea", "Frustums" })
        m_layerManager->addLayer(layerName);

    auto rootEntityPtr = std::make_unique<Entity>();
    m_sceneRoot = rootEntityPtr.get();
    m_sceneRoot->setObjectName("Root Entity");

    // Add Camera into the Scene
    m_camera = m_sceneRoot->createChildEntity<Serenity::StereoCamera>();
    m_frustumAmplifiedCamera = m_sceneRoot->createChildEntity<Serenity::StereoCamera>();
    m_frustumTopViewCamera = m_sceneRoot->createChildEntity<Serenity::Camera>();

    // Create Scene Content
    createScene();

    // Create Render Algo
    auto algo = std::make_unique<StereoRenderAlgorithm>();
    m_renderAlgorithm = algo.get();

    const uint32_t maxSupportedSwapchainArrayLayers = device.adapter()->swapchainProperties(m_window->surface().handle()).capabilities.maxImageArrayLayers;
    auto spatialAspect = m_engine.createAspect<Serenity::SpatialAspect>();

    m_pickingLayer = m_engine.createApplicationLayer<PickingApplicationLayer>(spatialAspect);
    m_pickingLayer->camera = m_camera;
    m_pickingLayer->window = m_window;
    m_pickingLayer->cursor = m_cursor;
    m_pickingLayer->focusArea = m_focusArea;
    m_pickingLayer->autoFocusDistanceChanged
            .connect([this](float afDistance) {
                // Notify Controllers our AF Distance is updated
                m_propertyUpdateNofitier("auto_focus_distance", afDistance);
            })
            .release();

    m_renderAspect = m_engine.createAspect<Serenity::RenderAspect>(std::move(device));

    m_supportsStereoSwapchain = maxSupportedSwapchainArrayLayers > 1;

    Serenity::RenderTargetRef windowRenderTargetRef{
        .type = Serenity::RenderTargetRef::Type::Surface,
        .surfaceHandle = m_window->surface().handle(),
        .extentWatcher = std::make_shared<SerenityWindowExtentWatcher>(m_window),
        .arrayLayers = std::min(2u, maxSupportedSwapchainArrayLayers), // Request 2 array layers for stereo (if possible)
        .additionalUsageFlags = Serenity::RenderTargetUsageFlagBits::ShaderReadable | Serenity::RenderTargetUsageFlagBits::Capture,
    };
    Serenity::RenderTargetRef offscreenRenderTargetRef{
        .type = Serenity::RenderTargetRef::Type::Texture,
        .extentWatcher = std::make_shared<SerenityWindowExtentWatcher>(m_window),
        .arrayLayers = 2, // Request 2 array layers
        .additionalUsageFlags = Serenity::RenderTargetUsageFlagBits::Capture,
    };

    if (m_supportsStereoSwapchain) {
        algo->renderTargetRefs = { std::move(windowRenderTargetRef) };
        algo->offscreenMultiViewRenderTargetRefIndex = 0;
        algo->presentRenderTargetRefIndex = 0;
        algo->renderMode = Serenity::StereoForwardAlgorithm::StereoRenderMode::Stereo;
    } else {
        algo->renderTargetRefs = { std::move(offscreenRenderTargetRef), std::move(windowRenderTargetRef) };
        algo->offscreenMultiViewRenderTargetRefIndex = 0;
        algo->presentRenderTargetRefIndex = 1;
        algo->renderMode = Serenity::StereoForwardAlgorithm::StereoRenderMode::SideBySide;
    }

    // The RenderAlgo works in a 2 render passes process:

    // If we support Stereo Swachain
    // 1) MultiView Rendering (stereo render views) of the Scene into the Swapchain
    // 2) Overlay pass where we draw the overlay to a single of the array layer

    // If we don't support Stereo Swapchain
    // 1) Multiview Rendering (using the Offscreen Render Target
    // 2) Onscreen FullScreenQuad drawing of offscreen content + optional Overlays

    algo->camera = m_camera;
    algo->msaaSamples = Serenity::RenderAlgorithm::SamplesCount::Samples_4;

    auto* imguiOverlay = createImGuiOverlay(m_window, &m_engine, algo.get());

#ifdef FLUTTER_UI_ASSET_DIR
    auto* flutterOverlay = createFlutterOverlay(m_window, algo.get());
    algo->overlays = { imguiOverlay, flutterOverlay };
#else
    algo->overlays = { imguiOverlay };
#endif

    m_renderAspect->setRenderAlgorithm(std::move(algo));

    updateRenderPhases();

    m_engine.setRootEntity(std::move(rootEntityPtr));

    m_engine.running = true;
}

void SerenityRenderer::loadImage(std::filesystem::path url)
{
    if (m_sceneRoot == nullptr)
        return;

    // Release previous scene model
    m_sceneRoot->takeEntity(m_stereoImage);

    m_stereoImage = m_sceneRoot->createChildEntity<Entity>();
    m_stereoImage->setObjectName("Stereo Image Entity");
    m_stereoImage->layerMask = m_layerManager->layerMask({ "StereoImage" });

    auto* shader = m_stereoImage->createChild<SpirVShaderProgram>();
    shader->vertexShader = SHADER_DIR "stereoimage.vert.spv";
    shader->fragmentShader = SHADER_DIR "stereoimage.frag.spv";

    auto* material = m_stereoImage->createChild<Material>();
    material->setObjectName("Stereo Image Material");
    material->shaderProgram = shader;

    auto* texture = m_stereoImage->createChild<Texture2D>();
    texture->setObjectName("Stereo Image Texture");
    texture->setPath(url.string());
    material->setTexture(2, 2, texture);

    struct ViewportData {
        float viewportSize[2];
        float textureSize[2];
    };
    static_assert(sizeof(ViewportData) == 4 * sizeof(float));

    StaticUniformBuffer* viewportUbo = m_stereoImage->createChild<StaticUniformBuffer>();
    viewportUbo->size = sizeof(ViewportData);
    material->setUniformBuffer(3, 0, viewportUbo);

    const Material::UboDataBuilder materialDataBuilder = [this, texture](uint32_t set, uint32_t binding) {
        const ViewportData data = {
            .viewportSize = { static_cast<float>(m_window->width()), static_cast<float>(m_window->height()) },
            .textureSize = { static_cast<float>(texture->width()), static_cast<float>(texture->height()) }
        };
        std::vector<uint8_t> rawData(sizeof(ViewportData));
        std::memcpy(rawData.data(), &data, sizeof(ViewportData));
        return rawData;
    };
    material->setUniformBufferDataBuilder(materialDataBuilder);

    auto* mesh = m_stereoImage->createChild<Mesh>();
    mesh->setObjectName("Stereo Image Mesh");

    {
        struct Vertex {
            glm::vec2 position;
            glm::vec2 texCoord;
        };

        VertexFormat vertexFormat;
        vertexFormat.attributes.emplace_back(KDGpu::VertexAttribute{
                .location = 0,
                .binding = 0,
                .format = KDGpu::Format::R32G32_SFLOAT,
                .offset = offsetof(Vertex, position) });
        vertexFormat.attributes.emplace_back(KDGpu::VertexAttribute{
                .location = 1,
                .binding = 0,
                .format = KDGpu::Format::R32G32_SFLOAT,
                .offset = offsetof(Vertex, texCoord) });
        vertexFormat.buffers.emplace_back(KDGpu::VertexBufferLayout{
                .binding = 0,
                .stride = sizeof(Vertex),
                .inputRate = KDGpu::VertexRate::Vertex });
        mesh->vertexFormat = vertexFormat;

        const std::array<Vertex, 4> vertexData = { {
                { { -1, -1 }, { 0, 1 } },
                { { -1, 1 }, { 0, 0 } },
                { { 1, -1 }, { 1, 1 } },
                { { 1, 1 }, { 1, 0 } },
        } };

        std::vector<Mesh::VertexBufferData> verts(1);
        const auto vertexBufferSize = vertexData.size() * sizeof(Vertex);
        verts[0].resize(vertexBufferSize);
        std::memcpy(verts[0].data(), vertexData.data(), vertexBufferSize);
        mesh->setVertices(std::move(verts));
    }

    auto* renderer = m_stereoImage->createComponent<MeshRenderer>();
    renderer->mesh = mesh;
    renderer->material = material;
}

void SerenityRenderer::createScene()
{
    // Lights
    auto directionalLight = m_sceneRoot->createComponent<Light>();
    directionalLight->type = Light::Type::Directional;
    directionalLight->color = glm::vec4(0.6, 0.6, 0.7, 1.0f);
    directionalLight->worldDirection = glm::vec3(1.0f, -0.3f, 0.0f);

    auto pointLightEntity = m_sceneRoot->createChildEntity<Entity>();
    auto pointLightTransform =
            pointLightEntity->createComponent<SrtTransform>();
    pointLightTransform->translation = glm::vec3(0.0f, 10.0f, 0.0f);
    auto pointLight = pointLightEntity->createComponent<Light>();
    pointLight->type = Light::Type::Point;
    pointLight->color = glm::vec4(0.7, 0.5, 0.5, 1.0f);
    pointLight->intensity = 1.0f;

    // Create scene graph for the 3D scene
    loadModel("assets/motorbike.obj");

    // Create scene graph for the stereo image
    loadImage("assets/13_3840x2160_sbs.jpg");

    // 3D Cursor
    m_cursor = m_sceneRoot->createChildEntity<Cursor>(m_layerManager, m_window);
    m_cursor->type = CursorType::Ball;
    m_cursor->camera = m_camera;

    // Frustums
    {
        // Mirror m_camera with amplified eye separation
        m_frustumAmplifiedCamera->convergencePlaneDistance = KDBindings::makeBinding(m_camera->convergencePlaneDistance);
        m_frustumAmplifiedCamera->interocularDistance = KDBindings::makeBinding(m_camera->interocularDistance * 20.0f);
        m_frustumAmplifiedCamera->toeIn = KDBindings::makeBinding(m_camera->toeIn);
        m_frustumAmplifiedCamera->lens()->verticalFieldOfView = KDBindings::makeBinding(m_camera->lens()->verticalFieldOfView);
        m_frustumAmplifiedCamera->lens()->aspectRatio = KDBindings::makeBinding(m_camera->lens()->aspectRatio);
        m_frustumAmplifiedCamera->lens()->nearPlane = KDBindings::makeBinding(m_camera->lens()->nearPlane);
        m_frustumAmplifiedCamera->lens()->farPlane = KDBindings::makeBinding(m_camera->lens()->farPlane);
        m_frustumAmplifiedCamera->transform()->localTransform = KDBindings::makeBinding(m_camera->transform()->localTransform);

        // Create Binding to update frustum top view camera whenever m_frustumAmplifiedCamera changes
        m_topViewCameraLookAtInfo = KDBindings::makeBoundProperty(updateFrustumTopViewLookAtInfoBinding(m_frustumAmplifiedCamera->position,
                                                                                                        m_frustumAmplifiedCamera->viewDirection,
                                                                                                        m_frustumAmplifiedCamera->up,
                                                                                                        m_frustumAmplifiedCamera->lens()->nearPlane,
                                                                                                        m_frustumAmplifiedCamera->lens()->farPlane));
        m_topViewCameraProjInfo = KDBindings::makeBoundProperty(updateFrustumTopViewProjInfoBinding(m_frustumAmplifiedCamera->lens()->nearPlane,
                                                                                                    m_frustumAmplifiedCamera->lens()->farPlane,
                                                                                                    m_frustumAmplifiedCamera->lens()->verticalFieldOfView,
                                                                                                    m_frustumAmplifiedCamera->lens()->aspectRatio,
                                                                                                    m_frustumAmplifiedCamera->interocularDistance,
                                                                                                    m_frustumAmplifiedCamera->toeIn));

        m_topViewCameraLookAtInfo.valueChanged().connect([this](const TopViewCameraLookAtInfo& info) {
                                                    m_frustumTopViewCamera->lookAt(info.position, info.viewCenter, info.upVector);
                                                })
                .release();

        m_topViewCameraProjInfo.valueChanged().connect([this](const TopViewCameraProjInfo& info) {
                                                  m_frustumTopViewCamera->lens()->setOrthographicProjection(info.left, -info.left,
                                                                                                            info.top, -info.top,
                                                                                                            info.nearPlane, info.farPlane);
                                              })
                .release();

        m_frustumRect = m_sceneRoot->createChildEntity<FrustumRect>();
        m_frustumRect->layerMask = m_layerManager->layerMask({ "Frustums" });

        m_leftFrustum = m_sceneRoot->createChildEntity<Frustum>();
        m_leftFrustum->color = glm::vec4(0xff, 0x80, 0x80, 0x90) / 255.0f;
        m_leftFrustum->topViewCamera = m_frustumTopViewCamera;
        m_leftFrustum->viewMatrix = KDBindings::makeBinding(m_frustumAmplifiedCamera->leftViewMatrix);
        m_leftFrustum->projectionMatrix = KDBindings::makeBinding(m_frustumAmplifiedCamera->lens()->leftProjectionMatrix);
        m_leftFrustum->convergence = KDBindings::makeBinding(m_frustumAmplifiedCamera->convergencePlaneDistance);
        m_leftFrustum->layerMask = m_layerManager->layerMask({ "Frustums" });

        m_rightFrustum = m_sceneRoot->createChildEntity<Frustum>();
        m_rightFrustum->color = glm::vec4(0x00, 0x80, 0xff, 0x90) / 255.0f;
        m_rightFrustum->topViewCamera = m_frustumTopViewCamera;
        m_rightFrustum->viewMatrix = KDBindings::makeBinding(m_frustumAmplifiedCamera->rightViewMatrix);
        m_rightFrustum->projectionMatrix = KDBindings::makeBinding(m_frustumAmplifiedCamera->lens()->rightProjectionMatrix);
        m_rightFrustum->convergence = KDBindings::makeBinding(m_frustumAmplifiedCamera->convergencePlaneDistance);
        m_rightFrustum->layerMask = m_layerManager->layerMask({ "Frustums" });
    }
    // FocusArea
    {
        m_focusArea = m_sceneRoot->createChildEntity<FocusArea>();
        m_focusArea->camera = m_camera;
        m_focusArea->window = m_window;
        m_focusArea->layerMask = m_layerManager->layerMask({ "FocusArea" });
    }
    // FocusPlanePreview
    {
        m_focusPlanePreview = m_sceneRoot->createChildEntity<FocusPlanePreview>();
        m_focusPlanePreview->layerMask = m_layerManager->layerMask({ "Alpha" });
        m_focusPlanePreview->camera = m_camera;
    }
}

void SerenityRenderer::updateRenderPhases()
{
    auto* algo = static_cast<StereoForwardAlgorithm*>(m_renderAspect->renderAlgorithm());
    switch (m_mode) {
    case Mode::Scene:
        algo->renderPhases = { createOpaquePhase(), createTransparentPhase(), createFocusAreaPhase(), createFrustumPhase() };
        break;
    case Mode::StereoImage:
        algo->renderPhases = { createStereoImagePhase() };
        break;
    default:
        assert(false);
        break;
    }
}

void SerenityRenderer::updateDisplayMode(DisplayMode displayMode)
{
    switch (displayMode) {
    case all::DisplayMode::Stereo:
        m_renderAlgorithm->renderMode = (m_supportsStereoSwapchain) ? StereoForwardAlgorithm::StereoRenderMode::Stereo : StereoForwardAlgorithm::StereoRenderMode::SideBySide;
        break;
    case all::DisplayMode::Left:
        m_renderAlgorithm->renderMode = StereoForwardAlgorithm::StereoRenderMode::LeftOnly;
        break;
    case all::DisplayMode::Right:
        m_renderAlgorithm->renderMode = StereoForwardAlgorithm::StereoRenderMode::RightOnly;
        break;
    case all::DisplayMode::Mono:
        m_renderAlgorithm->renderMode = StereoForwardAlgorithm::StereoRenderMode::CenterOnly;
        break;
    }
}

Serenity::StereoForwardAlgorithm::RenderPhase SerenityRenderer::createOpaquePhase() const
{
    StereoForwardAlgorithm::RenderPhase phase{
        m_layerManager->layerMask({ "Opaque" }), StereoForwardAlgorithm::RenderPhase::Type::Opaque,
        LayerFilterType::AcceptAll
    };

    DepthStencilState depthState;
    depthState.depthTestEnabled = true;
    depthState.depthWritesEnabled = true;
    depthState.depthCompareOperation = KDGpu::CompareOperation::Less;
    phase.renderStates.setDepthStencilState(std::move(depthState));

    return phase;
}

Serenity::StereoForwardAlgorithm::RenderPhase SerenityRenderer::createFocusAreaPhase() const
{
    StereoForwardAlgorithm::RenderPhase phase{
        m_layerManager->layerMask({ "FocusArea" }), StereoForwardAlgorithm::RenderPhase::Type::Opaque,
        LayerFilterType::AcceptAll
    };

    DepthStencilState depthState;
    depthState.depthTestEnabled = false;
    depthState.depthWritesEnabled = false;
    depthState.depthCompareOperation = KDGpu::CompareOperation::Always;
    phase.renderStates.setDepthStencilState(std::move(depthState));

    return phase;
}

StereoForwardAlgorithm::RenderPhase SerenityRenderer::createFrustumPhase() const
{
    StereoForwardAlgorithm::RenderPhase phase{
        m_layerManager->layerMask({ "Frustums" }), StereoForwardAlgorithm::RenderPhase::Type::Alpha,
        LayerFilterType::AcceptAll
    };

    DepthStencilState depthState;
    depthState.depthTestEnabled = false;
    depthState.depthWritesEnabled = false;
    phase.renderStates.setDepthStencilState(std::move(depthState));

    ColorBlendState blendState;
    ColorBlendState::AttachmentBlendState attachmentBlendState;

    attachmentBlendState.format = KDGpu::Format::UNDEFINED;
    attachmentBlendState.blending.blendingEnabled = true;
    attachmentBlendState.blending.alpha.operation = KDGpu::BlendOperation::Add;
    attachmentBlendState.blending.color.operation = KDGpu::BlendOperation::Add;
    attachmentBlendState.blending.alpha.srcFactor = KDGpu::BlendFactor::SrcAlpha;
    attachmentBlendState.blending.color.srcFactor = KDGpu::BlendFactor::SrcAlpha;
    attachmentBlendState.blending.alpha.dstFactor = KDGpu::BlendFactor::DstAlpha;
    attachmentBlendState.blending.color.dstFactor = KDGpu::BlendFactor::DstAlpha;
    blendState.attachmentBlendStates = { attachmentBlendState };

    phase.renderStates.setColorBlendState(std::move(blendState));

    return phase;
}

Serenity::StereoForwardAlgorithm::RenderPhase SerenityRenderer::createTransparentPhase() const
{
    StereoForwardAlgorithm::RenderPhase phase{
        m_layerManager->layerMask({ "Alpha" }), StereoForwardAlgorithm::RenderPhase::Type::Alpha,
        LayerFilterType::AcceptAll
    };

    DepthStencilState depthState;
    depthState.depthTestEnabled = true;
    depthState.depthWritesEnabled = false;
    depthState.depthCompareOperation = KDGpu::CompareOperation::Less;
    phase.renderStates.setDepthStencilState(std::move(depthState));

    ColorBlendState blendState;
    ColorBlendState::AttachmentBlendState attachmentBlendState;

    attachmentBlendState.format = KDGpu::Format::UNDEFINED;
    attachmentBlendState.blending.blendingEnabled = true;
    attachmentBlendState.blending.alpha.operation = KDGpu::BlendOperation::Add;
    attachmentBlendState.blending.color.operation = KDGpu::BlendOperation::Add;
    attachmentBlendState.blending.alpha.srcFactor = KDGpu::BlendFactor::SrcAlpha;
    attachmentBlendState.blending.color.srcFactor = KDGpu::BlendFactor::SrcAlpha;
    attachmentBlendState.blending.alpha.dstFactor = KDGpu::BlendFactor::OneMinusSrcAlpha;
    attachmentBlendState.blending.color.dstFactor = KDGpu::BlendFactor::OneMinusSrcAlpha;
    blendState.attachmentBlendStates = { attachmentBlendState };

    phase.renderStates.setColorBlendState(std::move(blendState));

    return phase;
}

Serenity::StereoForwardAlgorithm::RenderPhase SerenityRenderer::createStereoImagePhase() const
{
    StereoForwardAlgorithm::RenderPhase phase = {
        .layers = m_layerManager->layerMask({ "StereoImage" }),
        .type = StereoForwardAlgorithm::RenderPhase::Type::Opaque,
        .layerFilterType = LayerFilterType::AcceptAll,
        .renderStates = {},
        .frustumCulling = false,
    };

    DepthStencilState depthState;
    depthState.depthTestEnabled = false;
    depthState.depthWritesEnabled = false;
    phase.renderStates.setDepthStencilState(std::move(depthState));

    PrimitiveRasterizerState inputAssemblyState;
    inputAssemblyState.topology = KDGpu::PrimitiveTopology::TriangleStrip;
    inputAssemblyState.cullMode = KDGpu::CullModeFlagBits::None;
    phase.renderStates.setPrimitiveRasterizerState(std::move(inputAssemblyState));

    return phase;
}

void SerenityRenderer::onMouseEvent(const KDFoundation::Event& event)
{
    auto* algo = static_cast<StereoForwardAlgorithm*>(m_renderAspect->renderAlgorithm());

    switch (event.type()) {
    case KDFoundation::Event::Type::MousePress: {
        m_focusArea->onMousePressed(*static_cast<const KDGui::MousePressEvent*>(&event));
        break;
    }
    case KDFoundation::Event::Type::MouseRelease: {
        m_focusArea->onMouseReleased(*static_cast<const KDGui::MouseReleaseEvent*>(&event));
        break;
    }
    case KDFoundation::Event::Type::MouseMove: {
        m_focusArea->onMouseMoved(*static_cast<const KDGui::MouseMoveEvent*>(&event));
        break;
    }
    default:
        break;
    }

    // Forward Events to the Overlays
    for (Serenity::AbstractOverlay* overlay : algo->overlays())
        overlay->event(overlay, const_cast<KDFoundation::Event*>(&event));
}

void SerenityRenderer::completeInitialization()
{
    viewAll();
}

} // namespace all::serenity
