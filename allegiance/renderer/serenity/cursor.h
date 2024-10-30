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
    struct ColorData {
        float ambient[4];
    };

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
    virtual void SetColor(const ColorData& colorData) { }

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
    void SetColor(const ColorData& colorData);

protected:
    std::unique_ptr<Serenity::Mesh> m_bb_mesh;
    std::unique_ptr<Serenity::Mesh> m_ball_mesh;
    std::unique_ptr<Serenity::Texture2D> m_texture;
    Serenity::StaticUniformBuffer* m_cbuf = nullptr;
    Serenity::StaticUniformBuffer* m_bb_cbuf = nullptr;
};

class CrossCursor : public CursorBase
{
public:
    CrossCursor(Serenity::LayerManager& layers)
    {
        MakeCross(this, layers);
    }

public:
    void SetColor(const ColorData& colorData) override;

protected:
    void MakeCross(Serenity::Entity* ec, Serenity::LayerManager& layers);

protected:
    std::unique_ptr<Serenity::Mesh> m_cross_mesh;
    Serenity::StaticUniformBuffer* m_cbuf = nullptr;
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

        if (cursor > CursorType::Cross) // not implemented
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
        m_currentCursor->SetColor(m_colorData);
        return m_currentCursor;
    }

    auto GetTransform() const noexcept
    {
        return m_currentCursor->GetTransform();
    }
    void SetColor(const CursorBase::ColorData& colorData)
    {
        m_colorData = colorData;
        m_currentCursor->SetColor(colorData);
    }

private:
    std::vector<std::unique_ptr<Serenity::Entity>> m_cursors;
    CursorBase* m_currentCursor = nullptr;
    CursorType m_currentType = CursorType(-1);
    CursorBase::ColorData m_colorData = { { 1.0f, 1.0f, 1.0f, 1.0f } };
};
} // namespace all::serenity
