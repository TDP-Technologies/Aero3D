#version 450

layout(location = 0) in vec2 fragUV;
layout(location = 1) in float fragTexIndex;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler uSampler;
layout(set = 0, binding = 1) uniform texture2D uTextures[32];

void main() {
    int index = int(fragTexIndex);
    outColor = texture(sampler2D(uTextures[index], uSampler), fragUV);
}