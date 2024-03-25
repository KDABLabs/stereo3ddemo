#include "qt3d_cursor.h"

#include <Qt3DExtras/QSphereMesh>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DExtras/QPlaneMesh>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QTextureMaterial>
#include <Qt3DRender/QTexture>
#include <Qt3DCore/QTransform>
#include <Qt3DRender/QCameraLens>
#include <Qt3DRender/QPickEvent>

using namespace all::qt3d;

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

all::qt3d::CursorEntity::CursorEntity(QNode* parent, const Qt3DCore::QEntity* camera, Qt3DExtras::Qt3DWindow* window)
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
    int targetSize = 20;
    QMatrix4x4 viewMatrix = m_cameraTransform->matrix();
    QVector3D cameraPosition = viewMatrix.inverted() * m_transform->matrix() * QVector3D(0, 0, 0); // Apply the transformMatrix to the local origin

    float distanceToCamera = cameraPosition.length();

    float fov = 2.0 * atan(1.0 / m_projectionMatrix(0, 0));
    float pixelsToAngle = fov / m_window->width();
    float radius = distanceToCamera * tan(targetSize * pixelsToAngle / 2.0);
    m_transform->setScale(radius); // Set the scale based on the calculated radius

    auto direction = (m_cameraTransform->translation() - m_transform->translation()).normalized();
    QQuaternion rotationToFaceTarget = QQuaternion::rotationTo(QVector3D(0, 1, 0), direction);

    m_transform->setRotation(rotationToFaceTarget);
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
