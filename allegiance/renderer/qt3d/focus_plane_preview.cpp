#include "focus_plane_preview.h"
#include "qt3d_shaders.h"

#include <Qt3DRender/QBlendEquation>
#include <Qt3DRender/QBlendEquationArguments>
#include <Qt3DRender/QDepthTest>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QLineWidth>
#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QNoDepthMask>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QRenderPass>
#include <Qt3DRender/QShaderProgram>
#include <Qt3DRender/QTechnique>

#include <Qt3DCore/QGeometry>
#include <Qt3DCore/QAttribute>

namespace all::qt3d {

FocusPlanePreview::FocusPlanePreview(QNode* parent)
    : Qt3DCore::QEntity(parent)
    , mVertexBuffer(new Qt3DCore::QBuffer)
{
    QObject::connect(this, &FocusPlanePreview::viewMatrixChanged, this, &FocusPlanePreview::updateGeometryQueued);
    QObject::connect(this, &FocusPlanePreview::projectionMatrixChanged, this, &FocusPlanePreview::updateGeometryQueued);
    QObject::connect(this, &FocusPlanePreview::convergenceChanged, this, &FocusPlanePreview::updateGeometryQueued);

    auto* geometryRenderer = new Qt3DRender::QGeometryRenderer();
    {
        geometryRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Triangles);

        auto* vertexPosAttribute = new Qt3DCore::QAttribute;
        vertexPosAttribute->setBuffer(mVertexBuffer);
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
        shaderProgram->setVertexShaderCode(focus_plane_preview_vs.data());
        shaderProgram->setFragmentShaderCode(focus_plane_preview_ps.data());

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

        material->setEffect(effect);
    }

    addComponent(material);
    addComponent(geometryRenderer);
}

void FocusPlanePreview::setViewMatrix(const QMatrix4x4& viewMatrix)
{
    if (viewMatrix == m_viewMatrix)
        return;
    m_viewMatrix = viewMatrix;
    Q_EMIT viewMatrixChanged();
}

void FocusPlanePreview::setProjectionMatrix(const QMatrix4x4& projectionMatrix)
{
    if (projectionMatrix == m_projectionMatrix)
        return;
    m_projectionMatrix = projectionMatrix;
    Q_EMIT projectionMatrixChanged();
}

void FocusPlanePreview::setConvergence(const float convergence)
{
    if (convergence == m_convergence)
        return;
    m_convergence = convergence;
    Q_EMIT convergenceChanged();
}

void FocusPlanePreview::updateGeometry()
{
    m_updateRequested = false;

    const QMatrix4x4 invViewMatrix = m_viewMatrix.inverted();
    const QVector3D camPosition = (invViewMatrix * QVector4D(0.0f, 0.0f, 0.0f, 1.0)).toVector3D();
    const QVector3D viewVector = (invViewMatrix * QVector4D(0.0f, 0.0f, -1.0f, 0.0f)).toVector3D().normalized();
    const QVector3D viewCenter = camPosition + viewVector * m_convergence;
    const QVector4D viewCenterScreen = m_projectionMatrix * m_viewMatrix * QVector4D(viewCenter, 1.0f);
    const float zFocus = viewCenterScreen.z() / viewCenterScreen.w();

    const std::vector<QVector3D> screenSpaceFocusPlanePreviewPoint{
        // Focus
        QVector3D(-0.5f, 0.5f, zFocus),
        QVector3D(0.5f, 0.5f, zFocus),
        QVector3D(0.5f, -0.5f, zFocus),
        QVector3D(-0.5f, -0.5f, zFocus),
    };

    std::vector<QVector3D> worldFocusPlanePreviewPoints;
    worldFocusPlanePreviewPoints.reserve(screenSpaceFocusPlanePreviewPoint.size());

    const QMatrix4x4 inverseMvp = (m_projectionMatrix * m_viewMatrix).inverted();
    for (const QVector3D& p : screenSpaceFocusPlanePreviewPoint) {
        const QVector4D v = inverseMvp * QVector4D(p, 1.0f);
        const float w = !qIsNaN(v.w()) ? v.w() : 1.0f;
        worldFocusPlanePreviewPoints.push_back(v.toVector3D() / w);
    }

    const std::vector<QVector3D> vertices{
        worldFocusPlanePreviewPoints[0],
        worldFocusPlanePreviewPoints[3],
        worldFocusPlanePreviewPoints[2],
        worldFocusPlanePreviewPoints[2],
        worldFocusPlanePreviewPoints[1],
        worldFocusPlanePreviewPoints[0],
    };

    QByteArray rawVertices;
    rawVertices.resize(vertices.size() * sizeof(QVector3D));
    std::memcpy(rawVertices.data(), vertices.data(), vertices.size() * sizeof(QVector3D));

    mVertexBuffer->setData(rawVertices);
}

void FocusPlanePreview::updateGeometryQueued()
{
    if (m_updateRequested)
        return;
    m_updateRequested = true;
    QMetaObject::invokeMethod(this, &FocusPlanePreview::updateGeometry, Qt::QueuedConnection);
}

} // namespace all::qt3d
