#pragma once

#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DCore/QGeometry>

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
