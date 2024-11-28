#include <stereo_image_mesh.h>

using namespace Qt3DCore;
using namespace Qt3DRender;
using namespace all::qt3d;

constexpr auto VertexByteStride = (3 + 2) * sizeof(float);
constexpr auto VertexCount = 4;

StereoImageGeometry::StereoImageGeometry(StereoImageMesh::Side side, Qt3DCore::QNode* parent)
    : QGeometry(parent)
    , m_side(side)
    , m_positionAttribute(new QAttribute(this))
    , m_texCoordAttribute(new QAttribute(this))
    , m_vertexBuffer(new Qt3DCore::QBuffer(this))
{
    m_positionAttribute->setName(QAttribute::defaultPositionAttributeName());
    m_positionAttribute->setVertexBaseType(QAttribute::Float);
    m_positionAttribute->setVertexSize(3);
    m_positionAttribute->setAttributeType(QAttribute::VertexAttribute);
    m_positionAttribute->setBuffer(m_vertexBuffer);
    m_positionAttribute->setByteStride(VertexByteStride);
    m_positionAttribute->setByteOffset(0);
    m_positionAttribute->setCount(VertexCount);

    m_texCoordAttribute->setName(QAttribute::defaultTextureCoordinateAttributeName());
    m_texCoordAttribute->setVertexBaseType(QAttribute::Float);
    m_texCoordAttribute->setVertexSize(2);
    m_texCoordAttribute->setAttributeType(QAttribute::VertexAttribute);
    m_texCoordAttribute->setBuffer(m_vertexBuffer);
    m_texCoordAttribute->setByteStride(VertexByteStride);
    m_texCoordAttribute->setByteOffset(3 * sizeof(float));
    m_texCoordAttribute->setCount(VertexCount);

    updateVertices();

    addAttribute(m_positionAttribute);
    addAttribute(m_texCoordAttribute);
}

void StereoImageGeometry::setViewportSize(const QVector2D& viewportSize)
{
    if (viewportSize == m_viewportSize)
        return;
    m_viewportSize = viewportSize;
    updateVertices();
    Q_EMIT viewportSizeChanged(viewportSize);
}

void StereoImageGeometry::setImageSize(const QVector2D& imageSize)
{
    if (imageSize == m_imageSize)
        return;
    m_imageSize = imageSize;
    updateVertices();
    Q_EMIT imageSizeChanged(imageSize);
}

void StereoImageGeometry::updateVertices()
{
    // Compute texture coordinates so that the image is zoomed as much as possible while preserving aspect ratio

    const auto viewportAspect = m_viewportSize.x() / m_viewportSize.y();
    const auto imageAspect = (0.5f * m_imageSize.x()) / m_imageSize.y();

    QVector2D texCoordMin, texCoordMax;
    if (viewportAspect > imageAspect) {
        const auto f = imageAspect / viewportAspect;
        texCoordMin = QVector2D(0, 0.5f - 0.5f * f);
        texCoordMax = QVector2D(1, 0.5f + 0.5f * f);
    } else {
        const auto f = viewportAspect / imageAspect;
        texCoordMin = QVector2D(0.5f - 0.5f * f, 0.0);
        texCoordMax = QVector2D(0.5f + 0.5f * f, 1.0);
    }

    // We want to show the left or the right half of the image
    texCoordMin.setX(texCoordMin.x() * 0.5f);
    texCoordMax.setX(texCoordMax.x() * 0.5f);
    if (m_side == StereoImageMesh::Side::Right) {
        texCoordMin += QVector2D(0.5f, 0.0f);
        texCoordMax += QVector2D(0.5f, 0.0f);
    }

    struct Vertex {
        QVector3D pos;
        QVector2D texCoords;
    };
    static_assert(sizeof(Vertex) == 5 * sizeof(float));

    const std::vector<Vertex> vertices = {
        Vertex{
                .pos = QVector3D(-1.0f, 1.0f, 0.0f),
                .texCoords = QVector2D(texCoordMin.x(), texCoordMin.y()),
        },
        Vertex{
                .pos = QVector3D(-1.0f, -1.0f, 0.0f),
                .texCoords = QVector2D(texCoordMin.x(), texCoordMax.y()),
        },
        Vertex{
                .pos = QVector3D(1.0f, 1.0f, 0.0f),
                .texCoords = QVector2D(texCoordMax.x(), texCoordMin.y()),
        },
        Vertex{
                .pos = QVector3D(1.0f, -1.0f, 0.0f),
                .texCoords = QVector2D(texCoordMax.x(), texCoordMax.y()),
        },
    };

    QByteArray bufferBytes;
    bufferBytes.resize(vertices.size() * sizeof(Vertex));
    std::memcpy(bufferBytes.data(), vertices.data(), vertices.size() * sizeof(Vertex));

    m_vertexBuffer->setData(bufferBytes);
}

StereoImageMesh::StereoImageMesh(Side side, Qt3DCore::QNode* parent)
    : QGeometryRenderer(parent)
{
    auto* geometry = new StereoImageGeometry(side);
    setGeometry(geometry);

    connect(geometry, &StereoImageGeometry::viewportSizeChanged, this, &StereoImageMesh::viewportSizeChanged);

    setPrimitiveType(TriangleStrip);
}

StereoImageMesh::Side StereoImageMesh::side() const
{
    return static_cast<StereoImageGeometry*>(geometry())->side();
}

QVector2D StereoImageMesh::viewportSize() const
{
    return static_cast<StereoImageGeometry*>(geometry())->viewportSize();
}

void StereoImageMesh::setViewportSize(const QVector2D& viewportSize)
{
    static_cast<StereoImageGeometry*>(geometry())->setViewportSize(viewportSize);
}

QVector2D StereoImageMesh::imageSize() const
{
    return static_cast<StereoImageGeometry*>(geometry())->imageSize();
}

void StereoImageMesh::setImageSize(const QVector2D& imageSize)
{
    static_cast<StereoImageGeometry*>(geometry())->setImageSize(imageSize);
}
