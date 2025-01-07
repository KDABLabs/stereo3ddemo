#pragma once

#include <Qt3DCore/QEntity>
#include <QColor>
#include <QMatrix4x4>

namespace Qt3DCore {
class QBuffer;
} // namespace Qt3DCore

namespace all::qt3d {

class FocusPlanePreview : public Qt3DCore::QEntity
{
    Q_OBJECT
public:
    explicit FocusPlanePreview(Qt3DCore::QNode* parent = nullptr);

    void setViewMatrix(const QMatrix4x4& viewMatrix);
    void setProjectionMatrix(const QMatrix4x4& projectionMatrix);
    void setConvergence(const float convergence);

    QMatrix4x4 viewMatrix() const { return m_viewMatrix; }
    QMatrix4x4 projectionMatrix() const { return m_viewMatrix; }
    float convergence() const { return m_convergence; }

signals:
    void viewMatrixChanged();
    void projectionMatrixChanged();
    void convergenceChanged();

private:
    void updateGeometry();
    void updateGeometryQueued();

    QMatrix4x4 m_viewMatrix;
    QMatrix4x4 m_projectionMatrix;
    float m_convergence{ 1.0f };
    bool m_updateRequested{ false };

    Qt3DCore::QBuffer* mVertexBuffer{ nullptr };
};

} // namespace all::qt3d
