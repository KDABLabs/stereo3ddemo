#pragma once
#include <Qt3DCore/QEntity>
#include <Qt3DRender/QObjectPicker>
#include <QMatrix4x4>

#include <ui/camera_controller.h>

namespace Qt3DRender {
class QMaterial;
class QCameraLens;
} // namespace Qt3DRender
namespace Qt3DExtras {
class QPlaneMesh;
class Qt3DWindow;
} // namespace Qt3DExtras
namespace Qt3DCore {
class QTransform;
}

namespace all::qt3d {
class CursorBillboard : public Qt3DCore::QEntity
{
public:
    CursorBillboard(Qt3DCore::QNode* parent);

    void setRotation(const QQuaternion &rotation);

protected:
    Qt3DCore::QTransform* m_transform;
    Qt3DExtras::QPlaneMesh* m_plane;
    Qt3DRender::QMaterial* m_material;
};

class CursorSphere : public Qt3DCore::QEntity
{
public:
    CursorSphere(QNode* parent = nullptr);
};

class CursorCross : public Qt3DCore::QEntity
{
public:
    CursorCross(QNode* parent = nullptr);
};

class CursorEntity : public Qt3DCore::QEntity
{
public:
    CursorEntity(QNode* parent, const Qt3DCore::QEntity* camera, Qt3DExtras::Qt3DWindow* window, CursorController *cursorController);

public:
    void setPosition(const QVector3D& positionInScene);

    void setCamera(const QEntity* camera);

    void setType(CursorController::CursorType type);

public:
    void onProjectionMatrixChanged(const QMatrix4x4& matrix)
    {
        m_projectionMatrix = matrix;
        updateSize();
    }

    void onCameraTransformChanged(const QMatrix4x4& worldMatrix)
    {
        updateSize();
    }

    void updateSize();

protected:
    CursorSphere* m_sphere;
    CursorCross* m_cross;
    CursorBillboard* m_billboard;

    Qt3DCore::QTransform* m_transform;
    const Qt3DCore::QEntity* m_camera;
    const Qt3DCore::QTransform* m_cameraTransform;
    const Qt3DRender::QCameraLens* m_cameraLens;

private:
    QMatrix4x4 m_worldMatrix;
    QMatrix4x4 m_projectionMatrix;

    Qt3DExtras::Qt3DWindow* m_window;
};

class Picker : public Qt3DRender::QObjectPicker
{
    Q_OBJECT
public:
    Q_PROPERTY(bool hidden MEMBER hidden)
public:
    Picker(Qt3DCore::QEntity* parent, CursorEntity* cursor);

public:
    Qt3DRender::QObjectPicker* picker;
    bool hidden = false;
    CursorEntity* m_cursor;
    QVector3D cursor_world;
};

} // namespace all::qt3d
