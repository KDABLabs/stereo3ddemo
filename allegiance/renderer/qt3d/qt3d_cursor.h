#pragma once
#include <Qt3DCore/QEntity>
#include <Qt3DRender/QObjectPicker>
#include <QMatrix4x4>

#include <QScreenRayCaster>
#include <shared/cursor.h>

namespace all {
class StereoCamera;
}

namespace Qt3DRender {
class QMaterial;
class QCameraLens;
} // namespace Qt3DRender
namespace Qt3DExtras {
class QPlaneMesh;
class Qt3DWindow;
class QDiffuseSpecularMaterial;
class QPhongMaterial;
} // namespace Qt3DExtras
namespace Qt3DCore {
class QTransform;
}

namespace all::qt3d {
class CursorBillboard : public Qt3DCore::QEntity
{
public:
    enum class CursorTexture {
        Default,
        CrossHair,
        Dot
    };

    CursorBillboard(Qt3DCore::QNode* parent);

    void setRotation(const QQuaternion& rotation);
    void setTexture(CursorTexture texture);
    void setCursorTintColor(const QColor& color);

protected:
    Qt3DCore::QTransform* m_transform;
    Qt3DExtras::QPlaneMesh* m_plane;
    Qt3DExtras::QDiffuseSpecularMaterial* m_material;
    QMatrix4x4 m_matrix;
};

class CursorSphere : public Qt3DCore::QEntity
{
public:
    CursorSphere(QNode* parent = nullptr);

public:
    void setCursorTintColor(const QColor& color);

private:
    Qt3DExtras::QPhongMaterial* m_material;
};

class CursorCross : public Qt3DCore::QEntity
{
public:
    CursorCross(QNode* parent = nullptr);

public:
    void setCursorTintColor(const QColor& color);

private:
    Qt3DExtras::QPhongMaterial* m_material;
};

class CursorEntity : public Qt3DCore::QEntity
{
public:
    CursorEntity(QEntity* parent, QEntity* scene, QEntity* camera, Qt3DExtras::Qt3DWindow* window, all::StereoCamera* pCamera);

public:
    void setPosition(const QVector3D& positionInScene);
    QVector3D position() const;

    void setCamera(const QEntity* camera);

    void setType(CursorType type);

    void setCursorTintColor(const QColor& color);

public:
    void onMouseMoveEvent(QVector3D pos, QPoint cursorPosition);

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

    void setScaleFactor(float scale_factor)
    {
        m_scale_factor = scale_factor;
        updateSize();
    }
    float scaleFactor() const noexcept
    {
        return m_scale_factor;
    }

    void setScalingEnabled(bool enabled)
    {
        m_scaling_enabled = enabled;
        updateSize();
    }

protected:
    CursorSphere* m_sphere;
    CursorCross* m_cross;
    CursorBillboard* m_billboard;

    Qt3DRender::QScreenRayCaster* m_raycaster;

    Qt3DCore::QTransform* m_transform;
    const Qt3DCore::QEntity* m_camera;
    const Qt3DCore::QTransform* m_cameraTransform;
    const Qt3DRender::QCameraLens* m_cameraLens;

    float m_scale_factor = 1.0f;
    bool m_scaling_enabled = true;

private:
    QMatrix4x4 m_worldMatrix;
    QMatrix4x4 m_projectionMatrix;

    Qt3DExtras::Qt3DWindow* m_window;
};

} // namespace all::qt3d
