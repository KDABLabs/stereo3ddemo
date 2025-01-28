#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_multiview : enable

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;

layout(location = 0) out vec3 worldNormal;
layout(location = 1) out vec3 worldPosition;

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

void main()
{
    vec4 wPos = (entity.model * vec4(vertexPosition, 1.0));
    worldPosition = wPos.xyz;
    mat3 normalMatrix = transpose(inverse(mat3(entity.model)));

    worldNormal = normalize(normalMatrix * vertexNormal);
    gl_Position = camera.data[gl_ViewIndex].viewProjectionMatrix * wPos;
}
