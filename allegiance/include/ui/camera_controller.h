#pragma once
#include <QObject>

class SceneController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUrl model READ Model WRITE LoadModel NOTIFY OnModelChanged)
public:
    SceneController(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

public:
    void LoadModel(QUrl model) noexcept
    {
        m_model = model;
        OnModelChanged(model);
    }
    QUrl Model() const noexcept
    {
        return m_model;
    }
Q_SIGNALS:
    void OnModelChanged(QUrl model);

private:
    QUrl m_model;
};

class CameraController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(float eyeDistance READ EyeDistance WRITE SetEyeDistance NOTIFY OnEyeDistanceChanged)
    Q_PROPERTY(float focusDistance READ FocusDistance WRITE SetFocusDistance NOTIFY OnFocusDistanceChanged)
    Q_PROPERTY(float focusAngle READ FocusAngle WRITE SetFocusAngle NOTIFY OnFocusAngleChanged)
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
Q_SIGNALS:
    void OnEyeDistanceChanged(float eyeDistance);
    void OnFocusDistanceChanged(float focusDistance);
    void OnFocusAngleChanged(float focusAngle);

private:
    float m_eyeDistance = 0.06f;
    float m_focusDistance = 10.0f;
    float m_focusAngle = 80.0f;
};

class CursorController : public QObject
{
public:
    // later a cursor type
    enum class CursorType {
        Default,
        Cross,
    };
    Q_ENUM(CursorType);

    Q_OBJECT
    Q_PROPERTY(float visible READ Visible WRITE SetVisible NOTIFY OnCursorVisibilityChanged)
    Q_PROPERTY(CursorType cursor READ Cursor WRITE SetCursorType NOTIFY OnCursorChanged)
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
        m_cursorType = cursorType;
        OnCursorChanged(cursorType);
    }
    CursorType Cursor() const noexcept
    {
        return m_cursorType;
    }
Q_SIGNALS:
    void OnCursorVisibilityChanged(bool state);
    void OnCursorChanged(CursorType cursorType);

private:
    bool m_visible = true;
    CursorType m_cursorType = CursorType::Default;
};