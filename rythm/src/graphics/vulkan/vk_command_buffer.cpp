#include "rmpch.h"
#include "vk_command_buffer.h"
#include "vk_render_device.h"
#include "vk_swapchain.h"
#include "vk_graphics_pipeline.h"
#include "vk_image.h"
#include "vk_helpers.h"

#include <volk.h>

namespace rm::vk {
    VKCommandBuffer::VKCommandBuffer(VKRenderDevice& device, const CommandBufferSpec& spec): device(device) {
        VkCommandPoolCreateInfo commandPoolCI{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = device.GetQueueFamily()
        };
        VK_CHECK(vkCreateCommandPool(device.GetDevice(), &commandPoolCI, nullptr, &pool));

        buffers.resize(spec.buffers);
        VkCommandBufferAllocateInfo cbAllocCI{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = pool,
            .commandBufferCount = spec.buffers
        };
        VK_CHECK(vkAllocateCommandBuffers(device.GetDevice(), &cbAllocCI, buffers.data()));

        RM_LOG_INFO("Created Command Buffer: Count = {}", buffers.size());
    }

    VKCommandBuffer::~VKCommandBuffer() {
        vkDestroyCommandPool(device.GetDevice(), pool, nullptr);
    }

    void VKCommandBuffer::Reset(unsigned int index) {
        RM_ASSERT(index < buffers.size(), "Index is outside of buffer pool size");

        VK_CHECK(vkResetCommandBuffer(buffers[index], 0));
        currentIndex = index;
    }

    void VKCommandBuffer::Begin() const {
        VkCommandBufferBeginInfo info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        };

        vkBeginCommandBuffer(buffers[currentIndex], &info);
    }

    void VKCommandBuffer::End() const {
        vkEndCommandBuffer(buffers[currentIndex]);
    }

    void VKCommandBuffer::BeginRendering(const Swapchain &swapchain, Color clear) const {
        const auto& vkSwapchain = static_cast<const VKSwapchain&>(swapchain);
        const auto& depthImage = static_cast<const VKImage&>(vkSwapchain.GetDepthImage());

        std::array<VkImageMemoryBarrier2, 2> outputBarriers{
            VkImageMemoryBarrier2{
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                .srcAccessMask = 0,
                .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
                .image = vkSwapchain.GetCurrentImage(),
                .subresourceRange{
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .levelCount = 1,
                    .layerCount = 1
                }
            },
            VkImageMemoryBarrier2{
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .srcStageMask =VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
                .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
                .image = depthImage.GetImage(),
                .subresourceRange{
                    .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
                    .levelCount = 1,
                    .layerCount = 1
                }
            }
        };
        VkDependencyInfo barrierDependencyInfo{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .imageMemoryBarrierCount = 2,
            .pImageMemoryBarriers = outputBarriers.data()
        };

        vkCmdPipelineBarrier2(buffers[currentIndex], &barrierDependencyInfo);

        VkRenderingAttachmentInfo colorAttachmentInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = vkSwapchain.GetCurrentImageView(),
            .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = { .color{ clear.r, clear.g, clear.b, clear.a } }
        };
        VkRenderingAttachmentInfo depthAttachmentInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = depthImage.GetView(),
            .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .clearValue = { .depthStencil = { 1.0f, 0 } }
        };
        VkRenderingInfo renderingInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .renderArea{
                .extent = vkSwapchain.GetExtent()
            },
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachmentInfo,
            .pDepthAttachment = &depthAttachmentInfo
        };

        vkCmdBeginRendering(buffers[currentIndex], &renderingInfo);

        VkViewport vp{
            .width = static_cast<float>(vkSwapchain.GetExtent().width),
            .height = static_cast<float>(vkSwapchain.GetExtent().height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f
        };
        vkCmdSetViewport(buffers[currentIndex], 0, 1, &vp);

        VkRect2D scissor{
            .extent = vkSwapchain.GetExtent()
        };

        vkCmdSetScissor(buffers[currentIndex], 0, 1, &scissor);
    }

    void VKCommandBuffer::EndRendering(const Swapchain& swapchain) const {
        const auto& vkSwapchain = static_cast<const VKSwapchain&>(swapchain);

        VkImageMemoryBarrier2 barrierPresent{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstAccessMask = 0,
            .oldLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
            .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .image = vkSwapchain.GetCurrentImage(),
            .subresourceRange{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .levelCount = 1,
                .layerCount = 1
            }
        };

        VkDependencyInfo barrierPresentDependencyInfo{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &barrierPresent
        };

        vkCmdEndRendering(buffers[currentIndex]);
        vkCmdPipelineBarrier2(buffers[currentIndex], &barrierPresentDependencyInfo);
    }

    void VKCommandBuffer::Draw(unsigned int count) const {
        vkCmdDrawIndexed(buffers[currentIndex], count, 1, 0, 0, 0);
    }

    void VKCommandBuffer::BindPipeline(const GraphicsPipeline &pipeline) const {
        const auto& vkPipeline = static_cast<const VKGraphicsPipeline&>(pipeline);

        vkCmdBindPipeline(buffers[currentIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline.GetHandle());
        vkCmdBindDescriptorSets(buffers[currentIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline.GetLayout(), 0, 1, &descriptorSetTex, 0, nullptr);

        VkDeviceSize vOffset{ 0 };
        vkCmdBindVertexBuffers(buffers[currentIndex], 0, 1, &vBuffer, &vOffset);
        vkCmdBindIndexBuffer(buffers[currentIndex], vBuffer, vBufSize, VK_INDEX_TYPE_UINT16);
        vkCmdPushConstants(buffers[currentIndex], vkPipeline.GetLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(VkDeviceAddress), &shaderDataBuffers[frameIndex].deviceAddress);
    }
}
