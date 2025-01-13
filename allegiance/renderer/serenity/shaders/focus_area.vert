#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_multiview : enable

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;

layout(location = 0) out vec3 color;

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
    color = vertexColor;
    gl_Position = camera.data[gl_ViewIndex].viewProjectionMatrix * vec4(vertexPosition, 1.0);
}
