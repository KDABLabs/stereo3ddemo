#pragma once

#include <Qt3DRender/QMaterial>

namespace Qt3DRender {
class QTextureLoader;
};

class StereoImageMaterial : public Qt3DRender::QMaterial
{
    Q_OBJECT
public:
    explicit StereoImageMaterial(const QUrl& source, Qt3DCore::QNode* parent = nullptr);

    QVector2D textureSize() const;

Q_SIGNALS:
    void textureSizeChanged();

private:
    Qt3DRender::QTextureLoader* m_texture;
};
