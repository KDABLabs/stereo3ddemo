#pragma once
#include <glm/glm.hpp>
#include <QObject>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace all {
class StereoCamera : public QObject
{
    Q_OBJECT
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
        position = pos;
        UpdateViewMatrix();
    }
    glm::vec3 GetPosition() const noexcept { return position; }

    void SetForwardVector(const glm::vec3& dir) noexcept
    {
        if (dir == glm::vec3(0.0f, 0.0f, 0.0f))
            return;

        forward = glm::normalize(dir);
        UpdateViewMatrix();
    }
    glm::vec3 GetForwardVector() const noexcept { return forward; }

    void SetUpVector(const glm::vec3& up) noexcept
    {
        if (up == glm::vec3(0.0f, 0.0f, 0.0f))
            return;

        this->up = glm::normalize(up);
        UpdateViewMatrix();
    }
    glm::vec3 GetUpVector() const noexcept { return up; }
signals:
    void OnViewChanged();
    void OnProjectionChanged();

protected:
    float ShearCoefficient() noexcept
    {
        return converge_on_near * interocular_distance * 0.5f / convergence_plane_distance;
    }
    static glm::mat4 StereoShear(float x) noexcept
    {
        glm::mat4 i{ 1.0f };
        i[2][0] = x;
        return i;
    }

    void UpdateViewMatrix() noexcept
    {
        // we can do that, since right is unit length
        auto right = glm::normalize(glm::cross(forward, up)) * interocular_distance * 0.5f;
        view_left = StereoShear(ShearCoefficient()) * glm::lookAt(position - right, -right + forward, up);
        view_right = StereoShear(-ShearCoefficient()) * glm::lookAt(position + right, +right + forward, up);
        view_center = glm::lookAt(position, forward, up);
        emit OnViewChanged();
    }
    void UpdateProjectionMatrix() noexcept
    {
        projection = glm::perspective(glm::radians(45.0f), aspect_ratio, near_plane, far_plane);
        emit OnProjectionChanged();
    }

private:
    glm::mat4x4 view_left;
    glm::mat4x4 view_right;
    glm::mat4x4 view_center;
    glm::mat4x4 projection;

private:
    float interocular_distance = 0.06f;
    float convergence_plane_distance = 10.0f;
    float near_plane = 0.1f;
    float far_plane = 1000.0f;
    float aspect_ratio = 1.0f;
    bool converge_on_near = true;

    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 forward = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
};

class OrbitalStereoCamera : public StereoCamera
{
    Q_OBJECT
public:
    OrbitalStereoCamera() = default;

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
        this->theta = theta;
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
        SetForwardVector(target - pos);
    }

private:
    float radius = 20.0f;
    float phi = 0.0f;
    float theta = 0.0f;
    glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
};
} // namespace all