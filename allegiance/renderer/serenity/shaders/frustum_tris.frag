#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_multiview : enable

layout(location = 0) in vec2 clipPos;

layout(location = 0) out vec4 fragColor;

layout(std140, set = 3, binding = 0) uniform SerenityColorMaterial
{
    vec4 color;
}
material;

void main()
{
    if (clipPos.x > (0.6 - 2.0 * 0.4) || clipPos.y < (-0.6 - 2.0 * 0.4))
        discard;
    fragColor = material.color;
}
