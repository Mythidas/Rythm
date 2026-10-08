#include "rmpch.h"
#include "renderer.h"
#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_core.h"
#include "vulkan_helpers.h"
#include "core/window.h"

#include <volk.h>
#include <vulkan/vulkan.hpp>
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE
#include <SDL3/SDL_vulkan.h>
#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>
#include <glm/glm.hpp>

#include <vector>

namespace rm::gfx {
    constexpr uint32_t MAX_QUADS = 10000;
    constexpr uint32_t MAX_INDICES = MAX_QUADS * 6;
    constexpr uint32_t MAX_VERTICES = MAX_QUADS * 4;
    constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

    struct Vertex {
        glm::vec3 position;
        glm::vec4 color;
        glm::vec2 uv;
        float texture;
    };

    struct RenderData {
        vk::Instance instance;

        // Device
        vk::Device device;
        vk::Queue deviceQueue;
        vk::PhysicalDevice physicalDevice;
        vk::PhysicalDeviceProperties2 deviceProperties;
        VmaAllocator allocator;
        uint32_t queueFamilyIndex{ 0 };

        // Swapchain
        vk::SurfaceKHR surface;
        vk::SwapchainKHR swapchain;
        std::vector<vk::Image> swapchainImages;
        std::vector<vk::ImageView> swapchainImageViews;
        vk::Format swapchainImageFormat{ vk::Format::eB8G8R8A8Srgb };
        vk::Format swapchainDepthFormat;
        vk::Image swapchainDepthImage;
        vk::ImageView swapchainDepthImageView;
        VmaAllocation swapchainDepthImageAlloc;

        // Command Pool
        vk::CommandPool commandPool;
        std::vector<vk::CommandBuffer> commandBuffers;

        // Buffers
        vk::Buffer vBuffer;
        vk::Buffer iBuffer;
        VmaAllocation vBuffAlloc;
        VmaAllocation iBuffAlloc;
        VmaAllocationInfo vBuffAllocInfo;
    } renderData;

    Renderer::Renderer(Window& window): window(window) {
        CreateInstance();
        CreateDevice();
        CreateSwapchain();
        CreateBuffers();
    }

    Renderer::~Renderer() {
        renderData.device.waitIdle();

        vmaDestroyBuffer(renderData.allocator, renderData.vBuffer, renderData.vBuffAlloc);
        vmaDestroyBuffer(renderData.allocator, renderData.iBuffer, renderData.iBuffAlloc);

        renderData.device.destroyCommandPool(renderData.commandPool, nullptr);

        renderData.device.destroyImageView(renderData.swapchainDepthImageView, nullptr);
        vmaDestroyImage(renderData.allocator, renderData.swapchainDepthImage, renderData.swapchainDepthImageAlloc);
        for (auto& imageView : renderData.swapchainImageViews) {
            renderData.device.destroyImageView(imageView, nullptr);
        }

        vmaDestroyAllocator(renderData.allocator);
        renderData.device.destroySwapchainKHR(renderData.swapchain, nullptr);
        renderData.device.destroy();
        renderData.instance.destroy();
    }

    void Renderer::CreateInstance() {
        bool sdlVulkanInit = SDL_Vulkan_LoadLibrary(NULL);
        RM_ASSERT(sdlVulkanInit == true, "SDL Vulkan failed to initialize");

        RM_VK_CHECK(volkInitialize());
        VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

        vk::ApplicationInfo appInfo("Rythm", 1, "Rythm", 1, VK_API_VERSION_1_3);

        uint32_t instanceExtensionCount{ 0 };
        char const* const* sdlExtensions{ SDL_Vulkan_GetInstanceExtensions(&instanceExtensionCount) };
        std::vector<const char*> instanceExtensions{
            sdlExtensions,
            sdlExtensions + instanceExtensionCount
        };
        const char* validationLayers[]{
            "VK_LAYER_KHRONOS_validation"
        };

        instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        vk::InstanceCreateInfo instanceCI({}, &appInfo, 1, validationLayers, static_cast<uint32_t>(instanceExtensions.size()), instanceExtensions.data());
        renderData.instance = vk::createInstance(instanceCI);
        volkLoadInstance(renderData.instance);
        VULKAN_HPP_DEFAULT_DISPATCHER.init(renderData.instance);
    }

    void Renderer::CreateDevice() {
        uint32_t deviceCount{ 0 };
        RM_VK_CHECK(renderData.instance.enumeratePhysicalDevices(&deviceCount, nullptr));

        std::vector<vk::PhysicalDevice> devices(deviceCount);
        RM_VK_CHECK(renderData.instance.enumeratePhysicalDevices(&deviceCount, devices.data()));

        uint32_t selectedDevice{ 0 };
        RM_ASSERT(selectedDevice < devices.size(), "No valid GPU found");

        renderData.physicalDevice = devices[selectedDevice];
        renderData.physicalDevice.getProperties2(&renderData.deviceProperties);

        RM_LOG_INFO("Selected GPU: {}", renderData.deviceProperties.properties.deviceName.data());

        uint32_t queueCount{ 0 };
        renderData.physicalDevice.getQueueFamilyProperties(&queueCount, nullptr);

        std::vector<vk::QueueFamilyProperties> queueFamilies(queueCount);
        renderData.physicalDevice.getQueueFamilyProperties(&queueCount, queueFamilies.data());

        for (auto i = 0; i < queueFamilies.size(); i++) {
            if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics) {
                renderData.queueFamilyIndex = i;
                break;
            }
        }

        bool sdlPresentationInit = SDL_Vulkan_GetPresentationSupport(renderData.instance, renderData.physicalDevice, renderData.queueFamilyIndex);
        RM_ASSERT(sdlPresentationInit, "SDL Vulkan Presentation support missing");

        float qfPriorities{ 1.0f };
        vk::DeviceQueueCreateInfo deviceQueueCI({}, renderData.queueFamilyIndex, 1, &qfPriorities);

        vk::PhysicalDeviceVulkan12Features vk12Features;
        vk12Features.descriptorIndexing = true;
        vk12Features.shaderSampledImageArrayNonUniformIndexing = true;
        vk12Features.descriptorBindingVariableDescriptorCount = true;
        vk12Features.runtimeDescriptorArray = true;
        vk12Features.bufferDeviceAddress = true;

        vk::PhysicalDeviceVulkan13Features vk13Features;
        vk13Features.pNext = &vk12Features;
        vk13Features.synchronization2 = true;
        vk13Features.dynamicRendering = true;

        vk::PhysicalDeviceFeatures vk10Features;
        vk10Features.samplerAnisotropy = VK_TRUE;

        const std::vector<const char*> deviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
        vk::DeviceCreateInfo deviceCI({}, 1, &deviceQueueCI, {}, {}, 1, deviceExtensions.data(), &vk10Features, &vk13Features);

        RM_VK_CHECK(renderData.physicalDevice.createDevice(&deviceCI, nullptr, &renderData.device));
        VULKAN_HPP_DEFAULT_DISPATCHER.init(renderData.device);
        renderData.device.getQueue(renderData.queueFamilyIndex, 0, &renderData.deviceQueue);

        VmaVulkanFunctions vkFunctions{ 
            .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
            .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
            .vkCreateImage = vkCreateImage 
        };
        VmaAllocatorCreateInfo allocatorCI{
            .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
            .physicalDevice = renderData.physicalDevice,
            .device = renderData.device,
            .pVulkanFunctions = &vkFunctions,
            .instance = renderData.instance
        };

        RM_VK_CHECK(vmaCreateAllocator(&allocatorCI, &renderData.allocator));
    }

    void Renderer::CreateSwapchain() {
        VkSurfaceKHR sdlSurface{ VK_NULL_HANDLE };
        bool sdlVulkanSurface = SDL_Vulkan_CreateSurface(window.GetNative(), renderData.instance, nullptr, &sdlSurface);
        RM_ASSERT(sdlSurface != VK_NULL_HANDLE, "Failed to get Vulkan Surface from SDL");

        renderData.surface = sdlSurface;

        vk::SurfaceCapabilitiesKHR surfaceCapabilities{};
        RM_VK_CHECK(renderData.physicalDevice.getSurfaceCapabilitiesKHR(renderData.surface, &surfaceCapabilities));

        vk::Extent2D extent{ surfaceCapabilities.currentExtent };
        if (surfaceCapabilities.currentExtent.width == 0xFFFFFFFF) {
            extent = {
                .width = window.GetWidth(),
                .height = window.GetHeight(),
            };
        }

        vk::SwapchainCreateInfoKHR swapchainCI = vk::SwapchainCreateInfoKHR()
            .setSurface(renderData.surface)
            .setMinImageCount(surfaceCapabilities.minImageCount)
            .setImageFormat(renderData.swapchainImageFormat)
            .setImageColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear)
            .setImageExtent(extent)
            .setImageArrayLayers(1)
            .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
            .setPreTransform(vk::SurfaceTransformFlagBitsKHR::eIdentity)
            .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
            .setPresentMode(vk::PresentModeKHR::eFifo);

        RM_VK_CHECK(renderData.device.createSwapchainKHR(&swapchainCI, nullptr, &renderData.swapchain));

        uint32_t imageCount{ 0 };
        RM_VK_CHECK(renderData.device.getSwapchainImagesKHR(renderData.swapchain, &imageCount, nullptr));

        renderData.swapchainImages.resize(imageCount);
        renderData.swapchainImageViews.resize(imageCount);
        RM_VK_CHECK(renderData.device.getSwapchainImagesKHR(renderData.swapchain, &imageCount, renderData.swapchainImages.data()));

        for (auto i = 0; i < imageCount; i++) {
            vk::ImageViewCreateInfo viewCI(
                {},
                renderData.swapchainImages[i], 
                vk::ImageViewType::e2D, 
                renderData.swapchainImageFormat,
                {}, 
                vk::ImageSubresourceRange()
                    .setAspectMask(vk::ImageAspectFlagBits::eColor)
                    .setLevelCount(1)
                    .setLayerCount(1)
            );

            RM_VK_CHECK(renderData.device.createImageView(&viewCI, nullptr, &renderData.swapchainImageViews[i]));
        }

        // Depth Attachment
        renderData.swapchainDepthFormat = vk::Format::eUndefined;
        std::array<vk::Format, 2> depthFormats{ vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint };
        for (const vk::Format& format : depthFormats) {
            vk::FormatProperties2 fProperties;
            renderData.physicalDevice.getFormatProperties2(format, &fProperties);
            if (fProperties.formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
                renderData.swapchainDepthFormat = format;
                break;
            }
        }

        if (renderData.swapchainDepthFormat == vk::Format::eUndefined) throw std::runtime_error("Failed to get depth image format");

        vk::Extent3D diExtent(window.GetWidth(), window.GetHeight(), 1);
        vk::ImageCreateInfo depthImageCI = vk::ImageCreateInfo()
            .setImageType(vk::ImageType::e2D)
            .setFormat(renderData.swapchainDepthFormat)
            .setExtent(diExtent)
            .setMipLevels(1)
            .setArrayLayers(1)
            .setSamples(vk::SampleCountFlagBits::e1)
            .setTiling(vk::ImageTiling::eOptimal)
            .setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
            .setInitialLayout(vk::ImageLayout::eUndefined);

        VmaAllocationCreateInfo allocCI{
            .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO
        };

        VkImage image{};
        RM_VK_CHECK(vmaCreateImage(
                    renderData.allocator, 
                    reinterpret_cast<const VkImageCreateInfo*>(&depthImageCI), 
                    &allocCI, 
                    &image, 
                    &renderData.swapchainDepthImageAlloc, 
                    nullptr));

        renderData.swapchainDepthImage = image;

        vk::ImageViewCreateInfo depthViewCI = vk::ImageViewCreateInfo()
            .setImage(renderData.swapchainDepthImage)
            .setViewType(vk::ImageViewType::e2D)
            .setFormat(renderData.swapchainDepthFormat)
            .setSubresourceRange(vk::ImageSubresourceRange().setAspectMask(vk::ImageAspectFlagBits::eDepth).setLevelCount(1).setLayerCount(1));

        RM_VK_CHECK(renderData.device.createImageView(&depthViewCI, nullptr, &renderData.swapchainDepthImageView));
    }

    void Renderer::CreateBuffers() {
        // Command

        vk::CommandPoolCreateInfo cpCI(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, renderData.queueFamilyIndex);
        RM_VK_CHECK(renderData.device.createCommandPool(&cpCI, nullptr, &renderData.commandPool));

        renderData.commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        vk::CommandBufferAllocateInfo cbAI(renderData.commandPool, vk::CommandBufferLevel::ePrimary, MAX_FRAMES_IN_FLIGHT, nullptr);
        RM_VK_CHECK(renderData.device.allocateCommandBuffers(&cbAI, renderData.commandBuffers.data()));

        // Vertex

        VkBuffer vBuffer;
        vk::BufferCreateInfo vbuffCI = vk::BufferCreateInfo()
            .setSize(MAX_VERTICES * sizeof(Vertex))
            .setUsage(vk::BufferUsageFlagBits::eVertexBuffer)
            .setSharingMode(vk::SharingMode::eExclusive);

        VmaAllocationCreateInfo vAllocCI{
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO
        };

        RM_VK_CHECK(vmaCreateBuffer(
                    renderData.allocator,
                    reinterpret_cast<const VkBufferCreateInfo*>(&vbuffCI),
                    &vAllocCI,
                    &vBuffer,
                    &renderData.vBuffAlloc,
                    &renderData.vBuffAllocInfo
                    ));

        // Index

        std::vector<uint16_t> indices(MAX_INDICES);
        for (uint32_t i = 0, offset = 0; i < MAX_INDICES; i += 6, offset += 4) {
            indices[i + 0] = offset + 0;
            indices[i + 1] = offset + 1;
            indices[i + 2] = offset + 2;
            indices[i + 3] = offset + 2;
            indices[i + 4] = offset + 3;
            indices[i + 5] = offset + 0;
        }
        vk::DeviceSize ibuffSize = indices.size() * sizeof(uint16_t);

        VkBuffer iStagingBuffer{};
        VmaAllocation stagingAlloc{};

        vk::BufferCreateInfo stagingCI = vk::BufferCreateInfo()
            .setSize(ibuffSize)
            .setUsage(vk::BufferUsageFlagBits::eTransferSrc)
            .setSharingMode(vk::SharingMode::eExclusive);

        VmaAllocationInfo stagingAllocInfo{};
        VmaAllocationCreateInfo stagingAllocCI{
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO
        };

        RM_VK_CHECK(vmaCreateBuffer(
                    renderData.allocator,
                    reinterpret_cast<const VkBufferCreateInfo*>(&stagingCI),
                    &stagingAllocCI,
                    &iStagingBuffer,
                    &stagingAlloc,
                    &stagingAllocInfo
                    ));

        memcpy(stagingAllocInfo.pMappedData, indices.data(), ibuffSize);
        RM_VK_CHECK(vmaFlushAllocation(renderData.allocator, stagingAlloc, 0, ibuffSize));

        vk::BufferCreateInfo indexCI = vk::BufferCreateInfo()
            .setSize(ibuffSize)
            .setUsage(vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst)
            .setSharingMode(vk::SharingMode::eExclusive);

        VmaAllocationCreateInfo indexAllocCI{
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
        };

        VkBuffer indexBuffer{};
        RM_VK_CHECK(vmaCreateBuffer(
                    renderData.allocator,
                    reinterpret_cast<const VkBufferCreateInfo*>(&indexCI),
                    &indexAllocCI,
                    &indexBuffer,
                    &renderData.iBuffAlloc,
                    nullptr
                    ));
        renderData.iBuffer = indexBuffer;

        vk::CommandBufferAllocateInfo cbOneTimeAI = vk::CommandBufferAllocateInfo()
            .setCommandPool(renderData.commandPool)
            .setLevel(vk::CommandBufferLevel::ePrimary)
            .setCommandBufferCount(1);
        vk::CommandBuffer cbOneTime;

        RM_VK_CHECK(renderData.device.allocateCommandBuffers(&cbOneTimeAI, &cbOneTime));

        vk::CommandBufferBeginInfo cbBI(vk::CommandBufferUsageFlagBits::eOneTimeSubmit, nullptr);
        RM_VK_CHECK(cbOneTime.begin(&cbBI));

        vk::BufferCopy copyRegion(0, 0, ibuffSize);
        cbOneTime.copyBuffer(iStagingBuffer, indexBuffer, 1, &copyRegion);

        vk::BufferMemoryBarrier2 barrier = vk::BufferMemoryBarrier2()
            .setSrcStageMask(vk::PipelineStageFlagBits2::eTransfer)
            .setSrcAccessMask(vk::AccessFlagBits2::eTransferWrite)
            .setDstStageMask(vk::PipelineStageFlagBits2::eIndexInput)
            .setDstAccessMask(vk::AccessFlagBits2::eIndexRead)
            .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored)
            .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
            .setSize(vk::WholeSize)
            .setBuffer(indexBuffer);

        vk::DependencyInfo dpInfo = vk::DependencyInfo()
            .setBufferMemoryBarrierCount(1)
            .setPBufferMemoryBarriers(&barrier);

        cbOneTime.pipelineBarrier2(&dpInfo);
        cbOneTime.end();

        vk::SubmitInfo submitInfo = vk::SubmitInfo()
            .setCommandBufferCount(1)
            .setPCommandBuffers(&cbOneTime);

        RM_VK_CHECK(renderData.deviceQueue.submit(1, &submitInfo, VK_NULL_HANDLE));
        renderData.deviceQueue.waitIdle();
        renderData.device.freeCommandBuffers(renderData.commandPool, 1, &cbOneTime);

        vmaDestroyBuffer(renderData.allocator, iStagingBuffer, stagingAlloc);
    }
}
