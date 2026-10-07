#include "graphics/render_types.h"
#include "rmpch.h"
#include "vk_helpers.h"
#include "core/bits.h"

#include <vulkan/vk_enum_string_helper.h>
#include <string>
#include <cstdlib>

namespace rm::vk {
    void VKHelpers::Check(VkResult result, const char* expression, std::source_location location) {
        if (result == VK_SUCCESS) return;

        RM_LOG_ERROR(
            "Vulkan call failed!\n"
            "  Expression: {}\n"
            "  Result: {} ({})\n"
            "  File: {}:{}\n"
            "  Function: {}",
            expression,
            std::string(string_VkResult(result)),
            static_cast<int>(result),
            location.file_name(),
            location.line(),
            location.function_name()
        );

        #ifndef NDEBUG
                __builtin_trap();
        #else
                std::abort();
        #endif
    }

    void VKHelpers::Check(bool result, const char* expression, std::source_location location) {
        if (result) return;

        RM_LOG_ERROR(
            "Call failed!\n"
            "  Expression: {}\n"
            "  File: {}:{}\n"
            "  Function: {}",
            expression,
            location.file_name(),
            location.line(),
            location.function_name()
        );

        #ifndef NDEBUG
                __builtin_trap();
        #else
                std::abort();
        #endif
    }

    VkFormat VKHelpers::ToVkFormat(rm::RenderFormat format) {
        switch (format) {
            case rm::RenderFormat::R8_UNORM:
                return VK_FORMAT_R8_UNORM;
            case rm::RenderFormat::RG8_UNORM:
                return VK_FORMAT_R8G8_UNORM;
            case rm::RenderFormat::RGBA8_UNORM:
                return VK_FORMAT_R8G8B8A8_UNORM;
            case rm::RenderFormat::RGBA8_SRGB:
                return VK_FORMAT_R8G8B8A8_SRGB;
            case rm::RenderFormat::BGRA8_UNORM:
                return VK_FORMAT_B8G8R8A8_UNORM;
            case rm::RenderFormat::BGRA8_SRGB:
                return VK_FORMAT_B8G8R8A8_SRGB;
            case rm::RenderFormat::RGBA16_FLOAT:
                return VK_FORMAT_R16G16B16A16_SFLOAT;
            case rm::RenderFormat::RGBA32_FLOAT:
                return VK_FORMAT_R32G32B32A32_SFLOAT;
            case rm::RenderFormat::D32_FLOAT:
                return VK_FORMAT_D32_SFLOAT;
            case rm::RenderFormat::D24_UNORM_S8_UINT:
                return VK_FORMAT_D24_UNORM_S8_UINT;
            case rm::RenderFormat::D32_FLOAT_S8_UINT:
                return VK_FORMAT_D32_SFLOAT_S8_UINT;
            case rm::RenderFormat::RG32_FLOAT:
                return VK_FORMAT_R32G32_SFLOAT;
            case rm::RenderFormat::RGB32_FLOAT:
                return VK_FORMAT_R32G32B32_SFLOAT;
            case rm::RenderFormat::R32_FLOAT:
                return VK_FORMAT_R32_SFLOAT;
        }

        RM_ASSERT(false, "Unknown RenderFormat");
        return VK_FORMAT_UNDEFINED;
    }

    VkImageType VKHelpers::ToVkImageType(rm::ImageType type) {
        switch (type) {
            case rm::ImageType::E3D:
                return VK_IMAGE_TYPE_3D;
            case rm::ImageType::E2D:
                return VK_IMAGE_TYPE_2D;
        }

        RM_ASSERT(false, "Unknown ImageType");
        return VK_IMAGE_TYPE_2D;
    }

    VkImageViewType VKHelpers::ToVkImageViewType(rm::ImageType type) {
        switch (type) {
            case rm::ImageType::E3D:
                return VK_IMAGE_VIEW_TYPE_3D;
            case rm::ImageType::E2D:
                return VK_IMAGE_VIEW_TYPE_2D;

        }
        
        RM_ASSERT(false, "Unknown ImageType");
        return VK_IMAGE_VIEW_TYPE_2D;
    }

    VkSampleCountFlagBits VKHelpers::ToVkSampleCount(rm::ImageSamples samples) {
        switch (samples) {
            case rm::ImageSamples::S1:
                return VK_SAMPLE_COUNT_1_BIT;
            case rm::ImageSamples::S2:
                return VK_SAMPLE_COUNT_2_BIT;
            case rm::ImageSamples::S4:
                return VK_SAMPLE_COUNT_4_BIT;
            case rm::ImageSamples::S8:
                return VK_SAMPLE_COUNT_8_BIT;
            case rm::ImageSamples::S16:
                return VK_SAMPLE_COUNT_16_BIT;
            case rm::ImageSamples::S32:
                return VK_SAMPLE_COUNT_32_BIT;
            case rm::ImageSamples::S64:
                return VK_SAMPLE_COUNT_64_BIT;
        }


        RM_ASSERT(false, "Unknown ImageSamples");
        return VK_SAMPLE_COUNT_1_BIT;
    }

    VkImageUsageFlags VKHelpers::ToVkImageUsage(rm::ImageUsage usage) {
        VkImageUsageFlags flags = 0;

        if (rm::HasFlag(usage, rm::ImageUsage::DEPTH_STENCIL)) {
            flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        }

        if (rm::HasFlag(usage, rm::ImageUsage::COLOR)) {
            flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }

        return flags;
    }

    VkImageTiling VKHelpers::ToVkImageTiling(rm::ImageTiling tiling) {
        switch (tiling) {
            case rm::ImageTiling::LINEAR:
                return VK_IMAGE_TILING_LINEAR;
            case rm::ImageTiling::NEAREST:
                return VK_IMAGE_TILING_OPTIMAL;
        }

        RM_ASSERT(false, "Unknown ImageTiling");
        return VK_IMAGE_TILING_OPTIMAL;
    }

    VkImageAspectFlags VKHelpers::ToVkImageAspect(rm::ImageAspectMask aspectMask) {
        VkImageAspectFlags flags = 0;

        if (rm::HasFlag(aspectMask, rm::ImageAspectMask::COLOR)) {
            flags |= VK_IMAGE_ASPECT_COLOR_BIT;
        }
        if (rm::HasFlag(aspectMask, rm::ImageAspectMask::DEPTH)) {
            flags |= VK_IMAGE_ASPECT_DEPTH_BIT;
        }
        if (rm::HasFlag(aspectMask, rm::ImageAspectMask::STENCIL)) {
            flags |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
        if (rm::HasFlag(aspectMask, rm::ImageAspectMask::METADATA)) {
            flags |= VK_IMAGE_ASPECT_METADATA_BIT;
        }

        return flags;
    }

    VkBufferUsageFlags VKHelpers::ToVkBufferUsage(BufferUsage usage) {
        VkBufferUsageFlags flags = 0;

        if (rm::HasFlag(usage, rm::BufferUsage::SHADER)) {
            flags |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        }

        return flags;
    }

    VmaAllocationCreateFlags VKHelpers::ToVmaAllocation(BufferAllocation allocation) {
        VmaAllocationCreateFlags flags = 0;

        if (rm::HasFlag(allocation, rm::BufferAllocation::SEQUENTIAL_WRITE)) {
            flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        }
        if (rm::HasFlag(allocation, rm::BufferAllocation::TRANSFER_INSTEAD)) {
            flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT;
        }
        if (rm::HasFlag(allocation, rm::BufferAllocation::MAPPED)) {
            flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
        }

        return flags;
    }

    unsigned int VKHelpers::GetFormatSizeBytes(rm::RenderFormat format) {
        switch (format) {
            case rm::RenderFormat::R8_UNORM:
                return 1;
            case rm::RenderFormat::RG8_UNORM:
                return 2;
            case rm::RenderFormat::RGBA8_UNORM:
            case rm::RenderFormat::RGBA8_SRGB:
            case rm::RenderFormat::BGRA8_UNORM:
            case rm::RenderFormat::BGRA8_SRGB:
            case rm::RenderFormat::R32_FLOAT:
            case rm::RenderFormat::D32_FLOAT:
            case rm::RenderFormat::D24_UNORM_S8_UINT:
                return 4;
            case rm::RenderFormat::RGBA16_FLOAT:
            case rm::RenderFormat::RG32_FLOAT:
            case rm::RenderFormat::D32_FLOAT_S8_UINT:
                return 8;
            case rm::RenderFormat::RGB32_FLOAT:
                return 12;
            case rm::RenderFormat::RGBA32_FLOAT:
                return 16;
        }

        RM_ASSERT(false, "Unknown RenderFormat");
        return 0;
    }
}
