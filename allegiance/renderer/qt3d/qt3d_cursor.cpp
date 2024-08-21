#include "qt3d_cursor.h"
#include "shared/stereo_camera.h"
#include "util_qt.h"

#include <glm/ext/matrix_projection.hpp>
#include <Qt3DExtras/QCuboidMesh>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DExtras/QPlaneMesh>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QTextureMaterial>
#include <Qt3DRender/QTexture>
#include <Qt3DCore/QTransform>
#include <Qt3DRender/QCameraLens>
#include <Qt3DRender/QPickEvent>
#include <Qt3DRender/QRayCaster>
#include <qlogging.h>
#include <qquaternion.h>
#include <ranges>

using namespace all::qt3d;
using namespace Qt3DRender;

// Function to convert screen coordinates to world coordinates
static glm::vec3 screenToWorld(const QPoint& cursorPos, const QSize& frameSize, const QMatrix4x4& viewMatrix, const QMatrix4x4& projectionMatrix) {
    // Convert cursor position to NDC
    float x = (2.0f * cursorPos.x()) / frameSize.width() - 1.0f;
    float y = 1.0f - (2.0f * cursorPos.y()) / frameSize.height();

    // Direction vector in view space
    glm::vec4 rayView(x, y, -1.0f, 1.0f);

    // Convert view space to world space
    glm::vec4 rayWorld = glm::inverse(toGlmMat4x4(viewMatrix) * toGlmMat4x4(projectionMatrix))*(rayView);
    return glm::normalize(glm::vec3(rayWorld));
}

CursorBillboard::CursorBillboard(QNode* parent)
    : Qt3DCore::QEntity(parent)
{
    m_plane = new Qt3DExtras::QPlaneMesh;
    addComponent(m_plane);

    auto m = new Qt3DExtras::QTextureMaterial;
    auto tex = new Qt3DRender::QTextureLoader{};
    tex->setSource(QUrl("qrc:/cursor_billboard.png"));
    m->setTexture(tex);
    m->setAlphaBlendingEnabled(true);
    m_material = m;
    addComponent(m_material);

    m_transform = new Qt3DCore::QTransform;
    addComponent(m_transform);
    m_transform->setScale(12);

    m_matrix = m_transform->matrix();
}

void CursorBillboard::setRotation(const QQuaternion& rotation)
{
    QMatrix4x4 m = m_matrix;

    m.rotate(rotation);

    auto right = m.column(0).toVector3D().normalized();

    auto right0 = right;
    right0.setY(0);
    right0.normalize();

    auto r0 = QQuaternion::rotationTo(right, right0);


    QMatrix4x4 x = m_matrix;

    x.rotate(r0);
    x.rotate(rotation);

    QQuaternion turn_up = QQuaternion::rotationTo(QVector3D(0, 1, 0), QVector3D(0, 0, 1));

    x.rotate(turn_up);

    m_transform->setMatrix(x);
}

void CursorBillboard::setTexture(CursorTexture texture)
{
    auto tex = new Qt3DRender::QTextureLoader{};

    switch (texture) {
    case CursorTexture::Default:
        tex->setSource(QUrl("qrc:/cursor_billboard.png"));
        break;
    case CursorTexture::CrossHair:
        tex->setSource(QUrl("qrc:/cursor_billboard_crosshair.png"));
        break;
    case CursorTexture::Dot:
        tex->setSource(QUrl("qrc:/cursor_billboard_dot.png"));
        break;
    }

    m_material->setTexture(tex);
}

CursorSphere::CursorSphere(QNode* parent)
    : Qt3DCore::QEntity(parent)
{
    auto* sphereMesh = new Qt3DExtras::QSphereMesh();
    addComponent(sphereMesh);

    auto* material = new Qt3DExtras::QPhongMaterial();
    material->setShininess(100.0);
    material->setDiffuse(QColor("lightyellow"));
    material->setAmbient(QColor("lightyellow"));
    addComponent(material);
}

CursorCross::CursorCross(QNode* parent)
    : Qt3DCore::QEntity(parent)
{
    auto* c1Entity = new Qt3DCore::QEntity(this);
    auto* c2Entity = new Qt3DCore::QEntity(this);
    auto* c3Entity = new Qt3DCore::QEntity(this);

    auto* c1Mesh = new Qt3DExtras::QCuboidMesh();
    c1Mesh->setXExtent(6);
    c1Mesh->setYExtent(0.2f);
    c1Mesh->setZExtent(0.2f);
    c1Entity->addComponent(c1Mesh);

    auto* c2Mesh = new Qt3DExtras::QCuboidMesh();
    c2Mesh->setXExtent(0.2f);
    c2Mesh->setYExtent(6);
    c2Mesh->setZExtent(0.2f);
    c2Entity->addComponent(c2Mesh);

    auto* c3Mesh = new Qt3DExtras::QCuboidMesh();
    c3Mesh->setXExtent(0.2f);
    c3Mesh->setYExtent(0.2f);
    c3Mesh->setZExtent(6);
    c3Entity->addComponent(c3Mesh);

    auto* material = new Qt3DExtras::QPhongMaterial();
    material->setShininess(100.0);
    material->setDiffuse(QColor("lightyellow"));
    material->setAmbient(QColor("lightyellow"));
    c1Entity->addComponent(material);
    c2Entity->addComponent(material);
    c3Entity->addComponent(material);
}

all::qt3d::CursorEntity::CursorEntity(QEntity* parent, QEntity* scene, QEntity* camera, Qt3DExtras::Qt3DWindow* window, all::StereoCamera* pCamera)
    : QEntity(parent)
{

    m_transform = new Qt3DCore::QTransform();

    m_transform->setTranslation({ -1, -1, -1 });
    addComponent(m_transform);

    connect(window, &Qt3DExtras::Qt3DWindow::widthChanged,
            this, &CursorEntity::updateSize);
    connect(window, &Qt3DExtras::Qt3DWindow::heightChanged,
            this, &CursorEntity::updateSize);
    m_window = window;

    setCamera(camera);

    m_billboard = new CursorBillboard{ this };
    m_sphere = new CursorSphere{ this };
    m_cross = new CursorCross{ this };

    m_raycaster = new Qt3DRender::QScreenRayCaster{scene};
    m_raycaster->setRunMode(Qt3DRender::QAbstractRayCaster::SingleShot);
    connect(m_raycaster, &QRayCaster::hitsChanged,
            [this, pCamera](const Qt3DRender::QRayCaster::Hits& hits) {
                auto filteredHits = hits | std::ranges::views::filter([this](const QRayCasterHit& hit) {
                                        return hit.entity() != m_sphere && hit.entity() != m_billboard;
                                    });
                auto nearestHitIterator = std::ranges::min_element(filteredHits, {}, &QRayCasterHit::distance);
                if (nearestHitIterator == filteredHits.end()) {
                    auto frame = m_window->frameGeometry();
                    auto cursorPos = m_window->mapFromGlobal(m_window->cursor().pos());
                    auto unv = glm::unProject(glm::vec3(cursorPos.x(), frame.height() - cursorPos.y(), 1.0f),
                                              pCamera->GetViewCenter(),
                                              pCamera->GetProjection(),
                                              glm::vec4{frame.x(), frame.y(), frame.width(), frame.height()});
                    auto pos= glm::inverse(pCamera->GetViewCenter()) * glm::vec4(0, 0, 0, 1);
                    setPosition(toQVector3D(glm::vec3(pos) + 0.1f * (unv - glm::vec3(pos))));
                    return;
                }
                setPosition(nearestHitIterator->worldIntersection());
            });

    m_raycaster->setEnabled(true);
    scene->addComponent(m_raycaster);
}

void all::qt3d::CursorEntity::setType(CursorType type)
{
    switch (type) {
    default:
    case CursorType::Ball:
        m_billboard->setTexture(CursorBillboard::CursorTexture::Default);
        m_billboard->setEnabled(true);
        m_sphere->setEnabled(true);
        m_cross->setEnabled(false);
        break;
    case CursorType::Cross:
        m_billboard->setEnabled(false);
        m_sphere->setEnabled(false);
        m_cross->setEnabled(true);
        break;
    case CursorType::CrossHair:
        m_billboard->setTexture(CursorBillboard::CursorTexture::CrossHair);
        m_billboard->setEnabled(true);
        m_sphere->setEnabled(false);
        m_cross->setEnabled(false);
        break;
    case CursorType::Dot:
        m_billboard->setTexture(CursorBillboard::CursorTexture::Dot);
        m_billboard->setEnabled(true);
        m_sphere->setEnabled(false);
        m_cross->setEnabled(false);
        break;
    }
}

void all::qt3d::CursorEntity::setPosition(const QVector3D& positionInScene)
{
    m_transform->setTranslation(positionInScene);
    updateSize();
}

void all::qt3d::CursorEntity::setCamera(const QEntity* camera)
{
    m_camera = camera;
    m_cameraLens = nullptr;
    m_cameraTransform = nullptr;
    if (camera == nullptr)
        return;

    auto ts = camera->componentsOfType<Qt3DCore::QTransform>();
    if (ts.size() > 0)
        m_cameraTransform = ts[0];

    auto ls = camera->componentsOfType<Qt3DRender::QCameraLens>();
    if (ts.size() > 0)
        m_cameraLens = ls[0];

    connect(m_cameraLens, &Qt3DRender::QCameraLens::projectionMatrixChanged,
            this, &CursorEntity::onProjectionMatrixChanged);
    connect(m_cameraTransform, &Qt3DCore::QTransform::worldMatrixChanged,
            this, &CursorEntity::onCameraTransformChanged);
}

void all::qt3d::CursorEntity::updateSize()
{
    constexpr float cursor_size = 0.06f;
    int targetSize = 20;
    QMatrix4x4 viewMatrix = m_cameraTransform->matrix();
    QVector3D cameraPosition = viewMatrix.inverted() * m_transform->matrix() * QVector3D(0, 0, 0); // Apply the transformMatrix to the local origin

    float distanceToCamera = cameraPosition.length();

    float fov = 2.0 * atan(1.0 / m_projectionMatrix(0, 0));
    float pixelsToAngle = fov / m_window->width();
    float radius = distanceToCamera * tan(targetSize * pixelsToAngle / 2.0);
    m_transform->setScale(m_scale_factor * (m_scaling_enabled ? radius : cursor_size)); // Set the scale based on the calculated radius

    auto direction = (m_cameraTransform->translation() - m_transform->translation()).normalized();
    auto cameraUp = QVector3D(viewMatrix.data()[4], viewMatrix.data()[5], viewMatrix.data()[6]); // Extract the up vector from the matrix

    QQuaternion rotationToFaceTarget = QQuaternion::fromDirection(direction, cameraUp);

    m_billboard->setRotation(rotationToFaceTarget);
}
void CursorEntity::onMouseMoveEvent(QVector3D pos, QPoint cursorPosition)
{
        this->m_raycaster->trigger(cursorPosition);
}

all::qt3d::Picker::Picker(Qt3DCore::QEntity* parent, CursorEntity* cursor)
    : Qt3DRender::QObjectPicker(parent), m_cursor(cursor)
{
    parent->addComponent(this);

    setHoverEnabled(true);
    setDragEnabled(true);

    QObject::connect(this, &Qt3DRender::QObjectPicker::moved, this, [this](Qt3DRender::QPickEvent* pick) {
        // qDebug() << "move" << parentNode()->objectName();
        m_cursor->setPosition(pick->worldIntersection());
        cursor_world = pick->worldIntersection();
    });

    connect(this, &Qt3DRender::QObjectPicker::entered, []() {
    });

    connect(this, &Qt3DRender::QObjectPicker::pressed, this, [this]() {
        hidden = true;
    });

    QObject::connect(this, &Qt3DRender::QObjectPicker::released, this, [this]() {
        hidden = false;
    });
}
