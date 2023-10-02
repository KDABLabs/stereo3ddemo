#pragma once
#include "qt3d_shaders.h"
#include "sliders.h"
#include <QWidget>
#include <QBoxLayout>
#include <QGroupBox>

namespace all {
inline std::unique_ptr<QWidget> ControlWindow(Qt3DRender::QMaterial* mat)
{
    QWidget* w = new QWidget;
    QVBoxLayout* l = new QVBoxLayout;
    for (auto* p : mat->parameters()) {
        const auto& val = p->value();
        if (val.type() == QMetaType::Float) {
            auto* s = new all::FloatSlider(val.toFloat(), p->name(), 0.0f, 4.0f, w);
            QObject::connect(s, &all::FloatSlider::OnValueChanged, [p](float v) { p->setValue(v); });
            l->addWidget(s);
            continue;
        }

        if (val.type() == QVariant::Vector3D) {
            QGroupBox* gb = new QGroupBox(p->name(), w);
            QHBoxLayout* l1 = new QHBoxLayout;

            auto valv = val.value<QVector3D>();
            auto* s1 = new all::FloatSlider(valv.x(), "x", -1.0f, 2.0f, w);
            auto* s2 = new all::FloatSlider(valv.y(), "y", -1.0f, 2.0f, w);
            auto* s3 = new all::FloatSlider(valv.z(), "z", -1.0f, 2.0f, w);
            QObject::connect(s1, &all::FloatSlider::OnValueChanged, [p](float v) { auto valv = p->value().value<QVector3D>(); p->setValue(QVector3D(v, valv.y(), valv.z())); });
            QObject::connect(s2, &all::FloatSlider::OnValueChanged, [p](float v) { auto valv = p->value().value<QVector3D>(); p->setValue(QVector3D(valv.x(), v, valv.z())); });
            QObject::connect(s3, &all::FloatSlider::OnValueChanged, [p](float v) { auto valv = p->value().value<QVector3D>(); p->setValue(QVector3D(valv.x(), valv.y(), v)); });

            l1->addWidget(s1);
            l1->addWidget(s2);
            l1->addWidget(s3);
            gb->setLayout(l1);
            l->addWidget(gb);
        }
    }
    w->setLayout(l);
    return std::unique_ptr<QWidget>{ w };
}

class GlossyMaterial : public Qt3DRender::QMaterial
{
public:
    GlossyMaterial(const all::shader_textures& textures, const all::shader_uniforms& uniforms, Qt3DCore::QNode* parent = nullptr)
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

        auto* shader = new Qt3DRender::QShaderProgram(this);
        shader->setVertexShaderCode(all::fresnel_vs.data());
        shader->setFragmentShaderCode(all::fresnel_ps.data());

        auto* rp = new Qt3DRender::QRenderPass(this);
        rp->setShaderProgram(shader);

        auto* t = new Qt3DRender::QTechnique(this);
        t->graphicsApiFilter()->setApi(Qt3DRender::QGraphicsApiFilter::OpenGL);
        t->graphicsApiFilter()->setProfile(Qt3DRender::QGraphicsApiFilter::CoreProfile);
        t->graphicsApiFilter()->setMajorVersion(3);
        t->graphicsApiFilter()->setMinorVersion(2);
        t->addRenderPass(rp);

        auto* effect = new Qt3DRender::QEffect(this);
        effect->addTechnique(t);
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
};

class SkyboxMaterial : public Qt3DRender::QMaterial
{
public:
    SkyboxMaterial(const all::shader_textures& textures, const all::shader_uniforms& uniforms, Qt3DCore::QNode* parent = nullptr)
        : Qt3DRender::QMaterial(parent) 
    {
        auto make_uniform = [this](QString name, const QVariant& val) {
            auto p = new Qt3DRender::QParameter(name, val, this);
            addParameter(p);
        };

        make_uniform(u"postGain"_qs, 1.0f);
        make_uniform(u"gammax"_qs, 1.2);

        auto* shader = new Qt3DRender::QShaderProgram(this);
        shader->setVertexShaderCode(all::skybox_vs.data());
        shader->setFragmentShaderCode(all::skybox_ps.data());

        auto* rp = new Qt3DRender::QRenderPass(this);
        rp->setShaderProgram(shader);

        auto* t = new Qt3DRender::QTechnique(this);
        t->graphicsApiFilter()->setApi(Qt3DRender::QGraphicsApiFilter::OpenGL);
        t->graphicsApiFilter()->setProfile(Qt3DRender::QGraphicsApiFilter::CoreProfile);
        t->graphicsApiFilter()->setMajorVersion(3);
        t->graphicsApiFilter()->setMinorVersion(2);
        t->addRenderPass(rp);

        auto* effect = new Qt3DRender::QEffect(this);
        effect->addTechnique(t);
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
};

} // namespace all