#include "qt3d_renderer.h"
#include "qt3d_cursor.h"
#include "qt3d_materials.h"
#include "mesh_loader.h"
#include "stereo_image_material.h"
#include "stereo_image_mesh.h"
#include "stereo_proxy_camera.h"
#include "frame_action.h"
#include "qt3d_shaders.h"
#include "util_qt.h"
#include "frustum.h"

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

Qt3DRenderer::Qt3DRenderer(Qt3DExtras::Qt3DWindow* view, all::StereoCamera& stereoCamera)
    : m_view(view), m_stereoCamera(&stereoCamera)
{
    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    if (!qEnvironmentVariableIsSet("DISABLE_STEREO"))
        format.setStereo(true);
    format.setSamples(8);
    QSurfaceFormat::setDefaultFormat(format);
}

Qt3DRenderer::~Qt3DRenderer()
{
}

void Qt3DRenderer::viewChanged()
{
    m_camera->updateViewMatrices(toQVector3D(m_stereoCamera->position()),
                                 toQVector3D(m_stereoCamera->forwardVector()),
                                 toQVector3D(m_stereoCamera->upVector()),
                                 m_stereoCamera->convergencePlaneDistance(),
                                 m_stereoCamera->interocularDistance(),
                                 m_stereoCamera->mode());

    m_centerFrustum->setViewMatrix(m_camera->centerCamera()->viewMatrix());
    m_leftFrustum->setViewMatrix(m_camera->leftCamera()->viewMatrix());
    m_rightFrustum->setViewMatrix(m_camera->rightCamera()->viewMatrix());

    const float convergenceDist = m_stereoCamera->convergencePlaneDistance();
    const QVector3D camPosition = toQVector3D(m_stereoCamera->position());
    const QVector3D viewVector = toQVector3D(m_stereoCamera->forwardVector());
    const QVector3D upVector = toQVector3D(m_stereoCamera->upVector());
    const QVector3D sideVec = QVector3D::crossProduct(viewVector, upVector).normalized();
    const QVector3D realUp = QVector3D::crossProduct(sideVec, viewVector).normalized();

    auto* frustumCamera = m_renderer->frustumCamera();
    // Place the camera to match the center camera but with a viewCenter placed at the center of the near and far planes
    frustumCamera->setPosition(camPosition);
    const float centerPlaneDist = m_stereoCamera->nearPlane() + (m_stereoCamera->farPlane() - m_stereoCamera->nearPlane()) * 0.5f;
    frustumCamera->setViewCenter(camPosition + viewVector * centerPlaneDist);
    // frustumCamera->setViewCenter(camPosition + viewVector * convergenceDist);
    frustumCamera->setUpVector(upVector);
    // Then Rotate 90 around the X-Axis with rotation origin viewCenter
    frustumCamera->tiltAboutViewCenter(90.0f);

    projectionChanged();
}

void Qt3DRenderer::projectionChanged()
{
    m_camera->updateProjection(m_stereoCamera->nearPlane(),
                               m_stereoCamera->farPlane(),
                               m_stereoCamera->fov(),
                               m_stereoCamera->aspectRatio(),
                               m_stereoCamera->convergencePlaneDistance(),
                               m_stereoCamera->interocularDistance(),
                               m_stereoCamera->mode());

    m_centerFrustum->setProjectionMatrix(m_camera->centerCamera()->projectionMatrix());
    m_leftFrustum->setProjectionMatrix(m_camera->leftCamera()->projectionMatrix());
    m_rightFrustum->setProjectionMatrix(m_camera->rightCamera()->projectionMatrix());

    m_centerFrustum->setConvergence(m_stereoCamera->convergencePlaneDistance());
    m_leftFrustum->setConvergence(m_stereoCamera->convergencePlaneDistance());
    m_rightFrustum->setConvergence(m_stereoCamera->convergencePlaneDistance());

    // Adjust Ortho Projection so that whole frustum is visible
    auto* frustumCamera = m_renderer->frustumCamera();
    const float frustumHalfLength = (m_stereoCamera->farPlane() - m_stereoCamera->nearPlane()) * 0.5f;
    const float hFov = qDegreesToRadians(m_stereoCamera->fov()) * m_stereoCamera->aspectRatio() * 0.5f;
    const float frustumHalfWidth = frustumHalfLength * std::tan(hFov);
    const float frustumMaxHalfSize = std::max(frustumHalfWidth, frustumHalfLength);

    frustumCamera->setTop(frustumMaxHalfSize * m_stereoCamera->aspectRatio());
    frustumCamera->setBottom(-frustumMaxHalfSize * m_stereoCamera->aspectRatio());
    frustumCamera->setLeft(-frustumMaxHalfSize);
    frustumCamera->setRight(frustumMaxHalfSize);
    frustumCamera->setNearPlane(m_stereoCamera->nearPlane());
    frustumCamera->setFarPlane(m_stereoCamera->farPlane());
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
    m_picker = new Picker(m_sceneEntity, m_cursor);

    // Frustums
    {
        m_centerFrustum = new Frustum(QColor(Qt::white), root);
        m_leftFrustum = new Frustum(QColor(Qt::red), root);
        m_rightFrustum = new Frustum(QColor(Qt::blue), root);

        m_centerFrustum->addComponent(m_renderer->frustumLayer());
        m_leftFrustum->addComponent(m_renderer->frustumLayer());
        m_rightFrustum->addComponent(m_renderer->frustumLayer());
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
    } else if (name == "camera_mode") {
        m_cameraMode = std::any_cast<CameraMode>(value);
        viewChanged();
    } else if (name == "frustum_view_enabled") {
        const bool frustumEnabled = std::any_cast<bool>(value);
        m_centerFrustum->setEnabled(frustumEnabled);
        m_leftFrustum->setEnabled(frustumEnabled);
        m_rightFrustum->setEnabled(frustumEnabled);
    } else if (name == "cursor_color") {
        auto color = std::any_cast<std::array<float, 4>>(value);
        m_cursor->setCursorTintColor(QColor::fromRgbF(color[0], color[1], color[2], color[3]));
    }
}

glm::vec3 Qt3DRenderer::cursorWorldPosition() const
{
    return toGlmVec3(m_picker->cursor_world);
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
#define MMat(name) m_materials[u## #name##_qs] = new GlossyMaterial(name##ST, name##SU, m_rootEntity.get())
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

    auto ext = calculateSceneExtent(m_userEntity);
    m_nav_params->min_extent = toGlmVec3(ext.min);
    m_nav_params->max_extent = toGlmVec3(ext.max);

    // scale model
    auto k = ext.max - ext.min;
    auto s = QVector3D{ 8, 8, 8 } / k;
    auto centerpoint = ext.min + k / 2;

    auto modelViewProjection = m_stereoCamera->viewCenter() * m_stereoCamera->projection();
    auto size = ext.max - ext.min;
    glm::vec4 dimensions(size.x(), size.y(), size.z(), 1.0f);
    glm::vec4 dimensionsClip = m_stereoCamera->projection() * m_stereoCamera->viewCenter() * dimensions;
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
        cam->setTarget(toGlmVec3((ext.min + k / 2) * scaleFactor));
        cam->rotate(0, 0);
    }
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

QVector2D Qt3DRenderer::calculateSceneDimensions(Qt3DCore::QEntity* scene) const
{

    QVector3D minBounds, maxBounds;
    _calculateSceneDimensions(scene, minBounds, maxBounds);
    QVector2D res{ qMin(qMin(minBounds.x(), minBounds.y()), minBounds.z()),
                   qMax(qMax(maxBounds.x(), maxBounds.y()), maxBounds.z()) };
    return res;
}

void Qt3DRenderer::_calculateSceneDimensions(Qt3DCore::QNode* node, QVector3D& minBounds, QVector3D& maxBounds) const
{
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
            const int positionAttributeIndex = 0; // Adjust this index based on your format
            const Qt3DCore::QAttribute* positionAttribute = geometry->attributes().at(positionAttributeIndex);
            size_t offset = -1, byteStride;
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

void Qt3DRenderer::modelExtentChanged(const QVector3D& min, const QVector3D& max)
{
    if (!m_nav_params)
        return;

    m_nav_params->min_extent = toGlmVec3(min);
    m_nav_params->max_extent = toGlmVec3(max);
}
void Qt3DRenderer::updateMouse()
{
    m_cursor->onMouseMoveEvent(toQVector3D(m_stereoCamera->position()), m_view->mapFromGlobal(m_view->cursor().pos()));
}

} // namespace all::qt3d
