#include "augustus_common.h"
#include "augustus_game.h"
#include "augustus_window.h"
#include "augustus_gfx.h"
#include "cglm/struct/cam.h"
#include "cglm/struct/mat4.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#include <cglm/cglm.h>
#include <cglm/struct.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define VULKAN_DEBUG

typedef struct {
    mat4s model, view, projection;
} UniformBufferObject;

static SDL_Window *window = NULL;
static u32 width = 800, height = 600;

static bool is_running = true;

static VkInstance instance;
static VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
static VkDevice device;

static VmaAllocator allocator;

static VkQueue graphicsQueue;
static VkSurfaceKHR surface;
static VkQueue presentQueue;
static VkSwapchainKHR swapChain;

static VkImage* swapChainImages = NULL;
static u32 swapChainImagesLen = 0;

static VkFormat swapChainImageFormat;
static VkExtent2D swapChainExtent;

static VkImageView* swapChainImageViews = NULL;

static VkImage depthImage;
static VmaAllocation depthImageAllocation;
static VkImageView depthImageView;

static VkRenderPass renderPass;

static VkFramebuffer* swapChainFramebuffers = NULL;

static VkCommandPool commandPool;
static VkCommandBuffer* commandBuffers;

static VkViewport viewport;

static VkRect2D scissor;

#define MAX_FRAMES_IN_FLIGHT 2
static VkSemaphore* imageAvailableSemaphores;
static VkSemaphore* renderFinishedSemaphores;
static VkFence* inFlightFences;

static bool framebufferResized = false;

static u32 currentFrame = 0;

static u32 imageIndex = 0;
VkCommandBuffer currentCommandBuffer(void) {
    return commandBuffers[currentFrame];
}

#define validationLayerCount (sizeof(validationLayers) / sizeof(validationLayers[0]))
static const char* validationLayers[] = {
    "VK_LAYER_KHRONOS_validation"
};

#define deviceExtensionCount (sizeof(deviceExtensions) / sizeof(deviceExtensions[0]))
static const char* deviceExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        // Message is important enough to show
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Validation layer: %s\n", pCallbackData->pMessage);
    }

    return VK_FALSE;
}

#ifdef VULKAN_DEBUG
static bool checkValidationLayers(void) {
    u32 layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);

    VkLayerProperties* availableLayers = malloc(sizeof(availableLayers[0]) * layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

    for(u32 i = 0; i < validationLayerCount; i++) {
        bool layer_found = false;

        const char* layer = validationLayers[i];

        for(u32 j = 0; j < layerCount; j++) {
            if(strcmp(layer, availableLayers[j].layerName) == 0) {
                layer_found = true;
                break;
            }
        }

        if(!layer_found) return false;
    }

    free(availableLayers);

    return true;
}
#endif//VULKAN_DEBUG

static const char** getRequiredExtensions(u32* count) {
  const char** sdlExtensions = NULL;
  SDL_Vulkan_GetInstanceExtensions(window, count, NULL);

  sdlExtensions = malloc(sizeof(sdlExtensions) * *count);
  SDL_Vulkan_GetInstanceExtensions(window, count, sdlExtensions);
  
#ifdef VULKAN_DEBUG
    (*count)++;

    const char** extensions = malloc(sizeof(extensions[0]) * *count);
    memcpy(extensions, sdlExtensions, sizeof(sdlExtensions[0]) * (*count - 1));
    extensions[*count - 1] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

    return extensions;
#else//VULKAN_DEBUG
    return sdlExtensions;
#endif//VULKAN_DEBUG
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != NULL) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != NULL) {
        func(instance, debugMessenger, pAllocator);
    }
}

typedef struct {
    bool foundGraphics;
    u32 graphicsFamily;

    bool foundPresent;
    u32 presentFamily;
} QueueFamilyIndices;

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices = { false, 0 };

    u32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);

    VkQueueFamilyProperties* queueFamilies = malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

    for(u32 i = 0; i < queueFamilyCount; i++) {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if(presentSupport) {
            indices.foundPresent = true;
            indices.presentFamily = i;
        }

        if(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.foundGraphics = true;
            indices.graphicsFamily = i;
        }
    }

    free(queueFamilies);

    return indices;
}

u32 findMemoryType(u32 typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for(u32 i = 0; i < memProperties.memoryTypeCount; i++) {
        if((typeFilter & (1 << i)) &&
           (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    SDL_Log("Failed to find a suitable memory type");
    SDL_Quit();

    return 0;
}

VkFormat findSupportedFormat(VkFormat* candidates, u32 candidatesLen, VkImageTiling tiling, VkFormatFeatureFlags features) {
    for(u32 i = 0; i < candidatesLen; i++) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, candidates[i], &props);

        if(tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return candidates[i];
        }
        else if(tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return candidates[i];
        }
    }

    SDL_Log("Failed to find a supported image format");
    SDL_Quit();

    return 0;
}

VkFormat findDepthFormat(void) {
    VkFormat candidates[] = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT,
    };
    u32 candidatesLen = sizeof(candidates) / sizeof(candidates[0]);

    return findSupportedFormat(candidates, candidatesLen, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

bool hasStencilComponent(VkFormat format) {
    return format ==VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

static bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
    u32 extensionsCount = 0;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionsCount, NULL);

    VkExtensionProperties* availableExtensions = malloc(sizeof(availableExtensions[0]) * extensionsCount);
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionsCount, availableExtensions);

    for(u32 i = 0; i < validationLayerCount; i++) {
        bool extensions_found = false;

        const char* extension = deviceExtensions[i];

        for(u32 j = 0; j < extensionsCount; j++) {
            if(strcmp(extension, availableExtensions[j].extensionName) == 0) {
                extensions_found = true;
                break;
            }
        }

        if(!extensions_found) return false;
    }

    free(availableExtensions);

    return true;
}

typedef struct {
    VkSurfaceCapabilitiesKHR capabilities;

    VkSurfaceFormatKHR* formats;
    u32 formatsLen;

    VkPresentModeKHR* presentModes;
    u32 presentModesLen;
} SwapChainSupportDetails;

static SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
    SwapChainSupportDetails details = { 0 };

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &details.formatsLen, NULL);

    if(details.formatsLen != 0) {
        details.formats = malloc(sizeof(details.formats[0]) * details.formatsLen);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &details.formatsLen, details.formats);
    }

    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &details.presentModesLen, NULL);

    if(details.presentModesLen != 0) {
        details.presentModes = malloc(sizeof(details.presentModes[0]) * details.presentModesLen);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &details.presentModesLen, details.presentModes);
    }

    return details;
}

static bool isDeviceSuitable(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    QueueFamilyIndices indices = findQueueFamilies(device);

    bool extensionsSupported = checkDeviceExtensionSupport(device);
    
    bool swapChainAdequete = false;
    if(extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequete = swapChainSupport.formatsLen != 0 && swapChainSupport.presentModesLen != 0;
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return (
        indices.foundPresent && indices.foundGraphics &&
        extensionsSupported && swapChainAdequete &&
        supportedFeatures.samplerAnisotropy
    );
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(VkSurfaceFormatKHR* available_formats, u32 len) {
    for(u32 i = 0; i < len; i++) {
        VkSurfaceFormatKHR fmt = available_formats[i];
        if(fmt.format == VK_FORMAT_B8G8R8_SRGB && fmt.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR) {
            return fmt;
        }
    }

    return available_formats[0];
}

VkPresentModeKHR chooseSwapPresentMode(VkPresentModeKHR* available_formats, u32 len) {
    for(u32 i = 0; i < len; i++) {
        VkPresentModeKHR mode = available_formats[i];
        if(mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent(VkSurfaceCapabilitiesKHR capabilities) {
    if(capabilities.currentExtent.width != U32_MAX) { // TODO: try and fix warning
        return capabilities.currentExtent;
    }
    else {
        i32 w, h;
        SDL_GetWindowSizeInPixels(window, &w, &h); // TODO: add error checks
                                                   // https://wiki.libsdl.org/SDL3/SDL_GetWindowSizeInPixels
                                                   // this can fail if result not zero

        VkExtent2D actual_extent = { (u32)w, (u32)h };

        actual_extent.width = clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actual_extent.height = clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actual_extent;
    }
}

VkCommandBuffer beginSingleTimeCommands(void) {
    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool = commandPool,
        .commandBufferCount = 1,
    };

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
    };

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .oldLayout = oldLayout,
        .newLayout = newLayout,

        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,

        .image = image,
        .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .subresourceRange.baseMipLevel = 0,
        .subresourceRange.levelCount = 1,
        .subresourceRange.baseArrayLayer = 0,
        .subresourceRange.layerCount = 1,

        .srcAccessMask = 0,
        .dstAccessMask = 0,
    };

    VkPipelineStageFlags srcStage, dstStage;

    if(newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if(hasStencilComponent(format)) {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if(oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if(oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else {
        return;
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        srcStage, dstStage,
        0,
        0, NULL,
        0, NULL,
        1, &barrier
    );

    endSingleTimeCommands(commandBuffer);
}

char* loadShaderCode(const char* filename, u32* len) {
    FILE* file = fopen(filename, "rb");

    if(!file) {
        SDL_Log("Failed to load (%s)", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    *len = ftell(file);

    fseek(file, 0, SEEK_SET);
    char* buffer = malloc(*len);

    fread(buffer, 1, *len, file);
    fclose(file);

    return buffer;
}

VkShaderModule createShaderModule(char* code, u32 len) {
    VkShaderModuleCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,

        .pCode = (u32*)code,
        .codeSize = len,
    };
    
    VkShaderModule module;
    if(vkCreateShaderModule(device, &createInfo, NULL, &module) != VK_SUCCESS) {
        SDL_Log("Failed to create logical device");
        SDL_Quit();
    }

    return module;
}

void createSwapChain(void) {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats, swapChainSupport.formatsLen);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes, swapChainSupport.presentModesLen);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    u32 imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if(swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapChainCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,

        .preTransform = swapChainSupport.capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,

        .presentMode = presentMode,
        .clipped = VK_TRUE,

        .oldSwapchain = VK_NULL_HANDLE,
    };

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    u32 queueFamilyIndices[] = { indices.graphicsFamily, indices.presentFamily };

    if(indices.graphicsFamily != indices.presentFamily) {
        swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapChainCreateInfo.queueFamilyIndexCount = 2;
        swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapChainCreateInfo.queueFamilyIndexCount = 0;
        swapChainCreateInfo.pQueueFamilyIndices = NULL;
    }

    if(vkCreateSwapchainKHR(device, &swapChainCreateInfo, NULL, &swapChain) != VK_SUCCESS) {
        SDL_Log("Failed to create swap chain");
        SDL_Quit();
    }

    vkGetSwapchainImagesKHR(device, swapChain, &swapChainImagesLen, NULL);
    swapChainImages = malloc(sizeof(swapChainImages[0]) * swapChainImagesLen);
    vkGetSwapchainImagesKHR(device, swapChain, &swapChainImagesLen, swapChainImages);

    swapChainImageFormat = surfaceFormat.format; 
    swapChainExtent = extent;
}

void cleanupSwapChains(void) {
    vkDestroyImageView(device, depthImageView, NULL);
    vmaDestroyImage(allocator, depthImage, depthImageAllocation);

    for(u32 i = 0; i < swapChainImagesLen; i++) {
        vkDestroyFramebuffer(device, swapChainFramebuffers[i], NULL);
    }
    free(swapChainFramebuffers);

    for(u32 i = 0; i < swapChainImagesLen; i++) {
        vkDestroyImageView(device, swapChainImageViews[i], NULL);
    }

    free(swapChainImageViews);
    vkDestroySwapchainKHR(device, swapChain, NULL);

    free(swapChainImages);
}

void createImage(u32 w, u32 h, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkImage* image, VmaAllocation* allocation) {
    VkImageCreateInfo imageCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,

        .imageType = VK_IMAGE_TYPE_2D,
        .extent.width = w,
        .extent.height = h,
        .extent.depth = 1,
        .mipLevels = 1,
        .arrayLayers = 1,

        .format = format,

        .tiling = tiling,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,

        .usage = usage,

        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,

        .samples = VK_SAMPLE_COUNT_1_BIT,
        .flags = 0,
    };

    VmaAllocationCreateInfo allocationCreateInfo = {
        .usage = VMA_MEMORY_USAGE_AUTO,
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
    };

    if(vmaCreateImage(allocator, &imageCreateInfo, &allocationCreateInfo, image, allocation, NULL) != VK_SUCCESS) {
        SDL_Log("Failed to allocated vertex buffers memory");
        SDL_Quit();
    }
}

VkImageView createImageView(VkImage img, VkFormat format, VkImageAspectFlags aspectFlags) {
    VkImageViewCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = img,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,

        .components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
        .components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
        .components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
        .components.a = VK_COMPONENT_SWIZZLE_IDENTITY,

        .subresourceRange.aspectMask = aspectFlags,
        .subresourceRange.baseMipLevel = 0,
        .subresourceRange.levelCount = 1,
        .subresourceRange.baseArrayLayer = 0,
        .subresourceRange.layerCount = 1,
    };
    
    VkImageView view;
    if(vkCreateImageView(device, &createInfo, NULL, &view) != VK_SUCCESS) {
        SDL_Log("Failed to create image view");
        SDL_Quit();
    }

    return view;
}

void createImageViews(void) {
    swapChainImageViews = malloc(sizeof(swapChainImageViews[0]) * swapChainImagesLen);
    for(u32 i = 0; i < swapChainImagesLen; i++) {
        swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

void createFramebuffers(void) {
    swapChainFramebuffers = malloc(swapChainImagesLen * sizeof(*swapChainFramebuffers));

    for(u32 i = 0; i < swapChainImagesLen; i++) {
        VkImageView attachments[] = {
            swapChainImageViews[i],
            depthImageView
        };
        u32 attachmentsLen = sizeof(attachments) / sizeof(attachments[0]);

        VkFramebufferCreateInfo framebufferCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = renderPass,
            .attachmentCount = attachmentsLen,
            .pAttachments = attachments,
            .width = swapChainExtent.width,
            .height = swapChainExtent.height,
            .layers = 1,
        };

        if(vkCreateFramebuffer(device, &framebufferCreateInfo, NULL, swapChainFramebuffers + i) != VK_SUCCESS) {
            SDL_Log("Failed to create framebuffer");
            SDL_Quit();
        }
    }
}

void createDepthBuffer(void) {
    VkFormat depthFormat = findDepthFormat();

    createImage(
        swapChainExtent.width,
        swapChainExtent.height,
        depthFormat,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        &depthImage,
        &depthImageAllocation
    );
    depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

    transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void createBuffer(usize bufferSize, u32 bufferUsage, VkBuffer* buffer, VmaAllocation* allocation) {
    VkBufferCreateInfo bufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = bufferSize,
        .usage = bufferUsage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    VmaAllocationCreateInfo allocInfo = {
        .usage = VMA_MEMORY_USAGE_AUTO,
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
    };

    if(vmaCreateBuffer(allocator, &bufferCreateInfo, &allocInfo, buffer, allocation, NULL) != VK_SUCCESS) {
        SDL_Log("Failed to create vertex buffers");
        SDL_Quit();
    }
}

void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferCopy copyRegion = {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = size,
    };

    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer);
}

void copyBufferToImage(VkBuffer buffer, VkImage image, u32 w, u32 h) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferImageCopy region = {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,

        .imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .imageSubresource.mipLevel = 0,
        .imageSubresource.baseArrayLayer = 0,
        .imageSubresource.layerCount = 1,

        .imageOffset = {0,0,0},
        .imageExtent = { w, h, 1 },
    };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    endSingleTimeCommands(commandBuffer);
}

void recreateSwapChains(void) {
    i32 w = 0, h = 0;
    while(w == 0 || h == 0) {
        SDL_GetWindowSizeInPixels(window, &w, &h);
        SDL_WaitEvent(NULL);
    }

    vkDeviceWaitIdle(device);

    cleanupSwapChains();

    createSwapChain();
    createImageViews();
    createDepthBuffer();
    createFramebuffers();
}

typedef struct {
    // mesh data
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;

    vec3s* vertices;
    vec2s* uvs;

    u32 verticesLen;

    u16* faces;
    u32 facesLen;

    VkBuffer vb, ib;
    VmaAllocation vba, iba;

    VkBuffer* uniformBuffers;
    VmaAllocation* uniformBuffersMem;
    void** uniformBuffersMapped;
} VkSprite;

static VkSprite VkSprite_instance;
static VkVertexInputBindingDescription VkSprite_getBindingDescription(void) {
    VkVertexInputBindingDescription desc = {
        .binding = 0,
        .stride = 0, // TODO: sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };

    return desc;
}

static VkVertexInputAttributeDescription* VkSprite_getAttributeDescription(u32* len) {
    VkVertexInputAttributeDescription* desc = malloc(sizeof(*desc) * 2);

    desc[0] = (VkVertexInputAttributeDescription) {
        .binding = 0,
        .location = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = 0,
    };

    desc[1] = (VkVertexInputAttributeDescription) {
        .binding = 0,
        .location = 1,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = sizeof(VkSprite_instance.vertices) * VkSprite_instance.verticesLen,
    };

    return desc;
}

void VkSprite_free(void) {
    for(u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vmaUnmapMemory(allocator, VkSprite_instance.uniformBuffersMem[i]);
        
        vmaDestroyBuffer(allocator, VkSprite_instance.uniformBuffers[i], VkSprite_instance.uniformBuffersMem[i]);
    }
    
    free(VkSprite_instance.uniformBuffers);
    free(VkSprite_instance.uniformBuffersMem);
    free(VkSprite_instance.uniformBuffersMapped);
    
    vmaDestroyBuffer(allocator, VkSprite_instance.vb, VkSprite_instance.vba);
    vmaDestroyBuffer(allocator, VkSprite_instance.ib, VkSprite_instance.iba);
}

void VkSprite_init(void) {
    u32 vertCodeLen;
    char* vertCode = loadShaderCode("resources/shaders/spriv/sprite.vert.spv", &vertCodeLen);

    u32 fragCodeLen;
    char* fragCode = loadShaderCode("resources/shaders/spriv/sprite.frag.spv", &fragCodeLen);

    VkShaderModule vertModule = createShaderModule(vertCode, vertCodeLen);
    VkShaderModule fragModule = createShaderModule(fragCode, fragCodeLen);

    VkPipelineShaderStageCreateInfo vertShaderStateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertModule,
        .pName = "main",
    };

    VkPipelineShaderStageCreateInfo fragShaderStateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = fragModule,
        .pName = "main",
    };

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStateInfo, fragShaderStateInfo };

    VkVertexInputBindingDescription bindingDesc = VkSprite_getBindingDescription();
    u32 attributeDescLen = 0;
    VkVertexInputAttributeDescription* attributeDesc = VkSprite_getAttributeDescription(&attributeDescLen);

    VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDesc,
        .vertexAttributeDescriptionCount = attributeDescLen,
        .pVertexAttributeDescriptions = attributeDesc,
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_VIEWPORT
    };
    u32 dynamicStatesLen = sizeof(dynamicStates) / sizeof(dynamicStates[0]);

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = dynamicStatesLen,
        .pDynamicStates = dynamicStates,
    };

    viewport = (VkViewport) {
        .x = 0.0f,
        .y = 0.0f,
        .width = (f32)swapChainExtent.width,
        .height = (f32)swapChainExtent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    scissor = (VkRect2D) {
        .offset = { 0, 0 },
        .extent = swapChainExtent,
    };

    VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor,
    };

    VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,

        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,

        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .lineWidth = 1.0f,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
    };

    VkPipelineMultisampleStateCreateInfo multisampleCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .sampleShadingEnable = VK_FALSE,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
    };

    VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,

        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,

        .depthCompareOp = VK_COMPARE_OP_LESS,

        .depthBoundsTestEnable = VK_FALSE,
        .minDepthBounds = 0.0f,
        .maxDepthBounds = 1.0f,

        .stencilTestEnable = VK_FALSE,
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
    };

    VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment
    };

    VkDescriptorSetLayoutBinding uboLayoutBinding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,

        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .pImmutableSamplers = NULL,
    };

    VkDescriptorSetLayoutBinding setLayoutBinding = {
        .binding = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,

        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .pImmutableSamplers = NULL,
    };

    VkDescriptorSetLayoutBinding bindings[] = {
        uboLayoutBinding,
        setLayoutBinding
    };
    u32 bindingsLen = sizeof(bindings) / sizeof(bindings[0]);

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = bindingsLen,
        .pBindings = bindings,
    };

    if(vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, NULL, &VkSprite_instance.descriptorSetLayout) != VK_SUCCESS) {
        SDL_Log("Failed to create descriptor set layout");
        SDL_Quit();
    }

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &VkSprite_instance.descriptorSetLayout,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = NULL,
    };

    if(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, NULL, &VkSprite_instance.pipelineLayout) != VK_SUCCESS) {
        SDL_Log("Failed to create pipeline layout");
        SDL_Quit();
    }

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = shaderStages,

        .pVertexInputState = &vertexInputCreateInfo,
        .pInputAssemblyState = &inputAssemblyCreateInfo,
        .pViewportState = &viewportStateCreateInfo,
        .pRasterizationState = &rasterizerCreateInfo,
        .pMultisampleState = &multisampleCreateInfo,
        .pDepthStencilState = &depthStencilCreateInfo,
        .pColorBlendState = &colorBlendCreateInfo,
        .pDynamicState = &dynamicStateCreateInfo,

        .layout = VkSprite_instance.pipelineLayout,

        .renderPass = renderPass,
        .subpass = 0,

        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1,
    };

    if(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, NULL, &VkSprite_instance.pipeline) != VK_SUCCESS) {
        SDL_Log("Failed to create graphics pipeline");
        SDL_Quit();
    }

    vkDestroyShaderModule(device, vertModule, NULL);
    free(vertCode);
    vkDestroyShaderModule(device, fragModule, NULL);
    free(fragCode);

    VkSprite_instance.uniformBuffers = malloc(sizeof(*VkSprite_instance.uniformBuffers) * MAX_FRAMES_IN_FLIGHT);
    VkSprite_instance.uniformBuffersMem = malloc(sizeof(*VkSprite_instance.uniformBuffersMem) * MAX_FRAMES_IN_FLIGHT);
    VkSprite_instance.uniformBuffersMapped = malloc(sizeof(*VkSprite_instance.uniformBuffersMapped) * MAX_FRAMES_IN_FLIGHT);

    VkDeviceSize uboSize = sizeof(UniformBufferObject);
    for(u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        createBuffer(uboSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VkSprite_instance.uniformBuffers + i, VkSprite_instance.uniformBuffersMem + i);
        vmaMapMemory(allocator, VkSprite_instance.uniformBuffersMem[i], VkSprite_instance.uniformBuffersMapped + i);
    }
}

void VkSprite_draw(void) {
    UniformBufferObject ubo = { 0 };

    ubo.model = glms_mat4_zero();
    ubo.projection = glms_ortho(0, width, 0, height, 0.01f, 10000.0f);
    ubo.view = glms_mat4_identity();

    memcpy(VkSprite_instance.uniformBuffersMapped[currentFrame], &ubo, sizeof(ubo));

    VkBuffer vertexBuffers[] = { VkSprite_instance.vb };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(currentCommandBuffer(), 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(currentCommandBuffer(), VkSprite_instance.ib, 0, VK_INDEX_TYPE_UINT16);

    vkCmdDrawIndexed(currentCommandBuffer(), VkSprite_instance.facesLen, 1, 0, 0, 0);
}

int main(void) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("SDL_Init failed (%s)", SDL_GetError());
        return 1;
    }

    SDL_Vulkan_LoadLibrary(NULL);
    window = SDL_CreateWindow("Stoner", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

    if(!window) {
        SDL_Log("SDL_CreateWindow failed (%s)", SDL_GetError());
        SDL_Quit();
        return 1;
    }

#ifdef VULKAN_DEBUG
    if(!checkValidationLayers()) {
        SDL_Log("validation layers requested, but not available");
        SDL_Quit();
        return 1;
    }
#endif//VULKAN_DEBUG

    u32 extensionCount = 0;
    const char* const* extensions = getRequiredExtensions(&extensionCount);

    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Stoner",
        .applicationVersion = VK_MAKE_VERSION(0, 1, 0),
        .pEngineName = "StonerEngine by James Barnfather",
        .engineVersion = VK_MAKE_VERSION(0, 1, 0),
        .apiVersion = VK_API_VERSION_1_0,
    };

    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = extensionCount,
        .ppEnabledExtensionNames = extensions,

#ifdef VULKAN_DEBUG
        .enabledLayerCount = validationLayerCount,
        .ppEnabledLayerNames = validationLayers,
#else//VULKAN_DEBUG
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = NULL,
#endif//VULKAN_DEBUG
    };

#ifdef VULKAN_DEBUG
    VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = debugCallback,
    };

    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugMessengerCreateInfo;
#endif//VULKAN_DEBUG

    if(vkCreateInstance(&createInfo, NULL, &instance) != VK_SUCCESS) {
        SDL_Log("Failed to create Vulkan instance! (%s)", SDL_GetError());
        SDL_Quit();
        return 1;
    }
      //
#ifdef VULKAN_DEBUG
    VkDebugUtilsMessengerEXT debugMessenger;
    if(CreateDebugUtilsMessengerEXT(instance, &debugMessengerCreateInfo, NULL, &debugMessenger) != VK_SUCCESS) {
        SDL_Log("failed to set up debug messenger!");
        SDL_Quit();
        return 1;
    }
#endif//VULKAN_DEBUG
    
    if(!SDL_Vulkan_CreateSurface(window, instance, &surface)) {
        SDL_Log("failed to create Vulkan surface using SDL! (%s)", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    u32 deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
    
    if(deviceCount == 0) {
        SDL_Log("failed to find GPU's with Vulkan's support!");
        SDL_Quit();
        return 1;
    }

    VkPhysicalDevice* devices = malloc(sizeof(VkPhysicalDevice) * deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices);

    for(u32 i = 0; i < deviceCount; i++) {
        if(isDeviceSuitable(devices[i])) {
            physicalDevice = devices[i];
            break;
        }
    }

    if(physicalDevice == VK_NULL_HANDLE) {
        SDL_Log("failed to find a suitable GPU!");
        SDL_Quit();
        return 1;
    }

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    u32 queueCreateInfosCount = 0;
    VkDeviceQueueCreateInfo* queueCreateInfos = NULL;

    u32 uniqueQueueFamiliesCount = 2;
    u32 uniqueQueueFamilies[] = { indices.presentFamily, indices.graphicsFamily }; // check if these are unique or not

    f32 queuePriority = 1.0f;
    for(u32 i = 0; i < uniqueQueueFamiliesCount; i++) {
        VkDeviceQueueCreateInfo queueCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = uniqueQueueFamilies[i],
            .queueCount = 1,
            .pQueuePriorities = &queuePriority,
        };

        u32 idx = queueCreateInfosCount;
        queueCreateInfosCount++;
        queueCreateInfos = realloc(queueCreateInfos, sizeof(queueCreateInfos[0]) * queueCreateInfosCount);
        queueCreateInfos[idx] = queueCreateInfo;
    }

    VkPhysicalDeviceFeatures deviceFeatures = {
        .samplerAnisotropy = VK_TRUE,
    };

    VkDeviceCreateInfo deviceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pQueueCreateInfos = queueCreateInfos,
        .queueCreateInfoCount = queueCreateInfosCount,
        .pEnabledFeatures = &deviceFeatures,

        .ppEnabledExtensionNames = deviceExtensions,
        .enabledExtensionCount = deviceExtensionCount,

#ifdef VULKAN_DEBUG
        .enabledLayerCount = validationLayerCount,
        .ppEnabledLayerNames = validationLayers,
#else//VULKAN_DEBUG
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = NULL,
#endif//VULKAN_DEBUG
    };

    if(vkCreateDevice(physicalDevice, &deviceCreateInfo, NULL, &device) != VK_SUCCESS) {
        SDL_Log("Failed to create logical device");
        SDL_Quit();
        return 1;
    }

    vkGetDeviceQueue(device, indices.graphicsFamily, 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily, 0, &presentQueue);

    VmaAllocatorCreateInfo vmaAllocatorCreateInfo = {
        .instance = instance,
        .device = device,
        .physicalDevice = physicalDevice,

        .flags = 0, 
    };

    
    if(vmaCreateAllocator(&vmaAllocatorCreateInfo, &allocator) != VK_SUCCESS) {
        SDL_Log("Failed to create logical device");
        SDL_Quit();
        return 1;
    }

    createSwapChain();
    createImageViews();

    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_VIEWPORT
    };
    u32 dynamicStatesLen = sizeof(dynamicStates) / sizeof(dynamicStates[0]);

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = dynamicStatesLen,
        .pDynamicStates = dynamicStates,
    };

    //
    // Render Pass
    //

    VkAttachmentDescription colorAttachment = {
        .format = swapChainImageFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,

        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,

        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    VkAttachmentReference colorAttachmentRef = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkAttachmentDescription depthAttachment = {
        .format = findDepthFormat(),
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,

        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,

        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    VkAttachmentReference depthAttachmentRef = {
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef,
        .pDepthStencilAttachment = &depthAttachmentRef,
    };

    VkSubpassDependency dependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,

        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .srcAccessMask = 0,

        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    };

    VkAttachmentDescription attachments[] = {
        colorAttachment,
        depthAttachment
    };
    u32 attachmentsLen = sizeof(attachments) / sizeof(attachments[0]);

    VkRenderPassCreateInfo renderPassCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,

        .attachmentCount = attachmentsLen,
        .pAttachments = attachments,

        .subpassCount = 1,
        .pSubpasses = &subpass,
        
        .dependencyCount = 1,
        .pDependencies = &dependency,
    };

    if(vkCreateRenderPass(device, &renderPassCreateInfo, NULL, &renderPass) != VK_SUCCESS) {
        SDL_Log("Failed to create render pass");
        SDL_Quit();
        return 1;
    }

    VkCommandPoolCreateInfo commandPoolCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = indices.graphicsFamily,
    };

    if(vkCreateCommandPool(device, &commandPoolCreateInfo, NULL, &commandPool) != VK_SUCCESS) {
        SDL_Log("Failed to create command pool");
        SDL_Quit();
        return 1;
    }

    createDepthBuffer();
    createFramebuffers();

    VkCommandBufferAllocateInfo commandBufferAllocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = MAX_FRAMES_IN_FLIGHT,
    };

    commandBuffers = malloc(sizeof(*commandBuffers) * MAX_FRAMES_IN_FLIGHT);

    if(vkAllocateCommandBuffers(device, &commandBufferAllocInfo, commandBuffers) != VK_SUCCESS) {
        SDL_Log("Failed to allocate command buffers");
        SDL_Quit();
        return 1;
    }

    VkSemaphoreCreateInfo semaphoreCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    
    VkFenceCreateInfo fenceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    imageAvailableSemaphores = malloc(sizeof(*imageAvailableSemaphores) * MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores = malloc(sizeof(*renderFinishedSemaphores) * MAX_FRAMES_IN_FLIGHT);
    inFlightFences = malloc(sizeof(*inFlightFences) * MAX_FRAMES_IN_FLIGHT);

    for(u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ ){
        if(vkCreateSemaphore(device, &semaphoreCreateInfo, NULL, imageAvailableSemaphores + i) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreCreateInfo, NULL, renderFinishedSemaphores + i) != VK_SUCCESS ||
            vkCreateFence(device, &fenceCreateInfo, NULL, inFlightFences + i) != VK_SUCCESS) {
            SDL_Log("Failed to create create synchronization objects for a frame");
            SDL_Quit();
            return 1;
        }
    }

    Augustus_main();

    vkDeviceWaitIdle(device);

    for(u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device, imageAvailableSemaphores[i], NULL);
        vkDestroySemaphore(device, renderFinishedSemaphores[i], NULL);
        vkDestroyFence(device, inFlightFences[i], NULL);
    }

    free(imageAvailableSemaphores);
    free(renderFinishedSemaphores);
    free(inFlightFences);

    vkFreeCommandBuffers(device, commandPool, MAX_FRAMES_IN_FLIGHT, commandBuffers);
    free(commandBuffers);

    vkDestroyCommandPool(device, commandPool, NULL);

    vmaDestroyAllocator(allocator);

    vkDestroyDevice(device, NULL);

#ifdef VULKAN_DEBUG
    DestroyDebugUtilsMessengerEXT(instance, debugMessenger, NULL);
#endif//VULKAN_DEBUG

    vkDestroyInstance(instance, NULL);

    SDL_DestroyWindow(window);

    SDL_Quit();
    return 0;
}

void GFX_ClearColor(vec3s color) {
}

void GFX_BeginFrame(void) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch(event.type) {
            case SDL_QUIT:
                is_running = false;
                break;
	    case SDL_WINDOWEVENT_SIZE_CHANGED:
                framebufferResized = true;
                break;
	    case SDL_KEYDOWN:
                if(event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                    is_running = false;
                }
                break;
        }
    }

    vkWaitForFences(device, 1, inFlightFences + currentFrame, VK_TRUE, U64_MAX);

    VkResult result = vkAcquireNextImageKHR(device, swapChain, U64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        recreateSwapChains();
        return;
        //continue;
    }
    else if(result != VK_SUCCESS) {
        SDL_Log("Failed to submit draw command buffer");
        SDL_Quit();
        return;
    }

    vkResetFences(device, 1, inFlightFences + currentFrame);

    vkResetCommandBuffer(commandBuffers[currentFrame], 0);
}

void GFX_EndFrame(void) {
    VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = waitSemaphores,
        .pWaitDstStageMask = waitStages,

        .commandBufferCount = 1,
        .pCommandBuffers = commandBuffers + currentFrame,

        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signalSemaphores
    };

    if(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
        SDL_Log("Failed to submit draw command buffer");
        SDL_Quit();
        return;
    }

    VkSwapchainKHR swapChains[] = { swapChain };

    VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,

        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signalSemaphores,
        
        .swapchainCount = 1,
        .pSwapchains = swapChains,
        .pImageIndices = &imageIndex,

        .pResults = NULL,
    };

    VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);

    if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
        framebufferResized = false;
        recreateSwapChains();
        //continue;
        return;
    }
    else if(result != VK_SUCCESS) {
        SDL_Log("Failed to submit draw command buffer");
        SDL_Quit();
        return;
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

vec2s Window_MousePosition(void) {
    return glms_vec2_zero();
}

vec2s Window_MouseDelta(void) {
    return glms_vec2_zero();
}

f32 Window_MouseWheelDelta(void) {
    return 0.0f;
}

bool Window_MouseButtonDown(MouseButton btn) {
    return false;
}

bool Window_MouseButtonUp(MouseButton btn) {
    return false;
}

bool Window_MouseButtonHeld(MouseButton btn) {
    return false;
}

bool Window_KeyDown(SDL_Scancode btn) {
    return false;
}

bool Window_KeyUp(SDL_Scancode btn) {
    return false;
}

bool Window_KeyHeld(SDL_Scancode btn) {
    return false;
}

bool Window_ShouldClose(void) {
    return !is_running;
}

f32 Window_DeltaTime(void) {
    return 1.0f;
}

vec2s ScreenToWorld(vec2s pos) {
    return pos;
}

Animation Animation_make(void) {
    return (Animation) {
        .frames = NULL,
        .len = 0,
        .fps = 12,
    };
}

void Animation_free(Animation* animation) {
    if(animation->frames) free(animation->frames);
}

void Animation_push(Animation* animation, Frame frame) {
}

static void AnimationMap_realloc(AnimationMap* map) {
    if(!map->buckets) {
        map->buckets = calloc(map->allocated, sizeof(Animation));
        return;
    }

    map->buckets = realloc(map->buckets, map->allocated * sizeof(Animation));
}

#define MAX_LOADFACTOR 0.75f
static f32 AnimationMap_loadfactor(AnimationMap* map) {
    return (f32)map->length / (f32)map->allocated;
}

static AnimationMapEntry* AnimationMap_find_entry(AnimationMap* map, Hash hash) {
    Hash idx = hash % map->allocated;

    AnimationMapEntry* entry = map->buckets + idx;

    while(entry->next) {
        if(entry->hash == hash) break;
        entry = entry->next;
    }

    return entry;
}

AnimationMap AnimationMap_make(void) {
    return (AnimationMap) {
        .buckets = NULL,
        .allocated = 100,
        .length = 0,
    };
}

void AnimationMap_free(AnimationMap* map) {
    if(map->buckets) free(map->buckets);
    *map = AnimationMap_make();
}

void AnimationMap_set(AnimationMap* map, char* key, Animation value) {
    if(!map->buckets) {
        AnimationMap_realloc(map);
    }

    if(AnimationMap_loadfactor(map) >= MAX_LOADFACTOR) {
        AnimationMap_realloc(map);
    }

    Hash hash = hash_string(key);

    AnimationMapEntry* entry = AnimationMap_find_entry(map, hash);
    if(entry->hash != hash) map->length++;

    *entry = (AnimationMapEntry) {
        .hash = hash,
        .key = key,
        .value = value,
        .next = NULL
    };
}

Animation AnimationMap_get(AnimationMap* map, char* key) {
    Hash idx = hash_string(key) % map->allocated;
    return map->buckets[idx].value;
}

bool AnimationMap_exists(AnimationMap* map, char* key) {
    Hash hash = hash_string(key);

    AnimationMapEntry* entry = AnimationMap_find_entry(map, hash);

    return entry->hash == hash;
}

AnimationMap AnimationMap_load(const char* filepath) {
    AnimationMap map = AnimationMap_make();

    FILE* file = fopen(filepath, "r");

    char* buffer = NULL;
    u64 len = 0;

    if(file) {

        fseek(file, 0, SEEK_END);
        len = ftell(file);
        fseek(file, 0, SEEK_SET);

        buffer = malloc(len);
        fread(buffer, 1, len, file);

        fclose(file);
    }
    else {
        printf("Failed to read file: \"%s\"", filepath);
    }

#if 0
    if(buffer) {
        json_tokener* tok = json_tokener_new();

        json_object* obj = json_tokener_parse_ex(tok, buffer, len);

        json_object_object_foreach(obj, key, val) {
            u64 array_len = json_object_array_length(val);
            for(u64 i = 0; i < array_len; i++) {
                json_object* frame = json_object_array_get_idx(val, i);
                json_object_object_foreach(frame, key, val) {
                    Frame frame = {0};

                    frame.x = json_object_get_int(json_object_object_get(val, "x"));
                    frame.y = json_object_get_int(json_object_object_get(val, "y"));
                    frame.width = json_object_get_int(json_object_object_get(val, "w"));
                    frame.height = json_object_get_int(json_object_object_get(val, "h"));
                }
            }
        }

        json_tokener_free(tok);
    }
#endif
    return map;
}

Sprite Sprite_make(char* filename) {
    Sprite result = {
        .animations = AnimationMap_make(),
        0
    };

        result.texture_filename = filename;
    //stbi_set_flip_vertically_on_load(true);
        u8* pixels = stbi_load(result.texture_filename, &result.w, &result.h, &result.channels, 0);

    result.vkSize = result.w * result.h * result.channels; 

    if(!pixels) {
        SDL_Log("Failed to load texture from '%s'", result.texture_filename);
        SDL_Quit();
    }

    result.format = VK_FORMAT_R8G8B8_SRGB;
    switch(result.channels) {
        case 3:
            result.format = VK_FORMAT_R8G8B8_SRGB;
        case 4:
            result.format = VK_FORMAT_R8G8B8A8_SRGB;
    }

    VkBuffer stagingBuffer;
    VmaAllocation stagingBufferAllocation;

    createBuffer(result.vkSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, &stagingBuffer, &stagingBufferAllocation);

    void* data;
    vmaMapMemory(allocator, stagingBufferAllocation, &data);
    memcpy(data, pixels, (usize)result.vkSize);
    vmaUnmapMemory(allocator, stagingBufferAllocation);

    stbi_image_free(pixels);

    createImage(result.w, result.h, result.format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, &result.image, &result.imageAllocation);

    transitionImageLayout(result.image, result.format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    copyBufferToImage(stagingBuffer, result.image, result.w, result.h);

    transitionImageLayout(result.image, result.format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vmaDestroyBuffer(allocator, stagingBuffer, stagingBufferAllocation);

    result.imageView = createImageView(result.image, result.format, VK_IMAGE_ASPECT_COLOR_BIT);

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

    VkSamplerCreateInfo samplerCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .minFilter = VK_FILTER_LINEAR,
        .magFilter = VK_FILTER_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,

        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = deviceProperties.limits.maxSamplerAnisotropy,

        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,

        .unnormalizedCoordinates = VK_FALSE,

        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,

        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .mipLodBias = 0.0f,
        .minLod = 0.0f,
        .maxLod = 0.0f,
    };

    if(vkCreateSampler(device, &samplerCreateInfo, NULL, &result.sampler) != VK_SUCCESS) {
        SDL_Log("Failed to create texture sampler");
        SDL_Quit();
    }

    return result;
}

void Sprite_free(Sprite* sprite) {
    AnimationMap_free(&sprite->animations);

    vkDestroySampler(device, sprite->sampler, NULL);
    vkDestroyImageView(device, sprite->imageView, NULL);

    vmaDestroyImage(allocator, sprite->image, sprite->imageAllocation);
}

void Sprite_draw(Sprite* sprite) {
    VkSprite_draw();
}
