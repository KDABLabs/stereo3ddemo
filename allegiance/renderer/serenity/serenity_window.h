#pragma once
#include <glm/glm.hpp>
#include <KDGpu/device.h>
#include <KDGpu/instance.h>

namespace all::serenity {
class SerenityWindow
{
public:
    virtual ~SerenityWindow() = default;

public:
    virtual uint32_t width() const = 0;
    virtual uint32_t height() const = 0;

    virtual glm::vec4 viewportRect() const = 0;

    virtual glm::vec2 cursorPos() const = 0;

    virtual KDGpu::Instance& instance() = 0;
    virtual KDGpu::Surface& surface() = 0;

    virtual KDGpu::Device createDevice() = 0;
};
} // namespace all::serenity
