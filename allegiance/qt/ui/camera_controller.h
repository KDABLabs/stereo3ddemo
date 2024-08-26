#pragma once
#include <QUrl>
#include <QObject>
#include <shared/cursor.h>
#include <shared/stereo_camera.h>

class SceneController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(float mouseSensitivity READ GetMouseSensitivity WRITE SetMouseSensitivity NOTIFY OnMouseSensitivityChanged)
public:
    float GetMouseSensitivity()
    {
        return m_mouseSensitivity;
    }
    void SetMouseSensitivity(float sensitivity)
    {
        if (sensitivity == m_mouseSensitivity)
            return;
        m_mouseSensitivity = sensitivity;
        Q_EMIT OnMouseSensitivityChanged();
    }

public:
Q_SIGNALS:
    void OpenLoadModelDialog();
    void OnMouseSensitivityChanged();

protected:
    float m_mouseSensitivity = 100;
};

class CameraController : public QObject
{
    Q_OBJECT
public:
    enum class CameraMode {
        Stereo = int(all::CameraMode::Stereo),
        Mono = int(all::CameraMode::Mono),
        Left = int(all::CameraMode::Left),
        Right = int(all::CameraMode::Right)
    };
    Q_ENUM(CameraMode);

    Q_PROPERTY(float eyeDistance READ EyeDistance WRITE SetEyeDistance NOTIFY OnEyeDistanceChanged)
    Q_PROPERTY(float focusDistance READ FocusDistance WRITE SetFocusDistance NOTIFY OnFocusDistanceChanged)
    Q_PROPERTY(float focusAngle READ FocusAngle WRITE SetFocusAngle NOTIFY OnFocusAngleChanged)
    Q_PROPERTY(float fov READ FOV WRITE SetFOV NOTIFY OnFOVChanged)
    Q_PROPERTY(bool flipped READ Flipped WRITE SetFlipped NOTIFY OnFlippedChanged)
    Q_PROPERTY(CameraMode cameraMode READ GetCameraMode WRITE SetCameraMode NOTIFY OnCameraModeChanged)
public:
    CameraController(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

public:
    void SetEyeDistance(float eyeDistance) noexcept
    {
        m_eyeDistance = eyeDistance;
        OnEyeDistanceChanged(m_eyeDistance);
    }
    float EyeDistance() const noexcept
    {
        return m_eyeDistance;
    }
    void SetFocusDistance(float focusDistance) noexcept
    {
        if (qFuzzyCompare(m_focusDistance, focusDistance))
            return;

        m_focusDistance = focusDistance;
        m_focusAngle = qRadiansToDegrees(std::atan(m_focusDistance / (m_eyeDistance * 0.5)));
        OnFocusDistanceChanged(m_focusDistance);
        OnFocusAngleChanged(m_focusAngle);
    }
    float FocusDistance() const noexcept
    {
        return m_focusDistance;
    }

    float FocusAngle() const noexcept
    {
        return m_focusAngle;
    }
    void SetFocusAngle(float focusAngle) noexcept
    {
        if (qFuzzyCompare(m_focusAngle, focusAngle))
            return;

        m_focusAngle = focusAngle;
        auto rad = qDegreesToRadians(m_focusAngle);
        m_focusDistance = qTan(rad) * m_eyeDistance * 0.5f;
        OnFocusAngleChanged(m_focusAngle);
        OnFocusDistanceChanged(m_focusDistance);
    }

    void SetFOV(float fov) noexcept
    {
        m_fov = fov;
        OnFOVChanged(fov);
    }
    float FOV() const noexcept
    {
        return m_fov;
    }

    void SetFlipped(bool flipped) noexcept
    {
        m_flipped = flipped;
        OnFlippedChanged(flipped);
    }
    bool Flipped() const noexcept
    {
        return m_flipped;
    }

    void SetCameraMode(CameraMode mode) noexcept
    {
        m_cameraMode = mode;
        OnCameraModeChanged(mode);
    }
    CameraMode GetCameraMode() const noexcept
    {
        return m_cameraMode;
    }
Q_SIGNALS:
    void OnEyeDistanceChanged(float eyeDistance);
    void OnFocusDistanceChanged(float focusDistance);
    void OnFocusAngleChanged(float focusAngle);
    void OnFOVChanged(float fov);
    void OnFlippedChanged(bool flipped);
    void OnCameraModeChanged(CameraMode mode);

private:
    CameraMode m_cameraMode = CameraMode::Stereo;
    float m_eyeDistance = 0.06f;
    float m_focusDistance = 10.0f;
    float m_focusAngle = 80.0f;
    float m_fov = 60.0f;
    bool m_flipped = false;
};

class CursorController : public QObject
{
public:
    enum class CursorType {
        Ball = int(all::CursorType::Ball),
        Cross = int(all::CursorType::Cross),
        CrossHair = int(all::CursorType::CrossHair),
        Dot = int(all::CursorType::Dot)
    };
    Q_ENUM(CursorType);

public:
    Q_OBJECT
    Q_PROPERTY(float visible READ Visible WRITE SetVisible NOTIFY OnCursorVisibilityChanged)
    Q_PROPERTY(CursorType cursor READ Cursor WRITE SetCursorType NOTIFY OnCursorChanged)
    Q_PROPERTY(bool scalingEnabled READ ScalingEnabled WRITE SetScalingEnabled NOTIFY OnCursorScalingEnableChanged)
    Q_PROPERTY(float scaleFactor READ ScaleFactor WRITE SetScaleFactor NOTIFY OnCursorScaleChanged)
    Q_PROPERTY(bool cursorFocus READ CursorChangesFocus WRITE SetCursorChangesFocus NOTIFY OnCursorFocusChanged)
public:
    CursorController(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

public:
    void SetVisible(bool visible) noexcept
    {
        m_visible = visible;
        OnCursorVisibilityChanged(visible);
    }
    bool Visible() const noexcept
    {
        return m_visible;
    }
    void SetCursorType(CursorType cursorType) noexcept
    {
        m_cursorType = all::CursorType(cursorType);
        OnCursorChanged(m_cursorType);
    }
    CursorType Cursor() const noexcept
    {
        return CursorType(m_cursorType);
    }
    void SetScalingEnabled(bool enabled) noexcept
    {
        m_scaling_enabled = enabled;
        OnCursorScalingEnableChanged(enabled);
    }
    void SetScaleFactor(float scale_factor) noexcept
    {
        m_scale_factor = scale_factor;
        OnCursorScaleChanged(scale_factor);
    }

    bool ScalingEnabled() const noexcept
    {
        return m_scaling_enabled;
    }
    float ScaleFactor() const noexcept
    {
        return m_scale_factor;
    }

    bool CursorChangesFocus() const noexcept
    {
        return m_cursor_focus;
    }
    void SetCursorChangesFocus(bool focus) noexcept
    {
        m_cursor_focus = focus;
        OnCursorFocusChanged(focus);
    }
Q_SIGNALS:
    void OnCursorVisibilityChanged(bool state);
    void OnCursorChanged(all::CursorType cursorType);
    void OnCursorScaleChanged(float scale);
    void OnCursorScalingEnableChanged(bool enabled);
    void OnCursorFocusChanged(bool focus);

private:
    bool m_visible = true;
    all::CursorType m_cursorType = all::CursorType::Ball;
    bool m_cursor_focus = false;
    bool m_scaling_enabled = true;
    float m_scale_factor = 1.0f;
};