#include "vulkan/vulkan_core.h"
#define VMA_IMPLEMENTATION

#include "rmpch.h"
#include "vk_render_device.h"
#include "vk_render_context.h"
#include "vk_helpers.h"

#include <vector>
#include <SDL3/SDL_vulkan.h>
#include <volk.h>

namespace rm {
    VKRenderDevice::VKRenderDevice(VKRenderContext& context, const RenderDeviceSpec& spec): context(context), spec(spec) {
        uint32_t deviceCount{ 0 };
        VKHelpers::Check(vkEnumeratePhysicalDevices(context.GetInstance(), &deviceCount, nullptr));

        std::vector<VkPhysicalDevice> devices(deviceCount);
        VKHelpers::Check(vkEnumeratePhysicalDevices(context.GetInstance(), &deviceCount, devices.data()));

        uint32_t deviceIndex{ 0 };
        if (spec.deviceIndex > 1) {
            deviceIndex = spec.deviceIndex;
            VKHelpers::Check(deviceIndex > deviceCount);
        }

        physicalDevice = devices[deviceIndex];

        vkGetPhysicalDeviceProperties2(physicalDevice, &physicalDeviceProperties);
        RM_LOG_INFO("Selected Device: {}", physicalDeviceProperties.properties.deviceName);

        // Queue
        uint32_t queueFamilyCount{ 0 };
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

        uint32_t queueFamily{ 0 };
        for (size_t i = 0; i < queueFamilies.size(); i++) {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                queueFamily = i;
                break;
            }
        }
        VKHelpers::Check(SDL_Vulkan_GetPresentationSupport(context.GetInstance(), physicalDevice, queueFamily));

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

        VKHelpers::Check(vkCreateDevice(physicalDevice, &deviceCI, nullptr, &device));
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

        VKHelpers::Check(vmaCreateAllocator(&allocatorCI, &allocator));

        Logger::Info("Created VKDevice");
    }

    VKRenderDevice::~VKRenderDevice() {
        vkDestroyDevice(device, nullptr);
    }

    Scope<Swapchain> VKRenderDevice::CreateSwapchain() {
        VkSurfaceKHR surface{ VK_NULL_HANDLE };
        VKHelpers::Check(SDL_Vulkan_CreateSurface(&spec.window.GetNative(), context.GetInstance(), nullptr, &surface));

        SwapchainSpec swapchainSpec{ .width = spec.window.GetWidth(), .height = spec.window.GetHeight() };
        return CreateScope<VKSwapchain>(*this, surface, swapchainSpec);
    }
}
