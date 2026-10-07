#include "rmpch.h"
#include "vk_render_sync.h"
#include "vk_render_device.h"
#include "vk_helpers.h"

#include <volk.h>

namespace rm::vk {
    VKRenderSync::VKRenderSync(VKRenderDevice& device): device(device) {
        VkSemaphoreCreateInfo semaphoreCI{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        VkFenceCreateInfo fenceCI{ .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT };

        VK_CHECK(vkCreateFence(device.GetDevice(), &fenceCI, nullptr, &fence));
        VK_CHECK(vkCreateSemaphore(device.GetDevice(), &semaphoreCI, nullptr, &semaphore));
    }

    VKRenderSync::~VKRenderSync() {
        vkDestroySemaphore(device.GetDevice(), semaphore, nullptr);
        vkDestroyFence(device.GetDevice(), fence, nullptr);
    }

    void VKRenderSync::Reset() const {
        VK_CHECK(vkResetFences(device.GetDevice(), 1, &fence));
    }

    void VKRenderSync::Wait() const {
        VK_CHECK(vkWaitForFences(device.GetDevice(), 1, &fence, true, UINT64_MAX));
    }
}
