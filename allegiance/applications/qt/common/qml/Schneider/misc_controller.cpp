#include "misc_controller.h"

MiscController::MiscController(QObject* parent)
    : QObject(parent)
{
}

bool MiscController::frustumViewEnabled() const
{
    return m_frustumViewEnabled;
}

void MiscController::setFrustumViewEnabled(bool newFrustumViewEnabled)
{
    if (newFrustumViewEnabled == m_frustumViewEnabled)
        return;
    m_frustumViewEnabled = newFrustumViewEnabled;
    Q_EMIT frustumViewEnabledChanged(newFrustumViewEnabled);
}

bool MiscController::wireframeEnabled() const
{
    return m_wireframeEnabled;
}

void MiscController::setWireframeEnabled(bool newWireframeEnabled)
{
    if (m_wireframeEnabled == newWireframeEnabled)
        return;
    m_wireframeEnabled = newWireframeEnabled;
    Q_EMIT wireframeEnabledChanged(m_wireframeEnabled);
}
