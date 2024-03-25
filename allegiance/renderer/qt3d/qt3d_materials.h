#pragma once
#include <Qt3DRender/QMaterial>

namespace all::qt3d {
struct shader_textures;
struct shader_uniforms;

struct GlossyMaterial : public Qt3DRender::QMaterial {
    GlossyMaterial(const all::qt3d::shader_textures& textures, const all::qt3d::shader_uniforms& uniforms, Qt3DCore::QNode* parent = nullptr);
};

class SkyboxMaterial : public Qt3DRender::QMaterial
{
public:
    SkyboxMaterial(const all::qt3d::shader_textures& textures, const all::qt3d::shader_uniforms& uniforms, Qt3DCore::QNode* parent = nullptr);
};
} // namespace all::qt3d