#pragma once

#include <string>

namespace all::serenity {

struct shader_uniforms {
    glm::vec3 normalScaling{ 1, 1, 1 };
    float normalMapGain{ 2.0f };

    glm::vec3 semInner{ 0, 0, 0 };
    float _pad1{ 0.0f };

    glm::vec3 semOuter{ 1, 1, 1 };
    float semGain{ 1.0f };

    glm::vec3 difInner{ 0, 0, 0 };
    float _pad2{ 0.0f };

    glm::vec3 difOuter{ 0, 0, 0 };
    float difGain{ 0.0f };

    float postVertexColor{ 0.0f };
    float postGain{ 1.0f };
    float gammax{ 1.2f };
    float _pad3{ 0.0f };
};

struct shader_textures {
    std::string semMap = "white.png";
    std::string diffuseMap = "white.png";
    std::string normalMap = "normal.png";
};

class GlossyMaterial : public Serenity::Material
{
public:
    explicit GlossyMaterial(const shader_textures& texture, const shader_uniforms& uniforms);
};

class SkyboxMaterial : public Serenity::Material
{
public:
    explicit SkyboxMaterial(const shader_textures& texture, const shader_uniforms& uniforms);
};

constexpr glm::vec3 broadcast(float a)
{
    return glm::vec3(a, a, a);
}

const shader_uniforms DarkGlassSU{
    .normalScaling{ -1, 1, 1.5 },
    .semInner = broadcast(0.08f),
    .semOuter = broadcast(1),
    .semGain = 1.5f,
};
const shader_textures DarkGlassST{};
const shader_uniforms DarkGlossSU{
    .semInner = broadcast(0),
    .semOuter = broadcast(0.7),
    .postVertexColor = 1
};
const shader_textures DarkGlossST{
    .semMap = "gltf/venice1-env-2k.png"
};
const shader_uniforms ChromeSU{
    .normalScaling{ 1, 1, 0.5 },
    .semInner = broadcast(1),
    .semOuter = broadcast(0),
    .semGain = 2,
    .postVertexColor = 0.8
};
const shader_textures ChromeST{
    .semMap = "gltf/venice1-env-spec0_05-512.png"
};

const shader_uniforms TireSU{
    .semGain = 0.5,
    .difOuter = broadcast(1),
    .difGain = 0.1,
    .postVertexColor = 0.09
};
const shader_textures TireST{
    .semMap = "gltf/venice1-env-spec0_05-512.png",
    .normalMap = "gltf/tire-normal.png"
};

const shader_uniforms DarkSU{
    .normalScaling{ 1, 1, 1.5 },
    .semGain = 0.3,
    .postVertexColor = 1
};
const shader_textures DarkST{
    .semMap = "gltf/venice1-env-spec0_05-512.png"
};

const shader_uniforms CarPaintSU{
    .normalScaling{ -1, 1, 1.5 },
    .semInner = broadcast(0.1),
    .semGain = 2.5,
    .difInner = { 1, 0.5, 0.7 },
    .postVertexColor = 1
};
const shader_textures CarPaintST{
    .semMap = "gltf/venice1-env-2k.png"
};

const shader_uniforms PlateSU{
    .semInner = broadcast(0.25),
    .difInner = broadcast(1),
    .difOuter = broadcast(1),
    .difGain = 1,
};
const shader_textures PlateST{
    .diffuseMap = "gltf/plate-schneider.png",
    .normalMap = "gltf/plate-schneider-normal.png"
};
const shader_uniforms ShadowPlaneSU{
    .semGain = 0,
    .difInner = { 0.37, 0.37, 0.42 },
    .difOuter = { 0.37, 0.37, 0.42 },
    .difGain = 1,
};
const shader_textures ShadowPlaneST{
    .diffuseMap = "gltf/shadow-car-2-opaque.png",
};

const shader_textures SkyboxST{
    .diffuseMap = "gltf/venice1-env-spec0_05-512.png"
};

} // namespace all::serenity
