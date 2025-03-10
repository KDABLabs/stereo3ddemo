#include "qt3d_cursor.h"
#include "qt3d_materials.h"
#include "shared/stereo_camera.h"
#include "util_qt.h"

#include <glm/ext/matrix_projection.hpp>
#include <Qt3DExtras/QCuboidMesh>
#include <Qt3DExtras/QDiffuseSpecularMaterial>
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

using namespace all::qt3d;
using namespace Qt3DRender;

CursorBillboard::CursorBillboard(QNode* parent)
    : Qt3DCore::QEntity(parent)
{
    m_plane = new Qt3DExtras::QPlaneMesh;
    addComponent(m_plane);

    auto m = new CursorBillboardMaterial;
    m_tex = new Qt3DRender::QTextureLoader{};
    m_tex->setSource(QUrl("qrc:/cursor_billboard.png"));
    m->setTexture(m_tex);
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
    switch (texture) {
    case CursorTexture::Default:
        m_tex->setSource(QUrl("qrc:/cursor_billboard.png"));
        break;
    case CursorTexture::CrossHair:
        m_tex->setSource(QUrl("qrc:/cursor_billboard_crosshair.png"));
        break;
    case CursorTexture::Dot:
        m_tex->setSource(QUrl("qrc:/cursor_billboard_dot.png"));
        break;
    }
}

void all::qt3d::CursorBillboard::setCursorTintColor(const QColor& color)
{
    m_material->setColor(color);
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
    material->setSpecular(QColor("white"));
    m_material = material;
    addComponent(material);
}

void CursorSphere::setCursorTintColor(const QColor& color)
{
    m_material->setAmbient(color);
    m_material->setDiffuse(color);
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
    material->setSpecular(QColor("white"));
    c1Entity->addComponent(material);
    c2Entity->addComponent(material);
    c3Entity->addComponent(material);
    m_material = material;
}

void CursorCross::setCursorTintColor(const QColor& color)
{
    m_material->setAmbient(color);
    m_material->setDiffuse(color);
}

CursorEntity::CursorEntity(QEntity* parent, QEntity* camera, Qt3DExtras::Qt3DWindow* window)
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
}

void CursorEntity::setType(CursorType type)
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

void CursorEntity::setCursorTintColor(const QColor& color)
{
    m_billboard->setCursorTintColor(color);
    m_sphere->setCursorTintColor(color);
    m_cross->setCursorTintColor(color);
}

void CursorEntity::setLocked(bool locked)
{
    m_locked = locked;
}

bool CursorEntity::locked() const
{
    return m_locked;
}

void CursorEntity::setPosition(const QVector3D& positionInScene)
{
    if (m_locked)
        return;
    m_transform->setTranslation(positionInScene);
    updateSize();
}

QVector3D CursorEntity::position() const
{
    return m_transform->translation();
}

void CursorEntity::setCamera(const QEntity* camera)
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

void CursorEntity::updateSize()
{
    constexpr float cursor_size = 0.06f;
    int targetSize = 20;
    QMatrix4x4 viewMatrix = m_cameraTransform->matrix();
    QVector3D cameraPosition = (viewMatrix.inverted() * m_transform->matrix()).map(QVector3D(0, 0, 0)); // Apply the transformMatrix to the local origin

    float distanceToCamera = cameraPosition.length();

    float fov = 2.0 * atan(1.0 / m_projectionMatrix(0, 0));
    float pixelsToAngle = fov / m_window->width();
    float radius = distanceToCamera * tan(targetSize * pixelsToAngle / 2.0);
    const float scaleValue = m_scale_factor * (m_scaling_enabled ? radius : cursor_size);
    m_transform->setScale(scaleValue); // Set the scale based on the calculated radius

    auto direction = (m_cameraTransform->translation() - m_transform->translation()).normalized();
    auto cameraUp = QVector3D(viewMatrix.data()[4], viewMatrix.data()[5], viewMatrix.data()[6]); // Extract the up vector from the matrix

    QQuaternion rotationToFaceTarget = QQuaternion::fromDirection(direction, cameraUp);

    m_billboard->setRotation(rotationToFaceTarget);
}
