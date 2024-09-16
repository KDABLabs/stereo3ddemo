#include "qt3d_impl.h"
#include "qt3d_cursor.h"
#include "qt3d_materials.h"
#include "mesh_loader.h"
#include "stereo_image_material.h"
#include "stereo_image_mesh.h"
#include "stereo_proxy_camera.h"
#include "frame_action.h"
#include "qt3d_shaders.h"
#include "util_qt.h"

#include <Qt3DRender/QSceneLoader>
#include <Qt3DRender/QPickingSettings>
#include <Qt3DRender/QRenderSettings>
#include <Qt3DRender/QMesh>
#include <Qt3DRender/QLayer>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/QDirectionalLight>
#include <Qt3DRender/QRayCaster>
#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QDiffuseMapMaterial>
#include <Qt3DExtras/QPhongMaterial>
#include <shared/cursor.h>
#include <QFileInfo>
#include <QImageReader>
#include <shared/stereo_camera.h>

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

all::qt3d::Qt3DImpl::Qt3DImpl(all::StereoCamera& stereoCamera)
    : m_stereoCamera(&stereoCamera)
{
    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    if (qgetenv("DISABLE_STEREO") == "")
        format.setStereo(true);
    format.setSamples(8);
    QSurfaceFormat::setDefaultFormat(format);
}

all::qt3d::Qt3DImpl::~Qt3DImpl()
{
}

void all::qt3d::Qt3DImpl::ViewChanged()
{
    switch (m_cameraMode) {
    case CameraMode::Stereo:
        m_camera->SetMatrices(toQMatrix4x4(m_stereoCamera->GetViewLeft()), toQMatrix4x4(m_stereoCamera->GetViewRight()));
        break;
    case CameraMode::Right:
        m_camera->SetMatrices(toQMatrix4x4(m_stereoCamera->GetViewRight()), toQMatrix4x4(m_stereoCamera->GetViewRight()));
        break;
    case CameraMode::Left:
        m_camera->SetMatrices(toQMatrix4x4(m_stereoCamera->GetViewLeft()), toQMatrix4x4(m_stereoCamera->GetViewLeft()));
        break;
    case CameraMode::Mono:
        m_camera->SetMatrices(toQMatrix4x4(m_stereoCamera->GetViewCenter()), toQMatrix4x4(m_stereoCamera->GetViewCenter()));
        break;
    }
    ProjectionChanged();
}

void all::qt3d::Qt3DImpl::ProjectionChanged()
{
    switch (m_cameraMode) {
    case CameraMode::Mono:
        m_camera->SetProjection(toQMatrix4x4(m_stereoCamera->GetProjection()), 0);
        break;
    case CameraMode::Stereo:
        m_camera->SetProjection(toQMatrix4x4(m_stereoCamera->GetProjection()), m_stereoCamera->ShearCoefficient());
        break;
    case CameraMode::Right:
        m_camera->SetProjection(toQMatrix4x4(m_stereoCamera->GetProjection()), m_stereoCamera->ShearCoefficient(), true);
        break;
    case CameraMode::Left:
        m_camera->SetProjection(toQMatrix4x4(m_stereoCamera->GetProjection()), -m_stereoCamera->ShearCoefficient(), true);
        break;
    }
}

void all::qt3d::Qt3DImpl::CreateAspects(std::shared_ptr<all::ModelNavParameters> nav_params)
{
    using namespace Qt3DCore;
    using namespace Qt3DRender;
    using namespace Qt3DExtras;

    m_nav_params = std::move(nav_params);

    m_rootEntity = std::make_unique<Qt3DCore::QEntity>();

    m_renderer = new QStereoForwardRenderer();
    m_view.setActiveFrameGraph(m_renderer);
    m_view.renderSettings()->pickingSettings()->setPickMethod(QPickingSettings::TrianglePicking);

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
    CreateScene(m_rootEntity.get());
}

void all::qt3d::Qt3DImpl::CreateScene(Qt3DCore::QEntity* root)
{
    m_sceneEntity = new Qt3DCore::QEntity{ m_rootEntity.get() };
    m_sceneEntity->setObjectName("SceneEntity");
    m_sceneEntity->addComponent(m_renderer->sceneLayer());
    m_userEntity = new Qt3DCore::QEntity{ m_sceneEntity };
    m_userEntity->setObjectName("UserEntity");

    m_cursor = new CursorEntity(m_rootEntity.get(), m_sceneEntity, m_camera->GetLeftCamera(), &m_view, m_stereoCamera);
    m_cursor->setObjectName("CursorEntity");
    m_cursor->addComponent(m_renderer->cursorLayer());
    m_cursor->setType(CursorType::Ball);
    m_picker = new Picker(m_sceneEntity, m_cursor);
    LoadModel();

    m_view.setRootEntity(m_rootEntity.get());

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

    LoadImage();
}

void all::qt3d::Qt3DImpl::ShowImage()
{
    m_renderer->setMode(QStereoForwardRenderer::Mode::StereoImage);
}

void all::qt3d::Qt3DImpl::ShowModel()
{
    m_renderer->setMode(QStereoForwardRenderer::Mode::Scene);
    LoadModel();
}

void all::qt3d::Qt3DImpl::OnPropertyChanged(std::string_view name, std::any value)
{
    if (name == "scale_factor") {
        m_cursor->setScaleFactor(std::any_cast<float>(value));
    } else if (name == "scaling_enabled") {
        m_cursor->setScalingEnabled(std::any_cast<bool>(value));
    } else if (name == "cursor_type") {
        m_cursor->setType(std::any_cast<CursorType>(value));
    } else if (name == "camera_mode") {
        m_cameraMode = std::any_cast<CameraMode>(value);
        ViewChanged();
    } else if (name == "cursor_color") {
        auto color = std::any_cast<std::array<float, 4>>(value);
        m_cursor->setCursorTintColor(QColor::fromRgbF(color[0], color[1], color[2], color[3]));
    }
}

glm::vec3 all::qt3d::Qt3DImpl::GetCursorWorldPosition() const
{
    return toGlmVec3(m_picker->cursor_world);
}

void all::qt3d::Qt3DImpl::LoadImage(QUrl path)
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

void all::qt3d::Qt3DImpl::LoadModel(std::filesystem::path path)
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
#define MMat(name) m_materials[u## #name##_qs] = new all::qt3d::GlossyMaterial(all::qt3d::name##ST, all::qt3d::name##SU, m_rootEntity.get())
    MMat(CarPaint);
    MMat(DarkGlass);
    MMat(DarkGloss);
    MMat(Dark);
    MMat(Chrome);
    MMat(Plate);
    MMat(Tire);
    MMat(ShadowPlane);
#undef MMat
    m_materials["Skybox"] = new all::qt3d::SkyboxMaterial(all::qt3d::SkyboxST, {}, m_rootEntity.get());

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

void all::qt3d::Qt3DImpl::SetCursorEnabled(bool enabled)
{
    if (enabled) {
        m_cursor->setScaleFactor(cursor_scale);
    } else {
        cursor_scale = m_cursor->getScaleFactor();
        m_cursor->setScaleFactor(0);
    }
}

QVector2D all::qt3d::Qt3DImpl::calculateSceneDimensions(Qt3DCore::QEntity* scene) const
{

    QVector3D minBounds, maxBounds;
    _calculateSceneDimensions(scene, minBounds, maxBounds);
    QVector2D res{ qMin(qMin(minBounds.x(), minBounds.y()), minBounds.z()),
                   qMax(qMax(maxBounds.x(), maxBounds.y()), maxBounds.z()) };
    return res;
}

void all::qt3d::Qt3DImpl::_calculateSceneDimensions(Qt3DCore::QNode* node, QVector3D& minBounds, QVector3D& maxBounds) const
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

void all::qt3d::Qt3DImpl::OnModelExtentChanged(const QVector3D& min, const QVector3D& max)
{
    if (!m_nav_params)
        return;

    m_nav_params->min_extent = toGlmVec3(min);
    m_nav_params->max_extent = toGlmVec3(max);
}
void all::qt3d::Qt3DImpl::UpdateMouse()
{
    m_cursor->onMouseMoveEvent(toQVector3D(m_stereoCamera->GetPosition()), m_view.mapFromGlobal(m_view.cursor().pos()));
}
