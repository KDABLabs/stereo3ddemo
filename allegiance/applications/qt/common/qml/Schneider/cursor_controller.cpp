#include "cursor_controller.h"

CursorController::CursorController(QObject* parent)
    : QObject(parent)
{
}

void CursorController::setCursorType(CursorType cursorType)
{
    if (m_cursorType == all::CursorType(cursorType))
        return;
    m_cursorType = all::CursorType(cursorType);
    cursorChanged(m_cursorType);
}

void CursorController::setScalingEnabled(bool enabled)
{
    if (m_scaling_enabled == enabled)
        return;
    m_scaling_enabled = enabled;
    cursorScalingEnableChanged(enabled);
}

void CursorController::setScaleFactor(float scale_factor)
{
    if (m_scale_factor == scale_factor)
        return;
    m_scale_factor = scale_factor;
    cursorScaleChanged(scale_factor);
}

void CursorController::setCursorTint(QColor color)
{
    if (m_tint == color)
        return;
    m_tint = color;
    cursorTintChanged(color);
}

float CursorController::scaleFactor() const
{
    return m_scale_factor;
}

bool CursorController::scalingEnabled() const
{
    return m_scaling_enabled;
}

CursorController::CursorType CursorController::cursor() const
{
    return CursorType(m_cursorType);
}

QColor CursorController::cursorTint() const
{
    return m_tint;
}

void CursorController::setHue(float hue)
{
    if (m_cursorHue == hue)
        return;

    m_cursorHue = hue;
    cursorHueChanged(m_cursorHue);
    setCursorTint(QColor::fromHsv(std::clamp(hue, 0.0f, 359.0f), 200, 255, 255));
}

float CursorController::hue()
{
    return m_cursorHue;
}

CursorController::CursorDisplayMode CursorController::displayMode() const
{
    return CursorDisplayMode(m_displayMode);
}

void CursorController::setDisplayMode(CursorDisplayMode displayMode)
{
    if (m_displayMode == all::CursorDisplayMode(displayMode))
        return;

    m_displayMode = all::CursorDisplayMode(displayMode);
    displayModeChanged(m_displayMode);
}

void CursorController::cycleDisplayMode()
{
    // both -> 3d cursor
    if (m_displayMode == all::CursorDisplayMode::Both) {
        setDisplayMode(CursorDisplayMode::ThreeDimensionalOnly);
        return;
    }

    // 3d cursor -> default
    if (m_displayMode == all::CursorDisplayMode::ThreeDimensionalOnly) {
        setDisplayMode(CursorDisplayMode::SystemCursorOnly);
        return;
    }

    // default -> both
    setDisplayMode(CursorDisplayMode::Both);
}
