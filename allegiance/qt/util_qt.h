#pragma once

#include <QMatrix4x4>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

inline QDebug operator<<(QDebug debug, const glm::mat4x4& matrix)
{
    // Set the format you want for the matrix elements
    debug.nospace() << "glm::mat4x4(";

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            debug.nospace() << matrix[i][j];
            if (i != 3 || j != 3) {
                debug.nospace() << ", ";
            }
        }
    }

    debug.nospace() << ")";
    return debug.space();
}

inline QDebug operator<<(QDebug debug, const glm::vec4& vec)
{
    debug << vec.x << ", " << vec.y << ", " << vec.z;
    return debug;
}

inline QVector3D toQVector3D(const glm::vec3& glmVec)
{
    return QVector3D(glmVec.x, glmVec.y, glmVec.z);
}

inline glm::vec3 toGlmVec3(const QVector3D& qVec)
{
    return glm::vec3(qVec.x(), qVec.y(), qVec.z());
}

// Cast operators for glm::mat4x4 <-> QMatrix4x4
inline QMatrix4x4 toQMatrix4x4(const glm::mat4x4& glmMat)
{
    QMatrix4x4 qMat;

    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            qMat(i, j) = glmMat[j][i]; // Note the transposition

    return qMat;
}

inline glm::mat4x4 toGlmMat4x4(const QMatrix4x4& qMat)
{
    glm::mat4x4 glmMat;

    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            glmMat[j][i] = qMat(i, j); // Note the transposition

    return glmMat;
}
