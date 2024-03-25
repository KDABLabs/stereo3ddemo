#pragma once

#include <Qt3DRender/QGeometryRenderer>

struct aiMesh;

class SceneMesh : public Qt3DRender::QGeometryRenderer
{
    Q_OBJECT
public:
    enum class VertexFlag {
        None = 0,
        HasTextureCoords = 1,
        HasColors = 2
    };
    Q_DECLARE_FLAGS(VertexFlags, VertexFlag)

    explicit SceneMesh(VertexFlags vertexFlags, Qt3DCore::QNode* parent = nullptr);

    void initializeFrom(const aiMesh* meshInfo, const QMatrix4x4& transform);
};
