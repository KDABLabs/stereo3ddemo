#include "scene_mesh.h"

#include <assimp/scene.h>
#include <QMatrix4x4>

using namespace Qt3DCore;

namespace {
QVector3D toQVector3D(const aiVector3D& vector)
{
    return QVector3D(vector.x, vector.y, vector.z);
}
} // namespace

class SceneMeshGeometry : public QGeometry
{
    Q_OBJECT

public:
    explicit SceneMeshGeometry(SceneMesh::VertexFlags vertexFlags, QNode* parent = nullptr);

    void initializeFrom(const aiMesh* meshInfo, const QMatrix4x4& transform);

private:
    std::size_t vertexByteStride() const;
    bool vertexHasTextureCoords() const { return m_vertexFlags.testFlag(SceneMesh::VertexFlag::HasTextureCoords); }
    bool vertexHasColors() const { return m_vertexFlags.testFlag(SceneMesh::VertexFlag::HasColors); }

    SceneMesh::VertexFlags m_vertexFlags{ SceneMesh::VertexFlag::None };
    QAttribute* m_positionAttribute{ nullptr };
    QAttribute* m_normalAttribute{ nullptr };
    QAttribute* m_texCoordAttribute{ nullptr };
    QAttribute* m_colorAttribute{ nullptr };
    QAttribute* m_indexAttribute{ nullptr };
    Qt3DCore::QBuffer* m_vertexBuffer{ nullptr };
    Qt3DCore::QBuffer* m_indexBuffer{ nullptr };
};

SceneMeshGeometry::SceneMeshGeometry(SceneMesh::VertexFlags vertexFlags, QNode* parent)
    : QGeometry(parent)
    , m_vertexFlags(vertexFlags)
    , m_indexAttribute(new QAttribute(this))
    , m_vertexBuffer(new Qt3DCore::QBuffer(this))
    , m_indexBuffer(new Qt3DCore::QBuffer(this))
{
    std::size_t byteOffset = 0;
    auto addVertexAttribute = [this, &byteOffset](const QString& name, int size) {
        auto* attribute = new QAttribute(this);
        attribute->setName(name);
        attribute->setVertexBaseType(QAttribute::Float);
        attribute->setVertexSize(size);
        attribute->setAttributeType(QAttribute::VertexAttribute);
        attribute->setBuffer(m_vertexBuffer);
        attribute->setByteStride(vertexByteStride());
        attribute->setByteOffset(byteOffset);
        addAttribute(attribute);
        byteOffset += size * sizeof(float);
        return attribute;
    };
    m_positionAttribute = addVertexAttribute(QAttribute::defaultPositionAttributeName(), 3);
    m_normalAttribute = addVertexAttribute(QAttribute::defaultNormalAttributeName(), 3);
    if (vertexHasTextureCoords()) {
        m_texCoordAttribute = addVertexAttribute(QAttribute::defaultTextureCoordinateAttributeName(), 2);
    }
    if (vertexHasColors()) {
        m_colorAttribute = addVertexAttribute(QAttribute::defaultColorAttributeName(), 4);
    }
    Q_ASSERT(byteOffset == vertexByteStride());

    m_indexAttribute->setAttributeType(QAttribute::IndexAttribute);
    m_indexAttribute->setVertexBaseType(QAttribute::UnsignedInt);
    m_indexAttribute->setBuffer(m_indexBuffer);
    addAttribute(m_indexAttribute);
}

void SceneMeshGeometry::initializeFrom(const aiMesh* meshInfo, const QMatrix4x4& transform)
{
    const auto normalMatrix = QMatrix4x4(transform.normalMatrix());

    const auto vertexCount = meshInfo->mNumVertices;

    const aiVector3D* positions = meshInfo->mVertices;
    const aiVector3D* normals = meshInfo->mNormals;
    const aiVector3D* texCoords = meshInfo->mTextureCoords[0];
    const aiColor4D* colors = meshInfo->mColors[0];

    QByteArray vertexBytes;
    vertexBytes.resize(vertexByteStride() * vertexCount);
    float* vertexData = reinterpret_cast<float*>(vertexBytes.data());
    for (size_t i = 0; i < vertexCount; ++i) {
        const QVector3D position = transform.map(toQVector3D(positions[i]));
        *vertexData++ = static_cast<float>(position.x());
        *vertexData++ = static_cast<float>(position.y());
        *vertexData++ = static_cast<float>(position.z());

        const QVector3D normal = normalMatrix.map(toQVector3D(normals[i])).normalized();
        *vertexData++ = static_cast<float>(normal.x());
        *vertexData++ = static_cast<float>(normal.y());
        *vertexData++ = static_cast<float>(normal.z());

        if (vertexHasTextureCoords()) {
            Q_ASSERT(texCoords);
            const aiVector3D& texCoord = texCoords[i];
            *vertexData++ = static_cast<float>(texCoord.x);
            *vertexData++ = static_cast<float>(texCoord.y);
        }

        if (vertexHasColors()) {
            Q_ASSERT(colors);
            const aiColor4D& color = colors[i];
            *vertexData++ = static_cast<float>(color.r);
            *vertexData++ = static_cast<float>(color.g);
            *vertexData++ = static_cast<float>(color.b);
            *vertexData++ = static_cast<float>(color.a);
        }
    }
    Q_ASSERT(reinterpret_cast<const char*>(vertexData) == vertexBytes.constData() + vertexBytes.size());
    m_vertexBuffer->setData(vertexBytes);

    m_positionAttribute->setCount(vertexCount);
    m_normalAttribute->setCount(vertexCount);
    if (vertexHasTextureCoords()) {
        m_texCoordAttribute->setCount(vertexCount);
    }
    if (vertexHasColors()) {
        m_colorAttribute->setCount(vertexCount);
    }

    const auto faceCount = meshInfo->mNumFaces;
    const aiFace* faces = meshInfo->mFaces;

    QByteArray faceBytes;
    faceBytes.resize(faceCount * 3 * sizeof(int32_t));
    int32_t* faceData = reinterpret_cast<int32_t*>(faceBytes.data());
    for (size_t i = 0; i < faceCount; ++i) {
        const aiFace& face = faces[i];
        Q_ASSERT(face.mNumIndices == 3);
        for (size_t j = 0; j < 3; ++j) {
            *faceData++ = face.mIndices[j];
        }
    }
    Q_ASSERT(reinterpret_cast<const char*>(faceData) == faceBytes.constData() + faceBytes.size());
    m_indexBuffer->setData(faceBytes);

    m_indexAttribute->setCount(faceCount * 3);
}

std::size_t SceneMeshGeometry::vertexByteStride() const
{
    auto elementSize = 3 + 3; // position, normal
    if (vertexHasTextureCoords()) {
        elementSize += 2; // texCoord
    }
    if (vertexHasColors()) {
        elementSize += 4; // color
    }
    return elementSize * sizeof(float);
}

SceneMesh::SceneMesh(VertexFlags vertexFlags, QNode* parent)
    : QGeometryRenderer(parent)
{
    auto* geometry = new SceneMeshGeometry(vertexFlags);
    setGeometry(geometry);
    setPrimitiveType(Triangles);
}

void SceneMesh::initializeFrom(const aiMesh* meshInfo, const QMatrix4x4& transform)
{
    static_cast<SceneMeshGeometry*>(geometry())->initializeFrom(meshInfo, transform);
}

#include "scene_mesh.moc"
