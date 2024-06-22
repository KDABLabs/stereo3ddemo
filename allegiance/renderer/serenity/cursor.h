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
class CursorBase : public Serenity::Entity
{
public:
    CursorBase()
    {
        m_transform = createComponent<Serenity::SrtTransform>();
    }

public:
    auto GetTransform() const noexcept
    {
        return m_transform;
    }

protected:
    Serenity::SrtTransform* m_transform;
};

class BallCursor : public CursorBase
{
public:
    BallCursor(Serenity::LayerManager& layers)
    {
        MakeBall(this, layers);
        MakeBillboard(this, layers);
    }

protected:
    void MakeBall(Serenity::Entity* ec, Serenity::LayerManager& layers);
    void MakeBillboard(Serenity::Entity* ec, Serenity::LayerManager& layers);

protected:
    std::unique_ptr<Serenity::Mesh> m_bb_mesh;
    std::unique_ptr<Serenity::Mesh> m_ball_mesh;
    std::unique_ptr<Serenity::Texture2D> m_texture;
};

class CrossCursor : public CursorBase
{
public:
    CrossCursor(Serenity::LayerManager& layers)
    {
        MakeCross(this, layers);
    }

protected:
    void MakeCross(Serenity::Entity* ec, Serenity::LayerManager& layers);

protected:
    std::unique_ptr<Serenity::Mesh> m_cross_mesh;
};

class Cursor
{

public:
    Cursor(Serenity::LayerManager& layers);

public:
    auto ChangeCursor(Serenity::Entity* parent, CursorType cursor) noexcept
    {
        if (m_currentType == cursor)
            return m_currentCursor;

        size_t index = static_cast<size_t>(cursor);
        size_t prevIndex = static_cast<size_t>(m_currentType);

        // super hacky way to move the cursor to the new parent
        auto& children = parent->childEntities();
        for (auto& child : children) {
            if (child.get() == m_currentCursor) {
                auto c = parent->takeEntity(m_currentCursor);
                m_cursors[prevIndex] = std::move(c);
                break;
            }
        }

        m_currentCursor = (CursorBase*)parent->addChildEntity(std::move(m_cursors[index]));
        m_currentType = cursor;
        return m_currentCursor;
    }

    auto GetTransform() const noexcept
    {
        return m_currentCursor->GetTransform();
    }

private:
    std::vector<std::unique_ptr<Serenity::Entity>> m_cursors;
    CursorBase* m_currentCursor = nullptr;
    CursorType m_currentType = CursorType(-1);
};
} // namespace all::serenity
