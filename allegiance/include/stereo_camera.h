#pragma once
// #define GLM_FORCE_LEFT_HANDED
#include <glm/glm.hpp>

#if ALLEGIANCE_SERENITY
#include <kdbindings/signal.h>
#else
#include <QObject>

// inline Q_LOGGING_CATEGORY(lStereoCam, "allegiance.stereo_camera", QtInfoMsg);
#endif

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace all {
#if ALLEGIANCE_SERENITY
class StereoCamera
#else
class StereoCamera : public QObject
#endif
{
#if !ALLEGIANCE_SERENITY
    Q_OBJECT
#endif
public:
    StereoCamera()
    {
        UpdateViewMatrix();
        UpdateProjectionMatrix();
    }

public:
    glm::mat4x4 GetProjection() const noexcept { return projection; }
    glm::mat4x4 GetViewLeft() const noexcept { return view_left; }
    glm::mat4x4 GetViewRight() const noexcept { return view_right; }
    glm::mat4x4 GetViewCenter() const noexcept { return view_center; }
    glm::mat4x4 GetCameraMatrix() const noexcept { return camera_matrix; }

    void SetInterocularDistance(float distance) noexcept
    {
        interocular_distance = distance;
        UpdateViewMatrix();
    }
    float GetInterocularDistance() const noexcept { return interocular_distance; }

    void SetConvergencePlaneDistance(float distance) noexcept
    {
        convergence_plane_distance = distance;
        UpdateViewMatrix();
    }
    float GetConvergencePlaneDistance() const noexcept { return convergence_plane_distance; }

    void SetNearPlane(float distance) noexcept
    {
        near_plane = distance;
        UpdateProjectionMatrix();
    }
    float GetNearPlane() const noexcept { return near_plane; }

    void SetFarPlane(float distance) noexcept
    {
        far_plane = distance;
        UpdateProjectionMatrix();
    }
    float GetFarPlane() const noexcept { return far_plane; }

    void SetAspectRatio(float aspect_ratio) noexcept
    {
        this->aspect_ratio = aspect_ratio;
        UpdateProjectionMatrix();
    }
    float GetAspectRatio() const noexcept { return aspect_ratio; }

    void SetConvergeOnNear(bool converge) noexcept
    {
        converge_on_near = converge;
        UpdateViewMatrix();
    }
    bool GetConvergeOnNear() const noexcept { return converge_on_near; }

    void SetPosition(const glm::vec3& pos) noexcept
    {
        camera_matrix[3] = glm::vec4{pos, 1};
        //qCDebug(lStereoCam) << "set position " << camera_matrix[3];
        UpdateViewMatrix();
    }
    glm::vec3 GetPosition() const noexcept { return camera_matrix[3]; }

    void SetForwardVector(const glm::vec3& dir) noexcept
    {
        if (dir == glm::vec3(0.0f, 0.0f, 0.0f))
            return;

        auto r = glm::lookAt({0, 0, 0}, dir, {0, 1, 0});
        r = glm::inverse(r);
        camera_matrix[0] = r[0];
        camera_matrix[1] = r[1];
        camera_matrix[2] = r[2];
        UpdateViewMatrix();
    }
    glm::vec3 GetForwardVector() const noexcept { return -camera_matrix[2]; }

    void SetUpVector(const glm::vec3& up) noexcept
    {
        if (up == glm::vec3(0.0f, 0.0f, 0.0f))
            return;

        camera_matrix[1] = glm::vec4{glm::normalize(up), 0};
        UpdateViewMatrix();
    }
    glm::vec3 GetUpVector() const noexcept { return camera_matrix[1]; }

    float ShearCoefficient()const noexcept
    {
        return converge_on_near * interocular_distance * 0.5f / convergence_plane_distance;
    }
    void SetShear(bool shear) noexcept
    {
        this->shear = shear;
        UpdateViewMatrix();
    }
#if ALLEGIANCE_SERENITY
    KDBindings::Signal<> OnViewChanged;
    KDBindings::Signal<> OnProjectionChanged;
#else
Q_SIGNALS:
    void OnViewChanged();
    void OnProjectionChanged();
#endif

    // protected:
public:
    static glm::mat4 StereoShear(float x) noexcept
    {
        glm::mat4 i{ 1.0f };
        i[2][0] = x;
        return i;
    }

    void UpdateViewMatrix() noexcept
    {
        // we can do that, since right is unit length
        auto position = GetPosition();
        auto forward = GetForwardVector();
        auto up = GetUpVector();
        auto right = glm::normalize(glm::cross(forward, up)) * interocular_distance * 0.5f;
        view_left = shear
                ? StereoShear(ShearCoefficient()) * glm::lookAt(position - right, position -right + forward, up)
                : glm::lookAt(position - right, position -right + forward, up);
        view_right = shear
                ? StereoShear(-ShearCoefficient()) * glm::lookAt(position + right, position +right + forward, up)
                : glm::lookAt(position + right, position +right + forward, up);
        view_center = glm::lookAt(position, position + forward, up);

#if ALLEGIANCE_SERENITY
        OnViewChanged.emit();
#else
        Q_EMIT OnViewChanged();
#endif
    }
    void UpdateProjectionMatrix() noexcept
    {
        projection = glm::perspective(glm::radians(fov_y), aspect_ratio, near_plane, far_plane);
#if ALLEGIANCE_SERENITY
        OnProjectionChanged.emit();
#else
        Q_EMIT OnProjectionChanged();
#endif
    }

    void SetCameraMatrix(const glm::mat4x4& viewMatrix)
    {
        camera_matrix = viewMatrix;
        UpdateViewMatrix();
    }

    float GetFov() const
    {
        return fov_y;
    }
    float GetHorizontalFov() const
    {
        return glm::degrees(2.0f * std::atan(std::tan(glm::radians(fov_y / 2.0f)) * aspect_ratio));
    }
    private:
    glm::mat4x4 view_left;
    glm::mat4x4 view_right;
    glm::mat4x4 view_center;
    glm::mat4x4 camera_matrix = glm::identity<glm::mat4x4>();
    glm::mat4x4 projection;
    const float fov_y{ 45 };

private:
    float interocular_distance = 0.06f;
    float convergence_plane_distance = 10.0f;
    float near_plane = 0.1f;
    float far_plane = 1000.0f;
    float aspect_ratio = 1.0f;
    bool converge_on_near = true;
    bool shear = true;
};

class OrbitalStereoCamera : public StereoCamera
{
#if !ALLEGIANCE_SERENITY
    Q_OBJECT
#endif
public:
    OrbitalStereoCamera()
    {
        UpdateViewMatrix();
    }

public:
    void SetRadius(float radius) noexcept
    {
        this->radius = radius;
        UpdateViewMatrix();
    }
    float GetRadius() const noexcept { return radius; }

    void SetPhi(float phi) noexcept
    {
        this->phi = phi;
        UpdateViewMatrix();
    }
    float GetPhi() const noexcept { return phi; }

    void SetTheta(float theta) noexcept
    {
        this->theta = std::clamp(theta, 0.001f, glm::pi<float>() - 0.001f);
        UpdateViewMatrix();
    }
    float GetTheta() const noexcept { return theta; }

    void SetTarget(const glm::vec3& target) noexcept
    {
        this->target = target;
        UpdateViewMatrix();
    }
    glm::vec3 GetTarget() const noexcept { return target; }

private:
    void UpdateViewMatrix() noexcept
    {
        auto pos = target + glm::vec3(radius * sin(theta) * cos(phi), radius * cos(theta), radius * sin(theta) * sin(phi));
        SetPosition(pos);
        SetForwardVector( target - pos);
    }

private:
    float radius = 20.0f;
    float phi = 0.0f;
    float theta = 0.001f;
    glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
};
} // namespace all
