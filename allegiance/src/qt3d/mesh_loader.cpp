#include <qt3d/mesh_loader.h>
#include <qt3d/scene_mesh.h>

#include <Qt3DCore/QGeometry>
#include <Qt3DRender/QGeometryRenderer>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <algorithm>

namespace {
QColor toQColor(const aiColor3D& color)
{
    return QColor::fromRgbF(color.r, color.g, color.b);
}

QMatrix4x4 toQMatrix4x4(const aiMatrix4x4& matrix)
{
    return QMatrix4x4(
            matrix.a1, matrix.a2, matrix.a3, matrix.a4,
            matrix.b1, matrix.b2, matrix.b3, matrix.b4,
            matrix.c1, matrix.c2, matrix.c3, matrix.c4,
            matrix.d1, matrix.d2, matrix.d3, matrix.d4);
}

Qt3DRender::QMaterial* materialFrom(const aiMaterial* materialInfo, const QString& modelPath)
{
    aiColor3D diffuse = { 0.45f, 0.45f, 0.85f };
    materialInfo->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);

    aiColor3D specular = { 0.18f, 0.18f, 0.18f };
    materialInfo->Get(AI_MATKEY_COLOR_SPECULAR, specular);

    float shininess = 0.2f;
    materialInfo->Get(AI_MATKEY_SHININESS, shininess);
    if (shininess > 1) {
        shininess /= 255;
    }

    aiColor3D ambient = { 0.6f, 0.6f, 0.6f };
    materialInfo->Get(AI_MATKEY_COLOR_AMBIENT, ambient);

    aiString texFilename;
    const bool hasDiffuseTexture = materialInfo->GetTextureCount(aiTextureType::aiTextureType_DIFFUSE) > 0;
    if (hasDiffuseTexture && materialInfo->GetTexture(aiTextureType_DIFFUSE, 0, &texFilename) == aiReturn_SUCCESS) {
        auto* material = new Qt3DExtras::QTextureMaterial;

        QString filename = texFilename.C_Str();
        if (filename.size() > 0) {
            if (filename.at(0) == '\"')
                filename = filename.removeFirst();
            if (filename.last(1) == '\"')
                filename.chop(1);
            qDebug() << filename;

        }
        const auto texturePath = QFileInfo(modelPath).absoluteDir().absoluteFilePath(filename);
        auto* diffuseTexture = new Qt3DRender::QTextureLoader(material);
        diffuseTexture->setSource(QUrl::fromLocalFile(texturePath));

        material->setTexture(diffuseTexture);
        return material;
    } else {
        auto* material = new Qt3DExtras::QPhongMaterial;
        material->setDiffuse(toQColor(diffuse));
        material->setSpecular(toQColor(specular));
        material->setShininess(shininess);
        return material;
    }
}

void addMeshes(const aiScene* scene, const aiNode* node, const QMatrix4x4& transform, Qt3DCore::QEntity* root, const QString& path)
{
    const auto worldTransform = transform * toQMatrix4x4(node->mTransformation);

    for (std::size_t i = 0; i < node->mNumMeshes; ++i) {
        const aiMesh* meshInfo = scene->mMeshes[node->mMeshes[i]];
        const auto meshName = QString::fromLocal8Bit(meshInfo->mName.C_Str());

        const aiMaterial* materialInfo = scene->mMaterials[meshInfo->mMaterialIndex];
        const auto materialName = QString::fromLocal8Bit(materialInfo->GetName().C_Str());

        const bool isSkybox = materialName.contains("skybox", Qt::CaseInsensitive);
        const QMatrix4x4 meshTransform = [worldTransform, isSkybox] {
            if (isSkybox) {
                // remove translations and scales if skybox
                const auto r0 = QVector3D(worldTransform.row(0)).normalized();
                const auto r1 = QVector3D(worldTransform.row(1)).normalized();
                const auto r2 = QVector3D(worldTransform.row(2)).normalized();

                QMatrix4x4 transform;
                transform.setRow(0, QVector4D(QVector3D::crossProduct(r1, r2), 0));
                transform.setRow(1, QVector4D(QVector3D::crossProduct(r2, r0), 0));
                transform.setRow(2, QVector4D(QVector3D::crossProduct(r0, r1), 0));
                return transform;
            }

            return worldTransform;
        }();

        auto* childEntity = new Qt3DCore::QEntity(root);

        const bool hasDiffuseTexture = materialInfo->GetTextureCount(aiTextureType::aiTextureType_DIFFUSE) > 0;

        SceneMesh::VertexFlags vertexFlags;
        if (meshInfo->HasTextureCoords(0)) {
            vertexFlags.setFlag(SceneMesh::VertexFlag::HasTextureCoords);
        }
        if (meshInfo->mColors[0] != nullptr) {
            vertexFlags.setFlag(SceneMesh::VertexFlag::HasColors);
        }
        auto* meshComponent = new SceneMesh(vertexFlags);
        meshComponent->initializeFrom(meshInfo, meshTransform);
        meshComponent->setProperty("name", meshName);

        auto* materialComponent = materialFrom(materialInfo, path);
        materialComponent->setProperty("name", materialName);

        childEntity->addComponent(meshComponent);
        childEntity->addComponent(materialComponent);
    }

    for (std::size_t i = 0; i < node->mNumChildren; ++i) {
        const aiNode* childNode = node->mChildren[i];
        addMeshes(scene, childNode, worldTransform, root, path);
    }
}
} // namespace

Qt3DCore::QEntity* all::MeshLoader::load(const QString& path)
{
    Assimp::Importer importer;

    const auto flags = aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices;
    const aiScene* scene = importer.ReadFile(path.toLocal8Bit().constData(), flags);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        return nullptr;
        throw std::runtime_error(QStringLiteral("Failed to load mesh: %1").arg(importer.GetErrorString()).toLocal8Bit().constData());
    }

    auto* root = new Qt3DCore::QEntity;
    addMeshes(scene, scene->mRootNode, {}, root, path);
    return root;
}
