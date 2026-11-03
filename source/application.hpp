#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED

#include "graphics_internal.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

namespace application {

bool initialize();
void shutdown();
void update(double time);
void render(const graphics::internal::FrameData& fd);

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec3 color;
};

struct PushConstants {
    glm::mat4 model;
    glm::vec4 color;
    int isLightSource;
};

struct alignas(16) UniformBufferObject {
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec3 lightDir;
    float lightIntensity;
    glm::vec3 lightColor;
    float padding1;
    glm::vec3 ambientColor;
    float shininess;
    glm::vec3 specularColor;
    float padding2;
    glm::vec3 cameraPos;
    float padding3;
    glm::vec3 albedo;
    float padding4;
};

extern VkPipeline pipeline;
extern VkPipelineLayout pipelineLayout;
extern VkDescriptorSetLayout descriptorSetLayout;
extern VkDescriptorPool descriptorPool;

extern VkBuffer vertexBuffer;
extern VmaAllocation vertexBufferAlloc;
extern VkBuffer indexBuffer;
extern VmaAllocation indexBufferAlloc;
extern VkBuffer uniformBuffer;
extern VmaAllocation uniformBufferAlloc;
extern VkDescriptorSet descriptorSet;

extern std::vector<Vertex> vertices;
extern std::vector<uint32_t> indices;

// Camera
extern float cameraDistance;
extern float cameraYaw;
extern float cameraPitch;
extern float fov;
extern bool useOrtho;
extern float orthoSize;

// Object Rotation
extern float cubeRotX;
extern float cubeRotY;
extern float cubeRotZ;

// Light
extern glm::vec3 lightDir;
extern float lightIntensity;
extern glm::vec3 lightColor;
extern bool showLightSource;

// Material
extern glm::vec3 ambientColor;
extern float shininess;
extern glm::vec3 specularColor;
extern glm::vec3 objectAlbedo;

// Mouse
extern bool isMouseDragging;
extern glm::vec2 lastMousePos;
extern float mouseSensitivity;

} // namespace application