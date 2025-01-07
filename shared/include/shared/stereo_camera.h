#pragma once
#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <algorithm>

namespace all {
// The camera is always stereo, this enum is used to determine which eye to render
enum class DisplayMode {
    Stereo,
    Mono,
    Left,
    Right
};

class StereoCamera
{
public:
    // TODO: Get Rid of this
    enum class API {
        OpenGL,
        Vulkan,
    };

    enum class Mode {
        ToeIn,
        AsymmetricFrustum,
    };

    explicit StereoCamera(API api)
        : m_api(api)
    {
        updateViewMatrix();
        updateProjectionMatrix();
    }

public:
    // TODO: Get Rid of this
    const glm::mat4x4& projection() const noexcept { return m_projection; }
    const glm::mat4x4& viewLeft() const noexcept { return m_viewLeft; }
    const glm::mat4x4& viewRight() const noexcept { return m_viewRight; }
    const glm::mat4x4& viewCenter() const noexcept { return m_viewCenter; }
    const glm::mat4x4& cameraMatrix() const noexcept { return m_cameraMatrix; }

    void setInterocularDistance(float distance) noexcept
    {
        m_interocularDistance = distance;
        updateViewMatrix();
        updateProjectionMatrix();
    }
    float interocularDistance() const noexcept { return m_interocularDistance; }

    void setConvergencePlaneDistance(float distance) noexcept
    {
        m_convergencePlaneDistance = distance;
        updateViewMatrix();
        updateProjectionMatrix();
    }
    float convergencePlaneDistance() const noexcept { return m_convergencePlaneDistance; }

    void setFlipped(bool flipped) noexcept
    {
        m_flipped = flipped;
        updateViewMatrix();
    }

    bool isFlipped() const noexcept { return m_flipped; }

    void setNearPlane(float distance) noexcept
    {
        m_nearPlane = distance;
        updateProjectionMatrix();
    }
    float nearPlane() const noexcept { return m_nearPlane; }

    void setFarPlane(float distance) noexcept
    {
        m_farPlane = distance;
        updateProjectionMatrix();
    }
    float farPlane() const noexcept { return m_farPlane; }

    void setAspectRatio(float aspect_ratio) noexcept
    {
        m_aspectRatio = aspect_ratio;
        updateProjectionMatrix();
    }
    float aspectRatio() const noexcept { return m_aspectRatio; }

    void setConvergeOnNear(bool converge) noexcept
    {
        m_convergeOnNear = converge;
        updateViewMatrix();
    }
    bool convergeOnNear() const noexcept { return m_convergeOnNear; }

    void setPosition(const glm::vec3& pos) noexcept
    {
        m_pos = pos;
        updateViewMatrix();
    }
    glm::vec3 position() const noexcept { return m_pos; }

    void setForwardVector(const glm::vec3& dir) noexcept
    {
        if (dir == glm::vec3(0.0f, 0.0f, 0.0f))
            return;

        m_forwardVec = glm::normalize(dir);
        updateViewMatrix();
    }
    glm::vec3 forwardVector() const noexcept { return m_forwardVec; }

    void setUpVector(const glm::vec3& up) noexcept
    {
        if (up == glm::vec3(0.0f, 0.0f, 0.0f))
            return;

        m_upVec = glm::normalize(up);
        updateViewMatrix();
    }
    glm::vec3 upVector() const noexcept { return m_upVec; }

    float shearCoefficient() const noexcept
    {
        if (!m_convergeOnNear)
            return 0.0f;
        float coef = m_interocularDistance * 0.5f / m_convergencePlaneDistance;
        return m_flipped ? -coef : coef;
    }
    void setShear(bool shear) noexcept
    {
        this->m_shear = shear;
        updateViewMatrix();
    }

    Mode mode() const noexcept { return m_mode; }

    void setMode(Mode mode)
    {
        if (m_mode == mode)
            return;
        m_mode = mode;
        updateViewMatrix();
        updateProjectionMatrix();
    }

    // TODO: Get Rid of this
    static glm::mat4 stereoShear(float x) noexcept
    {
        glm::mat4 i{ 1.0f };
        i[2][0] = x;
        return i;
    }

    // TODO: Get Rid of this
    virtual void updateViewMatrix() noexcept
    {
        // we can do that, since right is unit length
        auto pos = position();
        auto forward = forwardVector();
        auto up = upVector();
        auto right = glm::normalize(glm::cross(forward, up)) * m_interocularDistance * 0.5f;
        m_viewLeft = m_shear
                ? stereoShear(shearCoefficient()) * glm::lookAt(pos - right, pos - right + forward, up)
                : glm::lookAt(pos - right, pos - right + forward, up);
        m_viewRight = m_shear
                ? stereoShear(-shearCoefficient()) * glm::lookAt(pos + right, pos + right + forward, up)
                : glm::lookAt(pos + right, pos + right + forward, up);
        m_viewCenter = glm::lookAt(pos, pos + forward, up);
    }

    // TODO: Get Rid of this
    virtual void updateProjectionMatrix() noexcept
    {
        // Note: for OpenGL depth range is expected to be in [-1, 1] perspectiveRH_NO
        // Unlike Vulkan which expects depth range to be in [0, 1] perspectiveRH_ZO
        if (m_api == API::OpenGL) {
            m_projection = glm::perspectiveRH_NO(glm::radians(m_fovY), m_aspectRatio, m_nearPlane, m_farPlane);
        } else {
            m_projection = glm::perspectiveRH_ZO(glm::radians(m_fovY), m_aspectRatio, m_nearPlane, m_farPlane);
        }
    }

    float fov() const
    {
        return m_fovY;
    }
    void setFov(float fov)
    {
        m_fovY = fov;
        updateProjectionMatrix();
    }

    float horizontalFov() const
    {
        return glm::degrees(2.0f * std::atan(std::tan(glm::radians(m_fovY / 2.0f)) * m_aspectRatio));
    }

private:
    // TODO: Get Rid of the matrices
    glm::mat4x4 m_viewLeft;
    glm::mat4x4 m_viewRight;
    glm::mat4x4 m_viewCenter;
    glm::mat4x4 m_cameraMatrix = glm::identity<glm::mat4x4>();
    glm::mat4x4 m_projection;

    glm::vec3 m_forwardVec{ 0.0f, 0.0f, 1.0f };
    glm::vec3 m_upVec{ 0.0f, 1.0f, 0.0f };
    glm::vec3 m_pos{ 0.0f, 0.0f, 0.0f };

    float m_fovY{ 45.0f };
    float m_interocularDistance{ 0.06f };
    float m_convergencePlaneDistance{ 10.0f };
    float m_nearPlane{ 0.1f };
    float m_farPlane{ 1000.0f };
    float m_aspectRatio{ 1.0f };
    bool m_convergeOnNear{ true };
    bool m_shear{ true };
    bool m_flipped{ false };
    const API m_api;
    Mode m_mode{ Mode::AsymmetricFrustum };
};

class OrbitalStereoCamera : public StereoCamera
{
public:
    OrbitalStereoCamera(StereoCamera::API api)
        : StereoCamera(api)
    {
        updateViewMatrix2();
    }

public:
    void zoom(float d)
    {
        setPosition(position() + forwardVector() * d);
    }

    // return true, if Up Vector got flipped
    bool rotate(float dx, float dy)
    {
        auto up = upVector();
        glm::mat4 translateToPivot = glm::translate(glm::mat4(1.0f), m_target);
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), -dx, glm::vec3(0.0f, 1.0f, 0.0f));
        rotation = rotation * glm::rotate(glm::mat4(1.0f), dy, glm::cross(upVector(), forwardVector()));
        glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), -m_target);
        glm::mat4 finalTransform = translateToPivot * rotation * translateBack;
        glm::vec3 newPosition = glm::vec3(finalTransform * glm::vec4(position(), 1.0f));
        setPosition(newPosition);
        setForwardVector(m_target - newPosition);
        return glm::dot(up, upVector()) < 0;
    }

    void translate(float dx, float dy)
    {
        glm::vec3 forward = glm::normalize(forwardVector());
        glm::vec3 lateralTranslation = dx * glm::normalize(glm::cross(forward, upVector()));
        glm::vec3 verticalTranslation = -dy * upVector();
        glm::vec3 newPosition = position() + lateralTranslation + verticalTranslation;

        setPosition(newPosition);
    }

    void setRadius(float radius) noexcept
    {
        m_radius = radius;
        updateViewMatrix2();
    }
    float radius() const noexcept { return m_radius; }

    void setPhi(float phi) noexcept
    {
        m_phi = phi;
        updateViewMatrix2();
    }
    float phi() const noexcept { return m_phi; }

    void setTheta(float theta) noexcept
    {
        m_theta = std::clamp(theta, 0.001f, glm::pi<float>() - 0.001f);
        updateViewMatrix2();
    }
    float theta() const noexcept { return m_theta; }

    void setTarget(const glm::vec3& target) noexcept
    {
        m_target = target;
        updateViewMatrix2();
    }
    glm::vec3 target() const noexcept { return m_target; }

protected:
    void updateViewMatrix2() noexcept
    {
        const float sinTheta = sin(m_theta);
        auto pos = m_target + glm::vec3(m_radius * sinTheta * cos(m_phi), m_radius * cos(m_theta), m_radius * sinTheta * sin(m_phi));
        setPosition(pos);
        setForwardVector(m_target - pos);
    }

private:
    float m_radius = 20.0f;
    float m_phi = 0.0f;
    float m_theta = 0.001f;
    glm::vec3 m_target = glm::vec3(0.0f, 0.0f, 0.0f);
};
} // namespace all
