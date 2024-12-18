#include "frustum_rect.h"
#include "qt3d_shaders.h"

#include <Qt3DRender/QBlendEquation>
#include <Qt3DRender/QBlendEquationArguments>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QRenderPass>
#include <Qt3DRender/QShaderProgram>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QNoDepthMask>

#include <Qt3DCore/QBuffer>
#include <Qt3DCore/QGeometry>
#include <Qt3DCore/QAttribute>

namespace all::qt3d {

FrustumRect::FrustumRect(QNode* parent)
    : Qt3DCore::QEntity(parent)
{
    auto* geometryRenderer = new Qt3DRender::QGeometryRenderer();
    {
        geometryRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Triangles);

        QByteArray rawVertexData;
        rawVertexData.resize(6 * sizeof(QVector3D));
        QVector3D* vertices = reinterpret_cast<QVector3D*>(rawVertexData.data());
        vertices[0] = QVector3D(-1.0f, 1.0f, 0.0f);
        vertices[1] = QVector3D(-1.0f, -1.0f, 0.0f);
        vertices[2] = QVector3D(1.0f, -1.0f, 0.0f);
        vertices[3] = QVector3D(1.0f, -1.0f, 0.0f);
        vertices[4] = QVector3D(1.0f, 1.0f, 0.0f);
        vertices[5] = QVector3D(-1.0f, 1.0f, 0.0f);

        auto* vertexBuffer = new Qt3DCore::QBuffer;
        vertexBuffer->setData(rawVertexData);

        auto* vertexPosAttribute = new Qt3DCore::QAttribute;
        vertexPosAttribute->setBuffer(vertexBuffer);
        vertexPosAttribute->setAttributeType(Qt3DCore::QAttribute::VertexAttribute);
        vertexPosAttribute->setVertexBaseType(Qt3DCore::QAttribute::VertexBaseType::Float);
        vertexPosAttribute->setVertexSize(3);
        vertexPosAttribute->setByteStride(3 * sizeof(float));
        vertexPosAttribute->setCount(6);
        vertexPosAttribute->setName(Qt3DCore::QAttribute::defaultPositionAttributeName());

        auto* geometry = new Qt3DCore::QGeometry;
        geometry->addAttribute(vertexPosAttribute);

        geometryRenderer->setGeometry(geometry);
    }

    auto* material = new Qt3DRender::QMaterial;
    {
        auto* shaderProgram = new Qt3DRender::QShaderProgram;
        shaderProgram->setVertexShaderCode(frustum_rect_vs.data());
        shaderProgram->setFragmentShaderCode(frustum_rect_ps.data());

        auto* renderPass = new Qt3DRender::QRenderPass;
        renderPass->setShaderProgram(shaderProgram);

        auto* blendState = new Qt3DRender::QBlendEquationArguments();
        blendState->setSourceRgb(Qt3DRender::QBlendEquationArguments::SourceAlpha);
        blendState->setDestinationRgb(Qt3DRender::QBlendEquationArguments::OneMinusSourceAlpha);

        auto* blendEquation = new Qt3DRender::QBlendEquation();
        blendEquation->setBlendFunction(Qt3DRender::QBlendEquation::Add);

        auto* noDepthMask = new Qt3DRender::QNoDepthMask();

        renderPass->addRenderState(blendState);
        renderPass->addRenderState(blendEquation);
        renderPass->addRenderState(noDepthMask);

        auto* technique = new Qt3DRender::QTechnique;
        technique->addRenderPass(renderPass);
        technique->graphicsApiFilter()->setApi(Qt3DRender::QGraphicsApiFilter::OpenGL);
        technique->graphicsApiFilter()->setProfile(Qt3DRender::QGraphicsApiFilter::CoreProfile);
        technique->graphicsApiFilter()->setMajorVersion(3);
        technique->graphicsApiFilter()->setMinorVersion(2);

        auto* effect = new Qt3DRender::QEffect;
        effect->addTechnique(technique);

        auto* backgroundColorParameter = new Qt3DRender::QParameter;
        backgroundColorParameter->setName(QStringLiteral("backgroundColor"));
        backgroundColorParameter->setValue(m_backgroundColor);

        auto* outlineColorParameter = new Qt3DRender::QParameter;
        outlineColorParameter->setName(QStringLiteral("outlineColor"));
        outlineColorParameter->setValue(m_outlineColor);

        auto* outlineWidthParameter = new Qt3DRender::QParameter;
        outlineWidthParameter->setName(QStringLiteral("outlineWidth"));
        outlineWidthParameter->setValue(m_outlineWidth);

        material->addParameter(backgroundColorParameter);
        material->addParameter(outlineColorParameter);
        material->addParameter(outlineWidthParameter);
        material->setEffect(effect);
    }

    addComponent(material);
    addComponent(geometryRenderer);
}

} // namespace all::qt3d
