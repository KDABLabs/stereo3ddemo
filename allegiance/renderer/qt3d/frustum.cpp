#include "frustum.h"
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

#include <QUrl>

namespace all::qt3d {

Frustum::Frustum(const QColor& color, bool showTriangles, QNode* parent)
    : Qt3DCore::QEntity(parent)
{
    QObject::connect(this, &Frustum::viewMatrixChanged, this, &Frustum::updateGeometryQueued);
    QObject::connect(this, &Frustum::projectionMatrixChanged, this, &Frustum::updateGeometryQueued);
    QObject::connect(this, &Frustum::convergenceChanged, this, &Frustum::updateGeometryQueued);

    m_frustumLines = new FrustumLines(QColor(Qt::white));
    m_frustumLines->setParent(this);

    if (showTriangles) {
        m_frustumTriangles = new FrustumTriangles(color);
        m_frustumTriangles->setParent(this);
    }
}

void Frustum::setViewMatrix(const QMatrix4x4& viewMatrix)
{
    if (viewMatrix == m_viewMatrix)
        return;
    m_viewMatrix = viewMatrix;
    emit viewMatrixChanged();
}

void Frustum::setProjectionMatrix(const QMatrix4x4& projectionMatrix)
{
    if (projectionMatrix == m_projectionMatrix)
        return;
    m_projectionMatrix = projectionMatrix;
    emit projectionMatrixChanged();
}

void Frustum::setConvergence(const float convergence)
{
    if (convergence == m_convergence)
        return;
    m_convergence = convergence;
    emit convergenceChanged();
}

void Frustum::updateGeometry()
{
    m_updateRequested = false;

    const QMatrix4x4 invViewMatrix = m_viewMatrix.inverted();
    const QVector3D camPosition = (invViewMatrix * QVector4D(0.0f, 0.0f, 0.0f, 1.0)).toVector3D();
    const QVector3D viewVector = (invViewMatrix * QVector4D(0.0f, 0.0f, -1.0f, 0.0f)).toVector3D().normalized();
    const QVector3D viewCenter = camPosition + viewVector * m_convergence;
    const QVector4D viewCenterScreen = m_projectionMatrix * m_viewMatrix * QVector4D(viewCenter, 1.0f);
    const float zFocus = viewCenterScreen.z() / viewCenterScreen.w();

    const std::vector<QVector3D> screenSpaceFrustumPoint{
        // Near
        QVector3D(-1.0f, 1.0f, -1.0f),
        QVector3D(1.0f, 1.0f, -1.0f),
        QVector3D(1.0f, -1.0f, -1.0f),
        QVector3D(-1.0f, -1.0f, -1.0f),
        // Focus
        QVector3D(-1.0f, 1.0f, zFocus),
        QVector3D(1.0f, 1.0f, zFocus),
        QVector3D(1.0f, -1.0f, zFocus),
        QVector3D(-1.0f, -1.0f, zFocus),
        // Far
        QVector3D(-1.0f, 1.0f, 1.0f),
        QVector3D(1.0f, 1.0f, 1.0f),
        QVector3D(1.0f, -1.0f, 1.0f),
        QVector3D(-1.0f, -1.0f, 1.0f),
    };

    std::vector<QVector3D> worldFrustumPoints;
    worldFrustumPoints.reserve(screenSpaceFrustumPoint.size());

    const QMatrix4x4 inverseMvp = (m_projectionMatrix * m_viewMatrix).inverted();
    for (const QVector3D& p : screenSpaceFrustumPoint) {
        const QVector4D v = inverseMvp * QVector4D(p, 1.0f);
        const float w = !qIsNaN(v.w()) ? v.w() : 1.0f;
        worldFrustumPoints.push_back(v.toVector3D() / w);
    }

    const std::vector<QVector3D> frustumLineVertices{
        worldFrustumPoints[0],
        worldFrustumPoints[1],
        worldFrustumPoints[1],
        worldFrustumPoints[2],
        worldFrustumPoints[2],
        worldFrustumPoints[3],
        worldFrustumPoints[3],
        worldFrustumPoints[0],

        worldFrustumPoints[4],
        worldFrustumPoints[5],
        worldFrustumPoints[5],
        worldFrustumPoints[6],
        worldFrustumPoints[6],
        worldFrustumPoints[7],
        worldFrustumPoints[7],
        worldFrustumPoints[4],

        worldFrustumPoints[8],
        worldFrustumPoints[9],
        worldFrustumPoints[9],
        worldFrustumPoints[10],
        worldFrustumPoints[10],
        worldFrustumPoints[11],
        worldFrustumPoints[11],
        worldFrustumPoints[8],

        worldFrustumPoints[0],
        worldFrustumPoints[8],
        worldFrustumPoints[1],
        worldFrustumPoints[9],
        worldFrustumPoints[2],
        worldFrustumPoints[10],
        worldFrustumPoints[3],
        worldFrustumPoints[11],

        camPosition,
        viewCenter,
    };
    m_frustumLines->setVertices(frustumLineVertices);

    if (m_frustumTriangles != nullptr) {
        const std::vector<QVector3D> frustumTriangleVertices{
            worldFrustumPoints[3],
            worldFrustumPoints[2],
            worldFrustumPoints[11],

            worldFrustumPoints[2],
            worldFrustumPoints[10],
            worldFrustumPoints[11],
        };
        m_frustumTriangles->setVertices(frustumTriangleVertices);
    }
}

void Frustum::updateGeometryQueued()
{
    if (m_updateRequested)
        return;
    m_updateRequested = true;
    QMetaObject::invokeMethod(this, &Frustum::updateGeometry, Qt::QueuedConnection);
}

FrustumLines::FrustumLines(const QColor& color, QNode* parent)
    : Qt3DCore::QEntity(parent)
    , mVertexBuffer(new Qt3DCore::QBuffer)
{
    auto* geometryRenderer = new Qt3DRender::QGeometryRenderer();
    {
        geometryRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Lines);

        auto* vertexPosAttribute = new Qt3DCore::QAttribute;
        vertexPosAttribute->setBuffer(mVertexBuffer);
        vertexPosAttribute->setAttributeType(Qt3DCore::QAttribute::VertexAttribute);
        vertexPosAttribute->setVertexBaseType(Qt3DCore::QAttribute::VertexBaseType::Float);
        vertexPosAttribute->setVertexSize(3);
        vertexPosAttribute->setByteStride(3 * sizeof(float));
        vertexPosAttribute->setCount(34);
        vertexPosAttribute->setName(Qt3DCore::QAttribute::defaultPositionAttributeName());

        auto* geometry = new Qt3DCore::QGeometry;
        geometry->addAttribute(vertexPosAttribute);

        geometryRenderer->setGeometry(geometry);
    }

    auto* material = new Qt3DRender::QMaterial;
    {
        auto* shaderProgram = new Qt3DRender::QShaderProgram;
        shaderProgram->setVertexShaderCode(frustum_vs.data());
        shaderProgram->setFragmentShaderCode(frustum_ps.data());

        auto* renderPass = new Qt3DRender::QRenderPass;
        renderPass->setShaderProgram(shaderProgram);

        auto* depthState = new Qt3DRender::QDepthTest;
        depthState->setDepthFunction(Qt3DRender::QDepthTest::LessOrEqual);

        auto* noDepthMask = new Qt3DRender::QNoDepthMask();

        renderPass->addRenderState(depthState);
        renderPass->addRenderState(noDepthMask);

        auto* lineWidth = new Qt3DRender::QLineWidth();
        lineWidth->setValue(2.0f);
        lineWidth->setSmooth(true);
        renderPass->addRenderState(lineWidth);

        auto* technique = new Qt3DRender::QTechnique;
        technique->addRenderPass(renderPass);
        technique->graphicsApiFilter()->setApi(Qt3DRender::QGraphicsApiFilter::OpenGL);
        technique->graphicsApiFilter()->setProfile(Qt3DRender::QGraphicsApiFilter::CoreProfile);
        technique->graphicsApiFilter()->setMajorVersion(3);
        technique->graphicsApiFilter()->setMinorVersion(2);

        auto* effect = new Qt3DRender::QEffect;
        effect->addTechnique(technique);

        auto* colorParameter = new Qt3DRender::QParameter;
        colorParameter->setName(QStringLiteral("color"));
        colorParameter->setValue(color);

        material->addParameter(colorParameter);
        material->setEffect(effect);
    }

    addComponent(material);
    addComponent(geometryRenderer);
}

void FrustumLines::setVertices(const std::vector<QVector3D>& vertices)
{
    QByteArray rawVertices;
    rawVertices.resize(vertices.size() * sizeof(QVector3D));
    std::memcpy(rawVertices.data(), vertices.data(), vertices.size() * sizeof(QVector3D));

    mVertexBuffer->setData(rawVertices);
}

FrustumTriangles::FrustumTriangles(const QColor& color, QNode* parent)
    : Qt3DCore::QEntity(parent)
    , mVertexBuffer(new Qt3DCore::QBuffer)
{
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
        shaderProgram->setVertexShaderCode(frustum_vs.data());
        shaderProgram->setFragmentShaderCode(frustum_ps.data());

        auto* renderPass = new Qt3DRender::QRenderPass;
        renderPass->setShaderProgram(shaderProgram);

        auto* blendEquation = new Qt3DRender::QBlendEquation;
        blendEquation->setBlendFunction(Qt3DRender::QBlendEquation::Add);

        auto* blendState = new Qt3DRender::QBlendEquationArguments();
        blendState->setSourceRgb(Qt3DRender::QBlendEquationArguments::SourceAlpha);
        blendState->setDestinationRgb(Qt3DRender::QBlendEquationArguments::DestinationAlpha);
        blendState->setSourceAlpha(Qt3DRender::QBlendEquationArguments::SourceAlpha);
        blendState->setDestinationAlpha(Qt3DRender::QBlendEquationArguments::DestinationAlpha);

        auto* depthState = new Qt3DRender::QDepthTest;
        depthState->setDepthFunction(Qt3DRender::QDepthTest::LessOrEqual);

        auto* noDepthMask = new Qt3DRender::QNoDepthMask();

        renderPass->addRenderState(blendState);
        renderPass->addRenderState(blendEquation);
        renderPass->addRenderState(depthState);
        renderPass->addRenderState(noDepthMask);

        auto* technique = new Qt3DRender::QTechnique;
        technique->addRenderPass(renderPass);
        technique->graphicsApiFilter()->setApi(Qt3DRender::QGraphicsApiFilter::OpenGL);
        technique->graphicsApiFilter()->setProfile(Qt3DRender::QGraphicsApiFilter::CoreProfile);
        technique->graphicsApiFilter()->setMajorVersion(3);
        technique->graphicsApiFilter()->setMinorVersion(2);

        auto* effect = new Qt3DRender::QEffect;
        effect->addTechnique(technique);

        auto* colorParameter = new Qt3DRender::QParameter;
        colorParameter->setName(QStringLiteral("color"));
        colorParameter->setValue(color);

        material->addParameter(colorParameter);
        material->setEffect(effect);
    }

    addComponent(material);
    addComponent(geometryRenderer);
}

void FrustumTriangles::setVertices(const std::vector<QVector3D>& vertices)
{
    QByteArray rawVertices;
    rawVertices.resize(vertices.size() * sizeof(QVector3D));
    std::memcpy(rawVertices.data(), vertices.data(), vertices.size() * sizeof(QVector3D));

    mVertexBuffer->setData(rawVertices);
}

} // namespace all::qt3d
