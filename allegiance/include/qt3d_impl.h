#pragma once
#if !ALLEGIANCE_SERENITY
#include "qt3d_shaders.h"
#include "camera_control.h"
#include "stereo_camera.h"
#include "qt3d_materials.h"
#include "qt3d_cursor.h"
#include "stereo_image_mesh.h"
#include "stereo_image_material.h"
#include "qt3d/mesh_loader.h"
#include "spacemouse.h"
#include <QFileDialog>
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DRender/QMesh>
#include <Qt3DRender/QSceneLoader>
#include <Qt3DExtras/QPhongMaterial>
#include <util_qt.h>

inline void traverseEntities(Qt3DCore::QEntity *entity, QVector3D &minExtents, QVector3D &maxExtents)
{
    if (!entity)
        return;
    Qt3DRender::QMesh* mesh = entity->findChild<Qt3DRender::QMesh*>();
    Qt3DCore::QTransform* transform = entity->findChild<Qt3DCore::QTransform*>();

    if (mesh && transform) {
        // Iterate over attributes to find the position attribute
        const QList<Qt3DCore::QAttribute*> attributes = mesh->geometry()->attributes();
        for (Qt3DCore::QAttribute* attribute : attributes) {
            if (attribute->name() == Qt3DCore::QAttribute::defaultPositionAttributeName()) {
                const QByteArray& dataArray = attribute->buffer()->data();
                const float* data = reinterpret_cast<const float*>(dataArray.constData());

                // Transform the bounding volume by the entity's transform
                for (int i = 0; i < attribute->count(); ++i) {
                    QVector4D vertex(data[i * 3], data[i * 3 + 1], data[i * 3 + 2], 1.0);
                    QVector4D transformedVertex = transform->matrix() * vertex;

                    minExtents.setX(qMin(minExtents.x(), transformedVertex.x()));
                    minExtents.setY(qMin(minExtents.y(), transformedVertex.y()));
                    minExtents.setZ(qMin(minExtents.z(), transformedVertex.z()));

                    maxExtents.setX(qMax(maxExtents.x(), transformedVertex.x()));
                    maxExtents.setY(qMax(maxExtents.y(), transformedVertex.y()));
                    maxExtents.setZ(qMax(maxExtents.z(), transformedVertex.z()));
                }
            }
        }
    }

    QObjectList children = entity->children();
    for (auto c : children) {
        const auto childEntity = dynamic_cast<Qt3DCore::QEntity*>(c);
        traverseEntities(childEntity, minExtents, maxExtents);
    }
}

class FrameAction : public Qt3DLogic::QFrameAction
{
    Q_OBJECT
public:
    explicit FrameAction(Qt3DCore::QNode* parent = nullptr)
        : Qt3DLogic::QFrameAction(parent)
    {
        connect(this, &Qt3DLogic::QFrameAction::triggered, this, &FrameAction::countAndTrigger);
    }

public Q_SLOTS:
    void countAndTrigger(float dt)
    {
        if (! callbacks.size()) return;

        auto it = callbacks.begin();
        while (it != callbacks.end()) {
            it->first--;

            if (it->first < 1) {
                it->second();
                it = callbacks.erase(it); // erase returns the iterator to the next valid position
            } else {
                ++it; // move to the next element
            }
        }

    }
public:
    QVector<QPair<int, std::function<void()>>> callbacks;
};

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
            auto renderTargetSelector = makeRenderTargetSelector(parent, attachment);

            auto renderStateSet = new Qt3DRender::QRenderStateSet(renderTargetSelector);

            auto cullFace = new QCullFace(renderStateSet);
            cullFace->setMode(QCullFace::CullingMode::NoCulling);
            renderStateSet->addRenderState(cullFace);

            auto depthTest = new QDepthTest(renderStateSet);
            depthTest->setDepthFunction(QDepthTest::Less);
            renderStateSet->addRenderState(depthTest);

            auto clearBuffers = new Qt3DRender::QClearBuffers(renderStateSet);
            clearBuffers->setBuffers(Qt3DRender::QClearBuffers::ColorDepthBuffer);
            clearBuffers->setClearColor(QColor{ "#48536A" });
            //            cb->setClearColor(attachment == Qt3DRender::QRenderTargetOutput::AttachmentPoint::Left ? QColor(Qt::blue) : QColor(Qt::red));

            auto sortPolicy = new QSortPolicy{ clearBuffers };
            sortPolicy->setSortTypes(QList<QSortPolicy::SortType>{ QSortPolicy::BackToFront });

            return new Qt3DRender::QCameraSelector(sortPolicy);
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
        format.setSamples(8);
        QSurfaceFormat::setDefaultFormat(format);
    }

public:

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

        m_stereoCamera = camera;
        ResetCamera();


        m_camera->SetMatrices(toMatrix(camera->GetViewLeft()), toMatrix(camera->GetViewRight()));
        m_camera->SetProjection(toMatrix(camera->GetProjection()), camera->ShearCoefficient());

        QObject::connect(camera, &all::StereoCamera::OnViewChanged, [this, camera]() {
            m_camera->SetMatrices(toMatrix(camera->GetViewLeft()), toMatrix(camera->GetViewRight()));
            m_camera->SetProjection(toMatrix(camera->GetProjection()), camera->ShearCoefficient());
        });
        QObject::connect(camera, &all::StereoCamera::OnProjectionChanged, [this, camera]() {
            m_camera->SetProjection(toMatrix(camera->GetProjection()), camera->ShearCoefficient());
        });

        m_raycaster = new Qt3DRender::QRayCaster{m_rootEntity.get()};
        m_raycaster->setRunMode(Qt3DRender::QAbstractRayCaster::SingleShot);
        m_raycaster->setDirection(QVector3D(0.0f, 0.0f, 1.0f)); // Set your own direction
        m_raycaster->setOrigin(QVector3D(0.0f, 0.01f, 0.0f));     // Set your own position
        connect(m_raycaster, &QRayCaster::hitsChanged,
            [](const Qt3DRender::QRayCaster::Hits& hits) {
                if (hits.empty()) return ;
            });

        m_raycaster->setEnabled(true);
        m_rootEntity->addComponent(m_raycaster);

        auto ent = m_rootEntity.get();
        all::Controller::getInstance().hitTest = [this](glm::vec3 pos, glm::vec3 dir, double aperture) {

            this->m_raycaster->trigger(toQVector3D(pos), toQVector3D(dir), 1000);

            auto hits = this->m_raycaster->pick(toQVector3D(pos), toQVector3D(dir), 100);

            if (hits.isEmpty()) {
                return glm::vec3{-1};
            }

            // Use std::min_element along with a lambda function to find the minimum distance
            auto nearestHitIterator = std::min_element(hits.begin(), hits.end(),
                [](const QRayCasterHit& a, const QRayCasterHit& b) {
                    return a.distance() < b.distance();
                });

            return toGlmVec3(nearestHitIterator->worldIntersection());
        };
        CreateScene(m_rootEntity.get());
    }

    void CreateScene(Qt3DCore::QEntity* root)
    {
        m_sceneEntity = new Qt3DCore::QEntity{m_rootEntity.get()};
        m_sceneEntity->setObjectName("SceneEntity");
        m_userEntity = new Qt3DCore::QEntity{m_sceneEntity};
        m_userEntity->setObjectName("UserEntity");

        m_cursor = new CursorEntity(m_rootEntity.get(), m_camera->GetLeftCamera(), &m_view);
        new Picker(m_sceneEntity, m_cursor);
        LoadModel();

        m_view.setRootEntity(m_rootEntity.get());

        // look at this dirty workaround
        //  adding a light every three frames, to make sure we don't crash
        auto mFrameAction = new FrameAction{ m_rootEntity.get() };
        std::vector<QPair<QVector3D, QVector3D>> lightPositions{
            { { 0, -1, 0 }, { 0, 1, 0 } },
            { { 1, 0, 0 }, { -1, 0, 0 } },
            { { 0, 0, -1 }, { 0, 0, 1 } }
        };
        int actionFrame{ 3 };
        for (auto positionPair : lightPositions) {
            mFrameAction->callbacks.append({ actionFrame, [this, positionPair]() {
                                                AddDirectionalLight(m_rootEntity.get(), positionPair.first);
                                                AddDirectionalLight(m_rootEntity.get(), positionPair.second);
                                            } });
            actionFrame += 3;
        }

        LoadImage();
    }

    void ShowImage()
    {
        m_renderer->setMode(QStereoForwardRenderer::Mode::StereoImage);
    }

    void ShowModel()
    {
        m_renderer->setMode(QStereoForwardRenderer::Mode::Scene);
        LoadModel();
    }

    void SetCursorEnabled(bool /* enabled */)
    {
        // TODO
    }

    void ResetCamera()
    {
        m_stereoCamera->SetPosition({0.2, 5, -10});
        m_stereoCamera->SetForwardVector({0, -.5, 1});
    }

    static void AddDirectionalLight(Qt3DCore::QNode *node, QVector3D position) {
        auto le = new Qt3DCore::QEntity{ node };
        auto l = new Qt3DRender::QDirectionalLight{ le };
        l->setIntensity(0.3);
        auto lt = new Qt3DCore::QTransform{ le };
        lt->setTranslation(position);
        l->setWorldDirection(QVector3D{} - lt->translation());
        le->addComponent(lt);
        le->addComponent(l);
    }

    void LoadImage(const QUrl& path = QUrl::fromLocalFile(":/13_3840x2160_sbs.jpg"))
    {
        QImageReader::setAllocationLimit(0);
        // Create entities for the stereo image
        auto* stereoImageMaterial = new StereoImageMaterial(path);

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
    }

    void LoadModel(QUrl path = QUrl::fromLocalFile("scene/fbx/showroom2303.fbx"))
    {
        ResetCamera();
        delete m_skyBox;
        m_skyBox = nullptr;
        delete m_userEntity;
        m_userEntity = new Qt3DCore::QEntity{ m_sceneEntity };
        m_userEntity->setObjectName("UserEntity");

        QString filePath = path.toLocalFile();

        bool isFbx = filePath.endsWith(".fbx");

        auto* sceneRoot = all::MeshLoader::load(path.toLocalFile());
        sceneRoot->setParent(m_userEntity);

        if (isFbx) {
            constexpr float scale = 0.007;
            auto l = m_userEntity->componentsOfType<Qt3DCore::QTransform>();
            if (l.size() > 0) {
                l[0]->setScale(scale);
            } else {
                auto t = new Qt3DCore::QTransform;
                t->setScale(scale);
                m_userEntity->addComponent(t);
            }
        }

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

        const auto sceneEntities = sceneRoot->findChildren<Qt3DCore::QEntity*>();
        for (auto* e : sceneEntities) {
            Qt3DRender::QMaterial* material = [e] {
                auto components = e->componentsOfType<Qt3DRender::QMaterial>();
                return !components.isEmpty() ? components.first() : nullptr;
            }();
            if (!material)
                continue;
            const auto materialName = material->property("name").toString();

            if (materialName.contains("skybox", Qt::CaseInsensitive)) {
                e->setParent(m_sceneEntity);
                m_skyBox = e;
            }

            if (auto it = m_materials.find(materialName); it != m_materials.end()) {
                e->removeComponent(material);
                e->addComponent(it->second);
            }
        }

        auto ext = calculateSceneExtent(m_userEntity);
        auto& e = all::Controller::getInstance().modelExtent;
        e = { ext.min.x(), ext.min.y(), ext.min.z(), ext.max.x(), ext.max.y(), ext.max.z() };

        // scale model
        auto k = ext.max - ext.min;
        auto s = QVector3D{ 8, 8, 8 } / k;
        auto centerpoint = ext.min + k / 2;

        auto modelViewProjection = m_stereoCamera->GetViewCenter() * m_stereoCamera->GetProjection();
        auto size = ext.max - ext.min;
        glm::vec4 dimensions(size.x(), size.y(), size.z(), 1.0f);
        glm::vec4 dimensionsClip = m_stereoCamera->GetProjection() * m_stereoCamera->GetViewCenter() * dimensions;
        dimensionsClip /= dimensionsClip.w;
        float scaleFactor = 1 / std::max(std::abs(dimensionsClip.x), std::abs(dimensionsClip.y));

        auto ts = m_userEntity->componentsOfType<Qt3DCore::QTransform>();
        if (ts.size() > 0) {
            ts[0]->setScale(ts[0]->scale() * scaleFactor);
        } else {
            auto t = new Qt3DCore::QTransform;
            t->setScale(scaleFactor);
            m_userEntity->addComponent(t);
        }

        // set rotation point
        auto cam = dynamic_cast<all::OrbitalStereoCamera*>(m_stereoCamera);
        if (cam) {
            cam->SetTarget(toGlmVec3((ext.min + k / 2) * scaleFactor));
            cam->Rotate(0, 0);
        }
    }

     QVector2D calculateSceneDimensions(Qt3DCore::QEntity *scene) const{

        QVector3D minBounds, maxBounds;
        _calculateSceneDimensions(scene, minBounds, maxBounds);
        QVector2D res{qMin(qMin(minBounds.x(), minBounds.y()), minBounds.z()),
            qMax(qMax(maxBounds.x(), maxBounds.y()), maxBounds.z())};
        return res;
    }

    struct SceneExtent
    {
        QVector3D min, max;
    };
    SceneExtent calculateSceneExtent(Qt3DCore::QNode *node) {
        SceneExtent e;
        _calculateSceneDimensions(node, e.min, e.max);
        return e;
    }

    QWindow* GetWindow() { return &m_view; }
protected:

    void _calculateSceneDimensions(Qt3DCore::QNode* node, QVector3D& minBounds, QVector3D& maxBounds) const {
        if (node) {
            Qt3DCore::QEntity* entity = qobject_cast<Qt3DCore::QEntity*>(node);

            for (Qt3DCore::QNode* childNode : node->childNodes()) {
                _calculateSceneDimensions(childNode, minBounds, maxBounds);
            }
            if (!entity)
                return;


            // Check if the entity has a geometry renderer
            auto geometryRenderers = entity->componentsOfType<Qt3DRender::QGeometryRenderer>();
            if (geometryRenderers.size() > 0) {
                Qt3DRender::QGeometryRenderer* renderer = geometryRenderers.first();

                Qt3DCore::QGeometry* geometry = renderer->geometry();
                if (!geometry) {
                    for (auto c : renderer->children()) {
                        auto a = qobject_cast<Qt3DCore::QGeometryView*>(c);
                        if (a) {
                            geometry = a->geometry();
                            if (geometry)
                                break;
                        }

                    }

                    if (!geometry)
                    return;
                }

                auto l = m_userEntity->componentsOfType<Qt3DCore::QTransform>();
                double scale = 1;
                if (l.size() > 0) {
                    scale = l[0]->scale();
                }
                // You need to specify the correct attribute index for position data
                const int positionAttributeIndex = 0;  // Adjust this index based on your format
                const Qt3DCore::QAttribute* positionAttribute = geometry->attributes().at(positionAttributeIndex);
                size_t offset=-1, byteStride;
                for (auto a : geometry->attributes()) {
                    if (a->name() == "vertexPosition") {
                        offset = a->byteOffset();
                        byteStride = a->byteStride();
                    }
                }
                if (offset != -1) {
                    if (positionAttribute) {
                        const QByteArray& rawData = positionAttribute->buffer()->data();
                        int dataCount;
                        if (byteStride)
                            dataCount = rawData.size() / byteStride;
                        else
                            dataCount = rawData.size() / sizeof(QVector3D);

                        for (int i = 0; i < dataCount; i++) {
                            const QVector3D* position = reinterpret_cast<const QVector3D*>(rawData.data() + byteStride * i + offset);

                            minBounds.setX(qMin(minBounds.x(), position->x()));
                            minBounds.setY(qMin(minBounds.y(), position->y()));
                            minBounds.setZ(qMin(minBounds.z(), position->z()));
                            maxBounds.setX(qMax(maxBounds.x(), position->x()));
                            maxBounds.setY(qMax(maxBounds.y(), position->y()));
                            maxBounds.setZ(qMax(maxBounds.z(), position->z()));
                        }
                        minBounds.setX(minBounds.x() * scale);
                        minBounds.setY(minBounds.y() * scale);
                        minBounds.setZ(minBounds.z() * scale);
                        maxBounds.setX(maxBounds.x() * scale);
                        maxBounds.setY(maxBounds.y() * scale);
                        maxBounds.setZ(maxBounds.z() * scale);
                    }
                }
            }
        }
    }
private:
    Qt3DExtras::Qt3DWindow m_view;
    std::unique_ptr<Qt3DCore::QEntity> m_rootEntity;
    Qt3DCore::QEntity* m_sceneEntity = nullptr;
    Qt3DCore::QEntity* m_userEntity = nullptr;
    Qt3DCore::QEntity* m_skyBox = nullptr;
    Qt3DRender::QRayCaster* m_raycaster;

    std::unordered_map<QString, Qt3DRender::QMaterial*> m_materials;
    QStereoForwardRenderer* m_renderer;
    QStereoProxyCamera* m_camera;
    all::StereoCamera* m_stereoCamera;

    std::unique_ptr<QWidget> m_widget;

    CursorEntity *m_cursor;
};

#endif
