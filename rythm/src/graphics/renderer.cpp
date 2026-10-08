#include "rmpch.h"
#include "renderer.h"
#include "color.h"
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
#include <slang/slang.h>
#include <slang/slang-com-ptr.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

    struct CameraUniform {
        glm::mat4 projection;
        glm::mat4 view;
    };

    struct BufferInfo {
        vk::Buffer buffer;
        vk::DeviceAddress bufferAddress;
        VmaAllocation allocation;
        VmaAllocationInfo allocationInfo;
    };

    struct RenderData {
        vk::Instance instance;
        uint32_t queueFamilyIndex{ 0 };
        bool swapchainUpdate{ false };
        uint32_t indexCount{ 0 };
        uint32_t vertexCount{ 0 };

        // Device
        vk::Device device;
        vk::Queue deviceQueue;
        vk::PhysicalDevice physicalDevice;
        vk::PhysicalDeviceProperties2 deviceProperties;
        VmaAllocator allocator;

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
        std::vector<vk::Semaphore> renderSemaphores;

        // Command Pool
        vk::CommandPool commandPool;
        std::vector<vk::CommandBuffer> commandBuffers;

        // Buffers
        BufferInfo indexBuffer;
        std::array<BufferInfo, MAX_FRAMES_IN_FLIGHT> vertexBuffers;
        std::array<Vertex, MAX_VERTICES> vertexBuffer;

        // Shader
        Slang::ComPtr<slang::IGlobalSession> slangGS;
        vk::ShaderModule shader;

        // Pipeline
        vk::Pipeline pipeline;
        vk::PipelineLayout pipelineLayout;

        // Camera
        CameraUniform cameraUniform;
        std::array<BufferInfo, MAX_FRAMES_IN_FLIGHT> cameraBuffers;

        // Sync
        std::array<vk::Fence, MAX_FRAMES_IN_FLIGHT> fences;
        std::array<vk::Semaphore, MAX_FRAMES_IN_FLIGHT> acquireSemaphores;
    } renderData;

    Renderer::Renderer(Window& window): window(window) {
        CreateInstance();
        CreateDevice();
        CreateSwapchain();
        CreateBuffers();
        CreateShaders();
        CreatePipeline();
        CreateCameraBuffer();
        CreateSyncObjects();

        Logger::Info("Created Renderer");
    }

    Renderer::~Renderer() {
        renderData.device.waitIdle();

        for (auto i = 0; i < renderData.renderSemaphores.size(); i++) {
            renderData.device.destroySemaphore(renderData.renderSemaphores[i], nullptr);
        }

        for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            renderData.device.destroyFence(renderData.fences[i], nullptr);
            renderData.device.destroySemaphore(renderData.acquireSemaphores[i], nullptr);
            vmaDestroyBuffer(renderData.allocator, static_cast<VkBuffer>(renderData.cameraBuffers[i].buffer), renderData.cameraBuffers[i].allocation);
            vmaDestroyBuffer(renderData.allocator, renderData.vertexBuffers[i].buffer, renderData.vertexBuffers[i].allocation);
        }

        renderData.device.destroyPipelineLayout(renderData.pipelineLayout);
        renderData.device.destroyPipeline(renderData.pipeline);
        renderData.device.destroyShaderModule(renderData.shader, nullptr);

        vmaDestroyBuffer(renderData.allocator, renderData.indexBuffer.buffer, renderData.indexBuffer.allocation);

        renderData.device.destroyCommandPool(renderData.commandPool, nullptr);

        renderData.device.destroyImageView(renderData.swapchainDepthImageView, nullptr);
        vmaDestroyImage(renderData.allocator, renderData.swapchainDepthImage, renderData.swapchainDepthImageAlloc);
        for (auto& imageView : renderData.swapchainImageViews) {
            renderData.device.destroyImageView(imageView, nullptr);
        }

        renderData.device.destroySwapchainKHR(renderData.swapchain, nullptr);
        renderData.instance.destroySurfaceKHR(renderData.surface, nullptr);
        vmaDestroyAllocator(renderData.allocator);
        renderData.device.destroy();
        renderData.instance.destroy();

        Logger::Info("Destroyed Renderer");
    }

    void Renderer::BeginFrame() {
        renderData.cameraUniform.projection = glm::perspective(glm::radians(45.0f), (float)window.GetWidth() / (float)window.GetHeight(), 0.1f, 32.0f);
        renderData.cameraUniform.view = glm::translate(glm::mat4(1.0f), { 0.0, 0.0, -6.0f });

        RM_VK_CHECK(renderData.device.waitForFences(1, &renderData.fences[currentFrame], true, UINT64_MAX));
        RM_VK_CHECK(renderData.device.resetFences(1, &renderData.fences[currentFrame]));
        RM_VK_CHECK(renderData.device.acquireNextImageKHR(renderData.swapchain, UINT64_MAX, renderData.acquireSemaphores[currentFrame], VK_NULL_HANDLE, &currentImage));

        memcpy(renderData.cameraBuffers[currentFrame].allocationInfo.pMappedData, &renderData.cameraUniform, sizeof(CameraUniform));
    }

    void Renderer::EndFrame() {
        FlushFrame();
    }

    void Renderer::DrawQuad(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, rm::Color color) {
        if (renderData.indexCount + 6 > MAX_INDICES) {
            Logger::Error("Quad render buffer is full!");
            return;
        }

        glm::mat4 transform(1.0f);

        transform = glm::translate(transform, position);
        transform = glm::rotate(transform, glm::radians(rotation.x), { 1, 0, 0 });
        transform = glm::rotate(transform, glm::radians(rotation.y), { 0, 1, 0 });
        transform = glm::rotate(transform, glm::radians(rotation.z), { 0, 0, 1 });
        transform = glm::scale(transform, scale);

        constexpr glm::vec4 quadPositions[4] = {
            { -0.5f, -0.5f, 0.0f, 1.0f },
            {  0.5f, -0.5f, 0.0f, 1.0f },
            {  0.5f,  0.5f, 0.0f, 1.0f },
            { -0.5f,  0.5f, 0.0f, 1.0f }
        };

        constexpr glm::vec2 texCoords[4] = {
            { 0.0f, 0.0f },
            { 1.0f, 0.0f },
            { 1.0f, 1.0f },
            { 0.0f, 1.0f }
        };

        for (uint32_t i = 0; i < 4; i++) {
            glm::vec4 transformed = transform * quadPositions[i];

            renderData.vertexBuffer[renderData.vertexCount + i] = {
                glm::vec3(transformed),
                { color.r, color.g, color.b, color.a },
                texCoords[i],
                0.0f
            };
        }

        renderData.vertexCount += 4;
        renderData.indexCount += 6;
    }

    void Renderer::FlushFrame() {
        vk::DeviceSize vertexSize(renderData.vertexCount * sizeof(Vertex));
        memcpy(renderData.vertexBuffers[currentFrame].allocationInfo.pMappedData, renderData.vertexBuffer.data(), vertexSize);
        RM_VK_CHECK(vmaFlushAllocation(renderData.allocator, renderData.vertexBuffers[currentFrame].allocation, 0, vertexSize));

        vk::CommandBuffer cb = renderData.commandBuffers[currentFrame];
        cb.reset();

        vk::CommandBufferBeginInfo cbBI(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        RM_VK_CHECK(cb.begin(&cbBI));

        std::array<vk::ImageMemoryBarrier2, 2> outputBarriers{
            vk::ImageMemoryBarrier2()
                .setSrcStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput)
                .setSrcAccessMask(vk::AccessFlagBits2::eNone)
                .setDstStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput)
                .setDstAccessMask(vk::AccessFlagBits2::eColorAttachmentRead | vk::AccessFlagBits2::eColorAttachmentWrite)
                .setOldLayout(vk::ImageLayout::eUndefined)
                .setNewLayout(vk::ImageLayout::eAttachmentOptimal)
                .setImage(renderData.swapchainImages[currentImage])
                .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, {}, 1, {}, 1)),
            vk::ImageMemoryBarrier2()
                .setSrcStageMask(vk::PipelineStageFlagBits2::eLateFragmentTests)
                .setSrcAccessMask(vk::AccessFlagBits2::eDepthStencilAttachmentWrite)
                .setDstStageMask(vk::PipelineStageFlagBits2::eEarlyFragmentTests)
                .setDstAccessMask(vk::AccessFlagBits2::eDepthStencilAttachmentWrite)
                .setOldLayout(vk::ImageLayout::eUndefined)
                .setNewLayout(vk::ImageLayout::eAttachmentOptimal)
                .setImage(renderData.swapchainDepthImage)
                .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, {}, 1, {}, 1))
        };
        vk::DependencyInfo barrierDependencyInfo({}, {}, {}, {}, {}, 2, outputBarriers.data());
        cb.pipelineBarrier2(&barrierDependencyInfo);

        vk::RenderingAttachmentInfo colorAttachmentInfo = vk::RenderingAttachmentInfo()
            .setImageView(renderData.swapchainImageViews[currentImage])
            .setImageLayout(vk::ImageLayout::eAttachmentOptimal)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setClearValue(vk::ClearValue(vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f)));
        vk::RenderingAttachmentInfo depthAttachmentInfo = vk::RenderingAttachmentInfo()
            .setImageView(renderData.swapchainDepthImageView)
            .setImageLayout(vk::ImageLayout::eAttachmentOptimal)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setClearValue(vk::ClearValue(vk::ClearDepthStencilValue(1.0f, 0)));

        vk::Extent2D extent(window.GetWidth(), window.GetHeight());
        vk::RenderingInfo renderingInfo = vk::RenderingInfo()
            .setRenderArea(vk::Rect2D({}, extent))
            .setLayerCount(1)
            .setColorAttachmentCount(1)
            .setPColorAttachments(&colorAttachmentInfo)
            .setPDepthAttachment(&depthAttachmentInfo);

        cb.beginRendering(&renderingInfo);
        cb.bindPipeline(vk::PipelineBindPoint::eGraphics, renderData.pipeline);

        vk::Viewport viewport({}, {}, (float)window.GetWidth(), (float)window.GetHeight(), 0.0f, 1.0f);
        vk::Rect2D scissor({}, extent);

        cb.setViewport(0, 1, &viewport);
        cb.setScissor(0, 1, &scissor);

        vk::DeviceSize vOffset{ 0 };
        vk::DeviceSize iOffset{ 0 };
        cb.bindVertexBuffers(0, 1, &renderData.vertexBuffers[currentFrame].buffer, &vOffset);
        cb.bindIndexBuffer(renderData.indexBuffer.buffer, iOffset, vk::IndexType::eUint16);
        cb.pushConstants(renderData.pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(vk::DeviceAddress), &renderData.cameraBuffers[currentFrame].bufferAddress);
        cb.drawIndexed(renderData.indexCount, 1, 0, 0, 0);
        cb.endRendering();

        vk::ImageMemoryBarrier2 barrierPresent = vk::ImageMemoryBarrier2()
            .setSrcStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput)
            .setSrcAccessMask(vk::AccessFlagBits2::eColorAttachmentWrite)
            .setDstStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput)
            .setDstAccessMask(vk::AccessFlagBits2::eNone)
            .setOldLayout(vk::ImageLayout::eAttachmentOptimal)
            .setNewLayout(vk::ImageLayout::ePresentSrcKHR)
            .setImage(renderData.swapchainImages[currentImage])
            .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, {}, 1, {}, 1));
        vk::DependencyInfo barrierPresentDepdencyInfo({}, {}, {}, {}, {}, 1, &barrierPresent);

        renderData.vertexCount = 0;
        renderData.indexCount = 0;

        cb.pipelineBarrier2(&barrierPresentDepdencyInfo);
        cb.end();

        vk::SemaphoreSubmitInfo waitSemaphoreSI(renderData.acquireSemaphores[currentFrame], {}, vk::PipelineStageFlagBits2::eColorAttachmentOutput);
        vk::CommandBufferSubmitInfo commandBufferSI(cb);
        vk::SemaphoreSubmitInfo signalSemaphoreSI(renderData.renderSemaphores[currentImage], {}, vk::PipelineStageFlagBits2::eColorAttachmentOutput);
        vk::SubmitInfo2 submitInfo = vk::SubmitInfo2()
            .setWaitSemaphoreInfoCount(1)
            .setPWaitSemaphoreInfos(&waitSemaphoreSI)
            .setCommandBufferInfoCount(1)
            .setPCommandBufferInfos(&commandBufferSI)
            .setSignalSemaphoreInfoCount(1)
            .setPSignalSemaphoreInfos(&signalSemaphoreSI);

        RM_VK_CHECK(renderData.deviceQueue.submit2(1, &submitInfo, renderData.fences[currentFrame]));
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

        vk::PresentInfoKHR presentInfo(1, &renderData.renderSemaphores[currentImage], 1, &renderData.swapchain, &currentImage);
        vk::Result swapchainResult = renderData.deviceQueue.presentKHR(presentInfo);

        if (swapchainResult == vk::Result::eErrorOutOfDateKHR) {
            renderData.swapchainUpdate = true;
        } else {
            RM_VK_CHECK((swapchainResult));
        }
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

        vk::BufferCreateInfo vbuffCI = vk::BufferCreateInfo()
            .setSize(MAX_VERTICES * sizeof(Vertex))
            .setUsage(vk::BufferUsageFlagBits::eVertexBuffer)
            .setSharingMode(vk::SharingMode::eExclusive);

        VmaAllocationCreateInfo vAllocCI{
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO
        };

        for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VkBuffer vBuffer;
            RM_VK_CHECK(vmaCreateBuffer(
                    renderData.allocator,
                    reinterpret_cast<const VkBufferCreateInfo*>(&vbuffCI),
                    &vAllocCI,
                    &vBuffer,
                    &renderData.vertexBuffers[i].allocation,
                    &renderData.vertexBuffers[i].allocationInfo
                    ));

            renderData.vertexBuffers[i].buffer = vBuffer;
        }

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
                    &renderData.indexBuffer.allocation,
                    nullptr
                    ));
        renderData.indexBuffer.buffer = indexBuffer;

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

    void Renderer::CreateShaders() {
        slang::createGlobalSession(renderData.slangGS.writeRef());

        auto slangTargets{
            std::to_array<slang::TargetDesc>({ { .format = SLANG_SPIRV, .profile = renderData.slangGS->findProfile("spirv_1_4") } })
        };
        auto slangOptions{
            std::to_array<slang::CompilerOptionEntry>({ { slang::CompilerOptionName::EmitSpirvDirectly, { slang::CompilerOptionValueKind::Int, 1 } } })
        };

        slang::SessionDesc slangSDesc{
            .targets = slangTargets.data(),
            .targetCount = SlangInt(slangTargets.size()),
            .defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR,
            .compilerOptionEntries = slangOptions.data(),
            .compilerOptionEntryCount = uint32_t(slangOptions.size())
        };

        Slang::ComPtr<slang::ISession> slangSession;
        renderData.slangGS->createSession(slangSDesc, slangSession.writeRef());

        Slang::ComPtr<slang::IModule> slangModule{ slangSession->loadModuleFromSource("triangle", "assets/shader.slang", nullptr, nullptr) };
        Slang::ComPtr<ISlangBlob> spirv;
        slangModule->getTargetCode(0, spirv.writeRef());

        vk::ShaderModuleCreateInfo shaderModuleCI = vk::ShaderModuleCreateInfo()
            .setCodeSize(spirv->getBufferSize())
            .setPCode((uint32_t*)spirv->getBufferPointer());

        RM_VK_CHECK(renderData.device.createShaderModule(&shaderModuleCI, nullptr, &renderData.shader));
    }

    void Renderer::CreatePipeline() {
        vk::PushConstantRange constantRange(vk::ShaderStageFlagBits::eVertex, 0, sizeof(vk::DeviceAddress));
        vk::PipelineLayoutCreateInfo pipelineLayoutCI({}, {}, {}, 1, &constantRange);
        RM_VK_CHECK(renderData.device.createPipelineLayout(&pipelineLayoutCI, nullptr, &renderData.pipelineLayout));

        std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages{
            vk::PipelineShaderStageCreateInfo().setStage(vk::ShaderStageFlagBits::eVertex).setModule(renderData.shader).setPName("main"),
            vk::PipelineShaderStageCreateInfo().setStage(vk::ShaderStageFlagBits::eFragment).setModule(renderData.shader).setPName("main"),
        };

        vk::VertexInputBindingDescription vertexBinding(0, sizeof(Vertex), vk::VertexInputRate::eVertex);
        std::array<vk::VertexInputAttributeDescription, 4> vertexAttributes{
            vk::VertexInputAttributeDescription().setLocation(0).setBinding(0).setFormat(vk::Format::eR32G32B32Sfloat),
            vk::VertexInputAttributeDescription().setLocation(1).setBinding(0).setFormat(vk::Format::eR32G32B32A32Sfloat).setOffset(offsetof(Vertex, color)),
            vk::VertexInputAttributeDescription().setLocation(2).setBinding(0).setFormat(vk::Format::eR32G32Sfloat).setOffset(offsetof(Vertex, uv)),
            vk::VertexInputAttributeDescription().setLocation(3).setBinding(0).setFormat(vk::Format::eR32Sfloat).setOffset(offsetof(Vertex, texture))
        };

        vk::PipelineVertexInputStateCreateInfo vertexInputState = vk::PipelineVertexInputStateCreateInfo()
            .setVertexBindingDescriptionCount(1)
            .setPVertexBindingDescriptions(&vertexBinding)
            .setVertexAttributeDescriptionCount(static_cast<uint32_t>(vertexAttributes.size()))
            .setPVertexAttributeDescriptions(vertexAttributes.data());

        vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCI({}, vk::PrimitiveTopology::eTriangleList);
        std::array<vk::DynamicState, 2> dynamicStates{ vk::DynamicState::eViewport, vk::DynamicState::eScissor };
        vk::PipelineDynamicStateCreateInfo dynamicStateCI({}, 2, dynamicStates.data());
        vk::PipelineViewportStateCreateInfo viewportStateCI({}, 1, nullptr, 1, nullptr);
        vk::PipelineRasterizationStateCreateInfo rastStateCI = vk::PipelineRasterizationStateCreateInfo().setLineWidth(1.0f);
        vk::PipelineMultisampleStateCreateInfo multisampleStateCI({}, vk::SampleCountFlagBits::e1);
        vk::PipelineDepthStencilStateCreateInfo depthStencilStateCI({}, vk::True, vk::True, vk::CompareOp::eLessOrEqual);
        vk::PipelineColorBlendAttachmentState blendAttachment({ .colorWriteMask = 0xF });
        vk::PipelineColorBlendStateCreateInfo colorBlendStateCI({ .attachmentCount = 1, .pAttachments = (VkPipelineColorBlendAttachmentState*)&blendAttachment });
        vk::PipelineRenderingCreateInfo renderingCI = vk::PipelineRenderingCreateInfo()
            .setColorAttachmentCount(1)
            .setPColorAttachmentFormats(&renderData.swapchainImageFormat)
            .setDepthAttachmentFormat(renderData.swapchainDepthFormat);

        vk::GraphicsPipelineCreateInfo pipelineCI = vk::GraphicsPipelineCreateInfo()
            .setPNext(&renderingCI)
            .setStageCount(2)
            .setPStages(shaderStages.data())
            .setPVertexInputState(&vertexInputState)
            .setPInputAssemblyState(&inputAssemblyStateCI)
            .setPViewportState(&viewportStateCI)
            .setPRasterizationState(&rastStateCI)
            .setPMultisampleState(&multisampleStateCI)
            .setPDepthStencilState(&depthStencilStateCI)
            .setPColorBlendState(&colorBlendStateCI)
            .setPDynamicState(&dynamicStateCI)
            .setLayout(renderData.pipelineLayout);

        RM_VK_CHECK(renderData.device.createGraphicsPipelines(VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &renderData.pipeline));
    }

    void Renderer::CreateCameraBuffer() {
        for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vk::BufferCreateInfo bufferCI = vk::BufferCreateInfo()
                .setSize(sizeof(CameraUniform))
                .setUsage(vk::BufferUsageFlagBits::eShaderDeviceAddress);
            VmaAllocationCreateInfo bufferAllocCI{
                .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
                .usage = VMA_MEMORY_USAGE_AUTO
            };

            RM_VK_CHECK(vmaCreateBuffer(
                        renderData.allocator,
                        reinterpret_cast<const VkBufferCreateInfo*>(&bufferCI),
                        &bufferAllocCI,
                        reinterpret_cast<VkBuffer*>(&renderData.cameraBuffers[i].buffer),
                        &renderData.cameraBuffers[i].allocation,
                        &renderData.cameraBuffers[i].allocationInfo
                        ));

            renderData.cameraBuffers[i].bufferAddress = renderData.device.getBufferAddress({ renderData.cameraBuffers[i].buffer });
        }
    }

    void Renderer::CreateSyncObjects() {
        vk::SemaphoreCreateInfo semaphoreCI;
        vk::FenceCreateInfo fenceCI(vk::FenceCreateFlagBits::eSignaled);

        renderData.renderSemaphores.resize(renderData.swapchainImages.size());
        for (auto& semaphore : renderData.renderSemaphores) {
            RM_VK_CHECK(renderData.device.createSemaphore(&semaphoreCI, nullptr, &semaphore));
        }

        for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            RM_VK_CHECK(renderData.device.createFence(&fenceCI, nullptr, &renderData.fences[i]));
            RM_VK_CHECK(renderData.device.createSemaphore(&semaphoreCI, nullptr, &renderData.acquireSemaphores[i]));
        }
    }
}
