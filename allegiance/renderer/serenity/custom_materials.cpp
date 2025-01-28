#include "custom_materials.h"

#include <Serenity/gui/texture.h>

namespace all::serenity {

GlossyMaterial::GlossyMaterial(const shader_textures& texture, const shader_uniforms& uniforms)
{
    auto* shader = createChild<Serenity::SpirVShaderProgram>();
    shader->vertexShader = SHADER_DIR "glossy.vert.spv";
    shader->fragmentShader = SHADER_DIR "glossy.frag.spv";
    shaderProgram = shader;

    Serenity::StaticUniformBuffer* ubo = createChild<Serenity::StaticUniformBuffer>();
    ubo->size = sizeof(shader_uniforms);

    std::vector<uint8_t> rawData(sizeof(shader_uniforms));
    std::memcpy(rawData.data(), &uniforms, sizeof(shader_uniforms));
    ubo->data = rawData;

    setUniformBuffer(3, 0, ubo);

    auto semMapTexture = createChild<Serenity::Texture2D>();
    auto diffuseMapTexture = createChild<Serenity::Texture2D>();
    auto normalMapTexture = createChild<Serenity::Texture2D>();

    semMapTexture->minificationFilter = Serenity::Texture::FilterMode::Nearest;
    diffuseMapTexture->minificationFilter = Serenity::Texture::FilterMode::Nearest;
    normalMapTexture->minificationFilter = Serenity::Texture::FilterMode::Nearest;

    using namespace std::string_literals;
    const std::string assetsDir = "assets/"s;

    semMapTexture->setPath(assetsDir + texture.semMap);
    diffuseMapTexture->setPath(assetsDir + texture.diffuseMap);
    normalMapTexture->setPath(assetsDir + texture.normalMap);

    setTexture(4, 0, semMapTexture);
    setTexture(4, 1, diffuseMapTexture);
    setTexture(4, 2, normalMapTexture);
}

SkyboxMaterial::SkyboxMaterial(const shader_textures& texture, const shader_uniforms& uniforms)
{
    auto* shader = createChild<Serenity::SpirVShaderProgram>();
    shader->vertexShader = SHADER_DIR "skybox.vert.spv";
    shader->fragmentShader = SHADER_DIR "skybox.frag.spv";
    shaderProgram = shader;

    Serenity::StaticUniformBuffer* ubo = createChild<Serenity::StaticUniformBuffer>();
    ubo->size = sizeof(shader_uniforms);

    std::vector<uint8_t> rawData(sizeof(shader_uniforms));
    std::memcpy(rawData.data(), &uniforms, sizeof(shader_uniforms));
    ubo->data = rawData;

    setUniformBuffer(3, 0, ubo);

    using namespace std::string_literals;
    const std::string assetsDir = "assets/"s;

    auto skyboxTexture = createChild<Serenity::Texture2D>();
    skyboxTexture->minificationFilter = Serenity::Texture::FilterMode::Nearest;

    skyboxTexture->setPath(assetsDir + texture.diffuseMap);
    setTexture(4, 0, skyboxTexture);
}

} // namespace all::serenity
