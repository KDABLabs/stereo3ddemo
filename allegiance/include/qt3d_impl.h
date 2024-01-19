#pragma once
#if !ALLEGIANCE_SERENITY
#include "qt3d_shaders.h"
#include "camera_control.h"
#include "stereo_camera.h"
#include "qt3d_materials.h"
#include "qt3d_cursor.h"
#include "stereo_image_mesh.h"
#include "stereo_image_material.h"
#include "spacemouse.h"
#include <QFileDialog>


class QStereoProxyCamera : Qt3DCore::QEntity
{
public:
    QStereoProxyCamera(Qt3DCore::QNode* parent = nullptr)
        : Qt3DCore::QEntity(parent)
    {
        m_leftCamera = new Qt3DCore::QEntity(this);
        m_rightCamera = new Qt3DCore::QEntity(this);

        m_leftTransform = new Qt3DCore::QTransform;
        m_leftTransform->setObjectName("Left Camera Transform");
        m_leftCamera->addComponent(m_leftTransform);

        m_rightTransform = new Qt3DCore::QTransform;
        m_rightTransform->setObjectName("Right Camera Transform");
        m_rightCamera->addComponent(m_rightTransform);

        m_leftCameraLens = new Qt3DRender::QCameraLens;
        m_leftCameraLens->setObjectName("Left Camera Lens");
        m_leftCamera->addComponent(m_leftCameraLens);

        m_rightCameraLens = new Qt3DRender::QCameraLens;
        m_rightCameraLens->setObjectName("Right Camera Lens");
        m_rightCamera->addComponent(m_rightCameraLens);
    }

public:
    Qt3DCore::QEntity* GetLeftCamera() const
    {
        return m_leftCamera;
    }
    Qt3DCore::QEntity* GetRightCamera() const
    {
        return m_rightCamera;
    }

    void SetPositionAndForward(const QVector3D& position, const QQuaternion& rotation)
    {
        m_leftTransform->setTranslation(position);
        m_leftTransform->setRotation(rotation);
        m_rightTransform->setTranslation(position);
        m_rightTransform->setRotation(rotation);
    }

    void SetMatrices(const QMatrix4x4& left, const QMatrix4x4& right)
    {
        m_leftTransform->setMatrix(left.inverted());
        m_rightTransform->setMatrix(right.inverted());
    }
    void SetProjection(const QMatrix4x4& proj, qreal skew)
    {
        QMatrix4x4 m;
        m(0, 2) = -skew;
        m_leftCameraLens->setProjectionMatrix(m * proj);
        m(0, 2) = skew;
        m_rightCameraLens->setProjectionMatrix(m * proj);
    }

private:
    Qt3DCore::QEntity* m_leftCamera;
    Qt3DCore::QEntity* m_rightCamera;
    Qt3DCore::QTransform* m_leftTransform;
    Qt3DCore::QTransform* m_rightTransform;
    Qt3DRender::QCameraLens* m_leftCameraLens;
    Qt3DRender::QCameraLens* m_rightCameraLens;
};

class QStereoForwardRenderer : public Qt3DRender::QRenderSurfaceSelector
{
    Q_OBJECT
public:
    enum class Mode {
        Scene,
        StereoImage
    };
    Q_ENUM(Mode)

    QStereoForwardRenderer(Qt3DCore::QNode* parent = nullptr)
        : Qt3DRender::QRenderSurfaceSelector(parent)
        , m_camera(nullptr)
        , m_leftLayer(new Qt3DRender::QLayer(this))
        , m_rightLayer(new Qt3DRender::QLayer(this))
        , m_stereoImageLayer(new Qt3DRender::QLayer(this))
    {
        auto vp = new Qt3DRender::QViewport(this);
        auto ssel = new Qt3DRender::QRenderSurfaceSelector(vp);

        // Frame graph sub branch for the 3D scene

        m_sceneNoDraw = new Qt3DRender::QNoDraw(ssel);
        m_sceneNoDraw->setObjectName("Scene");
        m_sceneNoDraw->setEnabled(false);

        auto* stereoImageLayerFilter = new Qt3DRender::QLayerFilter(m_sceneNoDraw);
        stereoImageLayerFilter->addLayer(m_stereoImageLayer);
        stereoImageLayerFilter->setFilterMode(Qt3DRender::QLayerFilter::FilterMode::DiscardAnyMatchingLayers);

        auto makeRenderTargetSelector = [](Qt3DRender::QFrameGraphNode* parent, Qt3DRender::QRenderTargetOutput::AttachmentPoint attachment) {
            auto* output = new Qt3DRender::QRenderTargetOutput;
            output->setAttachmentPoint(attachment);

            auto* renderTarget = new Qt3DRender::QRenderTarget;
            renderTarget->addOutput(output);

            auto* selector = new Qt3DRender::QRenderTargetSelector(parent);
            selector->setTarget(renderTarget);

            return selector;
        };

        auto makeBranch = [makeRenderTargetSelector](Qt3DRender::QFrameGraphNode* parent, Qt3DRender::QRenderTargetOutput::AttachmentPoint attachment) {
            auto rts = makeRenderTargetSelector(parent, attachment);

            auto cb = new Qt3DRender::QClearBuffers(rts);
            cb->setBuffers(Qt3DRender::QClearBuffers::ColorDepthBuffer);
            //            cb->setClearColor(attachment == Qt3DRender::QRenderTargetOutput::AttachmentPoint::Left ? QColor(Qt::blue) : QColor(Qt::red));
            auto s = new QSortPolicy{cb};
            s->setSortTypes(QList<QSortPolicy::SortType>{QSortPolicy::BackToFront});

            return new Qt3DRender::QCameraSelector(s);
        };

        m_leftCamera = makeBranch(stereoImageLayerFilter, Qt3DRender::QRenderTargetOutput::Left);
        m_rightCamera = makeBranch(stereoImageLayerFilter, Qt3DRender::QRenderTargetOutput::Right);

        // Frame graph sub branch for the stereo image

        m_stereoImageNoDraw = new Qt3DRender::QNoDraw(ssel);
        m_stereoImageNoDraw->setObjectName("Stereo Image");
        m_stereoImageNoDraw->setEnabled(true);

        auto* imageLeftRenderTarget = makeRenderTargetSelector(m_stereoImageNoDraw, Qt3DRender::QRenderTargetOutput::Left);
        auto* leftLayerFilter = new Qt3DRender::QLayerFilter(imageLeftRenderTarget);
        leftLayerFilter->addLayer(m_leftLayer);

        auto* imageRightRenderTarget = makeRenderTargetSelector(m_stereoImageNoDraw, Qt3DRender::QRenderTargetOutput::Right);
        auto* rightLayerFilter = new Qt3DRender::QLayerFilter(imageRightRenderTarget);
        rightLayerFilter->addLayer(m_rightLayer);

#ifdef QT_DEBUG
        (void)new Qt3DRender::QDebugOverlay(m_rightCamera);
#endif
    }

    QStereoProxyCamera* camera() const
    {
        return m_camera;
    }

    Qt3DRender::QLayer* leftLayer() const
    {
        return m_leftLayer;
    }

    Qt3DRender::QLayer* rightLayer() const
    {
        return m_rightLayer;
    }

    Qt3DRender::QLayer* stereoImageLayer() const
    {
        return m_stereoImageLayer;
    }

    void setMode(Mode mode)
    {
        if (mode == m_mode)
            return;
        switch (mode) {
        case Mode::Scene:
            m_sceneNoDraw->setEnabled(false);
            m_stereoImageNoDraw->setEnabled(true);
            break;
        case Mode::StereoImage:
            m_sceneNoDraw->setEnabled(true);
            m_stereoImageNoDraw->setEnabled(false);
            break;
        }
        m_mode = mode;
    }

    void setCamera(QStereoProxyCamera* newCamera)
    {
        if (m_camera == newCamera)
            return;
        m_camera = newCamera;
        Q_EMIT cameraChanged();

        if (m_camera) {
            m_leftCamera->setCamera(m_camera->GetLeftCamera());
            m_rightCamera->setCamera(m_camera->GetRightCamera());
        }
    }

Q_SIGNALS:
    void cameraChanged();

private:
    Mode m_mode = Mode::Scene;
    Qt3DRender::QCameraSelector* m_leftCamera;
    Qt3DRender::QCameraSelector* m_rightCamera;
    Qt3DRender::QLayer* m_leftLayer;
    Qt3DRender::QLayer* m_rightLayer;
    Qt3DRender::QLayer* m_stereoImageLayer;
    Qt3DRender::QNoDraw* m_sceneNoDraw;
    Qt3DRender::QNoDraw* m_stereoImageNoDraw;
    QStereoProxyCamera* m_camera;
};

inline QMatrix4x4 toMatrix(const glm::mat4x4& m)
{
    QMatrix4x4 q{
        m[0][0], m[0][1], m[0][2], m[0][3],
        m[1][0], m[1][1], m[1][2], m[1][3],
        m[2][0], m[2][1], m[2][2], m[2][3],
        m[3][0], m[3][1], m[3][2], m[3][3]
    };
    // QMatrix4x4 q2{ };
    // std::copy(&m[0][0], &m[0][0] + 16, q2.data());
    return q.transposed();
}

class Qt3DImpl : public QObject
{
    Q_OBJECT
public:
    Qt3DImpl()
    {
        QSurfaceFormat format = QSurfaceFormat::defaultFormat();
        if (qgetenv("DISABLE_STEREO") == "")
            format.setStereo(true);
        format.setSamples(1);
        QSurfaceFormat::setDefaultFormat(format);
    }

public:
    void CreateScene(Qt3DCore::QEntity* root)
    {
    }

    void CreateAspects(all::StereoCamera* camera)
    {
        using namespace Qt3DCore;
        using namespace Qt3DRender;
        using namespace Qt3DExtras;

        m_rootEntity = std::make_unique<Qt3DCore::QEntity>();

        m_renderer = new QStereoForwardRenderer();
        m_view.setActiveFrameGraph(m_renderer);
        m_view.renderSettings()->pickingSettings()->setPickMethod(QPickingSettings::TrianglePicking);

        m_camera = new QStereoProxyCamera(m_rootEntity.get());
        m_renderer->setCamera(m_camera);

        camera->SetPosition({0.2, 5, -10});
        camera->SetForwardVector({0, .5, -1});

        m_camera->SetMatrices(toMatrix(camera->GetViewLeft()), toMatrix(camera->GetViewRight()));
        m_camera->SetProjection(toMatrix(camera->GetProjection()), camera->ShearCoefficient());

        QObject::connect(camera, &all::StereoCamera::OnViewChanged, [this, camera]() {
            m_camera->SetMatrices(toMatrix(camera->GetViewLeft()), toMatrix(camera->GetViewRight()));
            m_camera->SetProjection(toMatrix(camera->GetProjection()), camera->ShearCoefficient());
        });
        QObject::connect(camera, &all::StereoCamera::OnProjectionChanged, [this, camera]() {
            m_camera->SetProjection(toMatrix(camera->GetProjection()), camera->ShearCoefficient());
        });


        m_sceneEntity = new Qt3DCore::QEntity{m_rootEntity.get()};
        m_userEntity = new Qt3DCore::QEntity{m_sceneEntity};
        m_cursor = new CursorEntity(m_rootEntity.get(), m_camera->GetLeftCamera(), &m_view);
        new Picker(m_sceneEntity, m_cursor);
        m_view.setRootEntity(m_rootEntity.get());

        CreateScene(m_rootEntity.get());
        LoadModel();
    }

    QWindow* GetWindow() { return &m_view; }

    void ShowImage()
    {
        m_renderer->setMode(QStereoForwardRenderer::Mode::StereoImage);
    }

    void ShowModel()
    {
        m_renderer->setMode(QStereoForwardRenderer::Mode::Scene);
        LoadModelDialog();
    }

    void SetCursorEnabled(bool /* enabled */)
    {
        // TODO
    }

    void LoadModelDialog()
    {
        auto fn = QFileDialog::getOpenFileName(this->m_widget.get(), "Open Model", "", "Model Files (*.obj *.fbx *.gltf *.glb)");
        if(!fn.isEmpty())
            LoadModel(QUrl{"file:" + fn});
    }

    void LoadModel(QUrl path = QUrl::fromLocalFile("scene/fbx/showroom2303.fbx"))
    {
        auto* scene = new Qt3DRender::QSceneLoader(m_sceneEntity);
        scene->setObjectName("Model Scene");
        scene->setSource(path);

        QObject::connect(scene, &Qt3DRender::QSceneLoader::statusChanged, [scene, this](Qt3DRender::QSceneLoader::Status s) {
            if (s != Qt3DRender::QSceneLoader::Status::Ready)
                return;

            auto names = scene->entityNames();
            for (auto&& name : names) {
                auto* entity = scene->entity(name);

                auto* e = scene->entity(name);
                auto m = e->componentsOfType<Qt3DRender::QMaterial>();
                if (m.empty())
                    continue;

                int underscore = name.lastIndexOf(u"_"_qs);
                if (underscore == -1)
                    continue;

                QString materialName = name.mid(underscore + 1);

                if (auto it = m_materials.find(materialName); it != m_materials.end()) {
                    e->removeComponent(m[0]);
                    e->addComponent(it->second);
                    continue;
                }
            }
        });

        delete m_userEntity;
        m_userEntity = new Qt3DCore::QEntity{ m_sceneEntity };
        m_userEntity->addComponent(scene);

        // Create entities for the stereo image
        auto* stereoImageMaterial = new StereoImageMaterial(QUrl::fromLocalFile(":/13_3840x2160_sbs.jpg"));

        // Left image entity (displays the left half of the image)
        auto* leftImageMesh = new StereoImageMesh(StereoImageMesh::Side::Left);
        auto* leftImageEntity = new Qt3DCore::QEntity{ m_rootEntity.get() };
        leftImageEntity->setObjectName("Left Image");
        leftImageEntity->addComponent(leftImageMesh);
        leftImageEntity->addComponent(stereoImageMaterial);
        leftImageEntity->addComponent(m_renderer->leftLayer());
        leftImageEntity->addComponent(m_renderer->stereoImageLayer());

        // Right image entity (displays the right half of the image)
        auto* rightImageMesh = new StereoImageMesh(StereoImageMesh::Side::Right);
        auto* rightImageEntity = new Qt3DCore::QEntity{ m_rootEntity.get() };
        rightImageEntity->setObjectName("Right Image");
        rightImageEntity->addComponent(rightImageMesh);
        rightImageEntity->addComponent(stereoImageMaterial);
        rightImageEntity->addComponent(m_renderer->rightLayer());
        rightImageEntity->addComponent(m_renderer->stereoImageLayer());

        auto updateImageMeshes = [this, stereoImageMaterial, leftImageMesh, rightImageMesh] {
            const QVector2D viewportSize{ static_cast<float>(m_view.width()), static_cast<float>(m_view.height()) };
            const QVector2D imageSize = stereoImageMaterial->textureSize();

            leftImageMesh->setViewportSize(viewportSize);
            leftImageMesh->setImageSize(imageSize);

            rightImageMesh->setViewportSize(viewportSize);
            rightImageMesh->setImageSize(imageSize);
        };
        QObject::connect(&m_view, &QWindow::widthChanged, updateImageMeshes);
        QObject::connect(&m_view, &QWindow::heightChanged, updateImageMeshes);
        QObject::connect(stereoImageMaterial, &StereoImageMaterial::textureSizeChanged, updateImageMeshes);
        updateImageMeshes();

#define MMat(name) m_materials[u## #name##_qs] = new all::GlossyMaterial(all::name##ST, all::name##SU, m_rootEntity.get())
        MMat(CarPaint);
        MMat(DarkGlass);
        MMat(DarkGloss);
        MMat(Dark);
        MMat(Chrome);
        MMat(Plate);
        MMat(Tire);
        MMat(ShadowPlane);
#undef MMat
        m_materials["Skybox"] = new all::SkyboxMaterial(all::SkyboxST, {}, m_rootEntity.get());
        // CreateMaterial("Dummy", all::fresnel_vs, all::fresnel_ps, all::DarkGlossSU, all::DarkGlossST);
    }

private:
    Qt3DExtras::Qt3DWindow m_view;
    std::unique_ptr<Qt3DCore::QEntity> m_rootEntity;
    Qt3DCore::QEntity* m_sceneEntity = nullptr;
    Qt3DCore::QEntity* m_userEntity = nullptr;

    std::unordered_map<QString, Qt3DRender::QMaterial*> m_materials;
    QStereoForwardRenderer* m_renderer;
    QStereoProxyCamera* m_camera;

    std::unique_ptr<QWidget> m_widget;

    CursorEntity *m_cursor;
};

#endif
