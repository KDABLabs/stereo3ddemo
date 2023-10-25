#pragma once
#include "serenity/cursor.h"
#include "camera_control.h"
#include "stereo_camera.h"
#include "serenity_stereo_graph.h"

#include <QVulkanInstance>
#include <QFileDialog>
#include <QDirIterator>
#include <QMouseEvent>


#include <glm/gtx/compatibility.hpp>

#include <filesystem>

using namespace Serenity;

class StereoProxyCamera : public StereoCamera
{
public:
    using StereoCamera::StereoCamera;

public:
    void SetMatrices(const glm::mat4& left, const glm::mat4& right, const glm::mat4& center) noexcept
    {
        *(const_cast<KDBindings::Property<glm::mat4>*>(&leftEyeViewMatrix)) = left;
        *(const_cast<KDBindings::Property<glm::mat4>*>(&rightEyeViewMatrix)) = right;
        *(const_cast<KDBindings::Property<glm::mat4>*>(&centerEyeViewMatrix)) = center;
    }
};

class PickingApplicationLayer : public ApplicationLayer
{
public:
    PickingApplicationLayer(StereoProxyCamera* camera, QWindow* wnd, SpatialAspect* spatialAspect, SrtTransform* ctransform)
        : m_camera(camera), m_wnd(wnd), spatialAspect(spatialAspect), m_ctransform(ctransform)
    {
    }

    void onAfterRootEntityChanged(Entity* oldRoot, Entity* newRoot) override
    {
        m_pickedEntities.clear();
    }

    void update() override
    {
        if (!enabled) {
            return;
        }

        const glm::vec4 viewportRect = { 0.0f, 0.0f, m_wnd->width(), m_wnd->height() };

        // Perform ray cast
        const auto cursorPos = m_wnd->mapFromGlobal(QCursor::pos());
        const auto hits = spatialAspect->screenCast(glm::vec2(cursorPos.x(), cursorPos.y()), viewportRect, m_camera->centerEyeViewMatrix(), m_camera->lens()->projectionMatrix());

        auto unv = glm::unProject(glm::vec3(cursorPos.x(), m_wnd->size().height() - cursorPos.y(), 1.0f), m_camera->centerEyeViewMatrix(), m_camera->lens()->projectionMatrix(), viewportRect);

        if (!hits.empty()) {
            // Find closest intersection
            const auto closest = std::ranges::min_element(hits, [](const SpatialAspect::Hit& a, const SpatialAspect::Hit& b) {
                return a.distance < b.distance;
            });
            assert(closest != hits.end());
            m_ctransform->translation = closest->worldIntersection;
            m_ctransform->scale = glm::vec3(std::clamp(closest->distance * 10.f, 0.01f, 1.0f));
        } else {
            m_ctransform->translation = unv;
            m_ctransform->scale = glm::vec3(10.0f);
        }
    }
    void SetEnabled(bool en)
    {
        enabled = en;
    }

private:
    SrtTransform* m_ctransform;
    SpatialAspect* spatialAspect;
    QWindow* m_wnd;
    StereoProxyCamera* m_camera = nullptr;
    std::vector<Entity*> m_pickedEntities;
    bool enabled = true;
};

class SerenityImpl
{
public:
    SerenityImpl() { }

public:
    QWindow* GetWindow()
    {
        return &qwin;
    }

    void ShowModel()
    {
        // TODO
    }

    void ShowImage()
    {
        // TODO
    }

public:
    std::unique_ptr<Entity> CreateScene(LayerManager& layers) noexcept
    {
        auto rootEntity = std::make_unique<Entity>();
        rootEntity->setObjectName("Root Entity");

        auto shader = rootEntity->createChild<SpirVShaderProgram>();
        shader->vertexShader = "scene/multiview-scene.vert.spv";
        shader->fragmentShader = "scene/multiview-scene.frag.spv";

        // Lights
        auto directionalLight = rootEntity->createComponent<Light>();
        directionalLight->type = Light::Type::Directional;
        directionalLight->color = glm::vec4(0.6, 0.6, 0.7, 1.0f);
        directionalLight->worldDirection = glm::vec3(1.0f, -0.3f, 0.0f);

        auto pointLightEntity = rootEntity->createChildEntity<Entity>();
        auto pointLightTransform =
                pointLightEntity->createComponent<SrtTransform>();
        pointLightTransform->translation = glm::vec3(0.0f, 10.0f, 0.0f);
        auto pointLight = pointLightEntity->createComponent<Light>();
        pointLight->type = Light::Type::Point;
        pointLight->color = glm::vec4(0.7, 0.5, 0.5, 1.0f);
        pointLight->intensity = 1.0f;

        struct PhongData {
            float ambient[4];
            float diffuse[4];
            float specular[4];
            float shininess;
            int useTexture = true;
            float _pad[2];
        };
        static_assert(sizeof(PhongData) == (4 * 4 * sizeof(float)));

        const Material::UboDataBuilder materialDataBuilder[] = {
            [](uint32_t set, uint32_t binding) {
                const PhongData data{
                    { 0.2f, 0.2f, 0.2f, 1.0f },
                    { 0.6f, 0.6f, 0.6f, 1.0f },
                    { 1.0f, 1.0f, 1.0f, 1.0f },
                    5.0f,
                    true,
                    { 0.0f, 0.0f }
                };
                std::vector<uint8_t> rawData(sizeof(PhongData));
                std::memcpy(rawData.data(), &data, sizeof(PhongData));
                return rawData;
            },
            [](uint32_t set, uint32_t binding) {
                const PhongData data{
                    { 0.2f, 0.2f, 0.2f, 1.0f },
                    { 1.0f, 1.0f, 1.0f, 1.0f },
                    { 1.0f, 1.0f, 1.0f, 1.0f },
                    5.0f,
                    false,
                    { 0.0f, 0.0f }
                };
                std::vector<uint8_t> rawData(sizeof(PhongData));
                std::memcpy(rawData.data(), &data, sizeof(PhongData));
                return rawData;
            },
        };

        Entity* e = rootEntity->createChildEntity<Entity>();

        Material* material = e->createChild<Material>();
        material->shaderProgram = shader;

        StaticUniformBuffer* phongUbo = e->createChild<StaticUniformBuffer>();
        phongUbo->size = sizeof(PhongData);

        material->setUniformBuffer(3, 0, phongUbo);

        // This is how we feed Material properties
        material->setUniformBufferDataBuilder(materialDataBuilder[0]);

        SrtTransform* transform = e->createComponent<SrtTransform>();

        m_mesh = std::make_unique<Mesh>();
        m_mesh->setObjectName("Model Mesh");
        MeshLoader meshloader;
        meshloader.load("scene/cottage.obj", m_mesh.get());

        m_texture = std::make_unique<Texture2D>();
        m_texture->setObjectName("Model Texture");
        m_texture->setPath("scene/cottage_diffuse.png");
        material->setTexture(4, 0, m_texture.get());

        m_model = e->createComponent<MeshRenderer>();
        m_model->mesh = m_mesh.get();
        m_model->material = material;

        TriangleBoundingVolume* bv = e->createComponent<TriangleBoundingVolume>();
        bv->meshRenderer = m_model;
        bv->cacheTriangles = true;
        bv->cullBackFaces = false;

        e->layerMask = layers.layerMask({ "Opaque" });
        m_cursor.emplace(rootEntity.get(), layers);
        return std::move(rootEntity);
    }
    void CreateAspects(all::CameraControl* cc, all::OrbitalStereoCamera* camera)
    {
        // Create Qt Vulkan Instance
        vk_instance.setApiVersion(QVersionNumber(1, 2, 0));
        if (!vk_instance.create()) {
            qCritical() << "Failed to create QVulkanInstance";
            throw std::runtime_error{ "Failed to create QVulkanInstance" };
        }
        qwin.setSurfaceType(QSurface::SurfaceType::VulkanSurface);
        qwin.setVulkanInstance(&vk_instance);
        qwin.create();

        VkSurfaceKHR vkSurface = QVulkanInstance::surfaceForWindow(&qwin);

        // Initialize KDGpu
        m_graphicsApi = std::make_unique<KDGpu::VulkanGraphicsApi>();

        // Create KDGpu Instance from VkInstance
        m_instance = m_graphicsApi->createInstanceFromExistingVkInstance(
                vk_instance.vkInstance());
        m_surface = m_graphicsApi->createSurfaceFromExistingVkSurface(m_instance,
                                                                      vkSurface);
        KDGpu::AdapterAndDevice defaultDevice =
                m_instance.createDefaultDevice(m_surface);
        KDGpu::Device device = std::move(defaultDevice.device);

        auto layerManager = engine.createChild<LayerManager>();
        for (auto&& layerName : { "Alpha", "Opaque" })
            layerManager->addLayer(layerName);

        auto rootEntity = CreateScene(*layerManager);

        // Add Camera into the Scene

        m_camera = rootEntity->createChildEntity<StereoProxyCamera>();
        m_camera->SetMatrices(camera->GetViewLeft(), camera->GetViewRight(), camera->GetViewCenter());
        m_camera->lens()->setPerspectiveProjection(45.0f, camera->GetAspectRatio(), camera->GetNearPlane(), camera->GetFarPlane());

        QObject::connect(camera, &all::OrbitalStereoCamera::OnViewChanged, [this, camera]() {
            m_camera->SetMatrices(camera->GetViewLeft(), camera->GetViewRight(), camera->GetViewCenter());
        });
        QObject::connect(camera, &all::OrbitalStereoCamera::OnProjectionChanged, [this, camera]() {
            m_camera->lens()->setPerspectiveProjection(45.0f, camera->GetAspectRatio(), camera->GetNearPlane(), camera->GetFarPlane());
        });
        QObject::connect(cc, &all::CameraControl::OnToggleCursor, [this](bool checked) {
            if (!checked)
                m_cursor->GetTransform()->scale = glm::vec3(0.0f);
            m_pickingLayer->SetEnabled(checked);
        });

        // Create Render Algo
        auto algo = std::make_unique<all::StereoRenderAlgorithm>();

        auto createOpaquePhase = [layerManager]() {
            StereoForwardAlgorithm::RenderPhase phase{
                layerManager->layerMask({ "Opaque" }), StereoForwardAlgorithm::RenderPhase::Type::Opaque,
                LayerFilterType::AcceptAll
            };

            auto depthState = std::make_shared<DepthStencilState>();
            depthState->depthTestEnabled = true;
            depthState->depthWritesEnabled = true;
            depthState->depthCompareOperation = KDGpu::CompareOperation::Less;
            phase.renderStates.setDepthStencilState(std::move(depthState));

            return phase;
        };
        auto createtransparentPhase = [layerManager]() {
            StereoForwardAlgorithm::RenderPhase phase{
                layerManager->layerMask({ "Alpha" }), StereoForwardAlgorithm::RenderPhase::Type::Alpha,
                LayerFilterType::AcceptAll
            };

            auto depthState = std::make_shared<DepthStencilState>();
            depthState->depthTestEnabled = true;
            depthState->depthWritesEnabled = false;
            depthState->depthCompareOperation = KDGpu::CompareOperation::Less;
            phase.renderStates.setDepthStencilState(std::move(depthState));

            auto blendState = std::make_shared<ColorBlendState>();
            ColorBlendState::AttachmentBlendState attachmentBlendState;

            attachmentBlendState.format = KDGpu::Format::UNDEFINED;
            attachmentBlendState.blending.blendingEnabled = true;
            attachmentBlendState.blending.alpha.operation = KDGpu::BlendOperation::Add;
            attachmentBlendState.blending.color.operation = KDGpu::BlendOperation::Add;
            attachmentBlendState.blending.alpha.srcFactor = KDGpu::BlendFactor::SrcAlpha;
            attachmentBlendState.blending.color.srcFactor = KDGpu::BlendFactor::SrcAlpha;
            attachmentBlendState.blending.alpha.dstFactor = KDGpu::BlendFactor::OneMinusSrcAlpha;
            attachmentBlendState.blending.color.dstFactor = KDGpu::BlendFactor::OneMinusSrcAlpha;
            blendState->attachmentBlendStates = { attachmentBlendState };

            phase.renderStates.setColorBlendState(std::move(blendState));
            return phase;
        };
        auto spatialAspect = engine.createAspect<SpatialAspect>();
        m_pickingLayer = engine.createApplicationLayer<PickingApplicationLayer>(m_camera, &qwin, spatialAspect, m_cursor->GetTransform());

        auto renderAspect = engine.createAspect<RenderAspect>(std::move(device));
        auto logicAspect = engine.createAspect<LogicAspect>();

        RenderTargetRef windowRenderTargetRef{
            RenderTargetRef::Type::Surface, m_surface.handle(),
            std::make_shared<all::QWindowExtentWatcher>(&qwin),
            2, // Request 2 array layers
        };
        algo->renderTargetRefs = { std::move(windowRenderTargetRef) };

        // The RenderAlgo works in a 2 render passes process:
        // 1) Offscreen MultiView Rendering (stereo render views) of the Scene
        // (using the Offscreen Render Target) 2) Onscreen FullScreenQuad drawing of
        // offscreen content + optional Overlays on Window backbuffer (using the
        // Window Render Target)

        algo->camera = m_camera;
        algo->renderPhases = { createOpaquePhase(), createtransparentPhase() };
        algo->offscreenMultiViewRenderTargetRefIndex = 0;
        algo->presentRenderTargetRefIndex = 0;
        algo->msaaSamples = RenderAlgorithm::SamplesCount::Samples_4;
        algo->renderMode = StereoForwardAlgorithm::StereoRenderMode::Stereo;

        renderAspect->setRenderAlgorithm(std::move(algo));
        engine.setRootEntity(std::move(rootEntity));

        engine.running = true;
    }

private:
    QVulkanInstance vk_instance;
    QWindow qwin;

    std::unique_ptr<KDGpu::VulkanGraphicsApi> m_graphicsApi;
    KDGpu::Instance m_instance;
    KDGpu::Surface m_surface;

    AspectEngine engine;

    std::unique_ptr<Mesh> m_mesh;
    std::unique_ptr<Texture2D> m_texture;
    Entity* m_scene;
    MeshRenderer* m_model;

    // Camera
    StereoProxyCamera* m_camera;
    PickingApplicationLayer* m_pickingLayer;

    std::optional<all::Cursor> m_cursor;
};