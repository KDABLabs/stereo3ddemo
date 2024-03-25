#pragma once

namespace Serenity {
class Entity;
class LayerManager;
class SrtTransform;
class Mesh;
class Texture2D;
} // namespace Serenity

namespace all::serenity {
class Cursor
{
public:
    Cursor(Serenity::Entity* parent, Serenity::LayerManager& layers);

public:
    auto GetTransform() const noexcept
    {
        return m_transform;
    }

private:
    void MakeBall(Serenity::Entity* ec, Serenity::LayerManager& layers);
    void MakeBillboard(Serenity::Entity* ec, Serenity::LayerManager& layers);

private:
    Serenity::SrtTransform* m_transform;
    std::unique_ptr<Serenity::Mesh> m_ball_mesh;

    std::unique_ptr<Serenity::Mesh> m_bb_mesh;
    std::unique_ptr<Serenity::Texture2D> m_texture;
};
} // namespace all::serenity
