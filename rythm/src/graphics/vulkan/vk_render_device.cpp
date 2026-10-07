#include "rmpch.h"
#include "vk_render_device.h"
#include "vk_render_context.h"
#include "vk_image.h"
#include "vk_render_buffer.h"
#include "vk_swapchain.h"
#include "vk_render_sync.h"
#include "vk_command_buffer.h"
#include "vk_shader.h"
#include "vk_resource_layout.h"
#include "vk_resource_set.h"
#include "vk_graphics_pipeline.h"
#include "vk_helpers.h"

#include "graphics/shader_compiler.h"

#include <vector>
#include <SDL3/SDL_vulkan.h>
#include <volk.h>

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

namespace rm::vk {
    VKRenderDevice::VKRenderDevice(VKRenderContext& context, const RenderDeviceSpec& spec): context(context), spec(spec) {
        uint32_t deviceCount{ 0 };
        VK_CHECK(vkEnumeratePhysicalDevices(context.GetInstance(), &deviceCount, nullptr));

        std::vector<VkPhysicalDevice> devices(deviceCount);
        VK_CHECK(vkEnumeratePhysicalDevices(context.GetInstance(), &deviceCount, devices.data()));

        uint32_t deviceIndex{ 0 };
        if (spec.deviceIndex > 1) {
            deviceIndex = spec.deviceIndex;
            VK_CHECK(deviceIndex > deviceCount);
        }

        physicalDevice = devices[deviceIndex];

        vkGetPhysicalDeviceProperties2(physicalDevice, &physicalDeviceProperties);
        RM_LOG_INFO("Selected GPU: {}", physicalDeviceProperties.properties.deviceName);

        // Queue
        uint32_t queueFamilyCount{ 0 };
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

        for (size_t i = 0; i < queueFamilies.size(); i++) {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                queueFamily = i;
                break;
            }
        }
        VK_CHECK(SDL_Vulkan_GetPresentationSupport(context.GetInstance(), physicalDevice, queueFamily));

        // Logical
        const float qfPriorities{ 1.0f };
        VkDeviceQueueCreateInfo queueCI{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = queueFamily,
            .queueCount = 1,
            .pQueuePriorities = &qfPriorities
        };

        VkPhysicalDeviceVulkan12Features enabledVk12Features{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .descriptorIndexing = true,
            .shaderSampledImageArrayNonUniformIndexing = true,
            .descriptorBindingVariableDescriptorCount = true,
            .runtimeDescriptorArray = true,
            .bufferDeviceAddress = true
        };

        VkPhysicalDeviceVulkan13Features enabledVk13Features{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .pNext = &enabledVk12Features,
            .synchronization2 = true,
            .dynamicRendering = true
        };

        VkPhysicalDeviceFeatures enabledVk10Features{
            .samplerAnisotropy = VK_TRUE
        };

        const std::vector<const char*> deviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
        VkDeviceCreateInfo deviceCI{
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = &enabledVk13Features,
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &queueCI,
            .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
            .ppEnabledExtensionNames = deviceExtensions.data(),
            .pEnabledFeatures = &enabledVk10Features
        };

        VK_CHECK(vkCreateDevice(physicalDevice, &deviceCI, nullptr, &device));
        vkGetDeviceQueue(device, queueFamily, 0, &queue);

        // VMA
        VmaVulkanFunctions vkFunctions{ 
            .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
            .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
            .vkCreateImage = vkCreateImage 
        };
        VmaAllocatorCreateInfo allocatorCI{
            .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
            .physicalDevice = physicalDevice,
            .device = device,
            .pVulkanFunctions = &vkFunctions,
            .instance = context.GetInstance()
        };

        VK_CHECK(vmaCreateAllocator(&allocatorCI, &allocator));
    }

    VKRenderDevice::~VKRenderDevice() {
        vkDestroyDevice(device, nullptr);
    }

    Scope<Swapchain> VKRenderDevice::CreateSwapchain() {
        VkSurfaceKHR surface{ VK_NULL_HANDLE };
        VK_CHECK(SDL_Vulkan_CreateSurface(&spec.window.GetNative(), context.GetInstance(), nullptr, &surface));

        SwapchainSpec swapchainSpec{ .width = spec.window.GetWidth(), .height = spec.window.GetHeight() };
        return CreateScope<VKSwapchain>(*this, context, surface, swapchainSpec);
    }

    Scope<rm::Image> VKRenderDevice::CreateImage(const ImageSpec& spec) {
        return CreateScope<VKImage>(*this, spec);
    }

    Scope<RenderBuffer> VKRenderDevice::CreateBuffer(const RenderBufferSpec& spec) {
        return CreateScope<VKRenderBuffer>(*this, spec);
    }

    Scope<RenderSync> VKRenderDevice::CreateSync() {
        return CreateScope<VKRenderSync>(*this);
    }

    Scope<CommandBuffer> VKRenderDevice::CreateCommandBuffer(const CommandBufferSpec &spec) {
        return CreateScope<VKCommandBuffer>(*this, spec);
    }

    Scope<Shader> VKRenderDevice::CreateShader(const ShaderSpec &spec) {
        ShaderBytecode bytecode = this->spec.shaderCompiler.Compile(spec.path);
        return CreateScope<VKShader>(*this, bytecode, spec);
    }

    Scope<ResourceLayout> VKRenderDevice::CreateResourceLayout(const ResourceLayoutSpec &spec) {
        return CreateScope<VKResourceLayout>(*this, spec);
    }

    Scope<ResourceSet> VKRenderDevice::CreateResourceSet() {
        return CreateScope<VKResourceSet>(*this);
    }

    Scope<GraphicsPipeline> VKRenderDevice::CreateGraphicsPipeline(const GraphicsPipelineSpec &spec) {
        return CreateScope<VKGraphicsPipeline>(*this, spec);
    }
}
