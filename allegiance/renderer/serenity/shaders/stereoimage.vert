#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_multiview : enable

layout(location = 0) in vec2 vertexPosition;
layout(location = 1) in vec2 vertexTexCoord;

layout(location = 0) out vec2 texCoord;

layout(std140, set = 3, binding = 0) uniform ViewportData {
    vec2 viewportSize;
    vec2 stereoImageSize;
} viewportData;

void main()
{
    float viewportAspect = viewportData.viewportSize.x / viewportData.viewportSize.y;
    float imageAspect = (0.5 * viewportData.stereoImageSize.x) / viewportData.stereoImageSize.y;

    // Compute texture coordinates so that the image is zoomed as much as possible while preserving aspect ratio
    vec2 texCoordMin, texCoordMax;
    if (viewportAspect > imageAspect) {
        float f = imageAspect / viewportAspect;
        texCoordMin = vec2(0.0, 0.5 - 0.5 * f);
        texCoordMax = vec2(1.0, 0.5 + 0.5 * f);
    } else {
        float f = viewportAspect / imageAspect;
        texCoordMin = vec2(0.5 - 0.5 * f, 0.0);
        texCoordMax = vec2(0.5 + 0.5 * f, 1.0);
    }

    // We want to show the left or the right half of the image
    texCoordMin.x = 0.5 * texCoordMin.x + 0.5 * float(gl_ViewIndex);
    texCoordMax.x = 0.5 * texCoordMax.x + 0.5 * float(gl_ViewIndex);

    texCoord = vec2(mix(texCoordMin.x, texCoordMax.x, vertexTexCoord.x), mix(texCoordMin.y, texCoordMax.y, vertexTexCoord.y));
    gl_Position = vec4(vertexPosition, 0.0, 1.0);
}
