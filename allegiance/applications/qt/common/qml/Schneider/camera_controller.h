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
    float mouseSensitivity();
    void setMouseSensitivity(float sensitivity);

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
    Q_PROPERTY(bool flipped READ flipped WRITE setFlipped NOTIFY flippedChanged)

    Q_PROPERTY(bool frustumViewEnabled READ frustumViewEnabled WRITE setFrustumViewEnabled NOTIFY frustumViewEnabledChanged)
    Q_PROPERTY(CameraMode cameraMode READ cameraMode WRITE setCameraMode NOTIFY cameraModeChanged)
    Q_PROPERTY(StereoMode stereoMode READ stereoMode WRITE setStereoMode NOTIFY stereoModeChanged)

    Q_PROPERTY(bool autoFocus READ autoFocus WRITE setAutoFocus NOTIFY autoFocusChanged)
    Q_PROPERTY(bool showAutoFocusArea READ showAutoFocusArea WRITE setShowAutoFocusArea NOTIFY showAutoFocusAreaChanged)
    Q_PROPERTY(float focusDistance READ focusDistance WRITE setFocusDistance NOTIFY focusDistanceChanged)
    Q_PROPERTY(float popOut READ popOut WRITE setPopOut NOTIFY popOutChanged)

    Q_PROPERTY(float fov READ fov WRITE setFov NOTIFY fovChanged)
    Q_PROPERTY(bool fovByPhysicalDim READ fovByPhysicalDim WRITE setFovByPhysicalDim NOTIFY fovByPhysicalDimChanged)
    Q_PROPERTY(float screenHeight READ screenHeight WRITE setScreenHeight NOTIFY screenHeightChanged)
    Q_PROPERTY(float viewerDistance READ viewerDistance WRITE setViewerDistance NOTIFY viewerDistanceChanged)

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

    enum class StereoMode {
        ToeIn = int(all::StereoCamera::Mode::ToeIn),
        AsymmetricFrustum = int(all::StereoCamera::Mode::AsymmetricFrustum),
    };
    Q_ENUM(StereoMode)

    CameraController(QObject* parent = nullptr);

    void setEyeDistance(float eyeDistance);
    float eyeDistance() const;

    void setFocusDistance(float focusDistance);
    float focusDistance() const;

    float focusAngle() const;

    void setFov(float fov);
    float fov() const;

    void setFlipped(bool flipped);
    bool flipped() const;

    void setCameraMode(CameraMode mode);
    CameraMode cameraMode() const;

    StereoMode stereoMode() const;
    void setStereoMode(StereoMode newStereoMode);

    bool frustumViewEnabled() const;
    void setFrustumViewEnabled(bool newFrustumViewEnabled);

    bool autoFocus() const;
    void setAutoFocus(bool newAutoFocus);

    bool showAutoFocusArea() const;
    void setShowAutoFocusArea(bool newShowAutoFocusArea);

    float screenHeight() const;
    void setScreenHeight(float newScreenHeight);

    float viewerDistance() const;
    void setViewerDistance(float newViewerDistance);

    float popOut() const;
    void setPopOut(float newPopOut);

    bool fovByPhysicalDim() const;
    void setFovByPhysicalDim(bool newFovByPhysicalDim);

Q_SIGNALS:
    void eyeDistanceChanged(float);
    void flippedChanged(bool);

    void cameraModeChanged(CameraMode);
    void stereoModeChanged(StereoMode);
    void frustumViewEnabledChanged(bool);

    void focusDistanceChanged(float);
    void popOutChanged(float);
    void autoFocusChanged(bool);
    void showAutoFocusAreaChanged(bool);

    void fovChanged(float);
    void fovByPhysicalDimChanged(bool);
    void screenHeightChanged(float);
    void viewerDistanceChanged(float);

private:
    float m_eyeDistance = 0.06f;
    bool m_flipped = false;

    CameraMode m_cameraMode = CameraMode::Stereo;
    StereoMode m_stereoMode = StereoMode::AsymmetricFrustum;
    bool m_frustumViewEnabled = true;

    bool m_autoFocus = false;
    bool m_showAutoFocusArea = false;
    float m_focusDistance = 10.0f;
    float m_popOut = 0.0f;

    float m_fov = 60.0f;
    bool m_fovByPhysicalDim = false;
    float m_screenHeight = 1.0f;
    float m_viewerDistance = 1.0f;
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

    CursorController(QObject* parent = nullptr);

    void setVisible(bool visible);
    bool visible() const;

    void setCursorType(CursorType cursorType);
    CursorType cursor() const;

    void setScalingEnabled(bool enabled);
    void setScaleFactor(float scale_factor);

    bool scalingEnabled() const;
    float scaleFactor() const;

    bool cursorChangesFocus() const;
    void setCursorChangesFocus(bool focus);

    void setCursorTint(QColor color);
    QColor cursorTint() const;

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
