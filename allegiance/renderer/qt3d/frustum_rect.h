#pragma once

#include <Qt3DCore/QEntity>
#include <QColor>

namespace Qt3DCore {
class QBuffer;
} // namespace Qt3DCore

namespace all::qt3d {

class FrustumRect : public Qt3DCore::QEntity
{
    Q_OBJECT
public:
    explicit FrustumRect(Qt3DCore::QNode* parent = nullptr);

private:
    QColor m_backgroundColor{ QColor::fromRgb(0, 0, 0, 127) };
    QColor m_outlineColor{ QColor(Qt::white) };
    float m_outlineWidth{ 2.0f };
};

} // namespace all::qt3d
