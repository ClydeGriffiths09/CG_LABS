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

bool useOrtho = false;
glm::vec3 position = {0.0f, 0.0f, 0.0f};
glm::vec3 rotation = {0.0f, 0.0f, 0.0f};
glm::vec3 scale = {1.0f, 1.0f, 1.0f};
bool isAnimating = true;
float animSpeed = 0.5f;
float trajectoryRadius = 3.0f;
glm::vec4 tintColor = {1.0f, 1.0f, 1.0f, 1.0f};

float cameraDistance = 10.0f;
float orthoSize = 5.0f;
float fov = 45.0f;
bool isMouseDragging = false;
glm::vec2 lastMousePos = {0.0f, 0.0f};
float mouseSensitivity = 0.3f;

void generateParallelepiped() {
    auto makeVertex = [](float x, float y, float z) {
        glm::vec3 pos(x, y, z);
        glm::vec3 color = (pos + 1.0f) * 0.5f;
        return Vertex{pos, color};
    };

    vertices = {
        makeVertex(-1.0f, -1.0f, -1.0f),
        makeVertex( 1.0f, -1.0f, -1.0f),
        makeVertex( 1.0f,  1.0f, -1.0f),
        makeVertex(-1.0f,  1.0f, -1.0f),
        makeVertex(-1.0f, -1.0f,  1.0f),
        makeVertex( 1.0f, -1.0f,  1.0f),
        makeVertex( 1.0f,  1.0f,  1.0f),
        makeVertex(-1.0f,  1.0f,  1.0f)
    };

    indices = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        3, 2, 6, 6, 7, 3,
        0, 1, 5, 5, 4, 0,
        1, 5, 6, 6, 2, 1,
        0, 4, 7, 7, 3, 0
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

bool initialize() {
    generateParallelepiped();
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

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

    if (vkCreatePipelineLayout(ctx.device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        std::cerr << "Failed to create pipeline layout!\n";
        return false;
    }

    auto vertCode = loadShader("shaders/parallelepiped.vert.spv");
    auto fragCode = loadShader("shaders/parallelepiped.frag.spv");

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

    VkVertexInputAttributeDescription attributeDescs[2] = {};
    attributeDescs[0].location = 0;
    attributeDescs[0].binding = 0;
    attributeDescs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescs[0].offset = offsetof(Vertex, pos);

    attributeDescs[1].location = 1;
    attributeDescs[1].binding = 0;
    attributeDescs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescs[1].offset = offsetof(Vertex, color);

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDesc;
    vertexInputInfo.vertexAttributeDescriptionCount = 2;
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

    ImGui::Begin("Lab 1 Controls (Grade 4)");

    ImGui::Checkbox("Orthographic Projection", &useOrtho);
    ImGui::Separator();

    ImGui::SliderFloat3("Position", &position.x, -5.0f, 5.0f);
    ImGui::SliderFloat3("Rotation", &rotation.x, 0.0f, 360.0f);
    ImGui::SliderFloat3("Scale", &scale.x, 0.1f, 3.0f);
    ImGui::Separator();

    ImGui::Checkbox("Animate (Play/Pause)", &isAnimating);
    ImGui::SliderFloat("Animation Speed", &animSpeed, 0.1f, 5.0f);
    ImGui::SliderFloat("Trajectory Radius", &trajectoryRadius, 1.0f, 10.0f);
    ImGui::Separator();

    ImGui::ColorEdit4("Global Tint Color", &tintColor.x);
    ImGui::Separator();

    ImGui::SliderFloat("Camera Distance (Zoom)", &cameraDistance, 2.0f, 30.0f);
    ImGui::SliderFloat("FOV (Perspective)", &fov, 20.0f, 90.0f);
    ImGui::SliderFloat("Ortho Size", &orthoSize, 2.0f, 15.0f);
    ImGui::Separator();

    ImGui::SliderFloat("Mouse Sensitivity", &mouseSensitivity, 0.1f, 2.0f);
    ImGui::Text("Drag with left mouse button to rotate");
    ImGui::Text("Scroll to zoom");

    ImGui::End();

    ImGuiIO& io = ImGui::GetIO();

    if (ImGui::IsMouseDown(0) && !io.WantCaptureMouse) {
        if (!isMouseDragging) {
            lastMousePos = glm::vec2(io.MousePos.x, io.MousePos.y);
            isMouseDragging = true;
        } else {
            glm::vec2 currentMousePos(io.MousePos.x, io.MousePos.y);
            glm::vec2 delta = currentMousePos - lastMousePos;

            rotation.y += delta.x * mouseSensitivity;
            rotation.x += delta.y * mouseSensitivity;

            rotation.x = glm::clamp(rotation.x, -90.0f, 90.0f);

            if (rotation.y > 360.0f) rotation.y -= 360.0f;
            if (rotation.y < 0.0f) rotation.y += 360.0f;

            lastMousePos = currentMousePos;
        }
    } else {
        isMouseDragging = false;
    }

    if (io.MouseWheel != 0.0f && !io.WantCaptureMouse) {
        cameraDistance -= io.MouseWheel * 1.0f;
        cameraDistance = glm::clamp(cameraDistance, 2.0f, 30.0f);
    }

    UniformBufferObject ubo = {};
    glm::mat4 model = glm::mat4(1.0f);

    if (isAnimating) {
        float angle = (float)time * animSpeed;
        float animX = cos(angle) * trajectoryRadius;
        float animZ = sin(angle) * trajectoryRadius;
        model = glm::translate(model, glm::vec3(animX, 0.0f, animZ));
    }

    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, scale);

    ubo.model = model;

    ubo.view = glm::lookAt(
        glm::vec3(0.0f, cameraDistance * 0.6f, cameraDistance),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    float aspect = (float)ctx.swapchain_extent.width / (float)ctx.swapchain_extent.height;
    
    if (useOrtho) {
        ubo.proj = glm::ortho(-orthoSize * aspect, orthoSize * aspect, -orthoSize, orthoSize, 0.1f, 100.0f);
    } else {
        ubo.proj = glm::perspective(glm::radians(fov), aspect, 0.1f, 100.0f);
    }
    
    ubo.proj[1][1] *= -1.0f;

    ubo.tintColor = tintColor;

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

    vkCmdDrawIndexed(fd.command_buffer,
                     static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

    vkCmdEndRenderPass(fd.command_buffer);
    vkEndCommandBuffer(fd.command_buffer);
}

} // namespace application