#pragma once
#include <Qt3DRender/QMaterial>

namespace all::qt3d {
struct shader_textures;
struct shader_uniforms;

class GlossyMaterial : public Qt3DRender::QMaterial
{
    Q_OBJECT
public:
    explicit GlossyMaterial(const all::qt3d::shader_textures& textures, const all::qt3d::shader_uniforms& uniforms, Qt3DCore::QNode* parent = nullptr);
};

class SkyboxMaterial : public Qt3DRender::QMaterial
{
    Q_OBJECT
public:
    explicit SkyboxMaterial(const all::qt3d::shader_textures& textures, const all::qt3d::shader_uniforms& uniforms, Qt3DCore::QNode* parent = nullptr);
};

class CursorBillboardMaterial : public Qt3DRender::QMaterial
{
    Q_OBJECT
public:
    explicit CursorBillboardMaterial(Qt3DCore::QNode* parent = nullptr);

    void setColor(const QColor& color);
    void setTexture(const Qt3DRender::QAbstractTexture* texture);

private:
    Qt3DRender::QParameter* m_colorParameter{ nullptr };
    Qt3DRender::QParameter* m_textureParameter{ nullptr };
};

} // namespace all::qt3d
