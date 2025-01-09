#pragma once

#include <Qt3DCore/QEntity>
#include <QColor>
#include <QMatrix4x4>

namespace Qt3DCore {
class QBuffer;
} // namespace Qt3DCore

namespace all::qt3d {

class FrustumLines : public Qt3DCore::QEntity
{
    Q_OBJECT
public:
    explicit FrustumLines(const QColor& color, Qt3DCore::QNode* parent = nullptr);

    void setVertices(const std::vector<QVector3D>& vertices);

private:
    Qt3DCore::QBuffer* mVertexBuffer{ nullptr };
};

class FrustumTriangles : public Qt3DCore::QEntity
{
    Q_OBJECT
public:
    explicit FrustumTriangles(const QColor& color, Qt3DCore::QNode* parent = nullptr);

    void setVertices(const std::vector<QVector3D>& vertices);

private:
    Qt3DCore::QBuffer* mVertexBuffer{ nullptr };
};

class Frustum : public Qt3DCore::QEntity
{
    Q_OBJECT
public:
    explicit Frustum(const QColor& color, bool showTriangles, Qt3DCore::QNode* parent = nullptr);

    void setViewMatrix(const QMatrix4x4& viewMatrix);
    void setProjectionMatrix(const QMatrix4x4& projectionMatrix);
    void setConvergence(const float convergence);

    QMatrix4x4 viewMatrix() const { return m_viewMatrix; }
    QMatrix4x4 projectionMatrix() const { return m_viewMatrix; }
    float convergence() const { return m_convergence; }

Q_SIGNALS:
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

    FrustumLines* m_frustumLines{ nullptr };
    FrustumTriangles* m_frustumTriangles{ nullptr };
};

} // namespace all::qt3d
