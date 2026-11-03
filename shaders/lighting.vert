#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec3 fragColor;
layout(location = 2) out vec3 fragWorldPos;

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec3 lightDir;
    float lightIntensity;
    vec3 lightColor;
    float padding1;
    vec3 ambientColor;
    float shininess;
    vec3 specularColor;
    float padding2;
    vec3 cameraPos;
    float padding3;
    vec3 albedo;
    float padding4;
} ubo;

layout(push_constant) uniform PushConstants {
    mat4 model;
    vec4 color;
    int isLightSource;
} pc;

void main() {
    vec4 worldPos = pc.model * vec4(inPosition, 1.0);
    fragNormal = mat3(transpose(inverse(pc.model))) * inNormal;
    fragColor = inColor;
    fragWorldPos = worldPos.xyz;
    gl_Position = ubo.proj * ubo.view * worldPos;
}