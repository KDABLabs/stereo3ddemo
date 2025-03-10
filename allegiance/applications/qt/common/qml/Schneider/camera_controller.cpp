#include "camera_controller.h"

CameraController::CameraController(QObject* parent)
    : QObject(parent)
{
    QObject::connect(this, &CameraController::screenHeightChanged, this, &CameraController::updateFovFromDims);
    QObject::connect(this, &CameraController::viewerDistanceChanged, this, &CameraController::updateFovFromDims);
    QObject::connect(this, &CameraController::fovByPhysicalDimChanged, [this](bool computeFovFromDims) {
        if (computeFovFromDims)
            updateFovFromDims();
    });
}

void CameraController::setFov(float fov)
{
    if (m_fov == fov)
        return;
    m_fov = fov;
    Q_EMIT fovChanged(fov);
}

float CameraController::fov() const
{
    return m_fov;
}

void CameraController::setEyeDistance(float eyeDistance)
{
    const float safeEyeDistance = std::clamp(eyeDistance, CameraController::MinEyeDistance, CameraController::MaxEyeDistance);
    if (m_eyeDistance == safeEyeDistance)
        return;
    m_eyeDistance = safeEyeDistance;
    Q_EMIT eyeDistanceChanged(m_eyeDistance);
}

float CameraController::eyeDistance() const
{
    return m_eyeDistance;
}

bool CameraController::separationBasedOnFocusDistance() const
{
    return m_separationBasedOnFocusDistance;
}

void CameraController::setSeparationBasedOnFocusDistance(bool newSeparationBasedOnFocusDistance)
{
    if (m_separationBasedOnFocusDistance == newSeparationBasedOnFocusDistance)
        return;
    m_separationBasedOnFocusDistance = newSeparationBasedOnFocusDistance;
    Q_EMIT separationBasedOnFocusDistanceChanged(m_separationBasedOnFocusDistance);
}

void CameraController::setFocusDistance(float focusDistance)
{
    const float safeFocusDistance = std::clamp(focusDistance, CameraController::MinFocusDistance, CameraController::MaxFocusDistance);

    if (qFuzzyCompare(m_focusDistance, safeFocusDistance))
        return;
    m_focusDistance = safeFocusDistance;
    Q_EMIT focusDistanceChanged(m_focusDistance);
}

float CameraController::focusDistance() const
{
    return m_focusDistance;
}

void CameraController::setFlipped(bool flipped)
{
    if (m_flipped == flipped)
        return;
    m_flipped = flipped;
    Q_EMIT flippedChanged(flipped);
}

bool CameraController::flipped() const
{
    return m_flipped;
}

bool CameraController::autoFocus() const
{
    return m_autoFocus;
}

void CameraController::setAutoFocus(bool newAutoFocus)
{
    if (m_autoFocus == newAutoFocus)
        return;
    m_autoFocus = newAutoFocus;
    Q_EMIT autoFocusChanged(m_autoFocus);
}

bool CameraController::showAutoFocusArea() const
{
    return m_showAutoFocusArea;
}

void CameraController::setShowAutoFocusArea(bool newShowAutoFocusArea)
{
    if (m_showAutoFocusArea == newShowAutoFocusArea)
        return;
    m_showAutoFocusArea = newShowAutoFocusArea;
    Q_EMIT showAutoFocusAreaChanged(m_showAutoFocusArea);
}

float CameraController::screenHeight() const
{
    return m_screenHeight;
}

void CameraController::setScreenHeight(float newScreenHeight)
{
    if (m_screenHeight == newScreenHeight)
        return;
    m_screenHeight = newScreenHeight;
    Q_EMIT screenHeightChanged(m_screenHeight);
}

float CameraController::viewerDistance() const
{
    return m_viewerDistance;
}

void CameraController::setViewerDistance(float newViewerDistance)
{
    if (m_viewerDistance == newViewerDistance)
        return;
    m_viewerDistance = newViewerDistance;
    Q_EMIT viewerDistanceChanged(newViewerDistance);
}

float CameraController::popOut() const
{
    return m_popOut;
}

void CameraController::setPopOut(float newPopOut)
{
    const float safePopOut = std::clamp(newPopOut, -100.0f, 100.0f);
    if (m_popOut == safePopOut)
        return;
    m_popOut = safePopOut;
    Q_EMIT popOutChanged(m_popOut);
}

void CameraController::setStereoMode(StereoMode newStereoMode)
{
    if (newStereoMode == m_stereoMode)
        return;
    m_stereoMode = newStereoMode;
    Q_EMIT stereoModeChanged(newStereoMode);
}
CameraController::StereoMode CameraController::stereoMode() const
{
    return m_stereoMode;
}

void CameraController::setDisplayMode(DisplayMode mode)
{
    if (m_displayMode == mode)
        return;
    m_displayMode = mode;
    Q_EMIT displayModeChanged(mode);
}

CameraController::DisplayMode CameraController::displayMode() const
{
    return m_displayMode;
}

bool CameraController::fovByPhysicalDim() const
{
    return m_fovByPhysicalDim;
}

void CameraController::setFovByPhysicalDim(bool newFovByPhysicalDim)
{
    if (m_fovByPhysicalDim == newFovByPhysicalDim)
        return;
    m_fovByPhysicalDim = newFovByPhysicalDim;
    Q_EMIT fovByPhysicalDimChanged(m_fovByPhysicalDim);
}

void CameraController::updateFovFromDims()
{
    // We have distance to screen and screen height
    // Given tan = opposite / adjacent <==> screen height / distance
    // we can compute the vfov as 2 * atan(screen height / distance)
    const float vFov = 2.0f * qRadiansToDegrees(atan(screenHeight() / viewerDistance()));
    setFov(vFov);
}

bool CameraController::showFocusPlane() const
{
    return m_showFocusPlane;
}

void CameraController::setShowFocusPlane(bool newShowFocusPlane)
{
    if (m_showFocusPlane == newShowFocusPlane)
        return;
    m_showFocusPlane = newShowFocusPlane;
    Q_EMIT showFocusPlaneChanged(m_showFocusPlane);
}

int CameraController::separationBasedOnFocusDistanceDivider() const
{
    return m_separationBasedOnFocusDistanceDivider;
}

void CameraController::setSeparationBasedOnFocusDistanceDivider(int newSeparationBasedOnFocusDistanceDivider)
{
    if (m_separationBasedOnFocusDistanceDivider == newSeparationBasedOnFocusDistanceDivider)
        return;
    m_separationBasedOnFocusDistanceDivider = newSeparationBasedOnFocusDistanceDivider;
    Q_EMIT separationBasedOnFocusDistanceDividerChanged(m_separationBasedOnFocusDistanceDivider);
}

float CameraController::eyeDistanceDefaultValue() const
{
    return ms_eyeDistanceDefaultValue;
}

int CameraController::separationBasedOnFocusDistanceDividerDefaultValue() const
{
    return ms_separationBasedOnFocusDistanceDividerDefaultValue;
}

float CameraController::focusDistanceDefaultValue() const
{
    return ms_focusDistanceDefaultValue;
}

float CameraController::popOutDefaultValue() const
{
    return ms_popOutDefaultValue;
}

float CameraController::screenHeightDefaultValue() const
{
    return ms_screenHeightDefaultValue;
}

float CameraController::viewerDistanceDefaultValue() const
{
    return ms_viewerDistanceDefaultValue;
}

float CameraController::fovDefaultValue() const
{
    return ms_fovDefaultValue;
}
