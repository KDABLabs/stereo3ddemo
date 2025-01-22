#pragma once
#include <glm/glm.hpp>
#include <functional>

namespace all {
enum class CursorType {
    Ball,
    Cross,
    CrossHair,
    Dot
};

enum class CursorDisplayMode {
    Both,
    ThreeDimensionalOnly,
    SystemCursorOnly
};

struct ModelNavParameters {
public:
    std::function<glm::vec3(glm::vec3, glm::vec3)> hit_test = [](glm::vec3, glm::vec3) { return glm::vec3{ -1 }; };

    glm::vec3 min_extent{};
    glm::vec3 max_extent{};
    glm::vec3 pivot_point{};
};
} // namespace all
