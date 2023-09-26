#pragma once

namespace all {
// vertex shader
constexpr inline std::string_view simple_vs = R"(
#version 150 core

uniform mat4 modelMatrix;
uniform mat4 mvp;

in vec3 vertexPosition;

void main()
{
    vec3 positionW = vec3(modelMatrix * vec4(vertexPosition, 1.0));
    gl_Position = mvp * vec4(positionW, 1.0);
}
)";
// pixel shader
constexpr std::string_view simple_ps_yellow = R"(
#version 150 core
out vec4 fragColor;

void main()
{
    fragColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);
}
)";
constexpr std::string_view simple_ps_red = R"(
#version 150 core
out vec4 fragColor;

void main()
{
    fragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}
)";

} // namespace all