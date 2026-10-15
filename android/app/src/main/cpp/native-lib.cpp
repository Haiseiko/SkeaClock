#include "vulkan_renderer.h"

#include <android/log.h>
#include <chrono>
#include <cmath>
#include <cstring>

#define LOG_TAG "SkeaClockVulkan"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

VulkanRenderer::VulkanRenderer() = default;
VulkanRenderer::~VulkanRenderer() { destroy(); }

bool VulkanRenderer::isSupported() {
    uint32_t count = 0;
    VkResult result = vkEnumerateInstanceVersion(&count);
    return result == VK_SUCCESS || result == VK_INCOMPLETE;
}

bool VulkanRenderer::init(ANativeWindow* window) {
    if (!window) return false;
    window_ = window;
    if (!createInstance()) return false;
    if (!createSurface()) return false;
    if (!createDevice()) return false;
    if (!createSwapchain()) return false;
    if (!createCommandStructures()) return false;
    initialized_ = true;
    LOGI("Vulkan renderer initialized");
    return true;
}

bool VulkanRenderer::createInstance() {
    const char* layers[] = { "VK_LAYER_KHRONOS_validation" };
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "SkeaClock";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "SkeaClock";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledLayerCount = 1;
    createInfo.ppEnabledLayerNames = layers;

    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance_);
    if (result != VK_SUCCESS) {
        LOGE("vkCreateInstance failed: %d", result);
        return false;
    }
    return true;
}

bool VulkanRenderer::createSurface() {
    const VkAndroidSurfaceCreateInfoKHR createInfo{
        VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
        nullptr,
        0,
        window_
    };

    VkResult result = vkCreateAndroidSurfaceKHR(instance_, &createInfo, nullptr, &surface_);
    if (result != VK_SUCCESS) {
        LOGE("vkCreateAndroidSurfaceKHR failed: %d", result);
        return false;
    }
    return true;
}

bool VulkanRenderer::createDevice() {
    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(instance_, &physicalDeviceCount, nullptr);
    if (physicalDeviceCount == 0) {
        LOGE("No Vulkan physical devices found");
        return false;
    }

    VkPhysicalDevice devices[8];
    vkEnumeratePhysicalDevices(instance_, &physicalDeviceCount, devices);
    physicalDevice_ = devices[0];

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &queueFamilyCount, nullptr);
    VkQueueFamilyProperties queueProps[8];
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &queueFamilyCount, queueProps);
    for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice_, i, surface_, &presentSupport);
        if ((queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && presentSupport) {
            queueFamilyIndex_ = i;
            break;
        }
    }

    if (queueFamilyIndex_ == UINT32_MAX) {
        LOGE("No suitable queue family found");
        return false;
    }

    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueInfo{};
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueFamilyIndex = queueFamilyIndex_;
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures features{};
    VkDeviceCreateInfo deviceInfo{};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount = 1;
    deviceInfo.pQueueCreateInfos = &queueInfo;
    deviceInfo.pEnabledFeatures = &features;

    if (vkCreateDevice(physicalDevice_, &deviceInfo, nullptr, &device_) != VK_SUCCESS) {
        LOGE("vkCreateDevice failed");
        return false;
    }

    vkGetDeviceQueue(device_, queueFamilyIndex_, 0, &queue_);
    return true;
}

bool VulkanRenderer::createSwapchain() {
    VkSurfaceCapabilitiesKHR caps{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice_, surface_, &caps);

    width_ = static_cast<uint32_t>(ANativeWindow_getWidth(window_));
    height_ = static_cast<uint32_t>(ANativeWindow_getHeight(window_));

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice_, surface_, &formatCount, nullptr);
    VkSurfaceFormatKHR formats[8];
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice_, surface_, &formatCount, formats);

    VkSurfaceFormatKHR chosenFormat = formats[0];
    VkExtent2D extent = caps.currentExtent.width != UINT32_MAX ? caps.currentExtent : VkExtent2D{ width_, height_ };

    VkSwapchainCreateInfoKHR swapInfo{};
    swapInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapInfo.surface = surface_;
    swapInfo.minImageCount = caps.minImageCount;
    swapInfo.imageFormat = chosenFormat.format;
    swapInfo.imageColorSpace = chosenFormat.colorSpace;
    swapInfo.imageExtent = extent;
    swapInfo.imageArrayLayers = 1;
    swapInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapInfo.preTransform = caps.currentTransform;
    swapInfo.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
    swapInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    swapInfo.clipped = VK_TRUE;
    swapInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device_, &swapInfo, nullptr, &swapchain_) != VK_SUCCESS) {
        LOGE("vkCreateSwapchainKHR failed");
        return false;
    }
    return true;
}

bool VulkanRenderer::createCommandStructures() {
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndex_;
    if (vkCreateCommandPool(device_, &poolInfo, nullptr, &commandPool_) != VK_SUCCESS) {
        LOGE("vkCreateCommandPool failed");
        return false;
    }

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool_;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    if (vkAllocateCommandBuffers(device_, &allocInfo, &commandBuffer_) != VK_SUCCESS) {
        LOGE("vkAllocateCommandBuffers failed");
        return false;
    }

    VkSemaphoreCreateInfo semInfo{};
    semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    if (vkCreateSemaphore(device_, &semInfo, nullptr, &imageAvailableSemaphore_) != VK_SUCCESS) {
        LOGE("vkCreateSemaphore(imageAvailable) failed");
        return false;
    }
    if (vkCreateSemaphore(device_, &semInfo, nullptr, &renderFinishedSemaphore_) != VK_SUCCESS) {
        LOGE("vkCreateSemaphore(renderFinished) failed");
        return false;
    }
    return true;
}

void VulkanRenderer::renderFrame() {
    if (!initialized_) return;

    uint32_t imageIndex = 0;
    VkResult result = vkAcquireNextImageKHR(device_, swapchain_, UINT64_MAX,
        imageAvailableSemaphore_, VK_NULL_HANDLE, &imageIndex);
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        return;
    }

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkBeginCommandBuffer(commandBuffer_, &beginInfo);

    VkClearValue clearValue{};
    clearValue.color = {{0.05f, 0.05f, 0.1f, 1.0f}};

    VkRenderPassBeginInfo rpInfo{};
    rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpInfo.renderPass = VK_NULL_HANDLE;
    rpInfo.framebuffer = VK_NULL_HANDLE;
    rpInfo.renderArea.offset = {0, 0};
    rpInfo.renderArea.extent = {width_, height_};
    rpInfo.clearValueCount = 1;
    rpInfo.pClearValues = &clearValue;

    vkCmdBeginRenderPass(commandBuffer_, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdEndRenderPass(commandBuffer_);
    vkEndCommandBuffer(commandBuffer_);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &imageAvailableSemaphore_;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer_;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderFinishedSemaphore_;

    vkQueueSubmit(queue_, 1, &submitInfo, VK_NULL_HANDLE);

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderFinishedSemaphore_;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain_;
    presentInfo.pImageIndices = &imageIndex;
    vkQueuePresentKHR(queue_, &presentInfo);
}

void VulkanRenderer::destroy() {
    if (!initialized_) return;

    if (renderFinishedSemaphore_ != VK_NULL_HANDLE) {
        vkDestroySemaphore(device_, renderFinishedSemaphore_, nullptr);
    }
    if (imageAvailableSemaphore_ != VK_NULL_HANDLE) {
        vkDestroySemaphore(device_, imageAvailableSemaphore_, nullptr);
    }
    if (commandPool_ != VK_NULL_HANDLE) {
        vkDestroyCommandPool(device_, commandPool_, nullptr);
    }
    if (swapchain_ != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(device_, swapchain_, nullptr);
    }
    if (device_ != VK_NULL_HANDLE) {
        vkDestroyDevice(device_, nullptr);
    }
    if (surface_ != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(instance_, surface_, nullptr);
    }
    if (instance_ != VK_NULL_HANDLE) {
        vkDestroyInstance(instance_, nullptr);
    }

    initialized_ = false;
}
