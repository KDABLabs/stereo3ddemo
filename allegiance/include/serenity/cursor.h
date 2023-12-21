#pragma once

namespace Serenity {
class Cursor
{
public:
    Cursor(Serenity::Entity* parent, Serenity::LayerManager& layers)
    {
        auto ec = parent->createChildEntity<Serenity::Entity>();
        m_transform = ec->createComponent<Serenity::SrtTransform>();

        MakeBall(ec, layers);
        MakeBillboard(ec, layers);
    }

public:
    auto GetTransform() const noexcept
    {
        return m_transform;
    }

private:
    void MakeBall(Serenity::Entity* ec, Serenity::LayerManager& layers)
    {
        auto shader_ball = ec->createChild<Serenity::SpirVShaderProgram>();
        shader_ball->vertexShader = "scene/color.vert.spv";
        shader_ball->fragmentShader = "scene/color.frag.spv";

        m_ball_mesh = std::make_unique<Serenity::Mesh>();
        m_ball_mesh->setObjectName("Cursor Mesh");
        Serenity::MeshGenerators::sphereGenerator(m_ball_mesh.get(), 24, 24, 1.0f);

        struct ColorData {
            float ambient[4];
        };
        const Serenity::Material::UboDataBuilder materialDataBuilder[] = {
            [](uint32_t set, uint32_t binding) {
                const ColorData data{
                    { 1.0f, 1.0f, 1.0f, 1.0f },
                };
                std::vector<uint8_t> rawData(sizeof(ColorData));
                std::memcpy(rawData.data(), &data, sizeof(ColorData));
                return rawData;
            },
        };

        Serenity::StaticUniformBuffer* cbuf = ec->createChild<Serenity::StaticUniformBuffer>();
        cbuf->size = sizeof(ColorData);

        Serenity::Material* material = ec->createChild<Serenity::Material>();
        material->shaderProgram = shader_ball;
        material->setUniformBuffer(3, 0, cbuf);
        material->setUniformBufferDataBuilder(materialDataBuilder[0]);

        auto cmodel = ec->createComponent<Serenity::MeshRenderer>();
        cmodel->mesh = m_ball_mesh.get();
        cmodel->material = material;
        ec->layerMask = layers.layerMask({ "Opaque" });
    }
    void MakeBillboard(Serenity::Entity* ec, Serenity::LayerManager& layers)
    {
        auto shader_bb = ec->createChild<Serenity::SpirVShaderProgram>();
        shader_bb->vertexShader = "scene/billboard.vert.spv";
        shader_bb->fragmentShader = "scene/billboard.frag.spv";

        m_bb_mesh = std::make_unique<Serenity::Mesh>();
        m_bb_mesh->setObjectName("Billboard Mesh");
        Serenity::MeshGenerators::planeGenerator(m_bb_mesh.get(), 8, 8, { 2, 2 },
                                                 { { 1, 0, 0, 0 },
                                                   { 0, 0, -1, 0 },
                                                   { 0, 1, 0, 0 },
                                                   { 0, 0, 0, 1 } });

        Serenity::Material* material = ec->createChild<Serenity::Material>();
        material->shaderProgram = shader_bb;

        m_texture = std::make_unique<Serenity::Texture2D>();
        m_texture->setObjectName("Model Texture");
        m_texture->setPath("scene/cursor_billboard.png");
        material->setTexture(4, 0, m_texture.get());

        auto cmodel = ec->createComponent<Serenity::MeshRenderer>();
        cmodel->mesh = m_bb_mesh.get();
        cmodel->material = material;

        ec->layerMask = layers.layerMask({ "Alpha" });
    }

private:
    Serenity::SrtTransform* m_transform;
    std::unique_ptr<Serenity::Mesh> m_ball_mesh;

    std::unique_ptr<Serenity::Mesh> m_bb_mesh;
    std::unique_ptr<Serenity::Texture2D> m_texture;
};
} // namespace all
