#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 projection;
} ubo;

layout(location = 0) in vec2 inVertex;
layout(location = 1) in vec2 inUv;

layout(location = 0) out vec2 fragUv;

void main() {
    gl_Position = ubo.projection * ubo.view * vec4(inVertex, 0.0, 1.0);
    fragUv = inUv;
}
