#include "qt3d_materials.h"
#include "qt3d_shaders.h"

#include <Qt3DRender/QTexture>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QShaderProgram>
#include <Qt3DRender/QRenderPass>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QGraphicsApiFilter>


using namespace all::qt3d;

all::qt3d::GlossyMaterial::GlossyMaterial(const all::qt3d::shader_textures& textures, const all::qt3d::shader_uniforms& uniforms, Qt3DCore::QNode* parent)
    : Qt3DRender::QMaterial(parent)
{
    auto make_uniform = [this](QString name, const QVariant& val) {
        auto p = new Qt3DRender::QParameter(name, val, this);
        addParameter(p);
    };

    make_uniform(u"normalMapGain"_qs, 2.0f);

    make_uniform(u"semInner"_qs, uniforms.semInner);
    make_uniform(u"semOuter"_qs, uniforms.semOuter);
    make_uniform(u"semGain"_qs, uniforms.semGain);

    make_uniform(u"difInner"_qs, uniforms.difInner);
    make_uniform(u"difOuter"_qs, uniforms.difOuter);
    make_uniform(u"difGain"_qs, uniforms.difGain);

    make_uniform(u"normalScaling"_qs, uniforms.normalScaling);
    make_uniform(u"postVertexColor"_qs, uniforms.postVertexColor);
    make_uniform(u"postGain"_qs, 1.0f);
    make_uniform(u"gammax"_qs, 1.2);

    auto* effect = new Qt3DRender::QEffect(this);

    // GL 3.2
    {
        auto* shader = new Qt3DRender::QShaderProgram();
        shader->setVertexShaderCode(all::qt3d::fresnel_vs.data());
        shader->setFragmentShaderCode(all::qt3d::fresnel_ps.data());

        auto* rp = new Qt3DRender::QRenderPass();
        rp->setShaderProgram(shader);

        auto* t = new Qt3DRender::QTechnique();
        t->graphicsApiFilter()->setApi(Qt3DRender::QGraphicsApiFilter::OpenGL);
        t->graphicsApiFilter()->setProfile(Qt3DRender::QGraphicsApiFilter::CoreProfile);
        t->graphicsApiFilter()->setMajorVersion(3);
        t->graphicsApiFilter()->setMinorVersion(2);
        t->addRenderPass(rp);

        effect->addTechnique(t);
    }
    // RHI
    {
        auto* shader = new Qt3DRender::QShaderProgram();
        shader->setVertexShaderCode(all::qt3d::fresnel_vs_rhi.data());
        shader->setFragmentShaderCode(all::qt3d::fresnel_frag_rhi.data());

        auto* rp = new Qt3DRender::QRenderPass();
        rp->setShaderProgram(shader);

        auto* t = new Qt3DRender::QTechnique();
        t->graphicsApiFilter()->setApi(Qt3DRender::QGraphicsApiFilter::RHI);
        t->graphicsApiFilter()->setMajorVersion(1);
        t->graphicsApiFilter()->setMinorVersion(0);
        t->addRenderPass(rp);

        effect->addTechnique(t);
    }
    setEffect(effect);

    ///////////////////////////////////////////////////////////////////////

    auto texture = new Qt3DRender::QTexture2D(this);
    texture->setFormat(Qt3DRender::QAbstractTexture::TextureFormat::RGBA8_UNorm);
    texture->setGenerateMipMaps(true);
    texture->setMagnificationFilter(Qt3DRender::QAbstractTexture::Linear);
    texture->setMinificationFilter(Qt3DRender::QAbstractTexture::LinearMipMapLinear);
    texture->setMaximumAnisotropy(16.0f);
    auto image = new Qt3DRender::QTextureImage(texture);
    image->setSource(QUrl::fromLocalFile(textures.semMap.toString()));
    texture->addTextureImage(image);

    auto texture2 = new Qt3DRender::QTexture2D(this);
    texture2->setMinificationFilter(Qt3DRender::QAbstractTexture::Nearest);
    texture2->setMagnificationFilter(Qt3DRender::QAbstractTexture::Linear);
    texture2->setGenerateMipMaps(true);
    texture2->setWrapMode(Qt3DRender::QTextureWrapMode{ Qt3DRender::QTextureWrapMode::Repeat });
    texture2->setMaximumAnisotropy(16.0f);
    texture2->setFormat(Qt3DRender::QAbstractTexture::TextureFormat::RGBA8_UNorm);
    auto image2 = new Qt3DRender::QTextureImage(texture2);
    image2->setSource(QUrl::fromLocalFile(textures.diffuseMap.toString()));
    texture2->addTextureImage(image2);

    auto texture3 = new Qt3DRender::QTexture2D(this);
    texture3->setMinificationFilter(Qt3DRender::QAbstractTexture::Nearest);
    texture3->setFormat(Qt3DRender::QAbstractTexture::TextureFormat::RGBA8_UNorm);
    texture3->setWrapMode(Qt3DRender::QTextureWrapMode{ Qt3DRender::QTextureWrapMode::Repeat });
    auto image3 = new Qt3DRender::QTextureImage(texture3);
    image3->setSource(QUrl::fromLocalFile(textures.normalMap.toString()));
    texture3->addTextureImage(image3);

    auto make_texture = [this](QString name, Qt3DRender::QAbstractTexture* texture) {
        auto p = new Qt3DRender::QParameter(name, texture, this);
        addParameter(p);
    };

    make_texture(u"semMap"_qs, texture);
    make_texture(u"diffuseMap"_qs, texture2);
    make_texture(u"normalMap"_qs, texture3);
}


SkyboxMaterial::SkyboxMaterial(const all::qt3d::shader_textures& textures, const all::qt3d::shader_uniforms& uniforms, Qt3DCore::QNode* parent)
    : Qt3DRender::QMaterial(parent)
{
    auto make_uniform = [this](QString name, const QVariant& val) {
        auto p = new Qt3DRender::QParameter(name, val, this);
        addParameter(p);
    };

    make_uniform(u"postGain"_qs, 1.0f);
    make_uniform(u"gammax"_qs, 1.2);

auto* effect = new Qt3DRender::QEffect();

    // GL 3.2
    {
        auto* shader = new Qt3DRender::QShaderProgram();
        shader->setVertexShaderCode(all::qt3d::skybox_vs.data());
        shader->setFragmentShaderCode(all::qt3d::skybox_ps.data());

        auto* rp = new Qt3DRender::QRenderPass();
        rp->setShaderProgram(shader);

        auto* t = new Qt3DRender::QTechnique();
        t->graphicsApiFilter()->setApi(Qt3DRender::QGraphicsApiFilter::OpenGL);
        t->graphicsApiFilter()->setProfile(Qt3DRender::QGraphicsApiFilter::CoreProfile);
        t->graphicsApiFilter()->setMajorVersion(3);
        t->graphicsApiFilter()->setMinorVersion(2);
        t->addRenderPass(rp);

        effect->addTechnique(t);
    }
    // RHI
    {
        auto* shader = new Qt3DRender::QShaderProgram();
        shader->setVertexShaderCode(all::qt3d::skybox_vs_rhi.data());
        shader->setFragmentShaderCode(all::qt3d::skybox_frag_rhi.data());

        auto* rp = new Qt3DRender::QRenderPass();
        rp->setShaderProgram(shader);

        auto* t = new Qt3DRender::QTechnique();
        t->graphicsApiFilter()->setApi(Qt3DRender::QGraphicsApiFilter::RHI);
        t->graphicsApiFilter()->setMajorVersion(1);
        t->graphicsApiFilter()->setMinorVersion(0);
        t->addRenderPass(rp);

        effect->addTechnique(t);
    }
    setEffect(effect);

    ///////////////////////////////////////////////////////////////////////

    auto texture2 = new Qt3DRender::QTexture2D(this);
    texture2->setMinificationFilter(Qt3DRender::QAbstractTexture::Nearest);
    texture2->setMagnificationFilter(Qt3DRender::QAbstractTexture::Linear);
    texture2->setGenerateMipMaps(true);
    texture2->setWrapMode(Qt3DRender::QTextureWrapMode{ Qt3DRender::QTextureWrapMode::Repeat });
    texture2->setMaximumAnisotropy(16.0f);
    texture2->setFormat(Qt3DRender::QAbstractTexture::TextureFormat::RGBA8_UNorm);
    auto image2 = new Qt3DRender::QTextureImage(texture2);
    image2->setSource(QUrl::fromLocalFile(textures.diffuseMap.toString()));
    texture2->addTextureImage(image2);

    auto make_texture = [this](QString name, Qt3DRender::QAbstractTexture* texture) {
        auto p = new Qt3DRender::QParameter(name, texture, this);
        addParameter(p);
    };

    make_texture(u"diffuseMap"_qs, texture2);
}