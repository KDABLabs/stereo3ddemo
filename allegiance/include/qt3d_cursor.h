#pragma once
#if !ALLEGIANCE_SERENITY
#include <Qt3DCore/QEntity>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DCore/QTransform>
#include <Qt3DRender/QObjectPicker>
#include <Qt3DRender/QPickEvent>

using namespace Qt3DRender;

class CursorBillboard : public Qt3DCore::QEntity {
    Q_OBJECT
public:
    CursorBillboard(QNode *parent)
    :Qt3DCore::QEntity(parent) {
        m_plane = new Qt3DExtras::QPlaneMesh;
        // rendering is not working as expected, that is why this and setAlphaBlendingEnabled is commented out
        addComponent(m_plane);

        auto m = new Qt3DExtras::QTextureMaterial;
        auto tex = new QTextureLoader{};
        tex->setSource(QUrl("qrc:/cursor_billboard.png"));
        m->setTexture(tex);
        m->setAlphaBlendingEnabled(true);
        m_material = m;
        addComponent(m_material);

        m_transform = new Qt3DCore::QTransform;
        addComponent(m_transform);
        m_transform->setScale(12);
    }

protected:
    Qt3DCore::QTransform *m_transform;
    Qt3DExtras::QPlaneMesh *m_plane;
    QMaterial *m_material;

};

class CursorSphere : public Qt3DCore::QEntity {
    Q_OBJECT
public:
    CursorSphere(QNode* parent = nullptr)
    : Qt3DCore::QEntity(parent) {
        auto* sphereMesh = new Qt3DExtras::QSphereMesh();
        addComponent(sphereMesh);

        auto* material = new Qt3DExtras::QPhongMaterial();
        material->setShininess(100.0);
        material->setDiffuse(QColor("lightyellow"));
        material->setAmbient(QColor("lightyellow"));
        addComponent(material);
    }
};
class CursorEntity : public Qt3DCore::QEntity {
    Q_OBJECT
public:
    //Q_PROPERTY(bool hidden MEMBER hidden)

    CursorEntity(QNode* parent, const Qt3DCore::QEntity* camera, Qt3DExtras::Qt3DWindow *window)
    : QEntity(parent) {

        m_transform = new Qt3DCore::QTransform();

        m_transform->setTranslation({-1, -1, -1});
        addComponent(m_transform);

        connect(window, &Qt3DExtras::Qt3DWindow::widthChanged,
                this, &CursorEntity::updateSize);
        connect(window, &Qt3DExtras::Qt3DWindow::heightChanged,
                this, &CursorEntity::updateSize);
        m_window = window;

        setCamera(camera);

        m_billboard = new CursorBillboard{this};
        m_sphere = new CursorSphere{this};
    }

    void setPosition(const QVector3D& positionInScene) {
        m_transform->setTranslation(positionInScene);
        updateSize();
    }

    void setCamera(const QEntity *camera) {
        m_camera = camera;
        m_cameraLens = nullptr;
        m_cameraTransform = nullptr;
        if (camera == nullptr) return;

        auto ts = camera->componentsOfType<Qt3DCore::QTransform>();
        if (ts.size() > 0)
            m_cameraTransform = ts[0];

        auto ls = camera->componentsOfType<QCameraLens>();
        if (ts.size() > 0)
            m_cameraLens = ls[0];

        connect(m_cameraLens, &QCameraLens::projectionMatrixChanged,
                this, &CursorEntity::onProjectionMatrixChanged);
        connect(m_cameraTransform, &Qt3DCore::QTransform::worldMatrixChanged,
                this, &CursorEntity::onCameraTransformChanged);
    }
public Q_SLOTS:
    void onProjectionMatrixChanged(const QMatrix4x4& matrix) {
        m_projectionMatrix = matrix;
        updateSize();
    }

    void onCameraTransformChanged(const QMatrix4x4& worldMatrix) {
        updateSize();
    }

    void updateSize()
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
protected:
    CursorSphere *m_sphere;
    CursorBillboard *m_billboard;

    Qt3DCore::QTransform *m_transform;
    const Qt3DCore::QEntity *m_camera;
    const Qt3DCore::QTransform *m_cameraTransform;
    const QCameraLens *m_cameraLens;
private:
    QMatrix4x4 m_worldMatrix;
    QMatrix4x4 m_projectionMatrix;

    Qt3DExtras::Qt3DWindow *m_window;
};


class Picker : public Qt3DRender::QObjectPicker
{
    Q_OBJECT

public:
    Q_PROPERTY(bool hidden MEMBER hidden)// isHidden WRITE setHidden NOTIFY hiddenChanged)

    Picker(Qt3DCore::QEntity* parent, CursorEntity *cursor)
        : Qt3DRender::QObjectPicker(parent),
          m_cursor(cursor)
    {
        parent->addComponent(this);

        setHoverEnabled(true);
        setDragEnabled(true);

        QObject::connect(this, &Qt3DRender::QObjectPicker::moved, this, [this](Qt3DRender::QPickEvent* pick) {
            // qDebug() << "move" << parentNode()->objectName();
            m_cursor->setPosition(pick->worldIntersection());
        });

        connect(this, &Qt3DRender::QObjectPicker::entered, []() {
            qDebug("entered");
        });

        connect(this, &Qt3DRender::QObjectPicker::pressed, this, [this]() {
            qDebug() << "press";
            hidden = true;
        });

        QObject::connect(this, &Qt3DRender::QObjectPicker::released, this, [this]() {
            hidden = false;
        });
    }

    Qt3DRender::QObjectPicker *picker;
    bool hidden = false;
    CursorEntity *m_cursor;
};

#endif //!ALLEGIANCE_SERENITY