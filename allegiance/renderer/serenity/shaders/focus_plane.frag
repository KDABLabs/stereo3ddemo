#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_multiview : enable

layout(location = 0) out vec4 fragColor;

void main()
{
    fragColor = vec4(1.0, 1.0, 1.0, 0.5);
}
