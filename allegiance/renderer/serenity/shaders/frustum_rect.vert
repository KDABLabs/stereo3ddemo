#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_multiview : enable

layout(location = 0) in vec3 vertexPosition;
layout(location = 0) out vec3 uv;

void main()
{
    uv = vertexPosition;
    // Move to smaller viewport (0.4, 0.4) that is in the bottom left corner
    gl_Position = vec4(vertexPosition, 1.0) * vec4(0.4, 0.4, 1.0, 1.0) + vec4(-0.6, 0.6, 0.0, 0.0); // -1 to 1 range;
}
