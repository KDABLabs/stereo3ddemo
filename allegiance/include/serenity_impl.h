#pragma once
#include "serenity.h"
#include "camera_control.h"
#include <QVulkanInstance>
#include <filesystem>
#include <QFileDialog>
#include <QDirIterator>
#include <glm/gtx/compatibility.hpp>

using namespace Serenity;

class OrbitalStereoCamera : public StereoCamera
{
public:
    OrbitalStereoCamera()
    {
        angles = glm::vec2(0.0f, 0.0f);
        radius = 10.0f;
        static auto change = [this]() { updateViewMatrix(); };

        radius.valueChanged().connect(change);
        angles.valueChanged().connect(change);
    }

protected:
    void updateViewMatrix() override
    {
        auto pos = glm::vec3(radius(), radius(), radius());
        auto sins = glm::sin(angles());
        auto coss = glm::cos(angles());
        pos.x *= sins.x * coss.y;
        pos.y *= coss.x;
        pos.z *= sins.x * sins.y;

        (*const_cast<KDBindings::Property<glm::vec3>*>(&position)) = pos;
        const glm::vec3 right = glm::normalize(glm::cross(viewDirection(), up())) * interocularDistance() * 0.5f;

        *(const_cast<KDBindings::Property<glm::mat4>*>(&leftEyeViewMatrix)) =
                stereoShear(shearCoefficient(convergencePlaneDistance(), -interocularDistance() * 0.5f, convergeOnNear())) * glm::lookAt(position() + right, right + viewDirection(), up());
        *(const_cast<KDBindings::Property<glm::mat4>*>(&rightEyeViewMatrix)) =
                stereoShear(shearCoefficient(convergencePlaneDistance(), interocularDistance() * 0.5f, convergeOnNear())) * glm::lookAt(position() - right, -right + viewDirection(), up());
        *(const_cast<KDBindings::Property<glm::mat4>*>(&centerEyeViewMatrix)) =
                glm::lookAt(position(), viewDirection(), up());
    }

public:
    KDBindings::Property<float> radius;
    KDBindings::Property<glm::vec2> angles;
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
    void LoadModel()
    {
        QFileDialog fd;
        fd.setFileMode(QFileDialog::ExistingFile);
        fd.setNameFilter("*.obj");
        fd.setViewMode(QFileDialog::Detail);
        if (fd.exec()) {
            auto path = fd.selectedFiles()[0];
            auto folder = fd.directory().path();

            auto model = std::make_unique<Mesh>();
            model->setObjectName("Model Mesh");
            MeshLoader meshloader;
            meshloader.load(path.toStdString(), model.get());
            m_model->mesh = model.get();

            QString texturePath;
            QDirIterator it(fd.directory());
            while (it.hasNext()) {
                QString filename = it.next();
                QFileInfo file(filename);

                if (file.isDir()) { // Check if it's a dir
                    continue;
                }

                // If the filename contains target string - put it in the hitlist
                if (file.fileName().contains("diffuse", Qt::CaseInsensitive)) {
                    texturePath = file.filePath();
                    break;
                }
            }

            m_texture = std::make_unique<Texture2D>();
            m_texture->setObjectName("Model Texture");
            if (!texturePath.isEmpty())
                m_texture->setPath(texturePath.toStdString());
            m_model->material()->setTexture(2, 2, m_texture.get());

            m_mesh = std::move(model);
        }
    }

public:
    std::unique_ptr<Entity> CreateScene() noexcept
    {
        auto rootEntity = std::make_unique<Entity>();
        rootEntity->setObjectName("Root Entity");

        auto shader = rootEntity->createChild<SpirVShaderProgram>();
        shader->vertexShader = "multiview-scene.vert.spv";
        shader->fragmentShader = "multiview-scene.frag.spv";

        // Lights
        auto directionalLight = rootEntity->createComponent<Light>();
        directionalLight->type = Light::Type::Directional;
        directionalLight->color = glm::vec4(0.6, 0.6, 0.7, 1.0f);
        directionalLight->worldDirection = glm::vec3(1.0f, -0.3f, 0.0f);

        auto pointLightEntity = rootEntity->createChildEntity<Entity>();
        auto pointLightTransform =
                pointLightEntity->createComponent<SrtTransform>();
        pointLightTransform->translation = glm::vec3(0.0f, 2.0f, 0.0f);
        auto pointLight = pointLightEntity->createComponent<Light>();
        pointLight->type = Light::Type::Point;
        pointLight->color = glm::vec4(0.7, 0.5, 0.5, 1.0f);
        pointLight->intensity = 1.0f;

        auto spotLightEntity = rootEntity->createChildEntity<Entity>();
        auto spotLightTransform = spotLightEntity->createComponent<SrtTransform>();
        spotLightTransform->translation = glm::vec3(0.0f, 10.0f, 0.0f);
        auto spotLight = spotLightEntity->createComponent<Light>();
        spotLight->type = Light::Type::Spot;
        spotLight->color = glm::vec4(0.4, 0.7, 0.4, 1.0f);
        spotLight->localDirection = glm::vec3(0.0f, -1.0f, 0.0f);
        spotLight->cutOffAngleDegrees = 30.0f;

        struct PhongData {
            float ambient[4];
            float diffuse[4];
            float specular[4];
            float shininess;
            float _pad[3];
        };
        static_assert(sizeof(PhongData) == (4 * 4 * sizeof(float)));

        const Material::UboDataBuilder materialDataBuilder[] = {
            [](uint32_t set, uint32_t binding) {
                const PhongData data{ { 0.2f, 0.2f, 0.2f, 1.0f },
                                      { 0.6f, 0.6f, 0.6f, 1.0f },
                                      { 1.0f, 1.0f, 1.0f, 1.0f },
                                      5.0f,
                                      { 0.0f, 0.0f, 0.0f } };
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
        m_model = e->createComponent<MeshRenderer>();
        m_model->mesh = m_mesh.get();
        m_model->material = material;

        return std::move(rootEntity);
    }
    void CreateAspects(all::CameraControl* cc)
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

        auto rootEntity = CreateScene();

        // Add Camera into the Scene

        auto camera = rootEntity->createChildEntity<OrbitalStereoCamera>();
        camera->lookAt(glm::vec3(10.0f, 10.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                       glm::vec3(0.0f, 1.0f, 0.0f));
        camera->lens()->setPerspectiveProjection(
                45.0f, float(qwin.width()) / qwin.height(), 0.01f, 1000.0f);
        camera->lens()->aspectRatio = float(qwin.width()) / qwin.height();

        QObject::connect(&qwin, &QWindow::widthChanged, [camera, this](int width) {
            camera->lens()->aspectRatio = float(qwin.width()) / qwin.height();
        });
        QObject::connect(&qwin, &QWindow::heightChanged, [camera, this](int height) {
            camera->lens()->aspectRatio = float(qwin.width()) / qwin.height();
        });
        QObject::connect(cc, &all::CameraControl::OnFocusPlaneChanged, [camera, this](float v) {
            camera->convergencePlaneDistance = v;
        });
        QObject::connect(cc, &all::CameraControl::OnEyeDisparityChanged, [camera, this](float v) {
            camera->interocularDistance = v;
        });

        QObject::connect(cc, &all::CameraControl::OnLoadModel, [this]() {
            LoadModel();
        });

        camera->interocularDistance = 0.005f;
        camera->radius = 20.0f;
        camera->angles = { 0.5, 0.5 };

        // Create Render Algo
        auto algo = std::make_unique<StereoForwardAlgorithm>();

        auto createOpaquePhase = []() {
            StereoForwardAlgorithm::RenderPhase phase{
                0, StereoForwardAlgorithm::RenderPhase::Type::Opaque,
                LayerFilterType::AcceptAll
            };

            auto depthState = std::make_shared<DepthStencilState>();
            depthState->depthTestEnabled = true;
            depthState->depthWritesEnabled = true;
            depthState->depthCompareOperation = KDGpu::CompareOperation::Less;
            phase.renderStates.setDepthStencilState(std::move(depthState));

            return phase;
        };

        auto spatialAspect = engine.createAspect<SpatialAspect>();
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

        algo->camera = camera;
        algo->renderPhases = { createOpaquePhase() };
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
};