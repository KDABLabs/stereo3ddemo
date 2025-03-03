#pragma once
#include <glm/glm.hpp>

#include <kdbindings/property.h>
#include <kdbindings/signal.h>

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
    enum class Mode {
        ToeIn,
        AsymmetricFrustum,
    };

    KDBindings::Property<glm::vec3> forwardVector{ { 0.0f, 0.0f, 1.0f } };
    KDBindings::Property<glm::vec3> upVector{ { 0.0f, 1.0f, 0.0f } };
    KDBindings::Property<glm::vec3> position{ { 0.0f, 0.0f, 0.0f } };
    KDBindings::Property<glm::vec3> worldCursor{ { 0.0f, 0.0f, 0.0f } };
    KDBindings::Property<glm::vec3> viewCenter;

    KDBindings::Property<float> fov{ 45.0f };
    KDBindings::Property<float> horizontalFov;
    KDBindings::Property<float> interocularDistance{ 0.06f };
    KDBindings::Property<float> convergencePlaneDistance{ 10.0f };
    KDBindings::Property<bool> flipped{ false };

    KDBindings::Property<float> nearPlane{ 0.1f };
    KDBindings::Property<float> farPlane{ 1000.0f };
    KDBindings::Property<float> aspectRatio{ 1.0f };
    KDBindings::Property<Mode> mode{ Mode::AsymmetricFrustum };

    KDBindings::Signal<StereoCamera*> viewChanged;
    KDBindings::Signal<StereoCamera*> projectionChanged;

    StereoCamera();
    virtual ~StereoCamera() = default;

    void setForwardVector(const glm::vec3& dir);
    void setUpVector(const glm::vec3& up);
};

class OrbitalStereoCamera : public StereoCamera
{
public:
    OrbitalStereoCamera();

    KDBindings::Property<glm::vec3> target{ glm::vec3(0.0f, 0.0f, 0.0f) };

public:
    void zoom(float d);

    // return true, if Up Vector got flipped
    bool rotate(float dx, float dy);

    void translate(float dx, float dy);
};

} // namespace all
