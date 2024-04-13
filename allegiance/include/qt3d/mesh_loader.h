#pragma once

#include <QString>

namespace Qt3DCore {
class QEntity;
};

namespace all {

class MeshLoader
{
public:
    static Qt3DCore::QEntity* load(const QString& path);
};

} // namespace all
