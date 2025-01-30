#include "scene_controller.h"

float SceneController::mouseSensitivity()
{
    return m_mouseSensitivity;
}

void SceneController::setMouseSensitivity(float sensitivity)
{
    if (sensitivity == m_mouseSensitivity)
        return;
    m_mouseSensitivity = sensitivity;
    Q_EMIT mouseSensitivityChanged();
}

bool SceneController::shiftPressed() const
{
    return m_shiftPressed;
}

void SceneController::setShiftPressed(bool newShiftPressed)
{
    if (m_shiftPressed == newShiftPressed)
        return;
    m_shiftPressed = newShiftPressed;
    Q_EMIT shiftPressedChanged();
}

float SceneController::mousePressedX() const
{
    return m_mousePressedX;
}

void SceneController::setMousePressedX(float newMousePressedX)
{
    if (m_mousePressedX == newMousePressedX)
        return;
    m_mousePressedX = newMousePressedX;
    Q_EMIT mousePressedXChanged();
}

bool SceneController::lockMouseInPlace() const
{
    return m_lockMouseInPlace;
}

void SceneController::setLockMouseInPlace(bool newLockMouseInPlace)
{
    m_lockMouseInPlace = newLockMouseInPlace;
}
