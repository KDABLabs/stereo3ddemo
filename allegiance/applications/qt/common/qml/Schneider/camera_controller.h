#pragma once

#include <QObject>
#include <QColor>
#include <QtQml/qqmlregistration.h>

#include <shared/stereo_camera.h>

class CameraController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(float eyeDistance READ eyeDistance WRITE setEyeDistance NOTIFY eyeDistanceChanged)
    Q_PROPERTY(bool flipped READ flipped WRITE setFlipped NOTIFY flippedChanged)
    Q_PROPERTY(bool separationBasedOnFocusDistance READ separationBasedOnFocusDistance WRITE setSeparationBasedOnFocusDistance NOTIFY separationBasedOnFocusDistanceChanged)
    Q_PROPERTY(int separationBasedOnFocusDistanceDivider READ separationBasedOnFocusDistanceDivider WRITE setSeparationBasedOnFocusDistanceDivider NOTIFY separationBasedOnFocusDistanceDividerChanged)

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

    void focusDistanceChanged(float);
    void popOutChanged(float);
    void autoFocusChanged(bool);
    void showAutoFocusAreaChanged(bool);
    void showFocusPlaneChanged(bool);

    void fovChanged(float);
    void fovByPhysicalDimChanged(bool);
    void screenHeightChanged(float);
    void viewerDistanceChanged(float);

    void viewAll();

private:
    void updateFovFromDims();

    static constexpr float ms_eyeDistanceDefaultValue = 0.06f;
    static constexpr int ms_separationBasedOnFocusDistanceDividerDefaultValue = 30;
    static constexpr float ms_focusDistanceDefaultValue = 10.0f;
    static constexpr float ms_popOutDefaultValue = 0.0f;
    static constexpr float ms_screenHeightDefaultValue = 0.4f;
    static constexpr float ms_viewerDistanceDefaultValue = 1.0f;
    static constexpr float ms_fovDefaultValue = 40.0f;

    DisplayMode m_displayMode = DisplayMode::Stereo;
    StereoMode m_stereoMode = StereoMode::AsymmetricFrustum;

    float m_eyeDistance = ms_eyeDistanceDefaultValue;
    bool m_flipped = false;
    bool m_autoFocus = true;
    bool m_showAutoFocusArea = true;
    float m_focusDistance = ms_focusDistanceDefaultValue;
    float m_popOut = ms_popOutDefaultValue;

    float m_fov = ms_fovDefaultValue;
    bool m_fovByPhysicalDim = false;
    float m_screenHeight = ms_screenHeightDefaultValue;
    float m_viewerDistance = ms_viewerDistanceDefaultValue;
    bool m_separationBasedOnFocusDistance = true;
    bool m_showFocusPlane = false;
    int m_separationBasedOnFocusDistanceDivider = ms_separationBasedOnFocusDistanceDividerDefaultValue;
};
