#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_multiview : enable

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec3 vertexTexCoord;
layout(location = 3) in vec4 vertexColor;

layout(location = 0) out vec4 postColor;
layout(location = 1) out vec4 fragVertexColor;
layout(location = 2) smooth out vec3 normalSem;
layout(location = 3) out vec2 texCoord;

layout(set = 0, binding = 0) uniform SerenityEntity
{
    mat4 model;
}
entity;

struct CameraData {
    mat4 viewMatrix;
    mat4 inverseViewMatrix;
    mat4 projectionMatrix;
    mat4 inverseProjectionMatrix;
    mat4 viewProjectionMatrix;
    mat4 inverseViewProjectionMatrix;
    mat4 viewportMatrix;
};

layout(set = 1, binding = 0) uniform SerenityCamera
{
    CameraData data[2];
}
camera;

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

vec3 semNormal()
{
    vec3 n = (camera.data[gl_ViewIndex].viewMatrix * entity.model * vec4(vertexNormal, 0.0)).xyz; // ignore position
    n *= material.normalScaling;
    normalize(n);
    return n;
}

void main()
{
    normalSem = semNormal();
    texCoord = vertexTexCoord.xy;
    postColor = mix(vec4(1.0), vertexColor, material.postVertexColor) * material.postGain;
    fragVertexColor = vertexColor;

    vec4 wPos = (entity.model * vec4(vertexPosition, 1.0));
    gl_Position = camera.data[gl_ViewIndex].viewProjectionMatrix * wPos;
}
