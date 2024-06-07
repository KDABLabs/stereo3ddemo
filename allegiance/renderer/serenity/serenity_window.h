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
    virtual uint32_t GetWidth() const = 0;
    virtual uint32_t GetHeight() const = 0;

    virtual glm::vec4 GetViewportRect() const = 0;

    virtual glm::vec2 GetCursorPos() const = 0;

    virtual KDGpu::Instance& GetInstance() = 0;
    virtual KDGpu::Surface& GetSurface() = 0;

    virtual KDGpu::Device CreateDevice() = 0;
};
} // namespace all::serenity