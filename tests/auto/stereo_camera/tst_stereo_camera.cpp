#include <shared/stereo_camera.h>

#include <QMatrix4x4>
#include <QVector3D>
#include <Qt3DRender/QCamera>
#include <QDebug>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

namespace {

enum class Eye
{
    Left,
    Right
};

QMatrix4x4 glmToMat4(const glm::mat4x4 &m)
{
    return QMatrix4x4(m[0][0], m[1][0], m[2][0], m[3][0],
                      m[0][1], m[1][1], m[2][1], m[3][1],
                      m[0][2], m[1][2], m[2][2], m[3][2],
                      m[0][3], m[1][3], m[2][3], m[3][3]);
}

QVector3D glmToVec3(const glm::vec3 v)
{
    return {v[0], v[1], v[2]};
}

/*
    OpenGL Right Handed Coordinate System
            +Y
            ^ _ +Z
            | /|
            |/
    X+ <----/
*/

// QGIS Qt3D Assymetric Stereo Implementation
void updateQt3DStereoCamera(Qt3DRender::QCamera *centerCamera,
                            Qt3DRender::QCamera *eyeCamera,
                            float interocularDistance,
                            Eye eye,
                            bool toeIn = false)
{
    eyeCamera->setUpVector(centerCamera->upVector());
    eyeCamera->setExposure(centerCamera->exposure());
    eyeCamera->setNearPlane(centerCamera->nearPlane());
    eyeCamera->setFarPlane(centerCamera->farPlane());
    eyeCamera->setFieldOfView(centerCamera->fieldOfView());
    eyeCamera->setAspectRatio(centerCamera->aspectRatio());

    const QVector3D pos = centerCamera->position();
    const QVector3D up = centerCamera->upVector();
    const QVector3D viewDirection = (centerCamera->viewCenter() - pos).normalized();
    const QVector3D rightVector = QVector3D::crossProduct(viewDirection, up).normalized();
    const float halfInterocularDistance = interocularDistance * 0.5f;

    if (eye == Eye::Left) {
        const QVector3D leftPosition = pos - rightVector * halfInterocularDistance;
        eyeCamera->setPosition(leftPosition);

    } else {
        const QVector3D rightPosition = pos + rightVector * halfInterocularDistance;
        eyeCamera->setPosition(rightPosition);
    }

    if (toeIn) {
        // With ToeIn we have symmetrical frustums for left and right eyes camera
        // where each camera looks at the same focus point
        eyeCamera->setViewCenter(centerCamera->viewCenter());
        eyeCamera->setProjectionType(centerCamera->projectionType());
        eyeCamera->setLeft(centerCamera->left());
        eyeCamera->setRight(centerCamera->right());
        eyeCamera->setBottom(centerCamera->bottom());
        eyeCamera->setTop(centerCamera->top());
    } else {
        // Asymetric Frustum
        // Both Left and Right Cameras are looking parallel to each other
        eyeCamera->setProjectionType(Qt3DRender::QCameraLens::FrustumProjection);
        const float halfVFov = std::tan(qDegreesToRadians(centerCamera->fieldOfView()) * 0.5f);
        const float convergence = (centerCamera->viewCenter() - centerCamera->position()).length();
        const float halfHeightAtConvergence = halfVFov * centerCamera->nearPlane();
        const float halfWidthAtConvergence = centerCamera->aspectRatio() * halfHeightAtConvergence;
        const float nearConvergenceRatio = centerCamera->nearPlane() / convergence;
        const float frustumShift = halfInterocularDistance * nearConvergenceRatio;

        if (eye == Eye::Left) {
            eyeCamera->setLeft(-halfWidthAtConvergence + frustumShift);
            eyeCamera->setRight(halfWidthAtConvergence + frustumShift);
            eyeCamera->setViewCenter(centerCamera->viewCenter() - rightVector * halfInterocularDistance);
        } else {
            eyeCamera->setLeft(-halfWidthAtConvergence - frustumShift);
            eyeCamera->setRight(halfWidthAtConvergence - frustumShift);
            eyeCamera->setViewCenter(centerCamera->viewCenter() + rightVector * halfInterocularDistance);
        }
        eyeCamera->setTop(halfWidthAtConvergence);
        eyeCamera->setBottom(-halfWidthAtConvergence);
    }
};

#define COMPARE_MAT(m1, m2)      \
    CHECK(m1(0, 0) == m2(0, 0)); \
    CHECK(m1(1, 0) == m2(1, 0)); \
    CHECK(m1(2, 0) == m2(2, 0)); \
    CHECK(m1(3, 0) == m2(3, 0)); \
    CHECK(m1(0, 1) == m2(0, 1)); \
    CHECK(m1(1, 1) == m2(1, 1)); \
    CHECK(m1(2, 1) == m2(2, 1)); \
    CHECK(m1(3, 1) == m2(3, 1)); \
    CHECK(m1(0, 2) == m2(0, 2)); \
    CHECK(m1(1, 2) == m2(1, 2)); \
    CHECK(m1(2, 2) == m2(2, 2)); \
    CHECK(m1(3, 2) == m2(3, 2)); \
    CHECK(m1(0, 3) == m2(0, 3)); \
    CHECK(m1(1, 3) == m2(1, 3)); \
    CHECK(m1(2, 3) == m2(2, 3)); \
    CHECK(m1(3, 3) == m2(3, 3));

#define COMPARE_VEC3(v1, v2) \
    CHECK(v1[0] == v2[0]);   \
    CHECK(v1[1] == v2[1]);   \
    CHECK(v1[2] == v2[2]);

} // anonymouse

TEST_SUITE("StereoCamera")
{
    TEST_CASE("Check_Qt_glm_expectations")
    {
        // GIVEN
        QMatrix4x4 m_qt;
        glm::mat4x4 m_glm(1.0f);

        // THEN
        CHECK(m_qt == glmToMat4((m_glm)));

        // WHEN
        m_qt.translate(QVector3D(1.0f, 2.0f, 3.0f));
        m_glm = glm::translate(m_glm, {1.0f, 2.0f, 3.0f});

        // THEN
        CHECK(m_qt == glmToMat4((m_glm)));
    }

    TEST_CASE("Shearing Symmetric Frustum vs Assymetric Frustum")
    {
        // GIVEN
        using namespace all;
        StereoCamera camera(StereoCamera::API::OpenGL);
        const float interocularDistance = 10.0f;
        const float convergencePlaneDist = 30.0f;

        // WHEN
        camera.SetAspectRatio(1.0f);
        camera.SetFarPlane(1000.0f);
        camera.SetNearPlane(1.0f);
        camera.SetFov(45.0f);
        camera.SetUpVector(glm::vec3(0.0f, 1.0f, 0.0f));
        camera.SetPosition(glm::vec3(0.0f, 0.0f, 100.0f));
        camera.SetForwardVector(glm::vec3(0.0f, 0.0f, -1.0f));
        camera.SetInterocularDistance(interocularDistance);
        camera.SetConvergencePlaneDistance(convergencePlaneDist);
        camera.SetShear(false);

        // THEN
        CHECK(camera.GetAspectRatio() == 1.0f);
        CHECK(camera.GetFarPlane() == 1000.0f);
        CHECK(camera.GetNearPlane() == 1.0f);
        CHECK(camera.GetFov() == 45.0f);
        COMPARE_VEC3(camera.GetUpVector(), glm::vec3(0.0f, 1.0f, 0.0f));
        COMPARE_VEC3(camera.GetPosition(), glm::vec3(0.0f, 0.0f, 100.0f));
        COMPARE_VEC3(camera.GetForwardVector(), glm::vec3(0.0f, 0.0f, -1.0f));
        CHECK(camera.GetInterocularDistance() == interocularDistance);
        CHECK(camera.GetConvergencePlaneDistance() == convergencePlaneDist);

        COMPARE_VEC3(glm::vec3(camera.GetViewLeft() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)), glm::vec3(5.0f, 0.0f, -100.0f));
        COMPARE_VEC3(glm::vec3(camera.GetViewRight() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)), glm::vec3(-5.0f, 0.0f, -100.0f));

        {
            // If we are looking in the -Z dir and our up vector is +Y
            // The the right vector is +X
            const glm::vec3 right = glm::normalize(glm::cross(camera.GetForwardVector(), camera.GetUpVector()));
            COMPARE_VEC3(right, glm::vec3(1.0f, 0.0f, 0.0f));
        }

        // GIVEN
        Qt3DRender::QCamera qtCamera;
        qtCamera.setAspectRatio(1.0f);
        qtCamera.setFarPlane(1000.0f);
        qtCamera.setNearPlane(1.0f);
        qtCamera.setFieldOfView(45.0f);
        qtCamera.setUpVector(QVector3D(0.0f, 1.0f, 0.0f));
        qtCamera.setPosition(QVector3D(0.0f, 0.0f, 100.0f));
        // Looking toward -z
        qtCamera.setViewCenter(qtCamera.position() + QVector3D(0.0f, 0.0f, -convergencePlaneDist));

        // THEN
        CHECK(qtCamera.aspectRatio() == 1.0f);
        CHECK(qtCamera.farPlane() == 1000.0f);
        CHECK(qtCamera.nearPlane() == 1.0f);
        CHECK(qtCamera.fieldOfView() == 45.0f);
        COMPARE_VEC3(qtCamera.upVector(), QVector3D(0.0f, 1.0f, 0.0f));
        COMPARE_VEC3(qtCamera.position(), QVector3D(0.0f, 0.0f, 100.0f));
        COMPARE_VEC3(qtCamera.viewCenter(), QVector3D(0.0f, 0.0f, 70.0f));

        // WHEN
        const glm::mat4x4 projMatrix = camera.GetProjection();
        const glm::mat4x4 shearingLeft = StereoCamera::StereoShear(camera.ShearCoefficient());
        const glm::mat4x4 shearingRight = StereoCamera::StereoShear(-camera.ShearCoefficient());
        const glm::mat4x4 leftViewMatrix = camera.GetViewLeft();
        const glm::mat4x4 rightViewMatrix = camera.GetViewRight();
        const glm::mat4x4 leftProjectionMatrix = projMatrix * shearingLeft;
        const glm::mat4x4 rightProjectionMatrix = projMatrix * shearingRight;

        // WHEN
        Qt3DRender::QCamera leftEyeCamera;
        updateQt3DStereoCamera(&qtCamera, &leftEyeCamera, interocularDistance, Eye::Left, false);

        // THEN
        CHECK(leftEyeCamera.aspectRatio() == 1.0f);
        CHECK(leftEyeCamera.farPlane() == 1000.0f);
        CHECK(leftEyeCamera.nearPlane() == 1.0f);
        CHECK(leftEyeCamera.fieldOfView() == 45.0f);
        COMPARE_VEC3(leftEyeCamera.upVector(), QVector3D(0.0f, 1.0f, 0.0f));
        COMPARE_VEC3(leftEyeCamera.position(), QVector3D(-5.0f, 0.0f, 100.0f));
        COMPARE_VEC3(leftEyeCamera.viewCenter(), QVector3D(-5.0f, 0.0f, 70.0f));

        // WHEN
        Qt3DRender::QCamera rightEyeCamera;
        updateQt3DStereoCamera(&qtCamera, &rightEyeCamera, interocularDistance, Eye::Right, false);

        // THEN
        CHECK(rightEyeCamera.aspectRatio() == 1.0f);
        CHECK(rightEyeCamera.farPlane() == 1000.0f);
        CHECK(rightEyeCamera.nearPlane() == 1.0f);
        CHECK(rightEyeCamera.fieldOfView() == 45.0f);
        COMPARE_VEC3(rightEyeCamera.upVector(), QVector3D(0.0f, 1.0f, 0.0f));
        COMPARE_VEC3(rightEyeCamera.position(), QVector3D(5.0f, 0.0f, 100.0f));
        COMPARE_VEC3(rightEyeCamera.viewCenter(), QVector3D(5.0f, 0.0f, 70.0f));

        {
            // If we are looking in the -Z dir and our up vector is +Y
            // Then the right vector is +X
            const QVector3D viewDirection = (qtCamera.viewCenter() - qtCamera.position()).normalized();
            COMPARE_VEC3(viewDirection, QVector3D(0.0f, 0.0f, -1.0f));
            const QVector3D right = QVector3D::crossProduct(viewDirection, qtCamera.upVector()).normalized();
            COMPARE_VEC3(right, QVector3D(1.0f, 0.0f, 0.0f));
        }

        constexpr bool logOutValues = true;
        if constexpr (logOutValues) {

            qDebug() << "allegiance stereo Left View" << glmToMat4(leftViewMatrix);
            qDebug() << "allegiance stereo Right View" << glmToMat4(rightViewMatrix);
            qDebug() << "Shear Coefficient" << camera.ShearCoefficient();
            qDebug() << "allegiance Center Projection " << glmToMat4(projMatrix);
            qDebug() << "allegiance Left Proj Shearing" << glmToMat4(shearingLeft) << glmToMat4(leftProjectionMatrix);
            qDebug() << "allegiance Right Proj Shearing" << glmToMat4(shearingRight) << glmToMat4(rightProjectionMatrix);
            qDebug() << "allegiance Left VP" << glmToMat4(leftProjectionMatrix * leftViewMatrix);
            qDebug() << "allegiance Right VP" << glmToMat4(rightProjectionMatrix * rightViewMatrix);

            qDebug() << "Qt3D Left View" << leftEyeCamera.viewMatrix();
            qDebug() << "Qt3D Right View" << rightEyeCamera.viewMatrix();
            qDebug() << "Qt3D Center Proj" << qtCamera.projectionMatrix();
            qDebug() << "Qt3D Left Proj" << leftEyeCamera.projectionMatrix();
            qDebug() << "Qt3D Right Proj" << rightEyeCamera.projectionMatrix();
            qDebug() << "Qt3D Left VP" << leftEyeCamera.projectionMatrix() * leftEyeCamera.viewMatrix();
            qDebug() << "Qt3D Right VP" << rightEyeCamera.projectionMatrix() * rightEyeCamera.viewMatrix();
        }

        // THEN
        COMPARE_MAT(glmToMat4(projMatrix), qtCamera.projectionMatrix());
        COMPARE_MAT(glmToMat4(leftProjectionMatrix * leftViewMatrix), (leftEyeCamera.projectionMatrix() * leftEyeCamera.viewMatrix()));
        COMPARE_MAT(glmToMat4(rightProjectionMatrix * rightViewMatrix), (rightEyeCamera.projectionMatrix() * rightEyeCamera.viewMatrix()));

        // WHEN
        auto projectStereoCameraPoint = [](const glm::mat4x4& proj, const glm::mat4x4& view, const glm::vec3& p) {
            glm::vec4 pP = proj * view * glm::vec4(p, 1.0);
            return glm::vec3(pP) / pP.w;
        };
        auto projectQtStereoCameraPoint = [](Qt3DRender::QCamera* camera, const QVector3D& p) {
            const QVector4D pP = (camera->projectionMatrix() * camera->viewMatrix()).map(QVector4D(p, 1.0f));
            return (pP / pP.w()).toVector3D();
        };

        // THEN
        const glm::vec3 viewCenter{ 0.0f, 0.0f, 70.0f };
        COMPARE_VEC3(projectStereoCameraPoint(camera.GetProjection() * shearingLeft, camera.GetViewLeft(), viewCenter),
                     projectQtStereoCameraPoint(&leftEyeCamera, glmToVec3(viewCenter)));
        COMPARE_VEC3(projectStereoCameraPoint(camera.GetProjection() * shearingRight, camera.GetViewRight(), viewCenter),
                     projectQtStereoCameraPoint(&rightEyeCamera, glmToVec3(viewCenter)));
    }
}
