#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_multiview : enable

layout(location = 0) in vec3 vertexPosition;

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

layout(std140, set = 4, binding = 0) uniform SerenityTopView
{
    mat4 viewProjectionMatrix;
}
topViewCamera;

void main()
{
    vec4 ndc = topViewCamera.viewProjectionMatrix * vec4(vertexPosition, 1.0);
    // Move to smaller viewport (0.4, 0.4) that is in the bottom left corner
    gl_Position = ndc * vec4(0.4, 0.4, 1.0, 1.0) + vec4(-0.6, 0.6, 0.0, 0.0);
}
