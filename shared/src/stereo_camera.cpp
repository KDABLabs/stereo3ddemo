#include <shared/stereo_camera.h>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <kdbindings/binding.h>

#include <algorithm>

namespace {

static float updateHorizontalFov(float fovY, float aspectRatio)
{
    return glm::degrees(2.0f * std::atan(std::tan(glm::radians(fovY / 2.0f)) * aspectRatio));
}

KDBINDINGS_DECLARE_FUNCTION(horizontalFovBinding, updateHorizontalFov)

} // namespace

namespace all {

StereoCamera::StereoCamera()
{
    horizontalFov = KDBindings::makeBoundProperty(horizontalFovBinding(fov, aspectRatio));

    forwardVector.valueChanged().connect([this] {
                                    viewChanged.emit(this);
                                    projectionChanged.emit(this);
                                })
            .release();
    upVector.valueChanged().connect([this] {
                               viewChanged.emit(this);
                           })
            .release();
    position.valueChanged().connect([this] {
                               viewChanged.emit(this);
                           })
            .release();
    fov.valueChanged().connect([this] {
                          projectionChanged.emit(this);
                      })
            .release();
    interocularDistance.valueChanged().connect([this] {
                                          viewChanged.emit(this);
                                          projectionChanged.emit(this);
                                      })
            .release();
    convergencePlaneDistance.valueChanged().connect([this] {
                                               viewChanged.emit(this);
                                               projectionChanged.emit(this);
                                           })
            .release();
    flipped.valueChanged().connect([this] {
                              viewChanged.emit(this);
                              projectionChanged.emit(this);
                          })
            .release();
    nearPlane.valueChanged().connect([this] {
                                projectionChanged.emit(this);
                            })
            .release();
    farPlane.valueChanged().connect([this] {
                               projectionChanged.emit(this);
                           })
            .release();
    aspectRatio.valueChanged().connect([this] {
                                  projectionChanged.emit(this);
                              })
            .release();
    mode.valueChanged().connect([this] {
                           viewChanged.emit(this);
                           projectionChanged.emit(this);
                       })
            .release();
}

void StereoCamera::setForwardVector(const glm::vec3& dir)
{
    if (dir == glm::vec3(0.0f, 0.0f, 0.0f))
        return;
    forwardVector = glm::normalize(dir);
}

void StereoCamera::setUpVector(const glm::vec3& up)
{
    if (up == glm::vec3(0.0f, 0.0f, 0.0f))
        return;
    upVector = glm::normalize(up);
}

OrbitalStereoCamera::OrbitalStereoCamera()
    : StereoCamera()
{
    radius.valueChanged().connect([this] { update(); }).release();
    phi.valueChanged().connect([this] { update(); }).release();
    theta.valueChanged().connect([this] { update(); }).release();
    target.valueChanged().connect([this] { update(); }).release();
    update();
}

void OrbitalStereoCamera::zoom(float d)
{
    position = position() + forwardVector() * d;
}

// return true, if Up Vector got flipped
bool OrbitalStereoCamera::rotate(float dx, float dy)
{
    auto up = upVector();
    glm::mat4 translateToPivot = glm::translate(glm::mat4(1.0f), target());
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), -dx, glm::vec3(0.0f, 1.0f, 0.0f));
    rotation = rotation * glm::rotate(glm::mat4(1.0f), dy, glm::cross(upVector(), forwardVector()));
    glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), -target());
    glm::mat4 finalTransform = translateToPivot * rotation * translateBack;
    glm::vec3 newPosition = glm::vec3(finalTransform * glm::vec4(position(), 1.0f));
    position = newPosition;
    setForwardVector(target() - newPosition);
    return glm::dot(up, upVector()) < 0;
}

void OrbitalStereoCamera::translate(float dx, float dy)
{
    glm::vec3 forward = glm::normalize(forwardVector());
    glm::vec3 lateralTranslation = dx * glm::normalize(glm::cross(forward, upVector()));
    glm::vec3 verticalTranslation = -dy * upVector();
    glm::vec3 newPosition = position() + lateralTranslation + verticalTranslation;

    position = newPosition;
}

void OrbitalStereoCamera::update()
{
    const float sinTheta = sin(theta());
    auto pos = target() + glm::vec3(radius() * sinTheta * cos(phi()), radius() * cos(theta()), radius() * sinTheta * sin(phi()));
    position = pos;
    setForwardVector(target() - pos);
}

} // namespace all
