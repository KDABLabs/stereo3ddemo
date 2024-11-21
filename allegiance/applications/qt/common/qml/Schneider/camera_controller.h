#pragma once
#include <QUrl>
#include <QObject>
#include <QColor>
#include <shared/cursor.h>
#include <shared/stereo_camera.h>
#include <QtQml/qqmlregistration.h>

class SceneController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(float mouseSensitivity READ mouseSensitivity WRITE setMouseSensitivity NOTIFY mouseSensitivityChanged)
    QML_SINGLETON
    QML_NAMED_ELEMENT(Scene)
public:
    float mouseSensitivity()
    {
        return m_mouseSensitivity;
    }
    void setMouseSensitivity(float sensitivity)
    {
        if (sensitivity == m_mouseSensitivity)
            return;
        m_mouseSensitivity = sensitivity;
        Q_EMIT mouseSensitivityChanged();
    }

Q_SIGNALS:
    void OpenLoadModelDialog();
    void mouseSensitivityChanged();

protected:
    float m_mouseSensitivity = 100;
};

class CameraController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(float eyeDistance READ eyeDistance WRITE setEyeDistance NOTIFY eyeDistanceChanged)
    Q_PROPERTY(float focusDistance READ focusDistance WRITE setFocusDistance NOTIFY focusDistanceChanged)
    Q_PROPERTY(float focusAngle READ focusAngle WRITE setFocusAngle NOTIFY focusAngleChanged)
    Q_PROPERTY(float fov READ fov WRITE setFov NOTIFY fovChanged)
    Q_PROPERTY(bool flipped READ flipped WRITE setFlipped NOTIFY flippedChanged)
    Q_PROPERTY(CameraMode cameraMode READ cameraMode WRITE setCameraMode NOTIFY cameraModeChanged)
    QML_SINGLETON
    QML_NAMED_ELEMENT(Camera)

public:
    enum class CameraMode {
        Stereo = int(all::CameraMode::Stereo),
        Mono = int(all::CameraMode::Mono),
        Left = int(all::CameraMode::Left),
        Right = int(all::CameraMode::Right)
    };
    Q_ENUM(CameraMode);

    CameraController(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

    void setEyeDistance(float eyeDistance) noexcept
    {
        m_eyeDistance = eyeDistance;
        eyeDistanceChanged(m_eyeDistance);
    }
    float eyeDistance() const noexcept
    {
        return m_eyeDistance;
    }
    void setFocusDistance(float focusDistance) noexcept
    {
        if (qFuzzyCompare(m_focusDistance, focusDistance))
            return;

        m_focusDistance = focusDistance;
        m_focusAngle = qRadiansToDegrees(std::atan(m_focusDistance / (m_eyeDistance * 0.5)));
        focusDistanceChanged(m_focusDistance);
        focusAngleChanged(m_focusAngle);
    }
    float focusDistance() const noexcept
    {
        return m_focusDistance;
    }

    float focusAngle() const noexcept
    {
        return m_focusAngle;
    }
    void setFocusAngle(float focusAngle) noexcept
    {
        if (qFuzzyCompare(m_focusAngle, focusAngle))
            return;

        m_focusAngle = focusAngle;
        auto rad = qDegreesToRadians(m_focusAngle);
        m_focusDistance = qTan(rad) * m_eyeDistance * 0.5f;
        focusAngleChanged(m_focusAngle);
        focusDistanceChanged(m_focusDistance);
    }

    void setFov(float fov) noexcept
    {
        m_fov = fov;
        fovChanged(fov);
    }
    float fov() const noexcept
    {
        return m_fov;
    }

    void setFlipped(bool flipped) noexcept
    {
        m_flipped = flipped;
        flippedChanged(flipped);
    }
    bool flipped() const noexcept
    {
        return m_flipped;
    }

    void setCameraMode(CameraMode mode) noexcept
    {
        m_cameraMode = mode;
        cameraModeChanged(mode);
    }
    CameraMode cameraMode() const noexcept
    {
        return m_cameraMode;
    }
Q_SIGNALS:
    void eyeDistanceChanged(float);
    void focusDistanceChanged(float);
    void focusAngleChanged(float);
    void fovChanged(float);
    void flippedChanged(bool);
    void cameraModeChanged(CameraMode);

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
    Q_OBJECT
    Q_PROPERTY(float visible READ visible WRITE setVisible NOTIFY cursorVisibilityChanged)
    Q_PROPERTY(CursorType cursor READ cursor WRITE setCursorType NOTIFY cursorChanged)
    Q_PROPERTY(bool scalingEnabled READ scalingEnabled WRITE setScalingEnabled NOTIFY cursorScalingEnableChanged)
    Q_PROPERTY(float scaleFactor READ scaleFactor WRITE setScaleFactor NOTIFY cursorScaleChanged)
    Q_PROPERTY(bool cursorFocus READ cursorChangesFocus WRITE setCursorChangesFocus NOTIFY cursorFocusChanged)
    Q_PROPERTY(QColor cursorTint READ cursorTint WRITE setCursorTint NOTIFY cursorTintChanged)
    QML_SINGLETON
    QML_NAMED_ELEMENT(Cursor)

public:
    enum class CursorType {
        Ball = int(all::CursorType::Ball),
        Cross = int(all::CursorType::Cross),
        CrossHair = int(all::CursorType::CrossHair),
        Dot = int(all::CursorType::Dot)
    };
    Q_ENUM(CursorType);

    CursorController(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

    void setVisible(bool visible) noexcept
    {
        m_visible = visible;
        cursorVisibilityChanged(visible);
    }
    bool visible() const noexcept
    {
        return m_visible;
    }
    void setCursorType(CursorType cursorType) noexcept
    {
        m_cursorType = all::CursorType(cursorType);
        cursorChanged(m_cursorType);
    }
    CursorType cursor() const noexcept
    {
        return CursorType(m_cursorType);
    }
    void setScalingEnabled(bool enabled) noexcept
    {
        m_scaling_enabled = enabled;
        cursorScalingEnableChanged(enabled);
    }
    void setScaleFactor(float scale_factor) noexcept
    {
        m_scale_factor = scale_factor;
        cursorScaleChanged(scale_factor);
    }

    bool scalingEnabled() const noexcept
    {
        return m_scaling_enabled;
    }
    float scaleFactor() const noexcept
    {
        return m_scale_factor;
    }

    bool cursorChangesFocus() const noexcept
    {
        return m_cursor_focus;
    }
    void setCursorChangesFocus(bool focus) noexcept
    {
        m_cursor_focus = focus;
        cursorFocusChanged(focus);
    }

    void setCursorTint(QColor color) noexcept
    {
        m_tint = color;
        cursorTintChanged(color);
    }
    QColor cursorTint() const noexcept
    {
        return m_tint;
    }

Q_SIGNALS:
    void cursorVisibilityChanged(bool state);
    void cursorChanged(all::CursorType cursorType);
    void cursorScaleChanged(float scale);
    void cursorScalingEnableChanged(bool enabled);
    void cursorFocusChanged(bool focus);
    void cursorTintChanged(QColor color);

private:
    bool m_visible = true;
    all::CursorType m_cursorType = all::CursorType::Ball;
    bool m_cursor_focus = false;
    bool m_scaling_enabled = true;
    float m_scale_factor = 1.0f;
    QColor m_tint = QColor(255, 255, 255);
};
