#pragma once
#if !ALLEGIANCE_SERENITY

#include "camera_control.h"
#include "stereo_camera.h"
#include <QFileDialog>

class QStereoCamera : public Qt3DCore::QEntity
{
    Q_OBJECT
    Q_PROPERTY(float convergencePlaneDistance READ convergencePlaneDistance WRITE setConvergencePlaneDistance NOTIFY convergencePlaneDistanceChanged FINAL)
    Q_PROPERTY(float interocularDistance READ interocularDistance WRITE setInterocularDistance NOTIFY interocularDistanceChanged FINAL)
    Q_PROPERTY(bool convergeOnNear READ convergeOnNear WRITE setConvergeOnNear NOTIFY convergeOnNearChanged FINAL)
    Q_PROPERTY(QVector3D position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(QVector3D upVector READ upVector WRITE setUpVector NOTIFY upVectorChanged)
    Q_PROPERTY(QVector3D viewCenter READ viewCenter WRITE setViewCenter NOTIFY viewCenterChanged)
    Q_PROPERTY(QVector3D viewVector READ viewVector NOTIFY viewVectorChanged)
    Q_PROPERTY(Qt3DCore::QEntity* leftCamera READ leftCamera CONSTANT FINAL)
    Q_PROPERTY(Qt3DCore::QEntity* rightCamera READ rightCamera CONSTANT FINAL)
public:
    QStereoCamera(Qt3DCore::QNode* parent = nullptr)
        : Qt3DCore::QEntity(parent)
        , m_leftCamera(new Qt3DRender::QCamera(this))
        , m_rightCamera(new Qt3DRender::QCamera(this))
        , m_convergencePlaneDistance(.5f)
    {
    }
    float convergencePlaneDistance() const
    {
        return m_convergencePlaneDistance;
    }
    void setConvergencePlaneDistance(float newConvergencePlaneDistance)
    {
        if (qFuzzyCompare(m_convergencePlaneDistance, newConvergencePlaneDistance))
            return;
        m_convergencePlaneDistance = newConvergencePlaneDistance;
        emit convergencePlaneDistanceChanged(m_convergencePlaneDistance);
        updateViewMatrix();
    }

    float interocularDistance() const
    {
        return m_interocularDistance;
    }
    void setInterocularDistance(float newInterocularDistance)
    {
        if (qFuzzyCompare(m_interocularDistance, newInterocularDistance))
            return;
        m_interocularDistance = newInterocularDistance;
        emit interocularDistanceChanged(m_interocularDistance);
        updateViewMatrix();
    }

    bool convergeOnNear() const
    {
        return m_convergeOnNear;
    }
    void setConvergeOnNear(bool newConvergeOnNear)
    {
        if (m_convergeOnNear == newConvergeOnNear)
            return;
        m_convergeOnNear = newConvergeOnNear;
        emit convergeOnNearChanged(m_convergeOnNear);
        updateViewMatrix();
    }

    Qt3DCore::QEntity *leftCamera() const
    {
        return m_leftCamera;
    }
    Qt3DCore::QEntity *rightCamera() const
    {
        return m_rightCamera;
    }

    QVector3D position() const
    {
        return m_position;
    }
    void setPosition(const QVector3D &newPosition)
    {
        if (m_position == newPosition)
            return;
        m_position = newPosition;
        emit positionChanged(m_position);
        updateViewMatrix();
    }

    QVector3D upVector() const
    {
        return m_upVector;
    }
    void setUpVector(const QVector3D &newUpVector)
    {
        if (m_upVector == newUpVector)
            return;
        m_upVector = newUpVector;
        emit upVectorChanged(m_upVector);
        updateViewMatrix();
    }

    QVector3D viewCenter() const
    {
        return m_viewCenter;
    }
    void setViewCenter(const QVector3D &newViewCenter)
    {
        if (m_viewCenter == newViewCenter)
            return;
        m_viewCenter = newViewCenter;
        emit viewCenterChanged(m_viewCenter);
        updateViewMatrix();
    }

    QVector3D viewVector() const
    {
        return m_viewVector;
    }

    void setPerspectiveProjection(float fieldOfView, float aspect,
                                  float nearPlane, float farPlane)
    {
        m_leftCamera->lens()->setPerspectiveProjection(fieldOfView, aspect, nearPlane, farPlane);
        m_rightCamera->lens()->setPerspectiveProjection(fieldOfView, aspect, nearPlane, farPlane);
    }

    void setAspectRatio(float aspectRatio)
    {
        m_leftCamera->setAspectRatio(aspectRatio);
        m_rightCamera->setAspectRatio(aspectRatio);
    }

signals:
    void convergencePlaneDistanceChanged(float convergencePlaneDistance);
    void interocularDistanceChanged(float interocularDistance);
    void convergeOnNearChanged(bool convergeOnNear);
    void positionChanged(const QVector3D& position);
    void upVectorChanged(const QVector3D& upVector);
    void viewCenterChanged(const QVector3D& viewCenter);
    void viewVectorChanged(const QVector3D& viewVector);

private:
    void updateViewMatrix()
    {
        m_viewVector = m_viewCenter - m_position;

        auto stereoShear = [](float x) {
            QMatrix4x4 m;
            m(2, 0) = x;
            return m;
        };
        auto shearCoefficient = [](float focus_dist, float eye_dist, int conv) {
            return conv * eye_dist / focus_dist;
        };

        const QVector3D right = QVector3D::crossProduct(m_viewVector, m_upVector) * m_interocularDistance * .5f;
        {
            m_leftCamera->setPosition(m_position + right);
            m_leftCamera->setViewCenter(m_viewCenter + right);
            m_leftCamera->setUpVector(m_upVector);
            m_rightCamera->setPosition(m_position - right);
            m_rightCamera->setViewCenter(m_viewCenter - right);
            m_rightCamera->setUpVector(m_upVector);
        }
//        QMatrix4x4 m;
//        m.lookAt(m_position + right, m_position + right + m_viewVector, m_upVector);
//        m_leftTransform->setMatrix(stereoShear(shearCoefficient(m_convergencePlaneDistance, -m_interocularDistance * .5f, m_convergeOnNear)) * m);
//        m.lookAt(m_position - right, m_position - right + m_viewVector, m_upVector);
//        m_rightTransform->setMatrix(stereoShear(shearCoefficient(m_convergencePlaneDistance,  m_interocularDistance * .5f, m_convergeOnNear)) * m);
    }

    float m_convergencePlaneDistance = .5f;
    float m_interocularDistance = 0.f;
    bool m_convergeOnNear = true;
    Qt3DRender::QCamera *m_leftCamera;
    Qt3DRender::QCamera *m_rightCamera;
    QVector3D m_position;
    QVector3D m_upVector{0.f, 1.f, 0.f};
    QVector3D m_viewCenter;
    QVector3D m_viewVector;
};

class QStereoForwardRenderer : public Qt3DRender::QRenderSurfaceSelector
{
    Q_OBJECT
    Q_PROPERTY(QStereoCamera* camera READ camera WRITE setCamera NOTIFY cameraChanged FINAL)
public:
    QStereoForwardRenderer(Qt3DCore::QNode *parent = nullptr)
        : Qt3DRender::QRenderSurfaceSelector(parent)
        , m_camera(nullptr)
    {
        auto vp = new Qt3DRender::QViewport(this);
        auto ssel = new Qt3DRender::QRenderSurfaceSelector(vp);

        auto makeBranch = [](Qt3DRender::QFrameGraphNode* parent, Qt3DRender::QRenderTargetOutput::AttachmentPoint attachment){
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

    QStereoCamera *camera() const
    {
        return m_camera;
    }
    void setCamera(QStereoCamera *newCamera)
    {
        if (m_camera == newCamera)
            return;
        m_camera = newCamera;
        emit cameraChanged();

        if (m_camera) {
            m_leftCamera->setCamera(m_camera->leftCamera());
            m_rightCamera->setCamera(m_camera->rightCamera());
        }
    }

signals:
    void cameraChanged();

private:
    Qt3DRender::QCameraSelector *m_leftCamera;
    Qt3DRender::QCameraSelector *m_rightCamera;
    QStereoCamera *m_camera;
};

class ObjectManipulatorController : Qt3DCore::QEntity
{
    Q_OBJECT
    Q_PROPERTY(Qt3DCore::QTransform* targetTransform READ targetTransform WRITE setTargetTransform NOTIFY targetTransformChanged FINAL)
public:
    ObjectManipulatorController(Qt3DCore::QNode* parent)
        : Qt3DCore::QEntity(parent)
    {
        using namespace Qt3DCore;
        using namespace Qt3DInput;

        auto mouseDevice = new QMouseDevice(this);
        mouseDevice->setSensitivity(0.0005f);

        auto entity = new QEntity(this);
        auto logicalDevice = new QLogicalDevice;
        auto addAxis = [logicalDevice, mouseDevice](int axis){
            auto axisInput = new QAnalogAxisInput;
            axisInput->setSourceDevice(mouseDevice);
            axisInput->setAxis(axis);
            auto qaxis = new QAxis;
            qaxis->addInput(axisInput);
            logicalDevice->addAxis(qaxis);
            return qaxis;
        };
        m_rotationX = addAxis(QMouseDevice::Axis::X);
        m_rotationY = addAxis(QMouseDevice::Axis::Y);
        entity->addComponent(logicalDevice);

        connect(m_rotationX, &QAxis::valueChanged, this, [this](float v) {
            updateTransform(v, 0.f);
        });
        connect(m_rotationY, &QAxis::valueChanged, this, [this](float v) {
            updateTransform(0.f, -v);
        });
    }

    Qt3DCore::QTransform *targetTransform() const
    {
        return m_targetTransform;
    }
    void setTargetTransform(Qt3DCore::QTransform *newTargetTransform)
    {
        if (m_targetTransform == newTargetTransform)
            return;
        m_targetTransform = newTargetTransform;
        emit targetTransformChanged();
    }

signals:
    void targetTransformChanged();

private:
    void updateTransform(float x, float y)
    {
        if (nullptr == m_targetTransform)
            return;

        x *= 360.f;
        y *= 360.f;
        auto q = m_targetTransform->fromEulerAngles(y, x, 0);
        auto r = m_targetTransform->rotation();
        auto f = q * r;
        m_targetTransform->setRotation(f);
    }

    Qt3DCore::QTransform *m_targetTransform = nullptr;
    Qt3DInput::QAxis *m_rotationX = nullptr;
    Qt3DInput::QAxis *m_rotationY = nullptr;
};

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
        using namespace Qt3DCore;
        using namespace Qt3DRender;
        using namespace Qt3DExtras;

        m_transform = new Qt3DCore::QTransform{ m_rootEntity.get() };
        m_transform->setTranslation(QVector3D{ 0, -10, -50 });

        m_controller = new ObjectManipulatorController(m_rootEntity.get());
        m_controller->setTargetTransform(m_transform);
	}

    void CreateAspects(all::CameraControl* cc, all::OrbitalStereoCamera* camera)
	{
        using namespace Qt3DCore;
        using namespace Qt3DRender;
        using namespace Qt3DExtras;

        m_rootEntity = std::make_unique<Qt3DCore::QEntity>();

        m_renderer = new QStereoForwardRenderer();
        m_view.setActiveFrameGraph(m_renderer);

        m_camera = new QStereoCamera(m_rootEntity.get());
        m_renderer->setCamera(m_camera);

        m_camera->setPosition({.8f, 1.5f, 5.f});
        m_camera->setViewCenter({0.f, 0.f, 0.f});
        m_camera->setUpVector({0.f, 1.f, 0.f});

        m_camera->setPerspectiveProjection(45.0f, float(m_view.width()) / m_view.height(), 0.01f, 1000.0f);
        m_camera->setAspectRatio(float(m_view.width()) / m_view.height());

        QObject::connect(&m_view, &QWindow::widthChanged, [this](int width) {
            m_camera->setAspectRatio(float(m_view.width()) / m_view.height());
        });
        QObject::connect(&m_view, &QWindow::heightChanged, [this](int height) {
            m_camera->setAspectRatio(float(m_view.width()) / m_view.height());
        });

        CreateScene(m_rootEntity.get());

        QObject::connect(cc, &all::CameraControl::OnFocusPlaneChanged, [this](float v) {
            m_camera->setConvergencePlaneDistance(v);
        });
        QObject::connect(cc, &all::CameraControl::OnEyeDisparityChanged, [this](float v) {
            m_camera->setInterocularDistance(v);
        });
        QObject::connect(cc, &all::CameraControl::OnLoadModel,
                         [this]() { LoadModel(); });

        m_view.setRootEntity(m_rootEntity.get());
    }

    QWindow* GetWindow() { return &m_view; }

    void LoadModel()
    {
        QFileDialog fd;
        fd.setFileMode(QFileDialog::ExistingFile);
        fd.setNameFilter("*.obj");
        fd.setViewMode(QFileDialog::Detail);
        if (fd.exec()) {
            auto path = fd.selectedFiles()[0];
            auto folder = fd.directory().path();

            auto mesh = new Qt3DRender::QMesh();
            mesh->setObjectName("Model Mesh");
            mesh->setSource(QUrl::fromLocalFile(path));

            if (m_userEntity) {
                m_userEntity->removeComponent(m_transform);
                delete m_userEntity;
            }
            m_userEntity = new Qt3DCore::QEntity{ m_rootEntity.get() };
            m_userEntity->addComponent(mesh);
            m_userEntity->addComponent(m_transform);

            // reset transform


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

            if (false && !texturePath.isEmpty()) {
                // crashes on cottage
                qDebug() << "setting texutre: " << texturePath;
                auto m_texture = std::make_unique<Qt3DRender::QTextureImage>();
                m_texture->setObjectName("Model Texture");
                if (!texturePath.isEmpty())
                     m_texture->setSource(QUrl::fromLocalFile(texturePath));

                auto* material = new Qt3DExtras::QTextureMaterial(m_rootEntity.get());

                // Create a texture and set its image
                auto* texture2D = new Qt3DRender::QTexture2D(material);
                texture2D->addTextureImage(m_texture.get());

                // Assign the texture to the material
                material->setTexture(texture2D);

                // Apply the material to the mesh entity
                m_userEntity->addComponent(material);
            } else {
                auto m = new Qt3DExtras::QDiffuseSpecularMaterial;
                m->setShininess(30.f);
                m->setDiffuse(QColor{ 0, 255, 200 });
                m->setAmbient(QColor{ 255, 0, 255 });
                m->setSpecular(QColor{ 0, 255, 200 });
                m_userEntity->addComponent(m);
            }
        }
    }

private:
    Qt3DExtras::Qt3DWindow m_view;
    std::unique_ptr<Qt3DCore::QEntity> m_rootEntity;
    Qt3DCore::QEntity* m_userEntity{ nullptr };
    ObjectManipulatorController* m_controller;
    Qt3DCore::QTransform* m_transform;
    QStereoForwardRenderer* m_renderer;
    QStereoCamera* m_camera;
};

#endif

