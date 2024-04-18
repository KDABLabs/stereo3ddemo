#pragma once
#include <QVector3D>

namespace all::qt3d {
// vertex shader
constexpr inline std::string_view simple_vs = R"(
#version 150 core

uniform mat4 modelMatrix;
uniform mat4 mvp;

in vec3 vertexPosition;

void main()
{
    vec3 positionW = vec3(modelMatrix * vec4(vertexPosition, 1.0));
    gl_Position = mvp * vec4(positionW, 1.0);
}
)";
// pixel shader
constexpr std::string_view simple_ps_yellow = R"(
#version 150 core
out vec4 fragColor;

void main()
{
    fragColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);
}
)";
constexpr std::string_view simple_ps_red = R"(
#version 150 core
out vec4 fragColor;

void main()
{
    fragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}
)";

constexpr std::string_view fresnel_vs = R"(
#version 420 core

uniform vec3 normalScaling;
uniform float postVertexColor;
uniform float postGain;
uniform mat4 modelView;
uniform mat4 mvp;

in vec4 vertexColor;
in vec3 vertexPosition;
in vec3 vertexNormal;
in vec2 vertexTexCoord;

out vec4 postColor;
out vec4 fragVertexColor;
smooth out vec3 normalSem;
out vec2 texCoord;


vec3 semNormal()
{
    vec3 n = (modelView * vec4(vertexNormal, 0.0)).xyz; // ignore position
    n *= normalScaling;
    return normalize(n);
}

void main()
{
    normalSem = semNormal();
    texCoord = vertexTexCoord;
    postColor = mix(vec4(1.0), vertexColor, postVertexColor) * postGain;
    fragVertexColor = vertexColor;
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}

)";

constexpr std::string_view fresnel_ps = R"(
#version 420 core

in vec4 postColor;
in vec4 fragVertexColor;
smooth in vec3 normalSem;
in vec2 texCoord;
out vec4 fragColor;

uniform sampler2D semMap;
uniform sampler2D diffuseMap;
uniform sampler2D normalMap;

uniform float normalMapGain;

uniform vec3 semInner;
uniform vec3 semOuter;
uniform float semGain;

uniform vec3 difInner;
uniform vec3 difOuter;
uniform float difGain;
uniform float gammax;


float semFresnel(vec3 normalSem_)
{
    float fresnel = 1.0 - dot(normalSem_, vec3(0.0, 0.0, 1.0));
    return fresnel * fresnel;
}
vec2 semS(vec3 normalSem_)
{
    vec2 s = normalSem_.xy;
    s = s * 0.5 + vec2(0.5);
    return s;
}
 
void main()
{
    vec2 normalMap = (texture(normalMap, texCoord).xy * 2.0 - vec2(1.0)) * normalMapGain;
    vec3 normalSem_ = normalize(normalSem + vec3(normalMap, 0.0));
    float fresnel = semFresnel(normalSem_);

    if (fragVertexColor.r < 0.4)
        fresnel = 0 * fresnel;
    if (fresnel >= 0.3) {
        fresnel = 0.2;
    }

    vec3 diffuse = texture(diffuseMap, texCoord).xyz * mix(difInner, difOuter, fresnel) * difGain;
    vec3 semColor = texture(semMap, semS(normalSem_)).xyz * mix(semInner, semOuter, fresnel) * semGain;

    fragColor = postColor * vec4(diffuse + semColor, 1.0);
    //fragColor.rgb = pow(fragColor.rgb, vec3(gammax));

    //fragColor = vec4(fresnel, fresnel, fresnel, 1);
}
)";

constexpr std::string_view skybox_vs = R"(
#version 150 core

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform float postGain;

in vec3 vertexPosition;
in vec2 vertexTexCoord;

out vec2 texCoord;
out vec4 postColor;

void main()
{
    texCoord = vertexTexCoord;
    postColor = vec4(postGain,postGain,postGain,1.0);
    gl_Position = vec4(projectionMatrix * mat4(mat3(viewMatrix)) * modelMatrix * vec4(vertexPosition, 1.0));
}
)";

constexpr std::string_view skybox_ps = R"(
#version 150 core

in vec2 texCoord;
in vec4 postColor;
out vec4 fragColor;

uniform sampler2D diffuseMap;
uniform float gammax;

void main()
{
    fragColor = texture(diffuseMap, texCoord) * postColor;
    fragColor.rgb = pow(fragColor.rgb, vec3(gammax));
}
)";

constexpr std::string_view stereoImage_vs = R"(
#version 150 core

in vec2 vertexPosition;
in vec2 vertexTexCoord;

out vec2 texCoord;

void main()
{
    texCoord = vertexTexCoord;
    gl_Position = vec4(vertexPosition, 0.0, 1.0);
}
)";

constexpr std::string_view stereoImage_ps = R"(
#version 150 core

uniform sampler2D stereoImageMap;

in vec2 texCoord;
out vec4 fragColor;

void main()
{
    fragColor = texture(stereoImageMap, texCoord);
}
)";

constexpr std::string_view stereoImage_vs_rhi = R"(
#version 450

layout(location = 0) in vec2 vertexPosition;
layout(location = 1) in vec2 vertexTexCoord;

layout(location = 0) out vec2 texCoord;

void main()
{
    texCoord = vertexTexCoord;
    gl_Position = vec4(vertexPosition, 0.0, 1.0);
}
)";

constexpr std::string_view stereoImage_frag_rhi = R"(
#version 450

layout(binding = 3) uniform sampler2D stereoImageMap;

layout(location = 0) in vec2 texCoord;
layout(location = 0) out vec4 fragColor;

void main()
{
    fragColor = texture(stereoImageMap, texCoord);
}
)";

struct shader_uniforms {
    QVector3D normalScaling{ 1, 1, 1 };
    QVector3D semInner{ 0, 0, 0 };
    QVector3D semOuter{ 1, 1, 1 };
    float semGain = 1.0f;

    QVector3D difInner{ 0, 0, 0 };
    QVector3D difOuter{ 0, 0, 0 };
    float difGain = 0.0f;
    float postVertexColor = 0.0f;
};

struct shader_textures {
    QStringView semMap = u":/white.png";
    QStringView diffuseMap = u":/white.png";
    QStringView normalMap = u":/normal.png";
};

constexpr QVector3D broadcast(float a)
{
    return QVector3D(a, a, a);
}

constexpr shader_uniforms DarkGlassSU{
    .normalScaling{ -1, 1, 1.5 },
    .semInner = broadcast(0.08f),
    .semOuter = broadcast(1),
    .semGain = 1.5f,
};
constexpr shader_textures DarkGlassST{};
constexpr shader_uniforms DarkGlossSU{
    .semInner = broadcast(0),
    .semOuter = broadcast(0.7),
    .postVertexColor = 1
};
constexpr shader_textures DarkGlossST{
    .semMap = u":/gltf/venice1-env-2k.png"
};
constexpr shader_uniforms ChromeSU{
    .normalScaling{ 1, 1, 0.5 },
    .semInner = broadcast(1),
    .semOuter = broadcast(0),
    .semGain = 2,
    .postVertexColor = 0.8
};
constexpr shader_textures ChromeST{
    .semMap = u":/gltf/venice1-env-spec0_05-512.png"
};

constexpr shader_uniforms TireSU{
    .semGain = 0.5,
    .difOuter = broadcast(1),
    .difGain = 0.1,
    .postVertexColor = 0.09
};
constexpr shader_textures TireST{
    .semMap = u":/gltf/venice1-env-spec0_05-512.png",
    .normalMap = u":/gltf/tire-normal.png"
};

constexpr shader_uniforms DarkSU{
    .normalScaling{ 1, 1, 1.5 },
    .semGain = 0.3,
    .postVertexColor = 1
};
constexpr shader_textures DarkST{
    .semMap = u":/gltf/venice1-env-spec0_05-512.png"
};

constexpr shader_uniforms CarPaintSU{
    .normalScaling{ -1, 1, 1.5 },
    .semInner = broadcast(0.1),
    .semGain = 2.5,
    .difInner = { 1, 0.5, 0.7 },
    .postVertexColor = 1
};
constexpr shader_textures CarPaintST{
    .semMap = u":/gltf/venice1-env-2k.png"
};

constexpr shader_uniforms PlateSU{
    .semInner = broadcast(0.25),
    .difInner = broadcast(1),
    .difOuter = broadcast(1),
    .difGain = 1,
};
constexpr shader_textures PlateST{
    .diffuseMap = u":/gltf/plate-schneider.png",
    .normalMap = u":/gltf/plate-schneider-normal.png"
};
constexpr shader_uniforms ShadowPlaneSU{
    .semGain = 0,
    .difInner = { 0.37, 0.37, 0.42 },
    .difOuter = { 0.37, 0.37, 0.42 },
    .difGain = 1,
};
constexpr shader_textures ShadowPlaneST{
    .diffuseMap = u":/gltf/shadow-car-2-opaque.png",
};

constexpr shader_textures SkyboxST{
    .diffuseMap = u":/gltf/venice1-env-spec0_05-512.png"
};

} // namespace all