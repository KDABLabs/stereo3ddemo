#pragma once

namespace all {
class Image
{
public:
    Image(Serenity::Entity* parent)
    {
        auto ec = parent->createChildEntity<Serenity::Entity>();
        m_transform = ec->createComponent<Serenity::SrtTransform>();

        MakeImage(ec);
    }

private:
    void MakeImage(Serenity::Entity* ec)
    {
        auto shader = ec->createChild<Serenity::SpirVShaderProgram>();
        shader->vertexShader = "scene/billboard.vert.spv";
        shader->fragmentShader = "scene/billboard.frag.spv";

        m_mesh = std::make_unique<Serenity::Mesh>();
        m_mesh->setObjectName("Billboard Mesh");
        Serenity::MeshGenerators::planeGenerator(m_mesh.get(), 1, 1, { 2, 2 },
                                                 { { 1, 0, 0, 0 },
                                                   { 0, 0, -1, 0 },
                                                   { 0, 1, 0, 0 },
                                                   { 0, 0, 0, 1 } });

        Serenity::Material* material = ec->createChild<Serenity::Material>();
        material->shaderProgram = shader;

        m_texture = std::make_unique<Serenity::Texture2D>();
        m_texture->setObjectName("Model Texture");
        m_texture->setPath("scene/cursor_billboard.png");
        material->setTexture(4, 0, m_texture.get());

        auto cmodel = ec->createComponent<Serenity::MeshRenderer>();
        cmodel->mesh = m_mesh.get();
        cmodel->material = material;
    }
private:
    Serenity::SrtTransform* m_transform;

    std::unique_ptr<Serenity::Mesh> m_mesh;
    std::unique_ptr<Serenity::Texture2D> m_texture;
};
} // namespace all
