#pragma once

#include <Serenity/core/ecs/entity.h>
#include <Serenity/gui/buffer.h>

namespace all::serenity {

class FrustumRect : public Serenity::Entity
{
public:
    FrustumRect();

    KDBindings::Property<bool> enabled{ true };
    KDBindings::Property<glm::vec4> backgroundColor{ glm::vec4(0.0f, 0.0f, 0.0f, 0.5f) };
    KDBindings::Property<glm::vec4> outlineColor{ glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) };
    KDBindings::Property<float> outlineWidth{ 2.0f };


private:
    void updateUBO();

    Serenity::StaticUniformBuffer* m_materialUBO{ nullptr };
    std::unique_ptr<Serenity::Mesh> m_mesh;
};

} // namespace all::serenity
