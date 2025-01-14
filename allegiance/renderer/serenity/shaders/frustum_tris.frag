#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_multiview : enable

layout(location = 0) out vec4 fragColor;

layout(std140, set = 3, binding = 0) uniform SerenityColorMaterial
{
    vec4 color;
}
material;

void main()
{
    fragColor = material.color;
}
