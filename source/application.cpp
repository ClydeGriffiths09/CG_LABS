#include "application.hpp"
#include <imgui.h>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cstring>
#include <algorithm>

namespace application {

VkPipeline pipeline = VK_NULL_HANDLE;
VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

VkBuffer vertexBuffer = VK_NULL_HANDLE;
VmaAllocation vertexBufferAlloc = VK_NULL_HANDLE;
VkBuffer indexBuffer = VK_NULL_HANDLE;
VmaAllocation indexBufferAlloc = VK_NULL_HANDLE;
VkBuffer uniformBuffer = VK_NULL_HANDLE;
VmaAllocation uniformBufferAlloc = VK_NULL_HANDLE;
VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

std::vector<Vertex> vertices;
std::vector<uint32_t> indices;

// Camera
float cameraDistance = 8.0f;
float cameraYaw = 0.7f;
float cameraPitch = 0.4f;
float fov = 45.0f;
bool useOrtho = false;
float orthoSize = 4.0f;

// Object Rotation
float cubeRotX = 0.0f;
float cubeRotY = 0.0f;
float cubeRotZ = 0.0f;

// Light
glm::vec3 lightDir = glm::normalize(glm::vec3(0.5f, -1.0f, 0.3f));
float lightIntensity = 1.5f; 
glm::vec3 lightColor = glm::vec3(1.0f, 0.95f, 0.9f);
bool showLightSource = true;

// Material
glm::vec3 ambientColor = glm::vec3(0.3f, 0.3f, 0.35f); 
float shininess = 64.0f; 
glm::vec3 specularColor = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec3 objectAlbedo = glm::vec3(1.0f, 1.0f, 1.0f);

// Mouse
bool isMouseDragging = false;
glm::vec2 lastMousePos = glm::vec2(0.0f, 0.0f);
float mouseSensitivity = 0.005f;

void generateParallelepipedWithNormals() {
    vertices = {
        // Front face (z = 1) - normal (0, 0, 1) - Ярко-красный
        {{-1.0f, -1.0f,  1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.1f, 0.1f}},
        {{ 1.0f, -1.0f,  1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.1f, 0.1f}},
        {{ 1.0f,  1.0f,  1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.1f, 0.1f}},
        {{-1.0f,  1.0f,  1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.1f, 0.1f}},
        
        // Back face (z = -1) - normal (0, 0, -1) - Ярко-зеленый
        {{ 1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.1f, 1.0f, 0.1f}},
        {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.1f, 1.0f, 0.1f}},
        {{-1.0f,  1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.1f, 1.0f, 0.1f}},
        {{ 1.0f,  1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.1f, 1.0f, 0.1f}},
        
        // Top face (y = 1) - normal (0, 1, 0) - Ярко-синий
        {{-1.0f,  1.0f,  1.0f}, {0.0f, 1.0f, 0.0f}, {0.1f, 0.1f, 1.0f}},
        {{ 1.0f,  1.0f,  1.0f}, {0.0f, 1.0f, 0.0f}, {0.1f, 0.1f, 1.0f}},
        {{ 1.0f,  1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.1f, 0.1f, 1.0f}},
        {{-1.0f,  1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.1f, 0.1f, 1.0f}},
        
        // Bottom face (y = -1) - normal (0, -1, 0) - Желтый
        {{-1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 0.1f}},
        {{ 1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 0.1f}},
        {{ 1.0f, -1.0f,  1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 0.1f}},
        {{-1.0f, -1.0f,  1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 0.1f}},
        
        // Right face (x = 1) - normal (1, 0, 0) - Пурпурный
        {{ 1.0f, -1.0f,  1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.1f, 1.0f}},
        {{ 1.0f,  1.0f,  1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.1f, 1.0f}},
        {{ 1.0f,  1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.1f, 1.0f}},
        {{ 1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.1f, 1.0f}},
        
        // Left face (x = -1) - normal (-1, 0, 0) - Голубой
        {{-1.0f, -1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {0.1f, 1.0f, 1.0f}},
        {{-1.0f,  1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {0.1f, 1.0f, 1.0f}},
        {{-1.0f,  1.0f,  1.0f}, {-1.0f, 0.0f, 0.0f}, {0.1f, 1.0f, 1.0f}},
        {{-1.0f, -1.0f,  1.0f}, {-1.0f, 0.0f, 0.0f}, {0.1f, 1.0f, 1.0f}}
    };

    indices = {
        0,  1,  2,  2,  3,  0,
        4,  5,  6,  6,  7,  4,
        8,  9,  10, 10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20
    };
}

void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                  VmaMemoryUsage memoryUsage, VkBuffer& buffer,
                  VmaAllocation& allocation) {
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = memoryUsage;
    allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

    vmaCreateBuffer(graphics::internal::context.allocator, &bufferInfo,
                    &allocInfo, &buffer, &allocation, nullptr);
}

std::vector<char> loadShader(const char* filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error(std::string("Failed to open shader: ") + filename);
    }
    size_t fileSize = (size_t)file.tellg();
    std::vector<char> code(fileSize);
    file.seekg(0);
    file.read(code.data(), fileSize);
    return code;
}

VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule module;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &module) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create shader module!");
    }
    return module;
}

glm::vec3 getCameraPosition() {
    float x = cameraDistance * cos(cameraPitch) * sin(cameraYaw);
    float y = cameraDistance * sin(cameraPitch);
    float z = cameraDistance * cos(cameraPitch) * cos(cameraYaw);
    return glm::vec3(x, y, z);
}

glm::mat4 getViewMatrix() {
    glm::vec3 camPos = getCameraPosition();
    glm::vec3 target(0.0f, 0.0f, 0.0f);
    glm::vec3 up(0.0f, 1.0f, 0.0f);
    return glm::lookAt(camPos, target, up);
}

bool initialize() {
    generateParallelepipedWithNormals();
    auto& ctx = graphics::internal::context;

    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    if (vkCreateDescriptorSetLayout(ctx.device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        std::cerr << "Failed to create descriptor set layout!\n";
        return false;
    }

    VkPushConstantRange pushConstantRange = {};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(PushConstants);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(ctx.device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        std::cerr << "Failed to create pipeline layout!\n";
        return false;
    }

    auto vertCode = loadShader("shaders/lighting.vert.spv");
    auto fragCode = loadShader("shaders/lighting.frag.spv");

    VkShaderModule vertModule = createShaderModule(ctx.device, vertCode);
    VkShaderModule fragModule = createShaderModule(ctx.device, fragCode);

    VkPipelineShaderStageCreateInfo shaderStages[2] = {};
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = vertModule;
    shaderStages[0].pName = "main";

    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = fragModule;
    shaderStages[1].pName = "main";

    VkVertexInputBindingDescription bindingDesc = {};
    bindingDesc.binding = 0;
    bindingDesc.stride = sizeof(Vertex);
    bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attributeDescs[3] = {};
    attributeDescs[0].location = 0;
    attributeDescs[0].binding = 0;
    attributeDescs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescs[0].offset = offsetof(Vertex, pos);

    attributeDescs[1].location = 1;
    attributeDescs[1].binding = 0;
    attributeDescs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescs[1].offset = offsetof(Vertex, normal);

    attributeDescs[2].location = 2;
    attributeDescs[2].binding = 0;
    attributeDescs[2].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescs[2].offset = offsetof(Vertex, color);

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDesc;
    vertexInputInfo.vertexAttributeDescriptionCount = 3;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescs;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)ctx.swapchain_extent.width;
    viewport.height = (float)ctx.swapchain_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = ctx.swapchain_extent;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                          VK_COLOR_COMPONENT_G_BIT |
                                          VK_COLOR_COMPONENT_B_BIT |
                                          VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = ctx.render_pass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(ctx.device, VK_NULL_HANDLE, 1, &pipelineInfo,
                                  nullptr, &pipeline) != VK_SUCCESS) {
        std::cerr << "Failed to create graphics pipeline!\n";
        return false;
    }

    vkDestroyShaderModule(ctx.device, fragModule, nullptr);
    vkDestroyShaderModule(ctx.device, vertModule, nullptr);

    createBuffer(sizeof(vertices[0]) * vertices.size(),
                 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VMA_MEMORY_USAGE_AUTO, vertexBuffer, vertexBufferAlloc);

    createBuffer(sizeof(indices[0]) * indices.size(),
                 VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                 VMA_MEMORY_USAGE_AUTO, indexBuffer, indexBufferAlloc);

    createBuffer(sizeof(UniformBufferObject),
                 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                 VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
                 uniformBuffer, uniformBufferAlloc);

    void* data;
    vmaMapMemory(ctx.allocator, vertexBufferAlloc, &data);
    memcpy(data, vertices.data(), sizeof(vertices[0]) * vertices.size());
    vmaUnmapMemory(ctx.allocator, vertexBufferAlloc);

    vmaMapMemory(ctx.allocator, indexBufferAlloc, &data);
    memcpy(data, indices.data(), sizeof(indices[0]) * indices.size());
    vmaUnmapMemory(ctx.allocator, indexBufferAlloc);

    VkDescriptorPoolSize poolSize = {};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.maxSets = 1;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;

    if (vkCreateDescriptorPool(ctx.device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        std::cerr << "Failed to create descriptor pool!\n";
        return false;
    }

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout;

    if (vkAllocateDescriptorSets(ctx.device, &allocInfo, &descriptorSet) != VK_SUCCESS) {
        std::cerr << "Failed to allocate descriptor sets!\n";
        return false;
    }

    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = uniformBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);

    VkWriteDescriptorSet descriptorWrite = {};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(ctx.device, 1, &descriptorWrite, 0, nullptr);

    return true;
}

void shutdown() {
    auto& ctx = graphics::internal::context;
    vkQueueWaitIdle(ctx.graphics_queue);

    vkDestroyPipeline(ctx.device, pipeline, nullptr);
    vkDestroyPipelineLayout(ctx.device, pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(ctx.device, descriptorSetLayout, nullptr);

    vmaDestroyBuffer(ctx.allocator, vertexBuffer, vertexBufferAlloc);
    vmaDestroyBuffer(ctx.allocator, indexBuffer, indexBufferAlloc);
    vmaDestroyBuffer(ctx.allocator, uniformBuffer, uniformBufferAlloc);

    if (descriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(ctx.device, descriptorPool, nullptr);
    }
}

void update(double time) {
    auto& ctx = graphics::internal::context;

    ImGui::Begin("Lab 2 - Camera and Lighting");

    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Camera");
    ImGui::Separator();
    ImGui::SliderFloat("Distance", &cameraDistance, 2.0f, 20.0f);
    ImGui::SliderFloat("Yaw", &cameraYaw, -3.14159f, 3.14159f);
    ImGui::SliderFloat("Pitch", &cameraPitch, -1.5f, 1.5f);
    ImGui::Checkbox("Orthographic", &useOrtho);
    ImGui::SliderFloat("FOV", &fov, 20.0f, 90.0f);
    ImGui::SliderFloat("Ortho Size", &orthoSize, 2.0f, 10.0f);
    ImGui::Separator();

    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Object Rotation");
    ImGui::Separator();
    ImGui::SliderFloat("Rot X", &cubeRotX, 0.0f, 360.0f);
    ImGui::SliderFloat("Rot Y", &cubeRotY, 0.0f, 360.0f);
    ImGui::SliderFloat("Rot Z", &cubeRotZ, 0.0f, 360.0f);
    ImGui::Separator();

    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Directional Light");
    ImGui::Separator();
    ImGui::SliderFloat3("Direction", &lightDir.x, -1.0f, 1.0f);
    ImGui::ColorEdit3("Color", &lightColor.x);
    ImGui::SliderFloat("Intensity", &lightIntensity, 0.0f, 3.0f);
    ImGui::Checkbox("Show Light Source", &showLightSource);
    ImGui::Separator();

    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Material (Blinn-Phong)");
    ImGui::Separator();
    ImGui::ColorEdit3("Albedo", &objectAlbedo.x); // <-- НОВОЕ
    ImGui::ColorEdit3("Ambient", &ambientColor.x);
    ImGui::SliderFloat("Shininess", &shininess, 1.0f, 256.0f);
    ImGui::ColorEdit3("Specular", &specularColor.x);
    ImGui::Separator();

    ImGui::Text("Mouse: Left Drag = Rotate Camera, Scroll = Zoom");
    ImGui::SliderFloat("Sensitivity", &mouseSensitivity, 0.001f, 0.02f);

    ImGui::End();

    // Mouse handling
    ImGuiIO& io = ImGui::GetIO();

    if (ImGui::IsMouseDown(0) && !io.WantCaptureMouse) {
        if (!isMouseDragging) {
            lastMousePos = glm::vec2(io.MousePos.x, io.MousePos.y);
            isMouseDragging = true;
        } else {
            glm::vec2 currentMousePos(io.MousePos.x, io.MousePos.y);
            glm::vec2 delta = currentMousePos - lastMousePos;

            cameraYaw += delta.x * mouseSensitivity;
            cameraPitch += delta.y * mouseSensitivity;
            cameraPitch = glm::clamp(cameraPitch, -1.5f, 1.5f);

            lastMousePos = currentMousePos;
        }
    } else {
        isMouseDragging = false;
    }

    if (io.MouseWheel != 0.0f && !io.WantCaptureMouse) {
        cameraDistance -= io.MouseWheel * 0.5f;
        cameraDistance = glm::clamp(cameraDistance, 2.0f, 20.0f);
    }

    // Normalize light direction
    lightDir = glm::normalize(lightDir);

    // Get camera position for shader
    glm::vec3 camPos = getCameraPosition();

    // Update UBO
    UniformBufferObject ubo = {};
    ubo.view = getViewMatrix();

    float aspect = (float)ctx.swapchain_extent.width / (float)ctx.swapchain_extent.height;
    if (useOrtho) {
        ubo.proj = glm::ortho(-orthoSize * aspect, orthoSize * aspect, 
                              -orthoSize, orthoSize, 0.1f, 100.0f);
    } else {
        ubo.proj = glm::perspective(glm::radians(fov), aspect, 0.1f, 100.0f);
    }
    ubo.proj[1][1] *= -1.0f;

    ubo.lightDir = lightDir;
    ubo.lightIntensity = lightIntensity;
    ubo.lightColor = lightColor;
    ubo.ambientColor = ambientColor;
    ubo.shininess = shininess;
    ubo.specularColor = specularColor;
    ubo.cameraPos = camPos;
    ubo.albedo = objectAlbedo;
    ubo.padding1 = 0.0f;
    ubo.padding2 = 0.0f;
    ubo.padding3 = 0.0f;
    ubo.padding4 = 0.0f;

    void* data;
    vmaMapMemory(ctx.allocator, uniformBufferAlloc, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vmaUnmapMemory(ctx.allocator, uniformBufferAlloc);
}

void render(const graphics::internal::FrameData& fd) {
    vkResetCommandBuffer(fd.command_buffer, 0);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(fd.command_buffer, &beginInfo);

    VkClearValue clearValues[2] = {};
    clearValues[0].color = {{0.1f, 0.1f, 0.15f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = graphics::internal::context.render_pass;
    renderPassInfo.framebuffer = fd.framebuffer;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = graphics::internal::context.swapchain_extent;
    renderPassInfo.clearValueCount = 2;
    renderPassInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(fd.command_buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(fd.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    VkBuffer vertexBuffers[] = { vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(fd.command_buffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(fd.command_buffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdBindDescriptorSets(fd.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

    PushConstants pcMain = {};
    
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(cubeRotX), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(cubeRotY), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(cubeRotZ), glm::vec3(0.0f, 0.0f, 1.0f));
    
    pcMain.model = model;
    pcMain.isLightSource = 0;
    
    vkCmdPushConstants(fd.command_buffer, pipelineLayout, 
                       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
                       0, sizeof(PushConstants), &pcMain);
                       
    vkCmdDrawIndexed(fd.command_buffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

    if (showLightSource) {
        PushConstants pcLight = {};
        
        glm::vec3 lightGizmoPos = -lightDir * 8.0f; 
        
        glm::mat4 translation = glm::translate(glm::mat4(1.0f), lightGizmoPos);
        glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.3f));
        pcLight.model = translation * scale;
        
        pcLight.color = glm::vec4(lightColor * lightIntensity, 1.0f);
        pcLight.isLightSource = 1;

        vkCmdPushConstants(fd.command_buffer, pipelineLayout, 
                           VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
                           0, sizeof(PushConstants), &pcLight);

        vkCmdDrawIndexed(fd.command_buffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
    }

    vkCmdEndRenderPass(fd.command_buffer);
    vkEndCommandBuffer(fd.command_buffer);
}

} // namespace application