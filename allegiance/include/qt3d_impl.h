#pragma once
#if !ALLEGIANCE_SERENITY
#include "qt3d_shaders.h"
#include "camera_control.h"
#include "stereo_camera.h"
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

public:
    void SetMatrices(const QMatrix4x4& left, const QMatrix4x4& right)
    {
        m_leftTransform->setMatrix(left.inverted());
        m_rightTransform->setMatrix(right.inverted());
    }
    void SetProjection(const QMatrix4x4& proj)
    {
        m_leftCameraLens->setProjectionMatrix(proj);
        m_rightCameraLens->setProjectionMatrix(proj);
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
    QStereoForwardRenderer(Qt3DCore::QNode* parent = nullptr)
        : Qt3DRender::QRenderSurfaceSelector(parent)
        , m_camera(nullptr)
    {
        auto vp = new Qt3DRender::QViewport(this);
        auto ssel = new Qt3DRender::QRenderSurfaceSelector(vp);

        auto makeBranch = [](Qt3DRender::QFrameGraphNode* parent, Qt3DRender::QRenderTargetOutput::AttachmentPoint attachment) {
            auto rts = new Qt3DRender::QRenderTargetSelector(parent);
            auto rt = new Qt3DRender::QRenderTarget();
            auto rto = new Qt3DRender::QRenderTargetOutput;
            rto->setAttachmentPoint(attachment);
            rt->addOutput(rto);
            rts->setTarget(rt);

            auto cb = new Qt3DRender::QClearBuffers(rts);
            cb->setBuffers(Qt3DRender::QClearBuffers::ColorDepthBuffer);
            //            cb->setClearColor(attachment == Qt3DRender::QRenderTargetOutput::AttachmentPoint::Left ? QColor(Qt::blue) : QColor(Qt::red));
            return new Qt3DRender::QCameraSelector(cb);
        };

        m_leftCamera = makeBranch(ssel, Qt3DRender::QRenderTargetOutput::Left);
        m_rightCamera = makeBranch(ssel, Qt3DRender::QRenderTargetOutput::Right);

#ifdef QT_DEBUG
        (void)new Qt3DRender::QDebugOverlay(m_rightCamera);
#endif
    }

    QStereoProxyCamera* camera() const
    {
        return m_camera;
    }
    void setCamera(QStereoProxyCamera* newCamera)
    {
        if (m_camera == newCamera)
            return;
        m_camera = newCamera;
        emit cameraChanged();

        if (m_camera) {
            m_leftCamera->setCamera(m_camera->GetLeftCamera());
            m_rightCamera->setCamera(m_camera->GetRightCamera());
        }
    }

signals:
    void cameraChanged();

private:
    Qt3DRender::QCameraSelector* m_leftCamera;
    Qt3DRender::QCameraSelector* m_rightCamera;
    QStereoProxyCamera* m_camera;
};

inline QMatrix4x4 toMatrix(const glm::mat4x4& m)
{
    QMatrix4x4 q;
    std::copy(&m[0][0], &m[0][0] + 16, q.data());
    return q;
}

class Qt3DImpl
{
public:
    Qt3DImpl()
    {
        QSurfaceFormat format = QSurfaceFormat::defaultFormat();
        format.setStereo(true);
        format.setSamples(1);
        QSurfaceFormat::setDefaultFormat(format);
    }

public:
    void CreateScene(Qt3DCore::QEntity* root)
    {
    }

    void CreateAspects(all::CameraControl* cc, all::OrbitalStereoCamera* camera)
    {
        using namespace Qt3DCore;
        using namespace Qt3DRender;
        using namespace Qt3DExtras;

        m_rootEntity = std::make_unique<Qt3DCore::QEntity>();

        m_renderer = new QStereoForwardRenderer();
        m_view.setActiveFrameGraph(m_renderer);

        m_camera = new QStereoProxyCamera(m_rootEntity.get());
        m_renderer->setCamera(m_camera);

        m_camera->SetMatrices(toMatrix(camera->GetViewLeft()), toMatrix(camera->GetViewRight()));
        m_camera->SetProjection(toMatrix(camera->GetProjection()));

        QObject::connect(camera, &all::OrbitalStereoCamera::OnViewChanged, [this, camera]() {
            m_camera->SetMatrices(toMatrix(camera->GetViewLeft()), toMatrix(camera->GetViewRight()));
        });
        QObject::connect(camera, &all::OrbitalStereoCamera::OnProjectionChanged, [this, camera]() {
            m_camera->SetProjection(toMatrix(camera->GetProjection()));
        });

        CreateScene(m_rootEntity.get());

        QObject::connect(cc, &all::CameraControl::OnLoadModel,
                         [this]() { LoadModel(); });

        m_view.setRootEntity(m_rootEntity.get());
    }

    QWindow* GetWindow() { return &m_view; }

    void CreateMaterial(QString name, std::string_view vs, std::string_view ps)
    {
        auto material = new Qt3DRender::QMaterial;
        material->setObjectName(name);

        auto* shader = new Qt3DRender::QShaderProgram(material);
        shader->setVertexShaderCode(vs.data());
        shader->setFragmentShaderCode(ps.data());

        auto* rp = new Qt3DRender::QRenderPass(material);
        rp->setShaderProgram(shader);

        auto* t = new Qt3DRender::QTechnique(material);
        t->graphicsApiFilter()->setApi(Qt3DRender::QGraphicsApiFilter::OpenGL);
        t->graphicsApiFilter()->setProfile(Qt3DRender::QGraphicsApiFilter::CoreProfile);
        t->graphicsApiFilter()->setMajorVersion(3);
        t->graphicsApiFilter()->setMinorVersion(2);
        t->addRenderPass(rp);

        auto* effect = new Qt3DRender::QEffect(material);
        effect->addTechnique(t);

        material->setEffect(effect);
        m_materials[name] = material;
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

            auto* scene = new Qt3DRender::QSceneLoader(m_rootEntity.get());
            scene->setObjectName("Model Scene");
            scene->setSource(QUrl::fromLocalFile(path));

            QObject::connect(scene, &Qt3DRender::QSceneLoader::statusChanged, [scene, this](Qt3DRender::QSceneLoader::Status s) {
                if (s != Qt3DRender::QSceneLoader::Status::Ready)
                    return;

                auto names = scene->entityNames();
                for (auto&& name : names) {
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

            m_userEntity.reset(new Qt3DCore::QEntity{ m_rootEntity.get() });
            m_userEntity->addComponent(scene);

            auto e = m_rootEntity->findChildren<Qt3DCore::QEntity*>();

            CreateMaterial("CarPaint", all::simple_vs, all::simple_ps_yellow);
            CreateMaterial("DarkGlass", all::simple_vs, all::simple_ps_red);
        }
    }

private:
    Qt3DExtras::Qt3DWindow m_view;
    std::unique_ptr<Qt3DCore::QEntity> m_rootEntity;
    std::unique_ptr<Qt3DCore::QEntity> m_userEntity;

    std::unordered_map<QString, Qt3DRender::QMaterial*> m_materials;
    QStereoForwardRenderer* m_renderer;
    QStereoProxyCamera* m_camera;
};

#endif
