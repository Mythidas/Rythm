#include "rmpch.h"
#include "vk_render_context.h"
#include "vk_helpers.h"
#include "vk_render_device.h"

#include <volk.h>
#include <SDL3/SDL_vulkan.h>

namespace rm {
    VKRenderContext::VKRenderContext() {
        volkInitialize();
        VkApplicationInfo appInfo{
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "Rythm",
            .apiVersion = VK_API_VERSION_1_3
        };

        uint32_t instanceExtensionCount{ 0 };
        char const* const* instanceExtensions{ SDL_Vulkan_GetInstanceExtensions(&instanceExtensionCount) };

        VkInstanceCreateInfo instanceCI{
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &appInfo,
            .enabledExtensionCount = instanceExtensionCount,
            .ppEnabledExtensionNames = instanceExtensions,
        };
        VKHelpers::Check(vkCreateInstance(&instanceCI, nullptr, &instance));
        volkLoadInstance(instance);


        Logger::Info("Created VKRenderContext");
    }

    VKRenderContext::~VKRenderContext() {
        vkDestroyInstance(instance, nullptr);

        Logger::Info("Destroyed VKRenderContext");
    }

    Scope<RenderDevice> VKRenderContext::CreateDevice(const RenderDeviceSpec &spec) {
        return CreateScope<VKRenderDevice>(*this, spec);
    }
}
