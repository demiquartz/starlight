/**
 * @file
 * @brief
 * Manage the GPU device.
 *
 * @author
 * Takaaki Sato
 *
 * @copyright @parblock
 * (c) 2023, Demiquartz <info@demiquartz.jp>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * @endparblock
 */
#include <ranges>
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include "config.hpp"
#include "device.hpp"
#include "version.hpp"

namespace Starlight::Core {

static const char** GetRequiredInstanceLyrs(std::uint32_t* count, bool headless) {
#ifdef DEBUG
    static const char* layers[1];
    for (const auto& property : vk::enumerateInstanceLayerProperties()) {
        if (std::strcmp("VK_LAYER_KHRONOS_validation", property.layerName) == 0) {
            layers[0] = "VK_LAYER_KHRONOS_validation";
            if (count) *count = 1;
            return layers;
        }
    }
#endif
    if (count) *count = 0;
    return nullptr;
}

static const char** GetRequiredInstanceExts(std::uint32_t* count, bool headless) {
    if (!headless) {
        auto extensions = glfwGetRequiredInstanceExtensions(count);
        if (extensions) return extensions;
        const char* description;
        glfwGetError(&description);
        throw std::runtime_error(description);
    }
    if (count) *count = 0;
    return nullptr;
}

struct Device::Impl {
    SharedWindow                         window;
    vk::UniqueInstance                   instance;
    vk::PhysicalDevice                   phyDevice;
    vk::UniqueDevice                     lgcDevice;
    vk::Queue                            queueGraphics;
    vk::Queue                            queueCompute;
    vk::Queue                            queueTransfer;
    vk::UniqueCommandPool                commandPoolGraphics;
    vk::UniqueCommandPool                commandPoolCompute;
    vk::UniqueCommandPool                commandPoolTransfer;
    vk::UniqueSurfaceKHR                 surface;
    vk::UniqueSwapchainKHR               swapchain;
    vk::UniqueImage                      depthStencil;
    vk::UniqueDeviceMemory               depthStencilMemory;
    std::vector<vk::UniqueImageView>     colorImageViews;
    vk::UniqueImageView                  depthImageView;
    vk::UniqueRenderPass                 renderPass;
    std::vector<vk::UniqueFramebuffer>   framebuffers;
    std::vector<vk::UniqueCommandBuffer> commandBuffersGraphics;
    std::vector<vk::UniqueCommandBuffer> commandBuffersCompute;
    std::vector<vk::UniqueCommandBuffer> commandBuffersTransfer;
    vk::UniqueSemaphore                  presentCompletedSemaphore;
    vk::UniqueSemaphore                  renderCompletedSemaphore;
    std::vector<vk::UniqueFence>         presentFences;
    Impl(SharedWindow window) : window(window) {
        instance  = CreateInstance();
        phyDevice = ChoosePhysicalDevice();
        lgcDevice = CreateLogicalDevice();
        if (window) {
            surface   = CreateSurface();
            swapchain = CreateSwapchain();
            CreateDepthStencil();
            CreateImageViews();
            CreateRenderPass();
            CreateFramebuffers();
            CreateCommandBuffers();
            CreateSyncPrimitive();
            window->PollEvents();
            window->ShowWindow();
        }
    }
    vk::UniqueInstance CreateInstance(void) {
        auto requiredLyrCount = static_cast<std::uint32_t>(0);
        auto requiredLyrNames = GetRequiredInstanceLyrs(&requiredLyrCount, !window);
        auto requiredExtCount = static_cast<std::uint32_t>(0);
        auto requiredExtNames = GetRequiredInstanceExts(&requiredExtCount, !window);
        std::span lyrNames(requiredLyrNames, requiredLyrCount);
        std::span extNames(requiredExtNames, requiredExtCount);
        auto appName = Config::GetAppName();
        auto sysName = Version::Name;
        auto appVer = VK_MAKE_VERSION(Config::GetAppMajor(), Config::GetAppMinor(), Config::GetAppPatch());
        auto sysVer = VK_MAKE_VERSION(Version::Major       , Version::Minor       , Version::Patch       );
        vk::ApplicationInfo    appInfo(appName.c_str(), appVer, sysName, sysVer, VK_API_VERSION_1_3);
        vk::InstanceCreateInfo insInfo(vk::InstanceCreateFlags(), &appInfo, lyrNames, extNames);
        return vk::createInstanceUnique(insInfo);
    }
    vk::PhysicalDevice ChoosePhysicalDevice(void) {
        auto devices = instance->enumeratePhysicalDevices() |
        std::ranges::views::filter([&](const vk::PhysicalDevice& device) {
            auto queueFamilyProperties = device.getQueueFamilyProperties();
            vk::QueueFlags queueFlags;
            for (const auto& queueFamilyProperty : queueFamilyProperties) {
                queueFlags |= queueFamilyProperty.queueFlags;
            }
            if (queueFlags & (vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute | vk::QueueFlagBits::eTransfer)) {
                if (!window) return true;
                for (std::uint32_t i = 0, size = queueFamilyProperties.size(); i < size; ++i) {
                    if (queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics) {
                        if (glfwGetPhysicalDevicePresentationSupport(*instance, device, i)) return true;
                    }
                }
            }
            return false;
        });
        auto device = std::ranges::max_element(devices, {}, [](const auto &device) {
            auto deviceProperty = device.getProperties(); 
            auto memoryProperty = device.getMemoryProperties(); 
            std::size_t deviceType = 0;
            std::size_t memorySize = 0;
            switch (deviceProperty.deviceType) {
            case vk::PhysicalDeviceType::eDiscreteGpu:
                deviceType = 2;
                break;
            case vk::PhysicalDeviceType::eIntegratedGpu:
                deviceType = 1;
                break;
            default:
                break;
            }
            for (const auto& heap : std::span(memoryProperty.memoryHeaps).first(memoryProperty.memoryHeapCount)) {
                if (heap.flags & vk::MemoryHeapFlagBits::eDeviceLocal) memorySize += heap.size;
            }
            return std::make_tuple(deviceType, memorySize);
        });
        if (device != devices.end()) return *device;
        throw std::runtime_error("No suitable physical device found");
    }
    vk::UniqueDevice CreateLogicalDevice(void) {
        // TODO: The queue selection logic has room for improvement
        auto queueFamilyGraphics   = static_cast<std::uint32_t>(0);
        auto queueFamilyCompute    = static_cast<std::uint32_t>(0);
        auto queueFamilyTransfer   = static_cast<std::uint32_t>(0);
        auto queueFamilyProperties = phyDevice.getQueueFamilyProperties();
        for (std::uint32_t i = 0, size = queueFamilyProperties.size(); i < size; ++i) {
            if (queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics) queueFamilyGraphics = i;
            if (queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eCompute ) queueFamilyCompute  = i;
            if (queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eTransfer) queueFamilyTransfer = i;
        }
        std::vector<std::uint32_t> queueCountList(queueFamilyProperties.size());
        auto queueIndexGraphics = queueCountList[queueFamilyGraphics]++;
        auto queueIndexCompute  = queueCountList[queueFamilyCompute ]++;
        auto queueIndexTransfer = queueCountList[queueFamilyTransfer]++;
        std::vector<vk::DeviceQueueCreateInfo> queueInfos;
        std::vector<std::vector<float>> queuePrioritiesList;
        for (std::size_t i = 0, size = queueCountList.size(); i < size; ++i) if (queueCountList[i]) {
            if (queueCountList[i] > queueFamilyProperties[i].queueCount) {
                throw std::runtime_error("Attempted to create more queues than supported");
            }
            const auto& queuePriorities = queuePrioritiesList.emplace_back(queueCountList[i], 1.0f);
            queueInfos.emplace_back(vk::DeviceQueueCreateFlags(), i, queuePriorities);
        }
        auto device = [this, &queueInfos] {
            std::vector<const char*> lyrNames;
            std::vector<const char*> extNames;
            extNames.emplace_back("VK_KHR_swapchain");
            vk::DeviceCreateInfo info(vk::DeviceCreateFlags(), queueInfos, lyrNames, extNames);
            return phyDevice.createDeviceUnique(info);
        }();
        {
            vk::CommandPoolCreateInfo info(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
            info.setQueueFamilyIndex(queueFamilyGraphics);
            commandPoolGraphics = device->createCommandPoolUnique(info);
            info.setQueueFamilyIndex(queueFamilyCompute );
            commandPoolCompute  = device->createCommandPoolUnique(info);
            info.setQueueFamilyIndex(queueFamilyTransfer);
            commandPoolTransfer = device->createCommandPoolUnique(info);
        }
        queueGraphics = device->getQueue(queueFamilyGraphics, queueIndexGraphics);
        queueCompute  = device->getQueue(queueFamilyCompute,  queueIndexCompute );
        queueTransfer = device->getQueue(queueFamilyTransfer, queueIndexTransfer);
        return device;
    }
    vk::UniqueSurfaceKHR CreateSurface(void) {
        VkSurfaceKHR surface;
        auto handle = std::any_cast<GLFWwindow*>(window->GetHandle());
        if (glfwCreateWindowSurface(*instance, handle, nullptr, &surface) != VK_SUCCESS) {
            const char* description;
            glfwGetError(&description);
            throw std::runtime_error(description);
        }
        return vk::UniqueSurfaceKHR(surface, vk::ObjectDestroy<vk::Instance, vk::DispatchLoaderStatic>(*instance));
    }
    vk::UniqueSwapchainKHR CreateSwapchain(void) {
        // TODO: Review and optimize these parameters later
        auto fmt = phyDevice.getSurfaceFormatsKHR(*surface)[0];
        auto cap = phyDevice.getSurfaceCapabilitiesKHR(*surface);
        vk::SwapchainCreateInfoKHR info;
        info.surface          = *surface;
        info.minImageCount    = std::clamp(2u, cap.minImageCount, cap.maxImageCount);
        info.imageFormat      = fmt.format;
        info.imageColorSpace  = fmt.colorSpace;
        info.imageExtent      = cap.currentExtent;
        info.imageArrayLayers = 1;
        info.imageUsage       = cap.supportedUsageFlags;
        info.presentMode      = vk::PresentModeKHR::eFifo;
        info.clipped          = VK_TRUE;
        return lgcDevice->createSwapchainKHRUnique(info);
    }
    void CreateDepthStencil(void) {
        // TODO: Review and optimize these parameters later
        // TODO: Add support for headless mode
        auto cap = phyDevice.getSurfaceCapabilitiesKHR(*surface);
        vk::ImageCreateInfo info;
        info.imageType       = vk::ImageType::e2D;
        info.format          = vk::Format::eD32SfloatS8Uint;
        info.extent          = vk::Extent3D(cap.currentExtent, 1);
        info.mipLevels       = 1;
        info.arrayLayers     = 1;
        info.usage           = vk::ImageUsageFlagBits::eDepthStencilAttachment;
        depthStencil         = lgcDevice->createImageUnique(info);
        auto requirements    = lgcDevice->getImageMemoryRequirements(*depthStencil);
        auto properties      = phyDevice.getMemoryProperties();
        auto memoryTypeIndex = 0u;
        for (std::uint32_t i = 0, size = properties.memoryTypeCount; i < size; ++i) {
            if (requirements.memoryTypeBits & 1) {
                const auto& types = properties.memoryTypes[i];
                if (types.propertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal) {
                    memoryTypeIndex = i;
                    break;
                }
            }
            requirements.memoryTypeBits >>= 1;
        }
        vk::MemoryAllocateInfo allocInfo(requirements.size, memoryTypeIndex);
        depthStencilMemory = lgcDevice->allocateMemoryUnique(allocInfo);
        lgcDevice->bindImageMemory(*depthStencil, *depthStencilMemory, 0);
    }
    void CreateImageViews(void) {
        // TODO: Review and optimize these parameters later
        // TODO: Add support for headless mode
        vk::ComponentMapping components(vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA);
        auto fmt    = phyDevice.getSurfaceFormatsKHR(*surface)[0];
        auto images = lgcDevice->getSwapchainImagesKHR(*swapchain);
        for (const auto& image : images) {
            vk::ImageSubresourceRange subresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
            vk::ImageViewCreateInfo info(vk::ImageViewCreateFlags(), image, vk::ImageViewType::e2D, fmt.format, components, subresourceRange);
            colorImageViews.emplace_back(lgcDevice->createImageViewUnique(info));
        }
        vk::ImageSubresourceRange subresourceRange(vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, 1, 0, 1);
        vk::ImageViewCreateInfo info(vk::ImageViewCreateFlags(), *depthStencil, vk::ImageViewType::e2D, vk::Format::eD32SfloatS8Uint, components, subresourceRange);
        depthImageView = lgcDevice->createImageViewUnique(info);
    }
    void CreateRenderPass(void) {
        // TODO: Review and optimize these parameters later
        // TODO: Add support for headless mode
        auto fmt = phyDevice.getSurfaceFormatsKHR(*surface)[0];
        std::array<vk::AttachmentDescription, 2> attachments;
        std::array<vk::SubpassDescription,    1> subpasses;
        auto& colorAttachment         = attachments[0];
        auto& depthAttachment         = attachments[1];
        colorAttachment.format        = fmt.format;
        colorAttachment.loadOp        = vk::AttachmentLoadOp::eClear;
        colorAttachment.finalLayout   = vk::ImageLayout::ePresentSrcKHR;
        depthAttachment.format        = vk::Format::eD32SfloatS8Uint;
        depthAttachment.loadOp        = vk::AttachmentLoadOp::eClear;
        depthAttachment.finalLayout   = vk::ImageLayout::eDepthStencilAttachmentOptimal;
        std::array<vk::AttachmentReference, 1> colorReference;
        colorReference[0] = { 0, vk::ImageLayout::eColorAttachmentOptimal        };
        std::array<vk::AttachmentReference, 1> depthReference;
        depthReference[0] = { 1, vk::ImageLayout::eDepthStencilAttachmentOptimal };
        subpasses[0].setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
        subpasses[0].setColorAttachments       (colorReference       );
        subpasses[0].setPDepthStencilAttachment(depthReference.data());
        vk::RenderPassCreateInfo info(vk::RenderPassCreateFlags(), attachments, subpasses);
        renderPass = lgcDevice->createRenderPassUnique(info);
    }
    void CreateFramebuffers(void) {
        auto cap = phyDevice.getSurfaceCapabilitiesKHR(*surface);
        vk::FramebufferCreateInfo info;
        info.renderPass = *renderPass;
        info.width      = cap.currentExtent.width;
        info.height     = cap.currentExtent.height;
        info.layers     = 1;
        for (const auto& view : colorImageViews) {
            std::array<vk::ImageView, 2> attachments{ *view, *depthImageView };
            info.setAttachments(attachments);
            framebuffers.emplace_back(lgcDevice->createFramebufferUnique(info));
        }
    }
    void CreateCommandBuffers(void) {
        // TODO: Review and optimize these parameters later
        vk::CommandBufferAllocateInfo info;
        info.commandPool        = *commandPoolGraphics;
        info.level              = vk::CommandBufferLevel::ePrimary;
        info.commandBufferCount = colorImageViews.size();
        commandBuffersGraphics  = lgcDevice->allocateCommandBuffersUnique(info);
        info.commandPool        = *commandPoolCompute;
        info.level              = vk::CommandBufferLevel::ePrimary;
        info.commandBufferCount = colorImageViews.size();
        commandBuffersCompute   = lgcDevice->allocateCommandBuffersUnique(info);
        info.commandPool        = *commandPoolTransfer;
        info.level              = vk::CommandBufferLevel::ePrimary;
        info.commandBufferCount = colorImageViews.size();
        commandBuffersTransfer  = lgcDevice->allocateCommandBuffersUnique(info);
    }
    void CreateSyncPrimitive(void) {
        // TODO: Review and optimize these parameters later
        for (std::size_t i = 0, size = colorImageViews.size(); i < size; ++i) {
            vk::FenceCreateInfo info(vk::FenceCreateFlagBits::eSignaled);
            presentFences.push_back(lgcDevice->createFenceUnique(info));
        }
        presentCompletedSemaphore = lgcDevice->createSemaphoreUnique({});
        renderCompletedSemaphore  = lgcDevice->createSemaphoreUnique({});
    }
    void Clear(float r, float g, float b, float a) {
        // TODO: Temporary implementation for debug
        auto cap = phyDevice.getSurfaceCapabilitiesKHR(*surface);
        std::uint32_t imageIndex;
        lgcDevice->acquireNextImageKHR(*swapchain, std::numeric_limits<std::uint64_t>::max(), *presentCompletedSemaphore, nullptr, &imageIndex);
        lgcDevice->waitForFences(1, &presentFences[imageIndex].get(), VK_TRUE, std::numeric_limits<std::uint64_t>::max());
        std::array<vk::ClearValue, 2> clearValues;
        clearValues[0].color.float32[0]     =   r;
        clearValues[0].color.float32[1]     =   g;
        clearValues[0].color.float32[2]     =   b;
        clearValues[0].color.float32[3]     =   a;
        clearValues[1].depthStencil.depth   = 1.0;
        clearValues[1].depthStencil.stencil =   0;
        vk::CommandBufferBeginInfo info;
        commandBuffersGraphics[imageIndex]->begin(info);
        vk::Rect2D renderArea({ 0, 0 }, cap.currentExtent);
        vk::RenderPassBeginInfo rpInfo(*renderPass, *framebuffers[imageIndex], renderArea, clearValues);
        commandBuffersGraphics[imageIndex]->beginRenderPass(rpInfo, vk::SubpassContents::eInline);
        // Draw here
        commandBuffersGraphics[imageIndex]->endRenderPass();
        commandBuffersGraphics[imageIndex]->end();
        std::array<vk::SubmitInfo, 1> submitInfos;
        vk::PipelineStageFlags waitStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        submitInfos[0].commandBufferCount   = 1;
        submitInfos[0].pCommandBuffers      = &commandBuffersGraphics[imageIndex].get();
        submitInfos[0].pWaitDstStageMask    = &waitStageMask;
        submitInfos[0].waitSemaphoreCount   = 1;
        submitInfos[0].pWaitSemaphores      = &presentCompletedSemaphore.get();
        submitInfos[0].signalSemaphoreCount = 1;
        submitInfos[0].pSignalSemaphores    = &renderCompletedSemaphore.get();
        lgcDevice->resetFences(1, &presentFences[imageIndex].get());
        queueGraphics.submit(submitInfos, *presentFences[imageIndex]);
        vk::PresentInfoKHR presentInfo;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores    = &renderCompletedSemaphore.get();
        presentInfo.swapchainCount     = 1;
        presentInfo.pSwapchains        = &swapchain.get();
        presentInfo.pImageIndices      = &imageIndex;
        queueGraphics.presentKHR(presentInfo);
    }
};

Device::Device() :
Device(nullptr) {
}

Device::Device(SharedWindow window) :
pImpl(std::make_unique<Impl>(window)) {
}

Device::~Device() {
}

void Device::Clear(float r, float g, float b) {
    pImpl->Clear(r, g, b, 1.0f);
}

} // namespace Starlight::Core
