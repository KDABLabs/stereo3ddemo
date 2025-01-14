#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_multiview : enable

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

layout(std140, set = 3, binding = 0) uniform SerenityColorMaterial
{
    vec4 backgroundColor;
    vec4 outlineColor;
    float outlineWidth;
}
material;

layout(location = 0) in vec3 uv;
layout(location = 0) out vec4 fragColor;

vec3 projectFromNDCToViewport(vec4 p)
{
    // Move to smaller viewport (0.4, 0.4) that is in the bottom left corner
    vec4 ndcCorrectPos = p * vec4(0.4, 0.4, 1.0, 1.0) + vec4(-0.6, 0.6, 0.0, 0.0);
    return vec3(camera.data[0].viewportMatrix * ndcCorrectPos);
}

void main(void)
{
    // Screen space position for fragment
    vec3 pxPos = projectFromNDCToViewport(vec4(vec3(uv.xy, 0.0), 1.0));

    // Compute Screen space distance of fragment against rect edges
    vec3 top = vec3(uv.x, 1.0, 0.0);
    vec3 bottom = vec3(uv.x, -1.0, 0.0);
    vec3 left = vec3(-1.0, uv.y, 0.0);
    vec3 right = vec3(1.0, uv.y, 0.0);

    float dTop = length(projectFromNDCToViewport(vec4(top, 1.0)) - pxPos);
    float dBottom = length(projectFromNDCToViewport(vec4(bottom, 1.0)) - pxPos);
    float dLeft = length(projectFromNDCToViewport(vec4(left, 1.0)) - pxPos);
    float dRight = length(projectFromNDCToViewport(vec4(right, 1.0)) - pxPos);

    // Find smallest distance to any edge
    float minDistance = min(dTop, min(dBottom, min(dLeft, dRight)));

    // Blend between  background color and outline color depending on fragment distance to rect edges
    fragColor = mix(material.outlineColor, material.backgroundColor, smoothstep(material.outlineWidth - 0.5, material.outlineWidth, minDistance));
}
