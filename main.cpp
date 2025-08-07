#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <vector>

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vk_platform.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

class HelloTriangleApplication {
public:
    void run() {
        initWindow();
        initVulkan();
        //mainLoop();
        cleanUp();
    }
private:
    GLFWwindow *window = nullptr;
    vk::raii::Context context;
    vk::raii::Instance instance = nullptr;
    vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
    vk::raii::PhysicalDevice physicalDevice = nullptr;




    std::vector<const char*> requiredDeviceExtensions = {
        vk::KHRSwapchainExtensionName,
        vk::KHRSpirv14ExtensionName,
        vk::KHRSynchronization2ExtensionName,
        vk::KHRCreateRenderpass2ExtensionName
    };

    void initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    }

    void initVulkan() {
        createInstance();
        setupDebugMessenger();
        pickPhysicalDevice();

    }

    void mainLoop() {
        while( !glfwWindowShouldClose(window) ){
            glfwPollEvents();
        }
    }

    void cleanUp() {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void createInstance(){
        constexpr vk::ApplicationInfo appInfo {
            .pApplicationName   = "Hello Triangle",
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName        = "No Engine",
            .engineVersion      = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion         = vk::ApiVersion14,
        };

        auto extensions = getRequiredExtensions();

        vk::InstanceCreateInfo createInfo{
            .pApplicationInfo           = &appInfo,
            .enabledLayerCount          = static_cast<uint32_t>(validationLayers.size()),
            .ppEnabledLayerNames        = validationLayers.data(),
            .enabledExtensionCount      = static_cast<uint32_t>(extensions.size()),
            .ppEnabledExtensionNames    = extensions.data(),
        };

        instance = vk::raii::Instance( context, createInfo );
    }

    std::vector<const char*> getRequiredExtensions(){
        uint32_t glfwExtCount = 0;
        auto glfwExts = glfwGetRequiredInstanceExtensions(&glfwExtCount);
        auto extensions = std::vector<const char*>( glfwExts, glfwExts + glfwExtCount );
        extensions.push_back( vk::EXTDebugUtilsExtensionName );

        return extensions;
    }

    void setupDebugMessenger(){
        vk::DebugUtilsMessengerCreateInfoEXT messengerCreateInfo{
            .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | \
                               vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | \
                               vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | \
                               vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose,
            .messageType     = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | \
                               vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | \
                               vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,
            .pfnUserCallback = &debugCallback
        };

        debugMessenger = instance.createDebugUtilsMessengerEXT(messengerCreateInfo);
    }

    void pickPhysicalDevice(){
        auto devices = instance.enumeratePhysicalDevices();
        const auto devSel = std::ranges::find_if(
            devices,
            [&]( const vk::raii::PhysicalDevice &device ){
                // Check Vulkan API version
                bool VK13_support = device.getProperties().apiVersion >= VK_API_VERSION_1_3;
                // Check Queue Family Graphics Support
                auto queueFams = device.getQueueFamilyProperties();
                bool GPX_support = std::ranges::any_of(
                    queueFams,
                    []( const vk::QueueFamilyProperties &qfp ){
                        return !!( qfp.queueFlags & vk::QueueFlagBits::eGraphics );
                    }
                );
                // Check Extensions Support
                auto deviceExts = device.enumerateDeviceExtensionProperties();
                bool EXT_support = std::ranges::all_of(
                    requiredDeviceExtensions,
                    [&deviceExts]( const char * &ext ){
                        return std::ranges::any_of(
                            deviceExts,
                            [ext]( const vk::ExtensionProperties &extProp ){
                                return strcmp(extProp.extensionName, ext) == 0;
                            }
                        );
                    }
                );
                // Check Features Support
                auto features = device.getFeatures2<vk::PhysicalDeviceFeatures2,vk::PhysicalDeviceVulkan13Features,vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
                bool FET_support = features.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering && \
                                   features.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

                return VK13_support && EXT_support && GPX_support && FET_support;
            }
        );

        if( devSel != devices.end() ){
            physicalDevice = *devSel;
            std::cout << "SELECTED DEVICE: " << physicalDevice.getProperties().deviceName << std::endl;
        } else {
            throw std::runtime_error( "Failed to find a suitable GPU." );
        }
    }

    static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
        vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
        vk::DebugUtilsMessageTypeFlagsEXT type,
        const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void*
    ){
        std::cerr << "VAL: " << pCallbackData->pMessage << std::endl;
        return vk::False;
    }
};

int main(){
    HelloTriangleApplication app;

    try {
        app.run();
    } catch ( const std::exception &e ){
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
