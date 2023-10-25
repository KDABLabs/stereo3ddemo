#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_multiview : enable

layout(location = 0) in vec3 vertexPosition;
layout(location = 2) in vec2 vertexTexCoord;

layout(location = 2) out vec2 texCoords;

layout(set = 0, binding = 0) uniform SerenityEntity {
    mat4 model;
} entity;

struct CameraData
{
  mat4 viewMatrix;
  mat4 inverseViewMatrix;
  mat4 projectionMatrix;
  mat4 inverseProjectionMatrix;
  mat4 viewProjectionMatrix;
  mat4 inverseViewProjectionMatrix;
  mat4 viewportMatrix;
};

layout(set = 1, binding = 0) uniform SerenityCamera {
  CameraData data[2];
} camera;

void main()
{
    mat4 modelView = camera.data[gl_ViewIndex].viewMatrix * entity.model;
    
    modelView[0][0] = entity.model[0][0];
    modelView[0][1] = 0;
    modelView[0][2] = 0;

    modelView[1][0] = 0;
    modelView[1][1] = entity.model[1][1];
    modelView[1][2] = 0;

    modelView[2][0] = 0;
    modelView[2][1] = 0;
    modelView[2][2] = entity.model[2][2];


    texCoords = vertexTexCoord;
    gl_Position = camera.data[gl_ViewIndex].projectionMatrix * modelView * vec4(vertexPosition, 1.0f);
}
