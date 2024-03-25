#pragma once

#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DCore/QGeometry>

namespace all::qt3d {
class StereoImageMesh : public Qt3DRender::QGeometryRenderer
{
    Q_OBJECT
    Q_PROPERTY(QVector2D viewportSize READ viewportSize WRITE setViewportSize NOTIFY viewportSizeChanged)
    Q_PROPERTY(QVector2D imageSize READ imageSize WRITE setImageSize NOTIFY imageSizeChanged)

public:
    enum class Side {
        Left,
        Right
    };
    Q_ENUM(Side)

    explicit StereoImageMesh(Side side, Qt3DCore::QNode* parent = nullptr);

    Side side() const;

    QVector2D viewportSize() const;
    void setViewportSize(const QVector2D& viewportSize);

    QVector2D imageSize() const;
    void setImageSize(const QVector2D& imageSize);

Q_SIGNALS:
    void viewportSizeChanged(const QVector2D& viewportSize);
    void imageSizeChanged(const QVector2D& imageSize);
};


class StereoImageGeometry : public Qt3DCore::QGeometry
{
    Q_OBJECT
    Q_PROPERTY(QVector2D viewportSize READ viewportSize WRITE setViewportSize NOTIFY viewportSizeChanged)
    Q_PROPERTY(QVector2D imageSize READ imageSize WRITE setImageSize NOTIFY imageSizeChanged)

public:
    explicit StereoImageGeometry(StereoImageMesh::Side side, Qt3DCore::QNode* parent = nullptr);

    StereoImageMesh::Side side() const { return m_side; }

    QVector2D viewportSize() const { return m_viewportSize; }
    void setViewportSize(const QVector2D& viewportSize);

    QVector2D imageSize() const { return m_imageSize; }
    void setImageSize(const QVector2D& imageSize);

Q_SIGNALS:
    void viewportSizeChanged(const QVector2D& viewportSize);
    void imageSizeChanged(const QVector2D& imageSize);

private:
    void updateVertices();

    StereoImageMesh::Side m_side;
    QVector2D m_viewportSize = QVector2D(1, 1);
    QVector2D m_imageSize = QVector2D(1, 1);
    Qt3DCore::QAttribute* m_positionAttribute;
    Qt3DCore::QAttribute* m_texCoordAttribute;
    Qt3DCore::QBuffer* m_vertexBuffer;
};
}

