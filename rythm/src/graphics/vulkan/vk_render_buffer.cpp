#include "rmpch.h"
#include "vk_render_buffer.h"
#include "vk_render_device.h"
#include "vk_helpers.h"

#include <cstring>
#include <volk.h>
#include <vk_mem_alloc.h>

namespace rm::vk {
    VKRenderBuffer::VKRenderBuffer(VKRenderDevice& device, const RenderBufferSpec& spec): device(device) {
        VkBufferCreateInfo bufferCI{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = spec.size,
            .usage = VKHelpers::ToVkBufferUsage(spec.usage)
        };
        VmaAllocationCreateInfo bufferAllocCI{
            .flags = VKHelpers::ToVmaAllocation(spec.allocation),
            .usage = VMA_MEMORY_USAGE_AUTO
        };

        VK_CHECK(vmaCreateBuffer(device.GetAllocator(), &bufferCI, &bufferAllocCI, &buffer, &allocation, &allocationInfo));

        VkBufferDeviceAddressInfo bdaInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
            .buffer = buffer
        };
        address = vkGetBufferDeviceAddress(device.GetDevice(), &bdaInfo);
    }

    VKRenderBuffer::~VKRenderBuffer() {
        vmaDestroyBuffer(device.GetAllocator(), buffer, allocation);
    }

    void VKRenderBuffer::SetData(void *data, unsigned long long size) {
        memcpy(allocationInfo.pMappedData, data, size);
    }
}
