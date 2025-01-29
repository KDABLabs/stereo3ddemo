#pragma once

#include <QObject>
#include <QColor>
#include <QtQml/qqmlregistration.h>

#include <shared/stereo_camera.h>

class MiscController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool frustumViewEnabled READ frustumViewEnabled WRITE setFrustumViewEnabled NOTIFY frustumViewEnabledChanged)
    Q_PROPERTY(bool wireframeEnabled READ wireframeEnabled WRITE setWireframeEnabled NOTIFY wireframeEnabledChanged)

    QML_SINGLETON
    QML_NAMED_ELEMENT(Misc)

public:
    explicit MiscController(QObject* parent = nullptr);

    bool frustumViewEnabled() const;
    void setFrustumViewEnabled(bool newFrustumViewEnabled);

    bool wireframeEnabled() const;
    void setWireframeEnabled(bool newWireframeEnabled);

Q_SIGNALS:
    void frustumViewEnabledChanged(bool);
    void wireframeEnabledChanged(bool);

private:
    bool m_frustumViewEnabled{ true };
    bool m_wireframeEnabled{ false };
};
