#version 450

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec3 fragColor;
layout(location = 2) in vec3 fragWorldPos;

layout(location = 0) out vec4 outColor;

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
    if (pc.isLightSource == 1) {
        outColor = pc.color;
        return;
    }

    vec3 normal = normalize(fragNormal);
    vec3 lightDirection = normalize(-ubo.lightDir);
    vec3 viewDir = normalize(ubo.cameraPos - fragWorldPos);

    vec3 baseColor = fragColor * ubo.albedo;

    vec3 ambient = ubo.ambientColor * baseColor;
    
    float diff = max(dot(normal, lightDirection), 0.0);
    vec3 diffuse = diff * ubo.lightColor * ubo.lightIntensity * baseColor;
    
    vec3 halfDir = normalize(lightDirection + viewDir);
    float specAngle = max(dot(normal, halfDir), 0.0);
    float specular = pow(specAngle, ubo.shininess);
    vec3 specularContrib = specular * ubo.specularColor * ubo.lightColor * ubo.lightIntensity;
    
    vec3 result = ambient + diffuse + specularContrib;
    outColor = vec4(result, 1.0);
}