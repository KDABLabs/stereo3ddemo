#pragma once

#include <QObject>
#include <QColor>
#include <QtQml/qqmlregistration.h>

#include <shared/cursor.h>

class CursorController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(CursorType cursor READ cursor WRITE setCursorType NOTIFY cursorChanged)
    Q_PROPERTY(CursorDisplayMode displayMode READ displayMode WRITE setDisplayMode NOTIFY displayModeChanged)
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

    enum class CursorDisplayMode {
        Both = int(all::CursorDisplayMode::Both),
        ThreeDimensionalOnly = int(all::CursorDisplayMode::ThreeDimensionalOnly),
        SystemCursorOnly = int(all::CursorDisplayMode::SystemCursorOnly),
    };
    Q_ENUM(CursorDisplayMode);

    CursorController(QObject* parent = nullptr);

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

    CursorDisplayMode displayMode() const;
    void setDisplayMode(CursorDisplayMode displayMode);
    void cycleDisplayMode();

Q_SIGNALS:
    void cursorChanged(all::CursorType cursorType);
    void cursorScaleChanged(float scale);
    void cursorScalingEnableChanged(bool enabled);
    void cursorTintChanged(QColor color);
    void cursorHueChanged(float);
    void displayModeChanged(all::CursorDisplayMode cursorType);

private:
    bool m_visible = true;
    all::CursorType m_cursorType = all::CursorType::Ball;
    all::CursorDisplayMode m_displayMode = all::CursorDisplayMode::Both;
    bool m_scaling_enabled = true;
    float m_scale_factor = 1.0f;
    QColor m_tint = QColor::fromHsv(167, 200, 255, 255);
    float m_cursorHue = 167.0f;
};
