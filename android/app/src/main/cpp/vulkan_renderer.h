#pragma once

#include <android/native_window.h>
#include <vulkan/vulkan.h>
#include <cstdint>

class VulkanRenderer {
public:
    VulkanRenderer();
    ~VulkanRenderer();

    static bool isSupported();
    bool init(ANativeWindow* window);
    void renderFrame();
    void destroy();
    bool isReady() const { return initialized_; }

private:
    bool createInstance();
    bool createSurface();
    bool createDevice();
    bool createSwapchain();
    bool createCommandStructures();

    ANativeWindow* window_ = nullptr;
    VkInstance instance_ = VK_NULL_HANDLE;
    VkSurfaceKHR surface_ = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    VkQueue queue_ = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
    VkCommandPool commandPool_ = VK_NULL_HANDLE;
    VkCommandBuffer commandBuffer_ = VK_NULL_HANDLE;
    VkSemaphore imageAvailableSemaphore_ = VK_NULL_HANDLE;
    VkSemaphore renderFinishedSemaphore_ = VK_NULL_HANDLE;

    uint32_t width_ = 0;
    uint32_t height_ = 0;
    uint32_t queueFamilyIndex_ = UINT32_MAX;
    bool initialized_ = false;
};
