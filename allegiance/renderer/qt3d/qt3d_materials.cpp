#include "qt3d_materials.h"
#include "qt3d_shaders.h"

#include <Qt3DRender/QTexture>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QShaderProgram>
#include <Qt3DRender/QRenderPass>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QNoDepthMask>
#include <Qt3DRender/QDepthTest>
#include <Qt3DRender/QBlendEquationArguments>
#include <Qt3DRender/QBlendEquation>

using namespace all::qt3d;

all::qt3d::GlossyMaterial::GlossyMaterial(const all::qt3d::shader_textures& textures, const all::qt3d::shader_uniforms& uniforms, Qt3DCore::QNode* parent)
    : Qt3DRender::QMaterial(parent)
{
    auto make_uniform = [this](QString name, const QVariant& val) {
        auto p = new Qt3DRender::QParameter(name, val, this);
        addParameter(p);
    };

    make_uniform(QStringLiteral("normalMapGain"), 2.0f);

    make_uniform(QStringLiteral("semInner"), uniforms.semInner);
    make_uniform(QStringLiteral("semOuter"), uniforms.semOuter);
    make_uniform(QStringLiteral("semGain"), uniforms.semGain);

    make_uniform(QStringLiteral("difInner"), uniforms.difInner);
    make_uniform(QStringLiteral("difOuter"), uniforms.difOuter);
    make_uniform(QStringLiteral("difGain"), uniforms.difGain);

    make_uniform(QStringLiteral("normalScaling"), uniforms.normalScaling);
    make_uniform(QStringLiteral("postVertexColor"), uniforms.postVertexColor);
    make_uniform(QStringLiteral("postGain"), 1.0f);
    make_uniform(QStringLiteral("gammax"), 1.2);

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

    make_texture(QStringLiteral("semMap"), texture);
    make_texture(QStringLiteral("diffuseMap"), texture2);
    make_texture(QStringLiteral("normalMap"), texture3);
}

SkyboxMaterial::SkyboxMaterial(const all::qt3d::shader_textures& textures, const all::qt3d::shader_uniforms& uniforms, Qt3DCore::QNode* parent)
    : Qt3DRender::QMaterial(parent)
{
    auto make_uniform = [this](QString name, const QVariant& val) {
        auto p = new Qt3DRender::QParameter(name, val, this);
        addParameter(p);
    };

    make_uniform(QStringLiteral("postGain"), 1.0f);
    make_uniform(QStringLiteral("gammax"), 1.2);

    auto* effect = new Qt3DRender::QEffect();

    // GL 3.2
    {
        auto* shader = new Qt3DRender::QShaderProgram();
        shader->setVertexShaderCode(all::qt3d::skybox_vs.data());
        shader->setFragmentShaderCode(all::qt3d::skybox_ps.data());

        auto* rp = new Qt3DRender::QRenderPass();
        rp->setShaderProgram(shader);

        auto* noDepthWrite = new Qt3DRender::QNoDepthMask{};

        auto* depthState = new Qt3DRender::QDepthTest{};
        depthState->setDepthFunction(Qt3DRender::QDepthTest::Less);

        rp->addRenderState(noDepthWrite);
        rp->addRenderState(depthState);

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

        auto* noDepthWrite = new Qt3DRender::QNoDepthMask{};

        auto* depthState = new Qt3DRender::QDepthTest{};
        depthState->setDepthFunction(Qt3DRender::QDepthTest::Less);

        rp->addRenderState(noDepthWrite);
        rp->addRenderState(depthState);

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

    make_texture(QStringLiteral("diffuseMap"), texture2);
}

CursorBillboardMaterial::CursorBillboardMaterial(Qt3DCore::QNode* parent)
    : Qt3DRender::QMaterial(parent)
{
    auto* effect = new Qt3DRender::QEffect();

    // GL 3.2
    {
        auto* shader = new Qt3DRender::QShaderProgram();
        shader->setVertexShaderCode(all::qt3d::cursor_billboard_vs.data());
        shader->setFragmentShaderCode(all::qt3d::cursor_billboard_frag.data());

        auto* rp = new Qt3DRender::QRenderPass();
        rp->setShaderProgram(shader);

        auto* noDepthWrite = new Qt3DRender::QNoDepthMask{};

        auto* depthState = new Qt3DRender::QDepthTest{};
        depthState->setDepthFunction(Qt3DRender::QDepthTest::LessOrEqual);

        auto* blendState = new Qt3DRender::QBlendEquationArguments{};
        blendState->setSourceRgb(Qt3DRender::QBlendEquationArguments::SourceAlpha);
        blendState->setSourceAlpha(Qt3DRender::QBlendEquationArguments::SourceAlpha);
        blendState->setDestinationRgb(Qt3DRender::QBlendEquationArguments::OneMinusSourceAlpha);
        blendState->setDestinationAlpha(Qt3DRender::QBlendEquationArguments::OneMinusSourceAlpha);

        auto* blendEquation = new Qt3DRender::QBlendEquation{};
        blendEquation->setBlendFunction(Qt3DRender::QBlendEquation::Add);

        rp->addRenderState(noDepthWrite);
        rp->addRenderState(depthState);
        rp->addRenderState(blendState);
        rp->addRenderState(blendEquation);

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
        shader->setVertexShaderCode(all::qt3d::cursor_billboard_vs_rhi.data());
        shader->setFragmentShaderCode(all::qt3d::cursor_billboard_frag_rhi.data());

        auto* rp = new Qt3DRender::QRenderPass();
        rp->setShaderProgram(shader);

        auto* noDepthWrite = new Qt3DRender::QNoDepthMask{};

        auto* depthState = new Qt3DRender::QDepthTest{};
        depthState->setDepthFunction(Qt3DRender::QDepthTest::LessOrEqual);

        auto* blendState = new Qt3DRender::QBlendEquationArguments{};
        blendState->setSourceRgb(Qt3DRender::QBlendEquationArguments::SourceAlpha);
        blendState->setSourceAlpha(Qt3DRender::QBlendEquationArguments::SourceAlpha);
        blendState->setDestinationRgb(Qt3DRender::QBlendEquationArguments::OneMinusSourceAlpha);
        blendState->setDestinationAlpha(Qt3DRender::QBlendEquationArguments::OneMinusSourceAlpha);

        auto* blendEquation = new Qt3DRender::QBlendEquation{};
        blendEquation->setBlendFunction(Qt3DRender::QBlendEquation::Add);

        rp->addRenderState(noDepthWrite);
        rp->addRenderState(depthState);
        rp->addRenderState(blendState);
        rp->addRenderState(blendEquation);

        auto* t = new Qt3DRender::QTechnique();
        t->graphicsApiFilter()->setApi(Qt3DRender::QGraphicsApiFilter::RHI);
        t->graphicsApiFilter()->setMajorVersion(1);
        t->graphicsApiFilter()->setMinorVersion(0);
        t->addRenderPass(rp);

        effect->addTechnique(t);
    }

    m_colorParameter = new Qt3DRender::QParameter(QStringLiteral("color"), QColor(Qt::white));
    m_textureParameter = new Qt3DRender::QParameter(QStringLiteral("cursor_texture"), QVariant());

    effect->addParameter(m_colorParameter);
    effect->addParameter(m_textureParameter);

    setEffect(effect);
}

void CursorBillboardMaterial::setColor(const QColor& color)
{
    m_colorParameter->setValue(color);
}

void CursorBillboardMaterial::setTexture(const Qt3DRender::QAbstractTexture* texture)
{
    m_textureParameter->setValue(QVariant::fromValue(texture));
}
