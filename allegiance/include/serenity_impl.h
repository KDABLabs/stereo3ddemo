#pragma once

#include "serenity/cursor.h"
#include "stereo_camera.h"
#include "serenity_stereo_graph.h"
#include "picking_application_layer.h"
#include "window_extent_watcher.h"

#include <glm/gtx/compatibility.hpp>

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

class SerenityWindow
{
public:
    virtual ~SerenityWindow() = default;

    virtual uint32_t GetWidth() const = 0;
    virtual uint32_t GetHeight() const = 0;

    virtual glm::vec2 GetCursorPos() const = 0;

    virtual KDGpu::Instance& GetInstance() = 0;
    virtual KDGpu::Surface& GetSurface() = 0;
};

class SerenityImpl
{
public:
    explicit SerenityImpl(std::unique_ptr<SerenityWindow> window)
        : m_window(std::move(window))
    {
    }

    virtual ~SerenityImpl() = default;

    void ShowModel()
    {
        setMode(Mode::Scene);
    }

    void ShowImage()
    {
        setMode(Mode::StereoImage);
    }

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
                    { 0.4f, 0.4f, 0.4f, 1.0f },
                    { 0.6f, 0.6f, 0.6f, 1.0f },
                    { 0.1f, 0.1f, 0.1f, 1.0f },
                    0.0f,
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

        // Create scene graph for the 3D scene
        {
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
            meshloader.load("scene/terrain.obj", m_mesh.get());

            m_texture = std::make_unique<Texture2D>();
            m_texture->setObjectName("Model Texture");
            m_texture->setPath("scene/terrain.png");
            material->setTexture(4, 0, m_texture.get());

            m_model = e->createComponent<MeshRenderer>();
            m_model->mesh = m_mesh.get();
            m_model->material = material;

            TriangleBoundingVolume* bv = e->createComponent<TriangleBoundingVolume>();
            bv->meshRenderer = m_model;
            bv->cacheTriangles = true;
            bv->cullBackFaces = false;

            e->layerMask = layers.layerMask({ "Opaque" });
        }

        // Create scene graph for the stereo image
        {
            auto* shader = rootEntity->createChild<SpirVShaderProgram>();
            shader->vertexShader = "scene/stereoimage.vert.spv";
            shader->fragmentShader = "scene/stereoimage.frag.spv";

            auto* material = rootEntity->createChild<Material>();
            material->setObjectName("Stereo Image Material");
            material->shaderProgram = shader;

            auto* texture = rootEntity->createChild<Texture2D>();
            texture->setObjectName("Stereo Image Texture");
            texture->setPath("scene/13_3840x2160_sbs.jpg");
            material->setTexture(2, 2, texture);

            struct ViewportData {
                float viewportSize[2];
                float textureSize[2];
            };
            static_assert(sizeof(ViewportData) == 4 * sizeof(float));

            StaticUniformBuffer* viewportUbo = rootEntity->createChild<StaticUniformBuffer>();
            viewportUbo->size = sizeof(ViewportData);
            material->setUniformBuffer(3, 0, viewportUbo);

            const Material::UboDataBuilder materialDataBuilder = [this, texture](uint32_t set, uint32_t binding) {
                const ViewportData data = {
                    .viewportSize = { static_cast<float>(m_window->GetWidth()), static_cast<float>(m_window->GetHeight()) },
                    .textureSize = { static_cast<float>(texture->width()), static_cast<float>(texture->height()) }
                };
                std::vector<uint8_t> rawData(sizeof(ViewportData));
                std::memcpy(rawData.data(), &data, sizeof(ViewportData));
                return rawData;
            };
            material->setUniformBufferDataBuilder(materialDataBuilder);

            auto* mesh = rootEntity->createChild<Mesh>();
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
                mesh->setVertexFormat(vertexFormat);

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

            auto* entity = rootEntity->createChildEntity<Entity>();
            entity->setObjectName("Stereo Image Entity");
            entity->layerMask = layers.layerMask({ "StereoImage" });

            auto* renderer = entity->createComponent<MeshRenderer>();
            renderer->mesh = mesh;
            renderer->material = material;
        }

        m_cursor.emplace(rootEntity.get(), layers);

        return std::move(rootEntity);
    }

    void CreateAspects(all::OrbitalStereoCamera* camera)
    {
        KDGpu::AdapterAndDevice defaultDevice =
                m_window->GetInstance().createDefaultDevice(m_window->GetSurface());
        KDGpu::Device device = std::move(defaultDevice.device);

        m_layerManager = m_engine.createChild<LayerManager>();
        for (auto&& layerName : { "Alpha", "Opaque", "StereoImage" })
            m_layerManager->addLayer(layerName);

        std::unique_ptr<Entity> rootEntity = CreateScene(*m_layerManager);

        // Add Camera into the Scene

        m_camera = rootEntity->createChildEntity<StereoProxyCamera>();
        m_camera->SetMatrices(camera->GetViewLeft(), camera->GetViewRight(), camera->GetViewCenter());
        m_camera->lens()->setPerspectiveProjection(45.0f, camera->GetAspectRatio(), camera->GetNearPlane(), camera->GetFarPlane());

        camera->OnViewChanged.connect([this, camera]() {
            m_camera->SetMatrices(camera->GetViewLeft(), camera->GetViewRight(), camera->GetViewCenter());
        });
        camera->OnProjectionChanged.connect([this, camera]() {
            m_camera->lens()->setPerspectiveProjection(45.0f, camera->GetAspectRatio(), camera->GetNearPlane(), camera->GetFarPlane());
        });

        // Create Render Algo
        auto algo = std::make_unique<all::StereoRenderAlgorithm>();

        auto spatialAspect = m_engine.createAspect<SpatialAspect>();
        m_pickingLayer = m_engine.createApplicationLayer<PickingApplicationLayer>(m_camera, m_window.get(), spatialAspect, m_cursor->GetTransform());

        m_renderAspect = m_engine.createAspect<RenderAspect>(std::move(device));
        auto logicAspect = m_engine.createAspect<LogicAspect>();

        RenderTargetRef windowRenderTargetRef{
            RenderTargetRef::Type::Surface, m_window->GetSurface().handle(),
            std::make_shared<all::SerenityWindowExtentWatcher>(m_window.get()),
            2, // Request 2 array layers
        };
        algo->renderTargetRefs = { std::move(windowRenderTargetRef) };

        // The RenderAlgo works in a 2 render passes process:
        // 1) Offscreen MultiView Rendering (stereo render views) of the Scene
        // (using the Offscreen Render Target) 2) Onscreen FullScreenQuad drawing of
        // offscreen content + optional Overlays on Window backbuffer (using the
        // Window Render Target)

        algo->camera = m_camera;
        algo->offscreenMultiViewRenderTargetRefIndex = 0;
        algo->presentRenderTargetRefIndex = 0;
        algo->msaaSamples = RenderAlgorithm::SamplesCount::Samples_4;
        algo->renderMode = StereoForwardAlgorithm::StereoRenderMode::Stereo;

        m_renderAspect->setRenderAlgorithm(std::move(algo));

        updateRenderPhases();

        m_engine.setRootEntity(std::move(rootEntity));

        m_engine.running = true;
    }

    void SetCursorEnabled(bool enabled)
    {
        m_cursor->GetTransform()->scale = glm::vec3(enabled ? 1.0f : 0.0f);
        m_pickingLayer->SetEnabled(enabled);
    }

protected:
    enum class Mode {
        Scene,
        StereoImage
    };

    void setMode(Mode mode)
    {
        if (m_mode == mode)
            return;
        m_mode = mode;
        updateRenderPhases();
    }

    void updateRenderPhases()
    {
        auto* algo = static_cast<all::StereoRenderAlgorithm*>(m_renderAspect->renderAlgorithm());
        switch (m_mode) {
        case Mode::Scene:
            algo->renderPhases = { createOpaquePhase(), createTransparentPhase() };
            break;
        case Mode::StereoImage:
            algo->renderPhases = { createStereoImagePhase() };
            break;
        default:
            assert(false);
            break;
        }
    }

    StereoForwardAlgorithm::RenderPhase createOpaquePhase() const
    {
        StereoForwardAlgorithm::RenderPhase phase{
            m_layerManager->layerMask({ "Opaque" }), StereoForwardAlgorithm::RenderPhase::Type::Opaque,
            LayerFilterType::AcceptAll
        };

        auto depthState = std::make_shared<DepthStencilState>();
        depthState->depthTestEnabled = true;
        depthState->depthWritesEnabled = true;
        depthState->depthCompareOperation = KDGpu::CompareOperation::Less;
        phase.renderStates.setDepthStencilState(std::move(depthState));

        return phase;
    }

    StereoForwardAlgorithm::RenderPhase createTransparentPhase() const
    {
        StereoForwardAlgorithm::RenderPhase phase{
            m_layerManager->layerMask({ "Alpha" }), StereoForwardAlgorithm::RenderPhase::Type::Alpha,
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
    }

    StereoForwardAlgorithm::RenderPhase createStereoImagePhase() const
    {
        StereoForwardAlgorithm::RenderPhase phase = {
            .layers = m_layerManager->layerMask({ "StereoImage" }),
            .type = StereoForwardAlgorithm::RenderPhase::Type::Opaque,
            .layerFilterType = LayerFilterType::AcceptAll,
            .renderStates = {},
            .frustumCulling = false,
        };

        auto depthState = std::make_shared<DepthStencilState>();
        depthState->depthTestEnabled = false;
        depthState->depthWritesEnabled = false;
        phase.renderStates.setDepthStencilState(std::move(depthState));

        auto inputAssemblyState = std::make_shared<PrimitiveRasterizerState>();
        inputAssemblyState->topology = KDGpu::PrimitiveTopology::TriangleStrip;
        inputAssemblyState->cullMode = KDGpu::CullModeFlagBits::None;
        phase.renderStates.setPrimitiveRasterizerState(std::move(inputAssemblyState));

        return phase;
    }

    std::unique_ptr<SerenityWindow> m_window;

    Mode m_mode{ Mode::Scene };
    AspectEngine m_engine;
    RenderAspect* m_renderAspect{ nullptr };
    LayerManager* m_layerManager{ nullptr };

    std::unique_ptr<Mesh> m_mesh;
    std::unique_ptr<Texture2D> m_texture;
    Entity* m_scene;
    MeshRenderer* m_model;

    // Camera
    StereoProxyCamera* m_camera;
    PickingApplicationLayer* m_pickingLayer;

    std::optional<all::Cursor> m_cursor;
};
