#include "qt3d_renderer.h"
#include "qt3d_cursor.h"
#include "qt3d_materials.h"
#include "qt3d_focusarea.h"
#include "focus_plane_preview.h"
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
#include <QMouseEvent>

#include <ranges>

namespace all::qt3d {

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
    const float flippedCorrection = m_stereoCamera->flipped() ? -1.0f : 1.0f;
    const float interocularDistance = flippedCorrection * m_stereoCamera->interocularDistance();

    m_camera->updateViewMatrices(toQVector3D(m_stereoCamera->position()),
                                 toQVector3D(m_stereoCamera->forwardVector()),
                                 toQVector3D(m_stereoCamera->upVector()),
                                 m_stereoCamera->convergencePlaneDistance(),
                                 interocularDistance,
                                 m_stereoCamera->mode());


    // Frustum
    {
        const QVector3D camPosition = toQVector3D(m_stereoCamera->position());
        const QVector3D viewVector = toQVector3D(m_stereoCamera->forwardVector());
        const QVector3D upVector = toQVector3D(m_stereoCamera->upVector());

        m_frustumAmplifiedCamera->updateViewMatrices(toQVector3D(m_stereoCamera->position()),
                                     toQVector3D(m_stereoCamera->forwardVector()),
                                     toQVector3D(m_stereoCamera->upVector()),
                                     m_stereoCamera->convergencePlaneDistance(),
                                     interocularDistance * 20.0f,
                                     m_stereoCamera->mode());

        m_leftFrustum->setViewMatrix(m_frustumAmplifiedCamera->leftCamera()->viewMatrix());
        m_rightFrustum->setViewMatrix(m_frustumAmplifiedCamera->rightCamera()->viewMatrix());

        auto* frustumCamera = m_renderer->frustumCamera();
        // Place the camera to match the center camera but with a viewCenter placed at the center of the near and far planes
        frustumCamera->setPosition(camPosition);
        const float centerPlaneDist = m_stereoCamera->nearPlane() + (m_stereoCamera->farPlane() - m_stereoCamera->nearPlane()) * 0.5f;
        frustumCamera->setViewCenter(camPosition + viewVector * centerPlaneDist);
        frustumCamera->setUpVector(upVector);
        // Then Rotate 90 around the X-Axis with rotation origin viewCenter
        frustumCamera->tiltAboutViewCenter(90.0f);
    }

    // FocusArea
    {
        m_focusArea->update();
        requestFocusForFocusArea();
    }

    // FocusPlanePreview
    {
        m_focusPlanePreview->setViewMatrix(m_camera->centerCamera()->viewMatrix());
    }
}

void Qt3DRenderer::projectionChanged()
{
    const float flippedCorrection = m_stereoCamera->flipped() ? -1.0f : 1.0f;
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
        m_frustumAmplifiedCamera->updateProjection(m_stereoCamera->nearPlane(),
                                                   m_stereoCamera->farPlane(),
                                                   m_stereoCamera->fov(),
                                                   m_stereoCamera->aspectRatio(),
                                                   m_stereoCamera->convergencePlaneDistance(),
                                                   interocularDistance * 20.0f,
                                                   m_stereoCamera->mode());

        m_leftFrustum->setProjectionMatrix(m_frustumAmplifiedCamera->leftCamera()->projectionMatrix());
        m_rightFrustum->setProjectionMatrix(m_frustumAmplifiedCamera->rightCamera()->projectionMatrix());

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
        frustumCamera->setFarPlane(m_stereoCamera->farPlane() * 3.0f);
    }

    // FocusArea
    {
        m_focusArea->update();
        requestFocusForFocusArea();
    }

    // FocusPlanePreview
    {
        m_focusPlanePreview->setProjectionMatrix(m_camera->centerCamera()->projectionMatrix());
        m_focusPlanePreview->setConvergence(m_stereoCamera->convergencePlaneDistance());
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

    m_frustumAmplifiedCamera = new QStereoProxyCamera(m_rootEntity.get());

    createScene(m_rootEntity.get());
}

void Qt3DRenderer::createScene(Qt3DCore::QEntity* root)
{
    m_sceneEntity = new Qt3DCore::QEntity{ m_rootEntity.get() };
    m_sceneEntity->setObjectName("SceneEntity");
    m_sceneEntity->addComponent(m_renderer->sceneLayer());

    m_userEntity = new Qt3DCore::QEntity{ m_sceneEntity };
    m_userEntity->setObjectName("UserEntity");

    m_cursor = new CursorEntity(m_rootEntity.get(), m_camera->centerCamera(), m_view);
    m_cursor->setObjectName("CursorEntity");
    m_cursor->addComponent(m_renderer->cursorLayer());
    m_cursor->setType(CursorType::Ball);

    m_cursorRaycaster = new Qt3DRender::QScreenRayCaster{ m_sceneEntity };
    m_cursorRaycaster->setRunMode(Qt3DRender::QAbstractRayCaster::SingleShot);
    m_cursorRaycaster->setFilterMode(Qt3DRender::QScreenRayCaster::DiscardAnyMatchingLayers);
    m_cursorRaycaster->addLayer(m_renderer->stereoImageLayer());
    m_cursorRaycaster->addLayer(m_renderer->leftLayer());
    m_cursorRaycaster->addLayer(m_renderer->rightLayer());
    m_cursorRaycaster->addLayer(m_renderer->frustumLayer());
    m_cursorRaycaster->addLayer(m_renderer->cursorLayer());
    m_cursorRaycaster->addLayer(m_renderer->focusAreaLayer());
    m_cursorRaycaster->addLayer(m_renderer->focusPlaneLayer());
    m_sceneEntity->addComponent(m_cursorRaycaster);
    QObject::connect(m_cursorRaycaster, &Qt3DRender::QScreenRayCaster::hitsChanged, this, &Qt3DRenderer::cursorHitResult);

    // Frustums
    {
        m_frustumRect = new FrustumRect(root);
        m_leftFrustum = new Frustum(QColor::fromRgb(0xff, 0x80, 0x80, 0x90), true, root);
        m_rightFrustum = new Frustum(QColor::fromRgb(0x00, 0x80, 0xff, 0x90), true, root);

        m_frustumRect->addComponent(m_renderer->frustumLayer());
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
            m_afRayCasters[i]->setFilterMode(Qt3DRender::QAbstractRayCaster::AcceptAnyMatchingLayers);
            m_afRayCasters[i]->addLayer(m_renderer->sceneLayer());
            QObject::connect(m_afRayCasters[i], &Qt3DRender::QScreenRayCaster::hitsChanged, [this, idx = i](const Qt3DRender::QAbstractRayCaster::Hits& hits) {
                afRaycasterHitResult(idx, hits);
            });
            m_userEntity->addComponent(m_afRayCasters[i]);
        }

        QObject::connect(m_focusArea, &FocusArea::centerChanged, this, &Qt3DRenderer::handleFocusForFocusArea);
        QObject::connect(m_focusArea, &FocusArea::extentChanged, this, &Qt3DRenderer::handleFocusForFocusArea);
    }

    // FocusPlanePreview
    {
        m_focusPlanePreview = new FocusPlanePreview(root);
        m_focusPlanePreview->addComponent(m_renderer->focusPlaneLayer());
    }

    loadModel();

    m_view->setRootEntity(m_rootEntity.get());

    const std::vector<QVector3D> lightDirs{
        { 1, -2, 1 },
        { -1, -2, -1 },
    };
    for (const auto& dir : lightDirs) {
        auto le = new Qt3DCore::QEntity;
        auto l = new Qt3DRender::QDirectionalLight;
        l->setIntensity(0.5f);
        l->setWorldDirection(dir.normalized());
        le->addComponent(l);
        le->addComponent(m_renderer->sceneLayer());
        le->addComponent(m_renderer->cursorLayer());
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

void Qt3DRenderer::onMouseEvent(::QMouseEvent* event)
{
    switch (event->type()) {
    case QEvent::MouseButtonPress:
        m_focusArea->onMousePressed(event);
        break;
    case QEvent::MouseButtonRelease:
        m_focusArea->onMouseReleased(event);
        break;
    case QEvent::MouseMove: {
        m_focusArea->onMouseMoved(event);
        // Note: ScreenRayCaster takes care of Qt -> OpenGL Y coordinate conversion
        if (!m_cursor->locked()) {
            const QPoint cursorPos = m_view->mapFromGlobal(m_view->cursor().pos());
            m_cursorRaycaster->trigger(cursorPos);
        }
        break;
    }
    default:
        break;
    }
}

void Qt3DRenderer::propertyChanged(std::string_view name, std::any value)
{
    if (name == "cursor_scale_factor") {
        m_cursor->setScaleFactor(std::any_cast<float>(value));
    } else if (name == "cursor_type") {
        m_cursor->setType(std::any_cast<CursorType>(value));
    } else if (name == "display_mode") {
        auto displayMode = std::any_cast<DisplayMode>(value);
        m_renderer->setDisplayMode(displayMode);
    } else if (name == "frustum_view_enabled") {
        const bool frustumEnabled = std::any_cast<bool>(value);
        m_frustumRect->setEnabled(frustumEnabled);
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
    } else if (name == "show_focus_plane") {
        const bool focusPlanePreviewEnabled = std::any_cast<bool>(value);
        qDebug() << focusPlanePreviewEnabled;
        m_focusPlanePreview->setEnabled(focusPlanePreviewEnabled);
    } else if (name == "wireframe_enabled") {
        const bool wireframeEnabled = std::any_cast<bool>(value);
        m_renderer->setWireframeEnabled(wireframeEnabled);
    } else if (name == "cursor_locked") {
        m_cursor->setLocked(std::any_cast<bool>(value));
        return;
    }
}

glm::vec3 Qt3DRenderer::cursorWorldPosition() const
{
    return toGlmVec3(m_cursor->position());
}

glm::vec3 Qt3DRenderer::sceneCenter() const
{
    return toGlmVec3(m_sceneCenter);
}

glm::vec3 Qt3DRenderer::sceneExtent() const
{
    return toGlmVec3(m_sceneExtent);
}

float Qt3DRenderer::fieldOfView() const
{
    return m_camera->centerCamera()->fieldOfView();
}

float Qt3DRenderer::aspectRatio() const
{
    return m_camera->centerCamera()->aspectRatio();
}

void Qt3DRenderer::completeInitialization()
{
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

void Qt3DRenderer::viewAll()
{
    setupCameraBasedOnSceneExtent();
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
    auto materials = entity->componentsOfType<Qt3DRender::QMaterial>();
    if (geometryRenderers.size() > 0) {
        Qt3DRender::QGeometryRenderer* geometryRenderer = geometryRenderers.first();
        Qt3DCore::QGeometry* geometry = geometryRenderer->geometry();

        if (geometry == nullptr)
            return;

        // Skip the Skybox
        if (materials.size() > 0) {
            if (qobject_cast<SkyboxMaterial*>(materials.front()) != nullptr)
                return;
        }

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

    m_sceneCenter = (ext.max + ext.min) * 0.5f;
    m_sceneExtent = ext.max - ext.min;

    m_propertyUpdateNofitier("scene_loaded", {});
}

void Qt3DRenderer::requestFocusForFocusArea()
{
    if (m_focusUpdateRequested)
        return;
    if (!m_autoFocus)
        return;
    m_focusUpdateRequested = true;
    QMetaObject::invokeMethod(this, &Qt3DRenderer::handleFocusForFocusArea, Qt::QueuedConnection);
}

void Qt3DRenderer::handleFocusForFocusArea()
{
    if (m_focusArea == nullptr)
        return;
    m_focusUpdateRequested = false;

    const QVector3D center = m_focusArea->center();
    const QVector3D extent = m_focusArea->extent();

    for (size_t y = 0; y < AFSamplesY; ++y) {
        QVector3D p;
        const float yPos = center.y() + ((float(y) / AFSamplesY) - 1.0f) * (extent.y() * 0.5f);
        for (size_t x = 0; x < AFSamplesX; ++x) {
            const float xPos = center.x() + ((float(x) / AFSamplesX) - 1.0f) * (extent.x() * 0.5f);

            const size_t rayCasterIdx = y * AFSamplesX + x;
            assert(rayCasterIdx < m_afRayCasters.size());
            m_lastAfHitDistances[rayCasterIdx] = 0.0f;
            // Note: ScreenRayCaster takes care of Qt -> OpenGL Y coordinate conversion
            m_afRayCasters[rayCasterIdx]->trigger({ int(xPos), int(yPos) });
        }
    }
}

void Qt3DRenderer::afRaycasterHitResult(size_t idx, const Qt3DRender::QAbstractRayCaster::Hits& hits)
{
    float nearestHitDistanceFromCamera = std::numeric_limits<float>::max();

    // Average result of hits distance (or we could keep smallest one)
    for (const auto& hit : hits) {
        nearestHitDistanceFromCamera = std::min(hit.distance(), nearestHitDistanceFromCamera);
    }

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

void Qt3DRenderer::cursorHitResult(const Qt3DRender::QAbstractRayCaster::Hits& hits)
{
    auto nearestHitIterator = std::ranges::min_element(hits, {}, &Qt3DRender::QRayCasterHit::distance);

    if (nearestHitIterator == hits.end()) {
        auto cursorPos = m_view->mapFromGlobal(m_view->cursor().pos());

        const QVector3D viewCenter = m_camera->centerCamera()->position() + m_camera->centerCamera()->viewVector().normalized() * m_stereoCamera->convergencePlaneDistance();
        const QVector4D viewCenterScreen = m_camera->centerCamera()->projectionMatrix() * m_camera->centerCamera()->viewMatrix() * QVector4D(viewCenter, 1.0f);
        const float zFocus = viewCenterScreen.z() / viewCenterScreen.w();
        const QVector3D cursorScreenPos(cursorPos.x(), m_view->height() - cursorPos.y(), zFocus);

        auto unprojectZO = [](const QVector3D& p, const QMatrix4x4& viewMatrix, const QMatrix4x4& projectionMatrix, const QRect& viewport) {
            const QMatrix4x4 inverse = QMatrix4x4(projectionMatrix * viewMatrix).inverted();
            QVector4D tmp(p, 1.0f);
            tmp.setX((tmp.x() - float(viewport.x())) / float(viewport.width()));
            tmp.setY((tmp.y() - float(viewport.y())) / float(viewport.height()));
            tmp.setX(tmp.x() * 2.0f - 1.0f);
            tmp.setY(tmp.y() * 2.0f - 1.0f);
            QVector4D obj = inverse * tmp;
            if (qFuzzyIsNull(obj.w()))
                obj.setW(1.0f);
            obj /= obj.w();
            return obj.toVector3D();
        };

        const QVector3D unv = unprojectZO(cursorScreenPos,
                                          m_camera->centerCamera()->viewMatrix(),
                                          m_camera->centerCamera()->projectionMatrix(),
                                          QRect{ 0, 0, m_view->width(), m_view->height() });
        m_cursor->setPosition(unv);
        return;
    }
    m_cursor->setPosition(nearestHitIterator->worldIntersection());
}

} // namespace all::qt3d
