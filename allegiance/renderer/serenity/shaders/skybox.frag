#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_multiview : enable

layout(location = 0) in vec2 texCoord;
layout(location = 1) in vec4 postColor;

layout(location = 0) out vec4 fragColor;

layout(set = 4, binding = 0) uniform sampler2D diffuseMap;

layout(std140, set = 3, binding = 0) uniform SerenitSkyBoxMaterial
{
    vec3 normalScaling;
    float normalMapGain;

    vec3 semInner;
    float _pad1;

    vec3 semOuter;
    float semGain;

    vec3 difInner;
    float _pad2;

    vec3 difOuter;
    float difGain;

    float postVertexColor;
    float postGain;
    float gammax;
    float _pad3;
}
material;

void main()
{
    vec2 tc = vec2(texCoord.x, 1.0 - texCoord.y);
    fragColor = texture(diffuseMap, tc) * postColor;
    fragColor.rgb = pow(fragColor.rgb, vec3(material.gammax));
}
