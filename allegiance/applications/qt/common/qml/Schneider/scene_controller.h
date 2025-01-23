#pragma once
#include <QObject>
#include <QColor>
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

