#pragma once

#include <Qt3DCore/QEntity>
#include <QVector3D>

namespace Qt3DCore {
class QBuffer;
} // namespace Qt3DCore

namespace Qt3DRender {
class QCamera;
class QCameraLens;
class QMaterial;
} // namespace Qt3DRender

namespace Qt3DExtras {
class QPlaneMesh;
} // namespace Qt3DExtras

namespace Qt3DCore {
class QTransform;
} // namespace Qt3DCore

namespace Qt3DInput {
class QMouseEvent;
} // namespace Qt3DInput

class QWindow;

namespace all::qt3d {

class FocusArea : public Qt3DCore::QEntity
{
    Q_OBJECT
public:
    explicit FocusArea(Qt3DCore::QNode* parent);

    QVector3D center() const;
    void setCenter(const QVector3D& center);

    QVector3D extent() const;
    void setExtent(const QVector3D& extent);

    void setViewSize(const QSize viewSize);
    QSize viewSize() const;

    void setCamera(const Qt3DRender::QCamera* camera);

    bool containsMouse() const { return m_containedArea != ContainedArea::None; }
    void update();

signals:
    void centerChanged();
    void extentChanged();
    void viewSizeChanged();

private:
    Q_INVOKABLE void updateMesh();

    void onMousePressed(Qt3DInput::QMouseEvent* mouse);
    void onMouseMoved(Qt3DInput::QMouseEvent* mouse);
    void onMouseReleased(Qt3DInput::QMouseEvent* mouse);
    void updateContainsMouse(Qt3DInput::QMouseEvent* mouse);

    Qt3DCore::QBuffer* m_buffer{ nullptr };
    const Qt3DRender::QCamera* m_camera{ nullptr };

    QVector3D m_center; // in px
    QVector3D m_extent; // in px

    enum class ContainedArea {
        Center,
        Resize,
        None,
    };
    ContainedArea m_containedArea = ContainedArea::None;

    enum class Operation {
        Translating,
        Scaling,
        None,
    };
    Operation m_operation = Operation::None;
    QVector3D m_distToCenterOnPress;
    QVector3D m_extentOnPress;
    QSize m_viewSize{ 1, 1 };
    bool m_updateRequested = false;
};

} // namespace all::qt3d
