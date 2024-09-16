#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_multiview : enable

layout(location = 0) out vec4 fragColor;
layout(location = 2) in vec2 texCoords;

layout(set = 4, binding = 0) uniform sampler2D texSampler;
layout(std140, set = 3, binding = 0) uniform UBlock {vec4 tintColor;}material;


void main()
{
    float alpha = texture(texSampler, texCoords).a;
    fragColor = vec4(material.tintColor.rgb, alpha * material.tintColor.a);
}
