#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_multiview : enable

layout(location = 0) in vec4 postColor;
layout(location = 1) in vec4 fragVertexColor;
layout(location = 2) smooth in vec3 normalSem;
layout(location = 3) in vec2 texCoord;

layout(location = 0) out vec4 fragColor;

layout(std140, set = 3, binding = 0) uniform SerenitGlossyMaterial
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

layout(set = 4, binding = 0) uniform sampler2D semMap;
layout(set = 4, binding = 1) uniform sampler2D diffuseMap;
layout(set = 4, binding = 2) uniform sampler2D normalMap;

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
    vec2 normalMap = (texture(normalMap, texCoord).xy * 2.0 - vec2(1.0)) * material.normalMapGain;
    vec3 normalSem_ = normalize(normalSem + vec3(normalMap, 0.0));
    float fresnel = semFresnel(normalSem_);

    if (fragVertexColor.r < 0.4)
        fresnel = 0 * fresnel;
    if (fresnel >= 0.3) {
        fresnel = 0.2;
    }

    vec3 diffuse = texture(diffuseMap, texCoord).xyz * mix(material.difInner, material.difOuter, fresnel) * material.difGain;
    vec3 semColor = texture(semMap, semS(normalSem_)).xyz * mix(material.semInner, material.semOuter, fresnel) * material.semGain;

    fragColor = postColor * vec4(diffuse + semColor, 1.0);
}
