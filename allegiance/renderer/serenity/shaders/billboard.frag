#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_multiview : enable

layout(location = 0) out vec4 fragColor;
layout(location = 2) in vec2 texCoords;

layout(set = 4, binding = 0) uniform sampler2D texSampler;

void main()
{
    fragColor = texture(texSampler, texCoords);
}
