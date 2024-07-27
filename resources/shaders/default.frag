#version 450

layout(location = 0) in vec3 fragUv;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(0.4, 0.7, 0.2, 1.0);
}
