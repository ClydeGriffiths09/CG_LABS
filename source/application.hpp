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
    glm::vec3 color;
};

struct alignas(16) UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec4 tintColor;
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

extern bool useOrtho;
extern glm::vec3 position;
extern glm::vec3 rotation;
extern glm::vec3 scale;
extern bool isAnimating;
extern float animSpeed;
extern float trajectoryRadius;
extern glm::vec4 tintColor;

extern float cameraDistance;
extern float orthoSize;
extern float fov;
extern bool isMouseDragging;
extern glm::vec2 lastMousePos;
extern float mouseSensitivity;

} // namespace application