#pragma once
#include <glm/glm.hpp>
#include <QObject>

namespace all {

struct extent_t
{
    union {
        struct { double min_x, min_y, min_z, max_x, max_y, max_z; };
        double arr[6];
    };
};

class Controller : public QObject
{
    Q_OBJECT
    Controller() = default;
public:
    static Controller& getInstance()
    {
        static Controller c;
        return c;
    }


   std::function<glm::vec3(glm::vec3, glm::vec3, double)> hitTest = [] (glm::vec3, glm::vec3, double) { return glm::vec3{-1}; };
    double hitAperture;
    glm::vec3 hitDirection;
    glm::vec3 hitFrom;

    extent_t modelExtent{-2, 0, -6, 2, 1.5, 6};

    void setModelExtent(const extent_t extent) {
        modelExtent = extent;
        Q_EMIT modelExtentChanged(modelExtent);
    }

Q_SIGNALS:
    void modelExtentChanged(const extent_t);
};

class Cursor : public QObject {
    Q_OBJECT

    Cursor() = default;

public:
    static Cursor& getInstance() {
        static Cursor instance;
        return instance;
    }

    void setWorldPosition(const glm::vec3& vec)
    {
        if (worldPosition == vec)
            return;
        worldPosition = vec;
        Q_EMIT worldPositionChanged(vec);
    }
    [[nodiscard]] glm::vec3 getWorldPosition() const
    {
        return worldPosition;
    }

    void setScreenPosition(const glm::vec2& screenPosition)
    {
        if (screenPosition == this->screenPosition)
            return;
        this->screenPosition = screenPosition;
        Q_EMIT screenPositionChanged(screenPosition);
    }
    [[nodiscard]] glm::vec2 getScreenPosition() const
    {
        return screenPosition;
    }
    Q_SIGNALS:

    void worldPositionChanged(glm::vec3 worldPosition);
    void screenPositionChanged(glm::vec2 screenPosition);
protected:
    glm::vec3 worldPosition;
    glm::vec2 screenPosition;
};

}