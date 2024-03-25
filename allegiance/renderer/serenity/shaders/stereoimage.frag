#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 texCoord;

layout(location = 0) out vec4 fragColor;

layout(set = 2, binding = 2) uniform sampler2D texSampler;

void main()
{
    fragColor = vec4(texture(texSampler, texCoord).rgb, 1.0);
}
