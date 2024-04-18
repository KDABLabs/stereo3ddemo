#include <stereo_image_material.h>

#include "qt3d_shaders.h"

#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QRenderPass>
#include <Qt3DRender/QShaderProgram>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QTexture>

using namespace Qt3DRender;

StereoImageMaterial::StereoImageMaterial(const QUrl& source, Qt3DCore::QNode* parent)
    : QMaterial(parent)
    , m_texture(new QTextureLoader(this))
{

    auto* effect = new QEffect();

    // GL 3.2
    {
        auto* shader = new QShaderProgram();
        shader->setVertexShaderCode(all::qt3d::stereoImage_vs.data());
        shader->setFragmentShaderCode(all::qt3d::stereoImage_ps.data());

        auto* renderPass = new QRenderPass();
        renderPass->setShaderProgram(shader);

        auto* technique = new QTechnique();
        technique->addRenderPass(renderPass);

        technique->graphicsApiFilter()->setApi(QGraphicsApiFilter::OpenGL);
        technique->graphicsApiFilter()->setMajorVersion(3);
        technique->graphicsApiFilter()->setMinorVersion(2);
        technique->graphicsApiFilter()->setProfile(QGraphicsApiFilter::CoreProfile);

        effect->addTechnique(technique);
    }
    // RHI
    {
        auto* shader = new QShaderProgram();
        shader->setVertexShaderCode(all::qt3d::stereoImage_vs_rhi.data());
        shader->setFragmentShaderCode(all::qt3d::stereoImage_frag_rhi.data());

        auto* renderPass = new QRenderPass();
        renderPass->setShaderProgram(shader);

        auto* technique = new QTechnique();
        technique->addRenderPass(renderPass);

        technique->graphicsApiFilter()->setApi(QGraphicsApiFilter::RHI);
        technique->graphicsApiFilter()->setMajorVersion(1);
        technique->graphicsApiFilter()->setMinorVersion(0);

        effect->addTechnique(technique);
    }

    setEffect(effect);

    m_texture->setSource(source);

    connect(m_texture, &QAbstractTexture::widthChanged, this, &StereoImageMaterial::textureSizeChanged);
    connect(m_texture, &QAbstractTexture::heightChanged, this, &StereoImageMaterial::textureSizeChanged);

    auto* textureParameter = new QParameter(u"stereoImageMap"_qs, m_texture, this);
    addParameter(textureParameter);
}

QVector2D StereoImageMaterial::textureSize() const
{
    return QVector2D(static_cast<float>(m_texture->width()), static_cast<float>(m_texture->height()));
}
