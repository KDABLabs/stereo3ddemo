#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_multiview : enable

layout(location = 0) in vec2 clipPos;
layout(location = 0) out vec4 fragColor;

void main()
{
    if (clipPos.x > (0.6 - 2.0 * 0.4) || clipPos.y < (-0.6 - 2.0 * 0.4))
        discard;
    fragColor = vec4(1.0);
}
