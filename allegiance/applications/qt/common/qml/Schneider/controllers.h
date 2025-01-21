#pragma once
#include <QObject>
#include <QColor>
#include <shared/cursor.h>
#include <shared/stereo_camera.h>
#include <QtQml/qqmlregistration.h>

class SceneController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(float mouseSensitivity READ mouseSensitivity WRITE setMouseSensitivity NOTIFY mouseSensitivityChanged)
    Q_PROPERTY(bool shiftPressed READ shiftPressed WRITE setShiftPressed NOTIFY shiftPressedChanged)
    Q_PROPERTY(float mousePressedX READ mousePressedX WRITE setMousePressedX NOTIFY mousePressedXChanged)
    Q_PROPERTY(bool lockMouseInPlace READ lockMouseInPlace WRITE setLockMouseInPlace NOTIFY lockMouseInPlaceChanged)
    QML_SINGLETON
    QML_NAMED_ELEMENT(Scene)
public:
    float mouseSensitivity();
    void setMouseSensitivity(float sensitivity);

    bool shiftPressed() const;
    void setShiftPressed(bool newShiftPressed);

    float mousePressedX() const;
    void setMousePressedX(float newMousePressedX);

    bool lockMouseInPlace() const;
    void setLockMouseInPlace(bool newLockMouseInPlace);

Q_SIGNALS:
    void OpenLoadModelDialog();
    void mouseSensitivityChanged();

    void shiftPressedChanged();
    void mousePressedXChanged();
    void lockMouseInPlaceChanged();

protected:
    float m_mouseSensitivity = 100;
    float m_mousePressedX = 0;
    bool m_lockMouseInPlace = false;

private:
    bool m_shiftPressed{ false };
};

class CameraController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(float eyeDistance READ eyeDistance WRITE setEyeDistance NOTIFY eyeDistanceChanged)
    Q_PROPERTY(bool flipped READ flipped WRITE setFlipped NOTIFY flippedChanged)
    Q_PROPERTY(bool separationBasedOnFocusDistance READ separationBasedOnFocusDistance WRITE setSeparationBasedOnFocusDistance NOTIFY separationBasedOnFocusDistanceChanged)
    Q_PROPERTY(int separationBasedOnFocusDistanceDivider READ separationBasedOnFocusDistanceDivider WRITE setSeparationBasedOnFocusDistanceDivider NOTIFY separationBasedOnFocusDistanceDividerChanged)

    Q_PROPERTY(bool frustumViewEnabled READ frustumViewEnabled WRITE setFrustumViewEnabled NOTIFY frustumViewEnabledChanged)
    Q_PROPERTY(DisplayMode displayMode READ displayMode WRITE setDisplayMode NOTIFY displayModeChanged)
    Q_PROPERTY(StereoMode stereoMode READ stereoMode WRITE setStereoMode NOTIFY stereoModeChanged)

    Q_PROPERTY(bool autoFocus READ autoFocus WRITE setAutoFocus NOTIFY autoFocusChanged)
    Q_PROPERTY(bool showAutoFocusArea READ showAutoFocusArea WRITE setShowAutoFocusArea NOTIFY showAutoFocusAreaChanged)
    Q_PROPERTY(bool showFocusPlane READ showFocusPlane WRITE setShowFocusPlane NOTIFY showFocusPlaneChanged)
    Q_PROPERTY(float focusDistance READ focusDistance WRITE setFocusDistance NOTIFY focusDistanceChanged)
    Q_PROPERTY(float popOut READ popOut WRITE setPopOut NOTIFY popOutChanged)

    Q_PROPERTY(float fov READ fov WRITE setFov NOTIFY fovChanged)
    Q_PROPERTY(bool fovByPhysicalDim READ fovByPhysicalDim WRITE setFovByPhysicalDim NOTIFY fovByPhysicalDimChanged)
    Q_PROPERTY(float screenHeight READ screenHeight WRITE setScreenHeight NOTIFY screenHeightChanged)
    Q_PROPERTY(float viewerDistance READ viewerDistance WRITE setViewerDistance NOTIFY viewerDistanceChanged)

    Q_PROPERTY(float eyeDistanceDefaultValue READ eyeDistanceDefaultValue CONSTANT)
    Q_PROPERTY(int separationBasedOnFocusDistanceDividerDefaultValue READ separationBasedOnFocusDistanceDividerDefaultValue CONSTANT)
    Q_PROPERTY(float focusDistanceDefaultValue READ focusDistanceDefaultValue CONSTANT)
    Q_PROPERTY(float popOutDefaultValue READ popOutDefaultValue CONSTANT)
    Q_PROPERTY(float screenHeightDefaultValue READ screenHeightDefaultValue CONSTANT)
    Q_PROPERTY(float viewerDistanceDefaultValue READ viewerDistanceDefaultValue CONSTANT)
    Q_PROPERTY(float fovDefaultValue READ fovDefaultValue CONSTANT)

    QML_SINGLETON
    QML_NAMED_ELEMENT(Camera)

public:
    enum class DisplayMode {
        Stereo = int(all::DisplayMode::Stereo),
        Mono = int(all::DisplayMode::Mono),
        Left = int(all::DisplayMode::Left),
        Right = int(all::DisplayMode::Right)
    };
    Q_ENUM(DisplayMode);

    enum class StereoMode {
        ToeIn = int(all::StereoCamera::Mode::ToeIn),
        AsymmetricFrustum = int(all::StereoCamera::Mode::AsymmetricFrustum),
    };
    Q_ENUM(StereoMode)

    CameraController(QObject* parent = nullptr);

    void setEyeDistance(float eyeDistance);
    float eyeDistance() const;

    bool separationBasedOnFocusDistance() const;
    void setSeparationBasedOnFocusDistance(bool newSeparationBasedOnFocusDistance);

    void setFocusDistance(float focusDistance);
    float focusDistance() const;

    float focusAngle() const;

    void setFov(float fov);
    float fov() const;

    void setFlipped(bool flipped);
    bool flipped() const;

    void setDisplayMode(DisplayMode mode);
    DisplayMode displayMode() const;

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

    inline static float MinEyeDistance = 0.001f;
    inline static float MaxEyeDistance = 0.5f;

    inline static float MinFocusDistance = 0.1f;
    inline static float MaxFocusDistance = 100.0f;

    bool showFocusPlane() const;
    void setShowFocusPlane(bool newShowFocusPlane);

    int separationBasedOnFocusDistanceDivider() const;
    void setSeparationBasedOnFocusDistanceDivider(int newSeparationBasedOnFocusDistanceDivider);

    float eyeDistanceDefaultValue() const;
    int separationBasedOnFocusDistanceDividerDefaultValue() const;
    float focusDistanceDefaultValue() const;
    float popOutDefaultValue() const;
    float screenHeightDefaultValue() const;
    float viewerDistanceDefaultValue() const;
    float fovDefaultValue() const;

Q_SIGNALS:
    void eyeDistanceChanged(float);
    void flippedChanged(bool);
    void separationBasedOnFocusDistanceChanged(bool);
    void separationBasedOnFocusDistanceDividerChanged(int);

    void displayModeChanged(DisplayMode);
    void stereoModeChanged(StereoMode);
    void frustumViewEnabledChanged(bool);

    void focusDistanceChanged(float);
    void popOutChanged(float);
    void autoFocusChanged(bool);
    void showAutoFocusAreaChanged(bool);
    void showFocusPlaneChanged(bool);

    void fovChanged(float);
    void fovByPhysicalDimChanged(bool);
    void screenHeightChanged(float);
    void viewerDistanceChanged(float);

private:
    void updateFovFromDims();

    static constexpr float ms_eyeDistanceDefaultValue = 0.06f;
    static constexpr int ms_separationBasedOnFocusDistanceDividerDefaultValue = 30;
    static constexpr float ms_focusDistanceDefaultValue = 10.0f;
    static constexpr float ms_popOutDefaultValue = 0.0f;
    static constexpr float ms_screenHeightDefaultValue = 0.3f;
    static constexpr float ms_viewerDistanceDefaultValue = 0.6f;
    static constexpr float ms_fovDefaultValue = 30.0f;

    DisplayMode m_displayMode = DisplayMode::Stereo;
    StereoMode m_stereoMode = StereoMode::AsymmetricFrustum;
    bool m_frustumViewEnabled = true;

    float m_eyeDistance = ms_eyeDistanceDefaultValue;
    bool m_flipped = false;
    bool m_autoFocus = false;
    bool m_showAutoFocusArea = false;
    float m_focusDistance = ms_focusDistanceDefaultValue;
    float m_popOut = ms_popOutDefaultValue;

    float m_fov = ms_fovDefaultValue;
    bool m_fovByPhysicalDim = false;
    float m_screenHeight = ms_screenHeightDefaultValue;
    float m_viewerDistance = ms_viewerDistanceDefaultValue;
    bool m_separationBasedOnFocusDistance = false;
    bool m_showFocusPlane = false;
    int m_separationBasedOnFocusDistanceDivider = ms_separationBasedOnFocusDistanceDividerDefaultValue;
};

class CursorController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(float visible READ visible WRITE setVisible NOTIFY cursorVisibilityChanged)
    Q_PROPERTY(CursorType cursor READ cursor WRITE setCursorType NOTIFY cursorChanged)
    Q_PROPERTY(bool scalingEnabled READ scalingEnabled WRITE setScalingEnabled NOTIFY cursorScalingEnableChanged)
    Q_PROPERTY(float scaleFactor READ scaleFactor WRITE setScaleFactor NOTIFY cursorScaleChanged)
    Q_PROPERTY(float hue READ hue WRITE setHue NOTIFY cursorHueChanged)

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
    [[nodiscard]] bool visible() const;

    void setCursorType(CursorType cursorType);
    [[nodiscard]] CursorType cursor() const;

    void setScalingEnabled(bool enabled);
    void setScaleFactor(float scale_factor);

    [[nodiscard]] bool scalingEnabled() const;
    [[nodiscard]] float scaleFactor() const;

    void setCursorTint(QColor color);
    [[nodiscard]] QColor cursorTint() const;

    void setHue(float hue);
    [[nodiscard]] float hue();

Q_SIGNALS:
    void cursorVisibilityChanged(bool state);
    void cursorChanged(all::CursorType cursorType);
    void cursorScaleChanged(float scale);
    void cursorScalingEnableChanged(bool enabled);
    void cursorTintChanged(QColor color);
    void cursorHueChanged(float);

private:
    bool m_visible = true;
    all::CursorType m_cursorType = all::CursorType::Ball;
    bool m_scaling_enabled = true;
    float m_scale_factor = 1.0f;
    QColor m_tint = QColor::fromHsv(180, 200, 255, 255);
    float m_cursorHue = 180.0f;
};
