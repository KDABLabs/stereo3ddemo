#pragma once
#include <shared/cursor.h>

namespace Serenity {
class Entity;
class LayerManager;
class SrtTransform;
class Mesh;
class Texture2D;
} // namespace Serenity

namespace all::serenity {

class SerenityWindow;

struct ColorData {
    std::array<float, 4> ambient;

    friend bool operator==(const ColorData&, const ColorData&) = default;
};

class CursorBase : public Serenity::Entity
{
public:
    KDBindings::Property<bool> enabled{ true };

    void setColor(const ColorData& colorData);

protected:
    explicit CursorBase();

    std::unique_ptr<Serenity::Mesh> m_mesh;
    Serenity::StaticUniformBuffer* m_cbuf = nullptr;
};

class BallCursor : public CursorBase
{
public:
    explicit BallCursor(const Serenity::LayerManager* layers);

private:
    void makeBall(Serenity::Entity* ec, const Serenity::LayerManager* layers);
};

class BillboardCursor : public CursorBase
{
public:
    explicit BillboardCursor(const Serenity::LayerManager* layers);

    enum class CursorTexture {
        Default,
        CrossHair,
        Dot,
    };
    KDBindings::Property<CursorTexture> texture{ CursorTexture::Default };

private:
    void makeBillboard(Serenity::Entity* ec, const Serenity::LayerManager* layers);

    Serenity::SrtTransform* m_transform;
    std::unique_ptr<Serenity::Texture2D> m_texture;
};

class CrossCursor : public CursorBase
{
public:
    explicit CrossCursor(const Serenity::LayerManager* layers);

protected:
    void makeCross(Serenity::Entity* ec, const Serenity::LayerManager* layers);
};

class Cursor : public Serenity::Entity
{
public:
    explicit Cursor(const Serenity::LayerManager* layers, SerenityWindow* window);

    void setPosition(const glm::vec3& worldPosition);
    glm::vec3 position() const;

    KDBindings::Property<Serenity::StereoCamera*> camera{ nullptr };
    KDBindings::Property<all::CursorType> type{ all::CursorType::Ball };
    KDBindings::Property<ColorData> color{ { 1.0f, 1.0f, 1.0f, 1.0f } };
    KDBindings::Property<float> scaleFactor{ 1.0f };
    KDBindings::Property<bool> scalingEnabled{ true };

private:
    void applyColor(const ColorData& colorData);
    void applyType(all::CursorType type);
    void updateSize();

    KDBindings::ConnectionHandle m_projectionChangedConnection;
    KDBindings::ConnectionHandle m_viewChangedConnection;
    SerenityWindow* m_window{ nullptr };

    float m_scale_factor{ 1.0f };
    bool m_scaling_enabled{ true };

    CrossCursor* m_cross{ nullptr };
    BallCursor* m_sphere{ nullptr };
    BillboardCursor* m_billboard{ nullptr };
    Serenity::SrtTransform* m_transform{ nullptr };
};

} // namespace all::serenity
