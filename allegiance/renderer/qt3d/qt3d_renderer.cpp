#include "qt3d_renderer.h"
#include "qt3d_cursor.h"
#include "qt3d_materials.h"
#include "qt3d_focusarea.h"
#include "mesh_loader.h"
#include "stereo_image_material.h"
#include "stereo_image_mesh.h"
#include "stereo_proxy_camera.h"
#include "frame_action.h"
#include "qt3d_shaders.h"
#include "util_qt.h"
#include "frustum.h"
#include "frustum_rect.h"

#include <Qt3DRender/QSceneLoader>
#include <Qt3DRender/QPickingSettings>
#include <Qt3DRender/QRenderSettings>
#include <Qt3DRender/QMesh>
#include <Qt3DRender/QLayer>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/QDirectionalLight>
#include <Qt3DRender/QRayCaster>
#include <Qt3DRender/QCameraLens>
#include <Qt3DRender/QCamera>
#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QDiffuseMapMaterial>
#include <Qt3DExtras/QPhongMaterial>
#include <shared/cursor.h>
#include <QFileInfo>
#include <QImageReader>
#include <shared/stereo_camera.h>
#include <QTimer>

namespace all::qt3d {
namespace {

static void traverseEntities(Qt3DCore::QEntity* entity, QVector3D& minExtents, QVector3D& maxExtents)
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

} // namespace

Qt3DRenderer::Qt3DRenderer(Qt3DExtras::Qt3DWindow* view,
                           all::StereoCamera& stereoCamera,
                           std::function<void(std::string_view, std::any)> propertyUpdateNotifier)
    : m_view(view), m_stereoCamera(&stereoCamera), m_propertyUpdateNofitier(propertyUpdateNotifier)
{
}

Qt3DRenderer::~Qt3DRenderer()
{
}

void Qt3DRenderer::viewChanged()
{
    const float flippedCorrection = m_stereoCamera->isFlipped() ? -1.0f : 1.0f;
    const float interocularDistance = flippedCorrection * m_stereoCamera->interocularDistance();

    m_camera->updateViewMatrices(toQVector3D(m_stereoCamera->position()),
                                 toQVector3D(m_stereoCamera->forwardVector()),
                                 toQVector3D(m_stereoCamera->upVector()),
                                 m_stereoCamera->convergencePlaneDistance(),
                                 interocularDistance,
                                 m_stereoCamera->mode());

    const QVector3D camPosition = toQVector3D(m_stereoCamera->position());
    const QVector3D viewVector = toQVector3D(m_stereoCamera->forwardVector());
    const QVector3D upVector = toQVector3D(m_stereoCamera->upVector());

    // Frustum
    {
        m_centerFrustum->setViewMatrix(m_camera->centerCamera()->viewMatrix());
        m_leftFrustum->setViewMatrix(m_camera->leftCamera()->viewMatrix());
        m_rightFrustum->setViewMatrix(m_camera->rightCamera()->viewMatrix());

        auto* frustumCamera = m_renderer->frustumCamera();
        // Place the camera to match the center camera but with a viewCenter placed at the center of the near and far planes
        frustumCamera->setPosition(camPosition);
        const float centerPlaneDist = m_stereoCamera->nearPlane() + (m_stereoCamera->farPlane() - m_stereoCamera->nearPlane()) * 0.5f;
        frustumCamera->setViewCenter(camPosition + viewVector * centerPlaneDist);
        frustumCamera->setUpVector(upVector);
        // Then Rotate 90 around the X-Axis with rotation origin viewCenter
        frustumCamera->tiltAboutViewCenter(90.0f);
    }

    projectionChanged();
}

void Qt3DRenderer::projectionChanged()
{
    const float flippedCorrection = m_stereoCamera->isFlipped() ? -1.0f : 1.0f;
    const float interocularDistance = flippedCorrection * m_stereoCamera->interocularDistance();

    m_camera->updateProjection(m_stereoCamera->nearPlane(),
                               m_stereoCamera->farPlane(),
                               m_stereoCamera->fov(),
                               m_stereoCamera->aspectRatio(),
                               m_stereoCamera->convergencePlaneDistance(),
                               interocularDistance,
                               m_stereoCamera->mode());

    // Frustum
    {
        m_centerFrustum->setProjectionMatrix(m_camera->centerCamera()->projectionMatrix());
        m_leftFrustum->setProjectionMatrix(m_camera->leftCamera()->projectionMatrix());
        m_rightFrustum->setProjectionMatrix(m_camera->rightCamera()->projectionMatrix());

        m_centerFrustum->setConvergence(m_stereoCamera->convergencePlaneDistance());
        m_leftFrustum->setConvergence(m_stereoCamera->convergencePlaneDistance());
        m_rightFrustum->setConvergence(m_stereoCamera->convergencePlaneDistance());

        // Adjust Ortho Projection so that whole frustum is visible
        auto* frustumCamera = m_renderer->frustumCamera();
        const float frustumLength = (m_stereoCamera->farPlane() - m_stereoCamera->nearPlane());
        const float vFov = qDegreesToRadians(m_stereoCamera->fov());
        const float hFov = 2.0f * std::atan(m_stereoCamera->aspectRatio() * tan(vFov * 0.5f));

        float frustumShiftAtFarPlane = 0.0f;
        if (m_stereoCamera->mode() == StereoCamera::Mode::AsymmetricFrustum) {
            frustumShiftAtFarPlane = 2.0f * std::fabs(interocularDistance);
        }

        const float frustumHalfWidth = frustumLength * std::tan(hFov * 0.5f) + frustumShiftAtFarPlane;
        const float frustumMaxHalfSize = std::max(frustumHalfWidth, frustumLength * 0.5f);

        frustumCamera->setTop(frustumMaxHalfSize * m_stereoCamera->aspectRatio());
        frustumCamera->setBottom(-frustumMaxHalfSize * m_stereoCamera->aspectRatio());
        frustumCamera->setLeft(-frustumMaxHalfSize);
        frustumCamera->setRight(frustumMaxHalfSize);
        frustumCamera->setNearPlane(m_stereoCamera->nearPlane());
        frustumCamera->setFarPlane(m_stereoCamera->farPlane() * 2.0f);
    }

    // FocusArea
    {
        m_focusArea->update();

        if (m_autoFocus) {
            handleFocusForFocusArea();
        }
    }
}

void Qt3DRenderer::createAspects(std::shared_ptr<all::ModelNavParameters> nav_params)
{
    using namespace Qt3DCore;
    using namespace Qt3DRender;
    using namespace Qt3DExtras;

    m_nav_params = std::move(nav_params);

    m_rootEntity = std::make_unique<Qt3DCore::QEntity>();

    m_renderer = new QStereoForwardRenderer();
    m_view->setActiveFrameGraph(m_renderer);
    m_view->renderSettings()->pickingSettings()->setPickMethod(QPickingSettings::TrianglePicking);

    m_camera = new QStereoProxyCamera(m_rootEntity.get());
    m_renderer->setCamera(m_camera);

    m_raycaster = new Qt3DRender::QRayCaster{ m_rootEntity.get() };
    m_raycaster->setRunMode(Qt3DRender::QAbstractRayCaster::SingleShot);
    m_raycaster->setDirection(QVector3D(0.0f, 0.0f, 1.0f)); // Set your own direction
    m_raycaster->setOrigin(QVector3D(0.0f, 0.01f, 0.0f)); // Set your own position
    QObject::connect(m_raycaster, &QRayCaster::hitsChanged,
                     [](const Qt3DRender::QRayCaster::Hits& hits) {
                         if (hits.empty())
                             return;
                     });

    m_raycaster->setEnabled(true);
    m_rootEntity->addComponent(m_raycaster);

    auto ent = m_rootEntity.get();

    if (m_nav_params)
        m_nav_params->hit_test = [this](glm::vec3 pos, glm::vec3 dir) {
            this->m_raycaster->trigger(toQVector3D(pos), toQVector3D(dir), 1000);

            auto hits = this->m_raycaster->pick(toQVector3D(pos), toQVector3D(dir), 100);

            if (hits.isEmpty()) {
                return glm::vec3{ -1 };
            }

            // Use std::min_element along with a lambda function to find the minimum distance
            auto nearestHitIterator = std::min_element(hits.begin(), hits.end(),
                                                       [](const QRayCasterHit& a, const QRayCasterHit& b) {
                                                           return a.distance() < b.distance();
                                                       });

            return toGlmVec3(nearestHitIterator->worldIntersection());
        };
    createScene(m_rootEntity.get());
}

void Qt3DRenderer::createScene(Qt3DCore::QEntity* root)
{
    m_sceneEntity = new Qt3DCore::QEntity{ m_rootEntity.get() };
    m_sceneEntity->setObjectName("SceneEntity");
    m_sceneEntity->addComponent(m_renderer->sceneLayer());
    m_userEntity = new Qt3DCore::QEntity{ m_sceneEntity };
    m_userEntity->setObjectName("UserEntity");

    m_cursor = new CursorEntity(m_rootEntity.get(), m_sceneEntity, m_camera->leftCamera(), m_view, m_stereoCamera);
    m_cursor->setObjectName("CursorEntity");
    m_cursor->addComponent(m_renderer->cursorLayer());
    m_cursor->setType(CursorType::Ball);

    // Frustums
    {
        m_frustumRect = new FrustumRect(root);
        m_centerFrustum = new Frustum(QColor(), false, root);
        m_leftFrustum = new Frustum(QColor::fromRgb(0xff, 0x80, 0x80, 0x90), true, root);
        m_rightFrustum = new Frustum(QColor::fromRgb(0x00, 0x80, 0xff, 0x90), true, root);

        m_frustumRect->addComponent(m_renderer->frustumLayer());
        m_centerFrustum->addComponent(m_renderer->frustumLayer());
        m_leftFrustum->addComponent(m_renderer->frustumLayer());
        m_rightFrustum->addComponent(m_renderer->frustumLayer());
    }

    // FocusArea
    {
        m_focusArea = new FocusArea(root);
        m_focusArea->setCamera(m_camera->centerCamera());
        m_focusArea->addComponent(m_renderer->focusAreaLayer());

        auto updateViewSize = [this] {
            m_focusArea->setViewSize(QSize(m_view->width(), m_view->height()));
        };
        QObject::connect(m_view, &Qt3DExtras::Qt3DWindow::widthChanged, m_focusArea, updateViewSize);
        QObject::connect(m_view, &Qt3DExtras::Qt3DWindow::heightChanged, m_focusArea, updateViewSize);
        updateViewSize();

        for (size_t i = 0, m = m_afRayCasters.size(); i < m; ++i) {
            m_afRayCasters[i] = new Qt3DRender::QScreenRayCaster(m_sceneEntity);
            m_afRayCasters[i]->setRunMode(Qt3DRender::QAbstractRayCaster::SingleShot);
            // m_afRayCaster->setFilterMode(Qt3DRender::QAbstractRayCaster::AcceptAnyMatchingLayers);
            // m_afRayCaster->addLayer(m_renderer->sceneLayer());
            QObject::connect(m_afRayCasters[i], &Qt3DRender::QScreenRayCaster::hitsChanged, [this, idx = i](const Qt3DRender::QAbstractRayCaster::Hits& hits) {
                afRaycasterHitResult(idx, hits);
            });
            m_userEntity->addComponent(m_afRayCasters[i]);
        }

        QObject::connect(m_focusArea, &FocusArea::centerChanged, this, &Qt3DRenderer::handleFocusForFocusArea);
        QObject::connect(m_focusArea, &FocusArea::extentChanged, this, &Qt3DRenderer::handleFocusForFocusArea);
    }

    loadModel();

    m_view->setRootEntity(m_rootEntity.get());

    std::vector<QVector3D> lightPositions{
        { 0, -1, 0 }, { 0, 1, 0 }, { 1, 0, 0 }, { -1, 0, 0 }, { 0, 0, -1 }, { 0, 0, 1 }
    };
    for (auto position : lightPositions) {
        auto le = new Qt3DCore::QEntity;
        auto l = new Qt3DRender::QDirectionalLight;
        l->setIntensity(0.3);
        auto lt = new Qt3DCore::QTransform;
        lt->setTranslation(position);
        l->setWorldDirection(QVector3D{} - lt->translation());
        le->addComponent(lt);
        le->addComponent(l);
        le->addComponent(m_renderer->sceneLayer());
        le->setParent(m_rootEntity.get());
    }

    loadImage();
}

void Qt3DRenderer::showImage()
{
    m_renderer->setMode(QStereoForwardRenderer::Mode::StereoImage);
}

void Qt3DRenderer::showModel()
{
    m_renderer->setMode(QStereoForwardRenderer::Mode::Scene);
    loadModel();
}

void Qt3DRenderer::propertyChanged(std::string_view name, std::any value)
{
    if (name == "scale_factor") {
        m_cursor->setScaleFactor(std::any_cast<float>(value));
    } else if (name == "scaling_enabled") {
        m_cursor->setScalingEnabled(std::any_cast<bool>(value));
    } else if (name == "cursor_type") {
        m_cursor->setType(std::any_cast<CursorType>(value));
    } else if (name == "display_mode") {
        auto displayMode = std::any_cast<DisplayMode>(value);
        m_renderer->setDisplayMode(displayMode);
    } else if (name == "frustum_view_enabled") {
        const bool frustumEnabled = std::any_cast<bool>(value);
        m_frustumRect->setEnabled(frustumEnabled);
        m_centerFrustum->setEnabled(frustumEnabled);
        m_leftFrustum->setEnabled(frustumEnabled);
        m_rightFrustum->setEnabled(frustumEnabled);
    } else if (name == "show_focus_area") {
        const bool showFocusArea = std::any_cast<bool>(value);
        m_focusArea->setEnabled(showFocusArea);
    } else if (name == "auto_focus") {
        const bool useAF = std::any_cast<bool>(value);
        m_autoFocus = useAF;
        if (m_autoFocus)
            handleFocusForFocusArea();
    } else if (name == "cursor_color") {
        auto color = std::any_cast<std::array<float, 4>>(value);
        m_cursor->setCursorTintColor(QColor::fromRgbF(color[0], color[1], color[2], color[3]));
    }
}

glm::vec3 Qt3DRenderer::cursorWorldPosition() const
{
    return toGlmVec3(m_cursor->position());
}

void Qt3DRenderer::loadImage(QUrl path)
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
        const QVector2D viewportSize{ static_cast<float>(m_view->width()), static_cast<float>(m_view->height()) };
        const QVector2D imageSize = stereoImageMaterial->textureSize();

        leftImageMesh->setViewportSize(viewportSize);
        leftImageMesh->setImageSize(imageSize);

        rightImageMesh->setViewportSize(viewportSize);
        rightImageMesh->setImageSize(imageSize);
    };
    QObject::connect(m_view, &QWindow::widthChanged, updateImageMeshes);
    QObject::connect(m_view, &QWindow::heightChanged, updateImageMeshes);
    QObject::connect(stereoImageMaterial, &StereoImageMaterial::textureSizeChanged, updateImageMeshes);
    updateImageMeshes();
}

void Qt3DRenderer::loadModel(std::filesystem::path path)
{
    delete m_skyBox;
    m_skyBox = nullptr;
    delete m_userEntity;
    m_userEntity = new Qt3DCore::QEntity{ m_sceneEntity };
    m_userEntity->setObjectName("UserEntity");

    QString filePath = QString::fromStdString(path.string());
    bool isFbx = filePath.endsWith(".fbx");

    auto* sceneRoot = MeshLoader::load(filePath);
    if (sceneRoot == nullptr) {
        qDebug() << "Failed to load model:" << filePath;
        return;
    }
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
#define MMat(name) m_materials[QStringLiteral(#name)] = new GlossyMaterial(name##ST, name##SU, m_rootEntity.get())
    MMat(CarPaint);
    MMat(DarkGlass);
    MMat(DarkGloss);
    MMat(Dark);
    MMat(Chrome);
    MMat(Plate);
    MMat(Tire);
    MMat(ShadowPlane);
#undef MMat
    m_materials["Skybox"] = new SkyboxMaterial(SkyboxST, {}, m_rootEntity.get());

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

    // Give Qt3D Time to process mesh extents
    auto* frameAction = new Qt3DLogic::QFrameAction;
    QObject::connect(frameAction, &Qt3DLogic::QFrameAction::triggered, m_userEntity, [this, frameAction] {
        setupCameraBasedOnSceneExtent();
        frameAction->deleteLater();
    });
    m_userEntity->addComponent(frameAction);

    // For AutoFocus Intersection Testing
    for (auto* rayCaster : m_afRayCasters)
        m_userEntity->addComponent(rayCaster);
}

void Qt3DRenderer::setCursorEnabled(bool enabled)
{
    if (enabled) {
        m_cursor->setScaleFactor(cursor_scale);
    } else {
        cursor_scale = m_cursor->scaleFactor();
        m_cursor->setScaleFactor(0);
    }
}

bool Qt3DRenderer::hoversFocusArea(int x, int y) const
{
    return m_focusArea->containsMouse();
}

Qt3DRenderer::SceneExtent Qt3DRenderer::calculateSceneExtent(Qt3DCore::QEntity* entity) const
{
    SceneExtent e;
    _calculateSceneDimensions(entity, e.min, e.max);
    return e;
}

void Qt3DRenderer::_calculateSceneDimensions(Qt3DCore::QEntity* entity, QVector3D& minBounds, QVector3D& maxBounds) const
{
    if (entity == nullptr)
        return;

    for (Qt3DCore::QNode* childNode : entity->childNodes()) {
        auto* childEntity = qobject_cast<Qt3DCore::QEntity*>(childNode);
        if (childEntity != nullptr)
            _calculateSceneDimensions(childEntity, minBounds, maxBounds);
    }

    // Check if the entity has a geometry renderer
    auto geometryRenderers = entity->componentsOfType<Qt3DRender::QGeometryRenderer>();
    if (geometryRenderers.size() > 0) {
        Qt3DRender::QGeometryRenderer* geometryRenderer = geometryRenderers.first();
        Qt3DCore::QGeometry* geometry = geometryRenderer->geometry();

        if (geometry == nullptr)
            return;

        const QVector3D& bvMin = geometry->minExtent();
        const QVector3D& bvMax = geometry->maxExtent();

        const QVector3D diagonal = bvMax - bvMin;

        // Local Coordinates
        const QVector3D sphereCenter = bvMin + diagonal * 0.5f;
        const float radius = diagonal.length();

        std::function<Qt3DCore::QTransform*(Qt3DCore::QEntity * e)> findNearestParentTransform = [&](Qt3DCore::QEntity* e) -> Qt3DCore::QTransform* {
            if (e == nullptr)
                return nullptr;
            auto transforms = e->componentsOfType<Qt3DCore::QTransform>();
            if (transforms.size() == 0)
                return findNearestParentTransform(e->parentEntity());
            return transforms.first();
        };

        Qt3DCore::QTransform* transform = findNearestParentTransform(entity);
        if (transform != nullptr) {

            const QMatrix4x4 worldMatrix = transform->worldMatrix();
            // Transform extremities in x, y, and z directions to find extremities
            // of the resulting ellipsoid
            const QVector3D x = worldMatrix.map(sphereCenter + QVector3D(radius, 0.0f, 0.0f));
            const QVector3D y = worldMatrix.map(sphereCenter + QVector3D(0.0f, radius, 0.0f));
            const QVector3D z = worldMatrix.map(sphereCenter + QVector3D(0.0f, 0.0f, radius));

            // Transform center and find maximum radius of ellipsoid
            const QVector3D worldCenter = worldMatrix.map(sphereCenter);
            const float worldRadius = sqrt(qMax(qMax((x - worldCenter).lengthSquared(),
                                                     (y - worldCenter).lengthSquared()),
                                                (z - worldCenter).lengthSquared()));

            const QVector3D worldBVMax = worldCenter + QVector3D(worldRadius, worldRadius, worldRadius) * 1.44f;
            const QVector3D worldBVMin = worldCenter - QVector3D(worldRadius, worldRadius, worldRadius) * 1.44f;

            maxBounds = QVector3D(std::max(worldBVMax.x(), maxBounds.x()),
                                  std::max(worldBVMax.y(), maxBounds.y()),
                                  std::max(worldBVMax.z(), maxBounds.z()));

            minBounds = QVector3D(std::min(worldBVMin.x(), minBounds.x()),
                                  std::min(worldBVMin.y(), minBounds.y()),
                                  std::min(worldBVMin.z(), minBounds.z()));
        } else {

            maxBounds = QVector3D(std::max(bvMax.x(), maxBounds.x()),
                                  std::max(bvMax.y(), maxBounds.y()),
                                  std::max(bvMax.z(), maxBounds.z()));

            minBounds = QVector3D(std::min(bvMin.x(), minBounds.x()),
                                  std::min(bvMin.y(), minBounds.y()),
                                  std::min(bvMin.z(), minBounds.z()));
        }
    }
}

void Qt3DRenderer::modelExtentChanged(const QVector3D& min, const QVector3D& max)
{
    if (!m_nav_params)
        return;

    m_nav_params->min_extent = toGlmVec3(min);
    m_nav_params->max_extent = toGlmVec3(max);
}

void Qt3DRenderer::setupCameraBasedOnSceneExtent()
{
    const SceneExtent ext = calculateSceneExtent(m_userEntity);
    m_nav_params->min_extent = toGlmVec3(ext.min);
    m_nav_params->max_extent = toGlmVec3(ext.max);

    const QVector3D viewCenter = (ext.max + ext.min) * 0.5f;
    const QVector3D extent = ext.max - ext.min;
    const float radius = std::max(extent.x(), std::max(extent.y(), extent.z())) * 0.5f;

    const QVector3D cameraPosition = viewCenter - QVector3D(0.0f, 0.0f, 1.0f) * radius;
    const QVector3D viewVector = viewCenter - cameraPosition;

    m_stereoCamera->setPosition(toGlmVec3(cameraPosition));
    m_stereoCamera->setForwardVector(toGlmVec3(viewVector.normalized()));
    m_stereoCamera->setConvergencePlaneDistance(viewVector.length());
    m_stereoCamera->setFarPlane(5 * radius);
    m_stereoCamera->setUpVector(glm::vec3(0.0f, 1.0f, 0.0f));

    // set rotation point
    auto cam = dynamic_cast<all::OrbitalStereoCamera*>(m_stereoCamera);
    if (cam) {
        cam->setRadius(viewVector.length());
        cam->setTarget(toGlmVec3(viewCenter));
        cam->rotate(glm::radians(45.0f), glm::radians(90.0f));
    }
}

void Qt3DRenderer::handleFocusForFocusArea()
{
    if (m_focusArea == nullptr)
        return;

    const QVector3D center = m_focusArea->center();
    const QVector3D extent = m_focusArea->extent();

    for (size_t y = 0; y < AFSamplesY; ++y) {
        QVector3D p;
        float yPos = center.y() + ((float(y) / AFSamplesY) - 1.0f) * (extent.y() * 0.5f);
        // To OpenGL Y
        yPos = m_view->height() - yPos;
        for (size_t x = 0; x < AFSamplesX; ++x) {
            const float xPos = center.x() + ((float(x) / AFSamplesX) - 1.0f) * (extent.x() * 0.5f);

            const size_t rayCasterIdx = y * AFSamplesX + x;
            assert(rayCasterIdx < m_afRayCasters.size());
            m_lastAfHitDistances[rayCasterIdx] = 0.0f;
            m_afRayCasters[rayCasterIdx]->trigger({ int(xPos), int(yPos) });
        }
    }
}

void Qt3DRenderer::afRaycasterHitResult(size_t idx, const Qt3DRender::QAbstractRayCaster::Hits& hits)
{
    float nearestHitDistanceFromCamera = std::numeric_limits<float>::max();

    // Average result of hits distance (or we could keep smallest one)
    for (const auto& hit : hits)
        nearestHitDistanceFromCamera = std::min(hit.distance(), nearestHitDistanceFromCamera);

    if (nearestHitDistanceFromCamera < std::numeric_limits<float>::max())
        m_lastAfHitDistances[idx] = nearestHitDistanceFromCamera;

    // Trigger delay evaluation once we know all raycasters have received their results
    if (!m_afResultUpdateRequested) {
        m_afResultUpdateRequested = true;

        // Average all hits
        QTimer::singleShot(0, [this] {
            m_afResultUpdateRequested = false;
            float averagedDistanceFromCamera = 0.0f;
            size_t validHits = 0;

            for (float hitDistance : m_lastAfHitDistances) {
                if (hitDistance > 0) {
                    averagedDistanceFromCamera += hitDistance;
                    ++validHits;
                }
            }

            if (validHits > 0) {
                averagedDistanceFromCamera /= float(validHits);
                // Notify Controllers our AF Distance is updated
                m_propertyUpdateNofitier("auto_focus_distance", averagedDistanceFromCamera);
            }
        });
    }
}

void Qt3DRenderer::updateMouse()
{
    m_cursor->onMouseMoveEvent(toQVector3D(m_stereoCamera->position()), m_view->mapFromGlobal(m_view->cursor().pos()));
}

} // namespace all::qt3d
