#include "qt3d_focusarea.h"
#include "qt3d_shaders.h"

#include <QMouseEvent>

#include <Qt3DExtras/QSphereMesh>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DExtras/QPlaneMesh>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DCore/QTransform>

#include <Qt3DCore/QBuffer>
#include <Qt3DCore/QAttribute>

#include <Qt3DRender/QCameraLens>
#include <Qt3DRender/QPickEvent>
#include <Qt3DRender/QRayCaster>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QCameraLens>
#include <Qt3DRender/QRenderPass>
#include <Qt3DRender/QDepthTest>
#include <Qt3DRender/QNoDepthMask>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DInput/QMouseDevice>

namespace all::qt3d {

FocusArea::FocusArea(Qt3DCore::QNode* parent)
    : QEntity(parent)
    , m_buffer(new Qt3DCore::QBuffer)
    , m_extent(QVector3D(100.0f, 100.0f, 0.0f))
{
    // Geometry
    auto* geometryRenderer = new Qt3DRender::QGeometryRenderer;
    {
        updateMesh();

        auto* positionAttribute = new Qt3DCore::QAttribute;
        positionAttribute->setName(Qt3DCore::QAttribute::defaultPositionAttributeName());
        positionAttribute->setAttributeType(Qt3DCore::QAttribute::VertexAttribute);
        positionAttribute->setCount(14);
        positionAttribute->setVertexBaseType(Qt3DCore::QAttribute::Float);
        positionAttribute->setVertexSize(3);
        positionAttribute->setByteStride(6 * sizeof(float));
        positionAttribute->setBuffer(m_buffer);

        auto* colorAttribute = new Qt3DCore::QAttribute;
        colorAttribute->setName(Qt3DCore::QAttribute::defaultColorAttributeName());
        colorAttribute->setAttributeType(Qt3DCore::QAttribute::VertexAttribute);
        colorAttribute->setCount(14);
        colorAttribute->setVertexBaseType(Qt3DCore::QAttribute::Float);
        colorAttribute->setVertexSize(3);
        colorAttribute->setByteOffset(3 * sizeof(float));
        colorAttribute->setByteStride(6 * sizeof(float));
        colorAttribute->setBuffer(m_buffer);

        auto* geometry = new Qt3DCore::QGeometry;
        geometry->addAttribute(positionAttribute);
        geometry->addAttribute(colorAttribute);
        geometryRenderer->setGeometry(geometry);
        geometryRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Lines);
    }

    // Material
    auto* material = new Qt3DRender::QMaterial;
    {
        auto effect = new Qt3DRender::QEffect;
        auto gl3Shader = new Qt3DRender::QShaderProgram();
        gl3Shader->setVertexShaderCode(all::qt3d::focus_area_vs.data());
        gl3Shader->setFragmentShaderCode(all::qt3d::focus_area_ps.data());

        auto gl3RenderPass = new Qt3DRender::QRenderPass(this);
        gl3RenderPass->setShaderProgram(gl3Shader);

        auto gl3Technique = new Qt3DRender::QTechnique;
        gl3Technique->graphicsApiFilter()->setApi(Qt3DRender::QGraphicsApiFilter::OpenGL);
        gl3Technique->graphicsApiFilter()->setMajorVersion(3);
        gl3Technique->graphicsApiFilter()->setMinorVersion(2);
        gl3Technique->graphicsApiFilter()->setProfile(Qt3DRender::QGraphicsApiFilter::CoreProfile);
        gl3Technique->addRenderPass(gl3RenderPass);

        effect->addTechnique(gl3Technique);
        material->setEffect(effect);
    }

    QObject::connect(this, &FocusArea::extentChanged, this, &FocusArea::update);
    QObject::connect(this, &FocusArea::centerChanged, this, &FocusArea::update);

    QObject::connect(this, &FocusArea::viewSizeChanged, this, [this] {
        QVector3D e = m_center;
        e.setX(m_viewSize.width() * 0.5);
        e.setY(m_viewSize.height() * 0.5);
        setCenter(e);
    });

    QObject::connect(this, &Qt3DCore::QEntity::enabledChanged, geometryRenderer, &Qt3DRender::QGeometryRenderer::setEnabled);

    addComponent(geometryRenderer);
    addComponent(material);
}

void FocusArea::setCamera(const Qt3DRender::QCamera* camera)
{
    m_camera = camera;
    if (camera == nullptr)
        return;

    connect(this, &FocusArea::viewSizeChanged, this, &FocusArea::update);
    update();
}

void FocusArea::update()
{
    if (!m_updateRequested) {
        m_updateRequested = true;
        QMetaObject::invokeMethod(this, "updateMesh", Qt::QueuedConnection);
        updateMesh();
    }
}

void FocusArea::updateMesh()
{
    m_updateRequested = false;

    QByteArray rawData;
    rawData.resize(14 * 2 * sizeof(QVector3D));
    auto vertices = reinterpret_cast<QVector3D*>(rawData.data());

    if (m_camera == nullptr)
        return;

    const QVector3D viewCenter = m_camera->viewCenter();
    const QVector3D viewCenterScreen = viewCenter.project(m_camera->viewMatrix(), m_camera->projectionMatrix(), QRect(0, 0, m_viewSize.width(), m_viewSize.height()));
    const float focusPlaneScreenCoords = viewCenterScreen.z();

    auto screenPosToWorldCoords = [&](QVector3D screenPos) {
        screenPos.setZ(focusPlaneScreenCoords);
        // To OpenGL Y
        screenPos.setY(float(m_viewSize.height()) - screenPos.y());

        return screenPos.unproject(m_camera->viewMatrix(), m_camera->projectionMatrix(), QRect(0, 0, m_viewSize.width(), m_viewSize.height()));
    };

    const QVector3D white = QVector3D(1.0f, 1.0f, 1.0f);
    const QVector3D red = QVector3D(1.0f, 0.0f, 0.0f);

    // Rect
    vertices[0] = screenPosToWorldCoords(m_center + QVector3D(-0.5f, 0.5f, 0.0f) * m_extent);
    vertices[1] = white;
    vertices[2] = screenPosToWorldCoords(m_center + QVector3D(-0.5f, -0.5f, 0.0f) * m_extent);
    vertices[3] = white;
    vertices[4] = screenPosToWorldCoords(m_center + QVector3D(-0.5f, -0.5f, 0.0f) * m_extent);
    vertices[5] = white;
    vertices[6] = screenPosToWorldCoords(m_center + QVector3D(0.5f, -0.5f, 0.0f) * m_extent);
    vertices[7] = white;
    vertices[8] = screenPosToWorldCoords(m_center + QVector3D(0.5f, -0.5f, 0.0f) * m_extent);
    vertices[9] = white;
    vertices[10] = screenPosToWorldCoords(m_center + QVector3D(0.5f, 0.5f, 0.0f) * m_extent);
    vertices[11] = white;
    vertices[12] = screenPosToWorldCoords(m_center + QVector3D(0.5f, 0.5f, 0.0f) * m_extent);
    vertices[13] = white;
    vertices[14] = screenPosToWorldCoords(m_center + QVector3D(-0.5f, 0.5f, 0.0f) * m_extent);
    vertices[15] = white;

    // Center
    const QVector3D centerColor = m_containedArea == ContainedArea::Center ? red : white;
    vertices[16] = screenPosToWorldCoords(m_center + QVector3D(0.0f, 1.0f, 0.0f) * 20.0f);
    vertices[17] = centerColor;
    vertices[18] = screenPosToWorldCoords(m_center + QVector3D(0.0f, -1.0f, 0.0f) * 20.0f);
    vertices[19] = centerColor;
    vertices[20] = screenPosToWorldCoords(m_center + QVector3D(-1.0f, 0.0f, 0.0f) * 20.0f);
    vertices[21] = centerColor;
    vertices[22] = screenPosToWorldCoords(m_center + QVector3D(1.0f, 0.0f, 0.0f) * 20.0f);
    vertices[23] = centerColor;

    // Resize
    const QVector3D resizeColor = m_containedArea == ContainedArea::Resize ? red : white;
    vertices[24] = screenPosToWorldCoords(m_center + QVector3D(0.5f, 0.3f, 0.0f) * m_extent);
    vertices[25] = resizeColor;
    vertices[26] = screenPosToWorldCoords(m_center + QVector3D(0.3f, 0.5f, 0.0f) * m_extent);
    vertices[27] = resizeColor;

    m_buffer->setData(rawData);
}

void FocusArea::onMousePressed(::QMouseEvent* mouse)
{
    m_distToCenterOnPress = m_center - QVector3D(mouse->x(), mouse->y(), 0.0f);
    m_extentOnPress = m_extent;
    if (m_containedArea == ContainedArea::Center) {
        m_operation = Operation::Translating;
    } else if (m_containedArea == ContainedArea::Resize) {
        m_operation = Operation::Scaling;
    }
}

void FocusArea::onMouseMoved(::QMouseEvent* mouse)
{
    if (m_operation == Operation::None) {
        updateContainsMouse(mouse);
    } else if (m_operation == Operation::Translating) {
        setCenter(m_distToCenterOnPress + QVector3D(mouse->x(), mouse->y(), 0.0f));
    } else if (m_operation == Operation::Scaling) {
        const QVector3D distToCenter = m_center - QVector3D(mouse->x(), mouse->y(), 0.0f);
        setExtent(m_extentOnPress * (distToCenter.length() / m_distToCenterOnPress.length()));
    }
}

void FocusArea::onMouseReleased(::QMouseEvent* mouse)
{
    m_operation = Operation::None;
}

void FocusArea::updateContainsMouse(::QMouseEvent* mouse)
{
    if (!isEnabled())
        return;

    const ContainedArea oldContainedArea = m_containedArea;
    m_containedArea = ContainedArea::None;

    if ((QVector3D(mouse->x(), mouse->y(), 0.0f) - m_center).lengthSquared() < (20.0f * 20.0f)) {
        // Within the Center Cross
        m_containedArea = ContainedArea::Center;
    } else {
        // Within the Resize Handle
        const QVector3D a = m_center + QVector3D(0.5f, 0.3f, 0.0f) * m_extent;
        const QVector3D b = m_center + QVector3D(0.3f, 0.5f, 0.0f) * m_extent;
        const QVector3D c = m_center + QVector3D(0.5f, 0.5f, 0.0f) * m_extent;

        const QVector3D p = QVector3D(mouse->x(), mouse->y(), 0.0f);

        const QVector3D ap = p - a;
        const QVector3D ab = b - a;
        const bool abXapPositive = (ab.x() * ap.y() - ab.y() * ap.x()) > 0.0f;

        const QVector3D bc = c - b;
        const QVector3D bp = p - b;
        const bool bcXbpPositive = (bc.x() * bp.y() - bc.y() * bp.x()) > 0.0f;

        const QVector3D ca = a - c;
        const QVector3D cp = p - c;
        const bool caXcpPositive = (ca.x() * cp.y() - ca.y() * cp.x()) > 0.0f;

        if (abXapPositive == bcXbpPositive && bcXbpPositive == caXcpPositive)
            m_containedArea = ContainedArea::Resize;
    }

    if (oldContainedArea != m_containedArea) {
        updateMesh();
    }
}

QVector3D FocusArea::center() const
{
    return m_center;
}

void FocusArea::setCenter(const QVector3D& center)
{
    if (center == m_center)
        return;
    m_center = center;
    Q_EMIT centerChanged();
}

QVector3D FocusArea::extent() const
{
    return m_extent;
}

void FocusArea::setExtent(const QVector3D& extent)
{
    if (extent == m_extent)
        return;
    m_extent = extent;
    Q_EMIT extentChanged();
}

void FocusArea::setViewSize(const QSize viewSize)
{
    if (viewSize == m_viewSize)
        return;
    m_viewSize = viewSize;
    Q_EMIT viewSizeChanged();
}

QSize FocusArea::viewSize() const
{
    return m_viewSize;
}

} // namespace all::qt3d
