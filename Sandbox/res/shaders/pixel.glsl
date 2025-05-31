#version 450

layout(set = 0, binding = 0) uniform MyConstants {
    vec4 data;
} ubo;

layout(location = 0) in vec3 fragColor;
layout(location = 0) out vec4 outColor;

void main() {
    outColor = ubo.data;
}