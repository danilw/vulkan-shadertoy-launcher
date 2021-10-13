
// Danil, 2021+ Vulkan shader launcher, self https://github.com/danilw/vulkan-shadertoy-launcher
// The MIT License

#include "vk_utils.h"

void vk_exit(VkInstance vk)
{
    vkDestroyInstance(vk, NULL);
}

vk_error vk_enumerate_devices(VkInstance vk, VkSurfaceKHR *surface, struct vk_physical_device *devs, uint32_t *idx, bool use_idx)
{
    
    vk_error retval = VK_ERROR_NONE;
    VkResult res;
    uint32_t count = 0;
    
    res = vkEnumeratePhysicalDevices(vk, &count, NULL);
    vk_error_set_vkresult(&retval, res);
    if (res < 0) {
        return retval;
    }
    if (count < 1){
        printf("No Vulkan device found.\n");
        vk_error_set_vkresult(&retval, VK_ERROR_INCOMPATIBLE_DRIVER);
        return retval;
    }
    
    VkPhysicalDevice *phy_devs;
    phy_devs = malloc(count * sizeof(VkPhysicalDevice));
    

    res = vkEnumeratePhysicalDevices(vk, &count, phy_devs);
    vk_error_set_vkresult(&retval, res);
    if (res < 0) {
        free(phy_devs);
        phy_devs = NULL;
        return retval;
    }
    
    for (uint32_t i = 0; i < count && (!use_idx); i++)
    {
        uint32_t qfc = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(phy_devs[i], &qfc, NULL);
        if (qfc < 1)continue;
        
        VkQueueFamilyProperties *queue_family_properties;
        queue_family_properties = malloc(qfc * sizeof(VkQueueFamilyProperties));
        
        vkGetPhysicalDeviceQueueFamilyProperties(phy_devs[i], &qfc, queue_family_properties);
        
        for (uint32_t j = 0; j < qfc; j++)
        {
            VkBool32 supports_present;
            vkGetPhysicalDeviceSurfaceSupportKHR(phy_devs[i], j, *surface, &supports_present);
            
            if ((queue_family_properties[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) && supports_present)
            {
              *idx = i;
              use_idx = true;
              break;
            }
        }
        free(queue_family_properties);
    }
    if (!use_idx){
        printf("Not found suitable queue which supports graphics.\n");
        vk_error_set_vkresult(&retval, VK_ERROR_INCOMPATIBLE_DRIVER);
        free(phy_devs);
        phy_devs = NULL;
        return retval;
    }
    if (*idx >= count){
        printf("Wrong GPU index %lu, max devices count %lu\n", (unsigned long)*idx, (unsigned long) count);
        vk_error_set_vkresult(&retval, VK_ERROR_INCOMPATIBLE_DRIVER);
        free(phy_devs);
        phy_devs = NULL;
        return retval;
    }
    
    printf("Using GPU device %lu\n", (unsigned long) *idx);


    devs[0].physical_device = phy_devs[*idx];

    vkGetPhysicalDeviceProperties(devs[0].physical_device, &devs[0].properties);
    vkGetPhysicalDeviceFeatures(devs[0].physical_device, &devs[0].features);
    vkGetPhysicalDeviceMemoryProperties(devs[0].physical_device, &devs[0].memories);

    uint32_t qfc = 0;
    devs[0].queue_family_count = VK_MAX_QUEUE_FAMILY;
    vkGetPhysicalDeviceQueueFamilyProperties(devs[0].physical_device, &qfc, NULL);
    vkGetPhysicalDeviceQueueFamilyProperties(devs[0].physical_device, &devs[0].queue_family_count, devs[0].queue_families);

    devs[0].queue_families_incomplete = devs[0].queue_family_count < qfc;
    
    free(phy_devs);
    phy_devs = NULL;
    return retval;
}


vk_error vk_get_commands(struct vk_physical_device *phy_dev, struct vk_device *dev, VkDeviceQueueCreateInfo queue_info[], uint32_t queue_info_count, uint32_t create_count)
{
    vk_error retval = VK_ERROR_NONE;
    VkResult res;
    bool create_num_cmd=false; //create many cmd buffers in one Queue
    
    if(create_count>0)
    {
        queue_info_count=1;
        create_num_cmd=true;
    }

    dev->command_pools = malloc(queue_info_count * sizeof *dev->command_pools);
    if (dev->command_pools == NULL)
    {
        vk_error_set_errno(&retval, errno);
        return retval;
    }

    for (uint32_t i = 0; i < queue_info_count; ++i)
    {
        struct vk_commands *cmd = &dev->command_pools[i];
        *cmd = (struct vk_commands){0};

        cmd->qflags = phy_dev->queue_families[queue_info[i].queueFamilyIndex].queueFlags;

        VkCommandPoolCreateInfo pool_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = queue_info[i].queueFamilyIndex,
        };

        res = vkCreateCommandPool(dev->device, &pool_info, NULL, &cmd->pool);
        vk_error_set_vkresult(&retval, res);
        if (res < 0)
            return retval;
        ++dev->command_pool_count;

        cmd->queues = malloc(queue_info[i].queueCount * sizeof *cmd->queues);
        if(!create_num_cmd){
            cmd->buffers = malloc(queue_info[i].queueCount * sizeof *cmd->buffers);
        }else {
            cmd->buffers = malloc(create_count * sizeof *cmd->buffers);
        }
        if (cmd->queues == NULL || cmd->buffers == NULL)
        {
            vk_error_set_errno(&retval, errno);
            return retval;
        }

        for (uint32_t j = 0; j < queue_info[i].queueCount; ++j)
            vkGetDeviceQueue(dev->device, queue_info[i].queueFamilyIndex, j, &cmd->queues[j]);
        cmd->queue_count = queue_info[i].queueCount;

        VkCommandBufferAllocateInfo buffer_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = cmd->pool,
            .commandBufferCount = create_num_cmd?create_count:queue_info[i].queueCount,
        };

        res = vkAllocateCommandBuffers(dev->device, &buffer_info, cmd->buffers);
        vk_error_set_vkresult(&retval, res);
        if (res)
            return retval;

        cmd->buffer_count = create_num_cmd?create_count:queue_info[i].queueCount;
    }
    return retval;

}

void vk_cleanup(struct vk_device *dev)
{
    vkDeviceWaitIdle(dev->device);

    for (uint32_t i = 0; i < dev->command_pool_count; ++i)
    {
        free(dev->command_pools[i].queues);
        free(dev->command_pools[i].buffers);
        vkDestroyCommandPool(dev->device, dev->command_pools[i].pool, NULL);
    }
    free(dev->command_pools);

    vkDestroyDevice(dev->device, NULL);

    *dev = (struct vk_device){0};
}

vk_error vk_load_shader(struct vk_device *dev, const uint32_t *code, VkShaderModule *shader, size_t size)
{
    vk_error retval = VK_ERROR_NONE;
    VkResult res;
    VkShaderModuleCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = size,
        .pCode = code,
    };

    res = vkCreateShaderModule(dev->device, &info, NULL, shader);
    vk_error_set_vkresult(&retval, res);
    
    return retval;
}

#ifdef YARIV_SHADER
vk_error vk_load_shader_yariv(struct vk_device *dev, const uint32_t *yariv_code, VkShaderModule *shader, size_t in_yariv_size)
{
    vk_error retval = VK_ERROR_NONE;
    void *in_yariv = (void *)yariv_code;
    size_t out_spirv_size = yariv_decode_size(in_yariv, in_yariv_size);
    uint32_t *out_spirv = malloc(out_spirv_size);
    yariv_decode(out_spirv, out_spirv_size, in_yariv, in_yariv_size);
    retval = vk_load_shader(dev, out_spirv, shader, out_spirv_size);
    
    free(out_spirv);
    return retval;
}
#endif

vk_error vk_load_shader_spirv_file(struct vk_device *dev, const char *spirv_file, VkShaderModule *shader)
{
    vk_error retval = VK_ERROR_NONE;
    VkResult res;
    char *code = NULL;
    size_t size = 0, cur = 0;
    FILE *fin = fopen(spirv_file, "rb");

    *shader = NULL;

    if (fin == NULL)
    {
        vk_error_set_errno(&retval, errno);
        return retval;
    }

    fseek(fin, 0, SEEK_END);
    size = ftell(fin);
    fseek(fin, 0, SEEK_SET);

    code = malloc(size);
    if (code == NULL)
    {
        vk_error_set_errno(&retval, errno);
        fclose(fin);
        return retval;
    }

    while (cur < size)
    {
        size_t read = fread(code + cur, 1, size - cur, fin);
        if (read == 0)
        {
            vk_error_set_errno(&retval, errno);
            free(code);
            fclose(fin);
            return retval;
        }
        cur += read;
    }
    
    retval = vk_load_shader(dev, (uint32_t*)code, shader, size);

    free(code);
    fclose(fin);
    return retval;
}

void vk_free_shader(struct vk_device *dev, VkShaderModule shader)
{
    vkDestroyShaderModule(dev->device, shader, NULL);
}

uint32_t vk_find_suitable_memory(struct vk_physical_device *phy_dev, struct vk_device *dev,
        VkMemoryRequirements *mem_req, VkMemoryPropertyFlags properties)
{
    for (uint32_t i = 0; i < phy_dev->memories.memoryTypeCount; ++i)
    {
        if ((mem_req->memoryTypeBits & 1 << i) == 0)
            continue;
        if (phy_dev->memories.memoryHeaps[phy_dev->memories.memoryTypes[i].heapIndex].size < mem_req->size)
            continue;
        if ((phy_dev->memories.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }

    return phy_dev->memories.memoryTypeCount;
}


vk_error vk_init_ext(VkInstance *vk, const char *ext_names[], uint32_t ext_count)
{
    vk_error retval = VK_ERROR_NONE;
    VkResult res;

    VkApplicationInfo app_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Vulkan Shader launcher",
        .applicationVersion = 0x010000,
        .pEngineName = "Vulkan Shader launcher",
        .engineVersion = 0x010000,
        .apiVersion = VK_API_VERSION_1_0,
    };
    VkInstanceCreateInfo info;

    info = (VkInstanceCreateInfo){
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = NULL,
        .pApplicationInfo = &app_info,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = NULL,
        .enabledExtensionCount = ext_count,
        .ppEnabledExtensionNames = ext_names,
    };

    res = vkCreateInstance(&info, NULL, vk);
    vk_error_set_vkresult(&retval, res);

    return retval;
}

vk_error vk_get_dev_ext(struct vk_physical_device *phy_dev, struct vk_device *dev, VkQueueFlags qflags,
        VkDeviceQueueCreateInfo queue_info[], uint32_t *queue_info_count,
        const char *ext_names[], uint32_t ext_count)
{
    vk_error retval = VK_ERROR_NONE;
    VkResult res;

    *dev = (struct vk_device){0};

    uint32_t max_queue_count = *queue_info_count;
    *queue_info_count = 0;

    uint32_t max_family_queues = 0;
    for (uint32_t i = 0; i < phy_dev->queue_family_count; ++i)
        if (max_family_queues < phy_dev->queue_families[i].queueCount)
            max_family_queues = phy_dev->queue_families[i].queueCount;
    float *queue_priorities;
    int tsize = 0;
    if (max_family_queues == 0) tsize = 1;
    queue_priorities = malloc((max_family_queues + tsize) * (sizeof(float)));
    memset(queue_priorities, 0, (max_family_queues + tsize) * (sizeof(float)));

    for (uint32_t i = 0; i < phy_dev->queue_family_count && i < max_queue_count; ++i)
    {
        if ((phy_dev->queue_families[i].queueFlags & qflags) == 0)
            continue;

        queue_info[(*queue_info_count)++] = (VkDeviceQueueCreateInfo){
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = i,
            .queueCount = phy_dev->queue_families[i].queueCount,
            .pQueuePriorities = queue_priorities,
        };
    }

    if (*queue_info_count == 0)
    {
        free(queue_priorities);
        queue_priorities = NULL;
        vk_error_set_vkresult(&retval, VK_ERROR_FEATURE_NOT_PRESENT);
        return retval;
    }

    VkDeviceCreateInfo dev_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = *queue_info_count,
        .pQueueCreateInfos = queue_info,
        .enabledExtensionCount = ext_count,
        .ppEnabledExtensionNames = ext_names,
        .pEnabledFeatures = &phy_dev->features,
    };

    res = vkCreateDevice(phy_dev->physical_device, &dev_info, NULL, &dev->device);
    vk_error_set_vkresult(&retval, res);
    
    free(queue_priorities);
    queue_priorities = NULL;
    return retval;
}

vk_error vk_create_surface(VkInstance vk, VkSurfaceKHR *surface, struct app_os_window *os_window){
    vk_error retval = VK_ERROR_NONE;
    VkResult res;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    VkWin32SurfaceCreateInfoKHR createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.hinstance = os_window->connection;
    createInfo.hwnd = os_window->window;

    res = vkCreateWin32SurfaceKHR(vk, &createInfo, NULL, surface);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
    VkXcbSurfaceCreateInfoKHR createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.connection = os_window->connection;
    createInfo.window = os_window->xcb_window;
    
    res = vkCreateXcbSurfaceKHR(vk, &createInfo, NULL, surface);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
    VkWaylandSurfaceCreateInfoKHR createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.display = os_window->display;
    createInfo.surface = os_window->surface;

    res = vkCreateWaylandSurfaceKHR(vk, &createInfo, NULL, surface);
#endif
    vk_error_set_vkresult(&retval, res);
    return retval;
}

vk_error vk_get_swapchain(VkInstance vk, struct vk_physical_device *phy_dev, struct vk_device *dev,
        struct vk_swapchain *swapchain, struct app_os_window *os_window, uint32_t thread_count, VkPresentModeKHR *present_mode)
{
    vk_error retval = VK_ERROR_NONE;
    VkResult res;
    
    VkSwapchainKHR oldSwapchain = swapchain->swapchain;

    res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phy_dev->physical_device, swapchain->surface, &swapchain->surface_caps);
    vk_error_set_vkresult(&retval, res);
    if (res)
        return retval;

    uint32_t image_count = swapchain->surface_caps.minImageCount + thread_count - 1;
    if (swapchain->surface_caps.maxImageCount < image_count && swapchain->surface_caps.maxImageCount != 0)
        image_count = swapchain->surface_caps.maxImageCount;

    uint32_t surface_format_count = 1;
    res = vkGetPhysicalDeviceSurfaceFormatsKHR(phy_dev->physical_device, swapchain->surface, &surface_format_count, &swapchain->surface_format);
    vk_error_set_vkresult(&retval, res);
    if (res < 0)
        return retval;

    if (surface_format_count == 1 && swapchain->surface_format.format == VK_FORMAT_UNDEFINED)
        swapchain->surface_format.format = VK_FORMAT_R8G8B8_UNORM;

    swapchain->present_modes_count = VK_MAX_PRESENT_MODES;

    res = vkGetPhysicalDeviceSurfacePresentModesKHR(phy_dev->physical_device, swapchain->surface,
            &swapchain->present_modes_count, swapchain->present_modes);
    vk_error_set_vkresult(&retval, res);
    if (res >= 0)
    {
        bool tret=false;
        for (uint32_t i = 0; i < swapchain->present_modes_count; ++i)
        {
            if (swapchain->present_modes[i] == *present_mode)
            {
                tret=true;
                break;
            }
        }
        if(!tret){
            bool a=false,b=false,c=false;
            for (uint32_t i = 0; i < swapchain->present_modes_count; ++i)
            {
                if (swapchain->present_modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
                {
                    c=true;
                }
                if (swapchain->present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
                {
                    b=true;
                }
                if (swapchain->present_modes[i] == VK_PRESENT_MODE_FIFO_KHR)
                {
                    a=true;
                }
            }
            if(a)*present_mode = VK_PRESENT_MODE_FIFO_KHR;
            else if(b)*present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
            else if(b)*present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
            
        }
    }
    
    VkExtent2D swapchainExtent;
    VkImageFormatProperties format_properties;
    res = vkGetPhysicalDeviceImageFormatProperties(phy_dev->physical_device, swapchain->surface_format.format, VK_IMAGE_TYPE_2D,
            VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 0, &format_properties);
    if (res==VK_SUCCESS
    && (format_properties.maxExtent.width >= swapchain->surface_caps.currentExtent.width && format_properties.maxExtent.height >= swapchain->surface_caps.currentExtent.height)
    || (swapchain->surface_caps.currentExtent.width == 0xFFFFFFFF)
    )
    {
      if (swapchain->surface_caps.currentExtent.width == 0xFFFFFFFF) {
          swapchainExtent.width = os_window->app_data.iResolution[0];
          swapchainExtent.height = os_window->app_data.iResolution[1];

          if (swapchainExtent.width < swapchain->surface_caps.minImageExtent.width) {
              swapchainExtent.width = swapchain->surface_caps.minImageExtent.width;
          }
          else if (swapchainExtent.width > swapchain->surface_caps.maxImageExtent.width) {
              swapchainExtent.width = swapchain->surface_caps.maxImageExtent.width;
          }

          if (swapchainExtent.height < swapchain->surface_caps.minImageExtent.height) {
              swapchainExtent.height = swapchain->surface_caps.minImageExtent.height;
          }
          else if (swapchainExtent.height > swapchain->surface_caps.maxImageExtent.height) {
              swapchainExtent.height = swapchain->surface_caps.maxImageExtent.height;
          }
      }
      else {
          swapchainExtent = swapchain->surface_caps.currentExtent;
          os_window->app_data.iResolution[0] = swapchain->surface_caps.currentExtent.width;
          os_window->app_data.iResolution[1] = swapchain->surface_caps.currentExtent.height;
      }
    }
    else {
        printf("Error: too large resolution, currentExtent width, height: %lu, %lu; iResolution.xy: %lu, %lu; maxExtent width, height: %lu, %lu \n", 
        (unsigned long)swapchain->surface_caps.currentExtent.width, (unsigned long)swapchain->surface_caps.currentExtent.height,
        (unsigned long)os_window->app_data.iResolution[0], (unsigned long)os_window->app_data.iResolution[1],
        (unsigned long)format_properties.maxExtent.width, (unsigned long)format_properties.maxExtent.height);
        os_window->app_data.iResolution[0] = swapchain->surface_caps.currentExtent.width;
        os_window->app_data.iResolution[1] = swapchain->surface_caps.currentExtent.height;
        if (format_properties.maxExtent.width < swapchain->surface_caps.currentExtent.width)os_window->app_data.iResolution[0] = format_properties.maxExtent.width;
        if (format_properties.maxExtent.height < swapchain->surface_caps.currentExtent.height)os_window->app_data.iResolution[1] = format_properties.maxExtent.height;
        swapchainExtent.width = os_window->app_data.iResolution[0];
        swapchainExtent.height = os_window->app_data.iResolution[1];
    }

    if (os_window->app_data.iResolution[0] <= 0 || os_window->app_data.iResolution[1] <= 0) {
        os_window->is_minimized = true;
        return VK_ERROR_NONE;
    }
    else {
        os_window->is_minimized = false;
    }

    VkSwapchainCreateInfoKHR swapchain_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = NULL,
        .flags = VK_KHR_swapchain,
        .surface = swapchain->surface,
        .minImageCount = image_count,
        .imageFormat = swapchain->surface_format.format,
        .imageColorSpace = swapchain->surface_format.colorSpace,
        .imageExtent =
        {
            .width = swapchainExtent.width,
            .height = swapchainExtent.height,
        },
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        //.queueFamilyIndexCount = >0,
        //.pQueueFamilyIndices = not null,
        .preTransform = swapchain->surface_caps.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = *present_mode,
        .oldSwapchain = oldSwapchain,
        .clipped = true,
    };
    
    uint32_t *presentable_queues = NULL;
    uint32_t presentable_queue_count = 0;

    retval = vk_get_presentable_queues(phy_dev, dev, swapchain->surface, &presentable_queues, &presentable_queue_count);
    if (!vk_error_is_success(&retval) || presentable_queue_count == 0)
        return retval;
    free(presentable_queues);

    res = vkCreateSwapchainKHR(dev->device, &swapchain_info, NULL, &swapchain->swapchain);
    vk_error_set_vkresult(&retval, res);
    
    if (oldSwapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(dev->device, oldSwapchain, NULL);
    }

    return retval;
}

void vk_free_swapchain(VkInstance vk, struct vk_device *dev, struct vk_swapchain *swapchain)
{
    vkDestroySwapchainKHR(dev->device, swapchain->swapchain, NULL);
    vkDestroySurfaceKHR(vk, swapchain->surface, NULL);

    *swapchain = (struct vk_swapchain){0};
}

VkImage *vk_get_swapchain_images(struct vk_device *dev, struct vk_swapchain *swapchain, uint32_t *count)
{
    vk_error retval = VK_ERROR_NONE;
    VkResult res;

    uint32_t image_count;
    res = vkGetSwapchainImagesKHR(dev->device, swapchain->swapchain, &image_count, NULL);
    vk_error_set_vkresult(&retval, res);
    if (res < 0)
    {
        vk_error_printf(&retval, "Failed to count the number of images in swapchain\n");
        return NULL;
    }

    VkImage *images = malloc(image_count * sizeof *images);
    if (images == NULL)
    {
        printf("Out of memory\n");
        return NULL;
    }

    res = vkGetSwapchainImagesKHR(dev->device, swapchain->swapchain, &image_count, images);
    vk_error_set_vkresult(&retval, res);
    if (res < 0)
    {
        vk_error_printf(&retval, "Failed to get the images in swapchain\n");
        return NULL;
    }

    if (count)
        *count = image_count;
    return images;
}

vk_error vk_create_images(struct vk_physical_device *phy_dev, struct vk_device *dev,
        struct vk_image *images, uint32_t image_count)
{
    uint32_t successful = 0;
    vk_error retval = VK_ERROR_NONE;
    VkResult res;

    for (uint32_t i = 0; i < image_count; ++i)
    {
        images[i].image = NULL;
        images[i].image_mem = NULL;
        images[i].view = NULL;
        images[i].sampler = NULL;

        VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
        VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
        VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
        if (images[i].will_be_initialized || images[i].host_visible)
        {
            images[i].usage &= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            layout = VK_IMAGE_LAYOUT_PREINITIALIZED;
            tiling = VK_IMAGE_TILING_LINEAR;
        }
        else if (images[i].multisample)
        {
            VkImageFormatProperties format_properties;
            res = vkGetPhysicalDeviceImageFormatProperties(phy_dev->physical_device, images[i].format, VK_IMAGE_TYPE_2D,
                    tiling, images[i].usage, 0, &format_properties);
            vk_error_sub_set_vkresult(&retval, res);
            if (res == 0)
            {
                for (uint32_t s = VK_SAMPLE_COUNT_16_BIT; s != 0; s >>= 1)
                    if ((format_properties.sampleCounts & s))
                    {
                        samples = s;
                        break;
                    }
            }
        }
        
        uint32_t mipLevels=1;
        if(images[i].mipmaps){
            mipLevels = (int)(log(MAX(images[i].extent.width, images[i].extent.height))/log(2)) + 1;
        }
        
        bool shared = images[i].sharing_queue_count > 1;
        struct VkImageCreateInfo image_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = images[i].format,
            .extent = {images[i].extent.width, images[i].extent.height, 1},
            .mipLevels = mipLevels,
            .arrayLayers = 1,
            .samples = samples,
            .tiling = tiling,
            .usage = images[i].usage,
            .sharingMode = shared?VK_SHARING_MODE_CONCURRENT:VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = shared?images[i].sharing_queue_count:0,
            .pQueueFamilyIndices = shared?images[i].sharing_queues:NULL,
            .initialLayout = layout,
        };
        res = vkCreateImage(dev->device, &image_info, NULL, &images[i].image);
        vk_error_sub_set_vkresult(&retval, res);
        if (res)
            continue;

        VkMemoryRequirements mem_req = {0};
        vkGetImageMemoryRequirements(dev->device, images[i].image, &mem_req);
        uint32_t mem_index = vk_find_suitable_memory(phy_dev, dev, &mem_req,
                images[i].host_visible?
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT:
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        if (mem_index >= phy_dev->memories.memoryTypeCount)
            continue;

        VkMemoryAllocateInfo mem_info = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = mem_req.size,
            .memoryTypeIndex = mem_index,
        };

        res = vkAllocateMemory(dev->device, &mem_info, NULL, &images[i].image_mem);
        vk_error_sub_set_vkresult(&retval, res);
        if (res)
            continue;

        res = vkBindImageMemory(dev->device, images[i].image, images[i].image_mem, 0);
        vk_error_sub_set_vkresult(&retval, res);
        if (res)
            continue;

        if (images[i].make_view)
        {
            VkImageViewCreateInfo view_info = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = images[i].image,
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = images[i].format,
                .subresourceRange = {
                    .aspectMask = (images[i].usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) == 0?
                            VK_IMAGE_ASPECT_COLOR_BIT:
                            VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
                    .baseMipLevel = 0,
                    .levelCount = VK_REMAINING_MIP_LEVELS,
                    .baseArrayLayer = 0,
                    .layerCount = VK_REMAINING_ARRAY_LAYERS,
                },
            };

            res = vkCreateImageView(dev->device, &view_info, NULL, &images[i].view);
            vk_error_sub_set_vkresult(&retval, res);
            if (res)
                continue;
        }

        if ((images[i].usage & VK_IMAGE_USAGE_SAMPLED_BIT))
        {
            VkSamplerCreateInfo sampler_info = {
                .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                .magFilter = images[i].linear?VK_FILTER_LINEAR:VK_FILTER_NEAREST,
                .minFilter = images[i].linear?VK_FILTER_LINEAR:VK_FILTER_NEAREST,
                .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
                .addressModeU = images[i].repeat_mode,
                .addressModeV = images[i].repeat_mode,
                .addressModeW = images[i].repeat_mode,
                .anisotropyEnable = images[i].anisotropyEnable, // false
                .maxAnisotropy = phy_dev->properties.limits.maxSamplerAnisotropy,
                .minLod = 0,
                .maxLod = 1,
            };
            if(images[i].mipmaps){
                sampler_info.maxLod = mipLevels;
                sampler_info.mipLodBias = 0;
                sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
                sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
            }

            res = vkCreateSampler(dev->device, &sampler_info, NULL, &images[i].sampler);
            vk_error_sub_set_vkresult(&retval, res);
            if (res)
                continue;
        }

        ++successful;
    }

    vk_error_set_vkresult(&retval, successful == image_count?VK_SUCCESS:VK_INCOMPLETE);
    return retval;
}

vk_error vk_create_buffers(struct vk_physical_device *phy_dev, struct vk_device *dev,
        struct vk_buffer *buffers, uint32_t buffer_count)
{
    uint32_t successful = 0;
    vk_error retval = VK_ERROR_NONE;
    VkResult res;

    for (uint32_t i = 0; i < buffer_count; ++i)
    {
        buffers[i].buffer = NULL;
        buffers[i].buffer_mem = NULL;
        buffers[i].view = NULL;

        bool shared = buffers[i].sharing_queue_count > 1;
        VkBufferCreateInfo buffer_info = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = buffers[i].size * sizeof(float),
            .usage = buffers[i].usage,
            .sharingMode = shared?VK_SHARING_MODE_CONCURRENT:VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = shared?buffers[i].sharing_queue_count:0,
            .pQueueFamilyIndices = shared?buffers[i].sharing_queues:NULL,
        };
        res = vkCreateBuffer(dev->device, &buffer_info, NULL, &buffers[i].buffer);
        vk_error_sub_set_vkresult(&retval, res);
        if (res)
            continue;

        VkMemoryRequirements mem_req = {0};
        vkGetBufferMemoryRequirements(dev->device, buffers[i].buffer, &mem_req);
        uint32_t mem_index = vk_find_suitable_memory(phy_dev, dev, &mem_req,
                buffers[i].host_visible?
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT:
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        if (mem_index >= phy_dev->memories.memoryTypeCount)
            continue;

        VkMemoryAllocateInfo mem_info = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = mem_req.size,
            .memoryTypeIndex = mem_index,
        };

        res = vkAllocateMemory(dev->device, &mem_info, NULL, &buffers[i].buffer_mem);
        vk_error_sub_set_vkresult(&retval, res);
        if (res)
            continue;

        res = vkBindBufferMemory(dev->device, buffers[i].buffer, buffers[i].buffer_mem, 0);
        vk_error_sub_set_vkresult(&retval, res);
        if (res)
            continue;

        if (buffers[i].make_view)
        {
            if ((buffers[i].usage & VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT) || (buffers[i].usage & VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT))
            {
                VkBufferViewCreateInfo view_info = {
                    .sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
                    .buffer = buffers[i].buffer,
                    .format = buffers[i].format,
                    .offset = 0,
                    .range = VK_WHOLE_SIZE,
                };

                res = vkCreateBufferView(dev->device, &view_info, NULL, &buffers[i].view);
                vk_error_sub_set_vkresult(&retval, res);
                if (res)
                    continue;
            }
        }

        ++successful;
    }

    vk_error_set_vkresult(&retval, successful == buffer_count?VK_SUCCESS:VK_INCOMPLETE);
    return retval;
}

vk_error vk_load_shaders(struct vk_device *dev,
        struct vk_shader *shaders, uint32_t shader_count)
{
    uint32_t successful = 0;
    vk_error retval = VK_ERROR_NONE;
    vk_error err;

    for (uint32_t i = 0; i < shader_count; ++i)
    {
        err = vk_load_shader_spirv_file(dev, shaders[i].spirv_file, &shaders[i].shader);
        vk_error_sub_merge(&retval, &err);
        if (!vk_error_is_success(&err))
            continue;

        ++successful;
    }

    vk_error_set_vkresult(&retval, successful == shader_count?VK_SUCCESS:VK_INCOMPLETE);
    return retval;
}



vk_error vk_get_presentable_queues(struct vk_physical_device *phy_dev, struct vk_device *dev,
        VkSurfaceKHR surface, uint32_t **presentable_queues, uint32_t *presentable_queue_count)
{
    vk_error retval = VK_ERROR_NONE;
    VkResult res;

    *presentable_queues = malloc(dev->command_pool_count * sizeof **presentable_queues);
    if (*presentable_queues == NULL)
    {
        vk_error_set_errno(&retval, errno);
        return retval;
    }
    *presentable_queue_count = 0;

    for (uint32_t i = 0; i < dev->command_pool_count; ++i)
    {
        VkBool32 supports = false;
        res = vkGetPhysicalDeviceSurfaceSupportKHR(phy_dev->physical_device, i, surface, &supports);
        vk_error_sub_set_vkresult(&retval, res);
        if (res || !supports)
            continue;

        (*presentable_queues)[(*presentable_queue_count)++] = i;
    }

    if (*presentable_queue_count == 0)
    {
        free(*presentable_queues);
        *presentable_queues = NULL;
    }

    vk_error_set_vkresult(&retval, *presentable_queue_count == 0?VK_ERROR_INCOMPATIBLE_DRIVER:VK_SUCCESS);
    
    return retval;
}

VkFormat vk_get_supported_depth_stencil_format(struct vk_physical_device *phy_dev)
{
    VkFormat depth_formats[] = {
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_X8_D24_UNORM_PACK32,
        VK_FORMAT_D16_UNORM,
    };
    VkFormat selected_format = VK_FORMAT_UNDEFINED;

    for (size_t i = 0; i < sizeof depth_formats / sizeof *depth_formats; ++i)
    {
        VkFormatProperties format_properties;
        vkGetPhysicalDeviceFormatProperties(phy_dev->physical_device, depth_formats[i], &format_properties);
        if ((format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT))
        {
            selected_format = depth_formats[i];
            break;
        }
    }

    return selected_format;
}

void vk_free_images(struct vk_device *dev, struct vk_image *images, uint32_t image_count)
{
    vkDeviceWaitIdle(dev->device);

    for (uint32_t i = 0; i < image_count; ++i)
    {
        vkDestroyImageView(dev->device, images[i].view, NULL);
        vkDestroyImage(dev->device, images[i].image, NULL);
        vkFreeMemory(dev->device, images[i].image_mem, NULL);
        vkDestroySampler(dev->device, images[i].sampler, NULL);
    }
}

void vk_free_buffers(struct vk_device *dev, struct vk_buffer *buffers, uint32_t buffer_count)
{
    vkDeviceWaitIdle(dev->device);

    for (uint32_t i = 0; i < buffer_count; ++i)
    {
        vkDestroyBufferView(dev->device, buffers[i].view, NULL);
        vkDestroyBuffer(dev->device, buffers[i].buffer, NULL);
        vkFreeMemory(dev->device, buffers[i].buffer_mem, NULL);
    }
}

void vk_free_shaders(struct vk_device *dev, struct vk_shader *shaders, uint32_t shader_count)
{
    vkDeviceWaitIdle(dev->device);

    for (uint32_t i = 0; i < shader_count; ++i)
        vk_free_shader(dev, shaders[i].shader);
}

void vk_free_graphics_buffers(struct vk_device *dev, struct vk_graphics_buffers *graphics_buffers, uint32_t graphics_buffer_count,
        VkRenderPass render_pass)
{
    vkDeviceWaitIdle(dev->device);

    for (uint32_t i = 0; i < graphics_buffer_count; ++i)
    {
        vk_free_images(dev, &graphics_buffers[i].depth, 1);
        vkDestroyImageView(dev->device, graphics_buffers[i].color_view, NULL);

        vkDestroyFramebuffer(dev->device, graphics_buffers[i].framebuffer, NULL);
    }

    vkDestroyRenderPass(dev->device, render_pass, NULL);
}


vk_error vk_make_graphics_layouts(struct vk_device *dev, struct vk_layout *layouts, uint32_t layout_count,
         bool w_img_pattern, uint32_t *img_pattern, uint32_t img_pattern_size)
{
    uint32_t successful = 0;
    vk_error retval = VK_ERROR_NONE;
    VkResult res;

    for (uint32_t i = 0; i < layout_count; ++i)
    {
        struct vk_layout *layout = &layouts[i];
        struct vk_resources *resources = layout->resources;

        layout->set_layout = NULL;
        layout->pipeline_layout = NULL;

        VkDescriptorSetLayoutBinding *set_layout_bindings;
        if(w_img_pattern){
            set_layout_bindings = malloc((img_pattern_size + resources->buffer_count)*sizeof(VkDescriptorSetLayoutBinding));
        }else{
            set_layout_bindings = malloc((resources->image_count + resources->buffer_count)*sizeof(VkDescriptorSetLayoutBinding));
        }
        uint32_t binding_count = 0;
        uint32_t tidx=0;
        for (uint32_t j = 0; j < (w_img_pattern?img_pattern_size:resources->image_count); ++j)
        {
            if ((resources->images[(w_img_pattern?tidx:j)].usage & (VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)) == 0)
                continue;
            tidx=0;
            if(w_img_pattern){
                for(int tj=0;tj<j;tj++)tidx+=img_pattern[tj];
            }
            set_layout_bindings[binding_count] = (VkDescriptorSetLayoutBinding){
                .binding = w_img_pattern?tidx:binding_count,
                .descriptorType = resources->images[(w_img_pattern?tidx:j)].usage & VK_IMAGE_USAGE_SAMPLED_BIT?
                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                    VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                .descriptorCount = (w_img_pattern?img_pattern[j]:1),
                .stageFlags = resources->images[j].stage,
            };

            ++binding_count;
        }

        for (uint32_t j = 0; j < resources->buffer_count; ++j)
        {
            if ((resources->buffers[j].usage & (VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)) == 0)
                continue;
            if(w_img_pattern)tidx++;
            set_layout_bindings[binding_count] = (VkDescriptorSetLayoutBinding){
                .binding = w_img_pattern?tidx:binding_count,
                .descriptorType = resources->buffers[j].usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT?
                    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = 1,
                .stageFlags = resources->buffers[j].stage,
            };

            ++binding_count;
        }
        
        VkDescriptorSetLayoutCreateInfo set_layout_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = binding_count,
            .pBindings = set_layout_bindings,
        };

        res = vkCreateDescriptorSetLayout(dev->device, &set_layout_info, NULL, &layout->set_layout);
        vk_error_sub_set_vkresult(&retval, res);
        if (res) {
            free(set_layout_bindings);
            set_layout_bindings = NULL;
            continue;
        }

        VkPipelineLayoutCreateInfo pipeline_layout_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 1,
            .pSetLayouts = &layout->set_layout,
            .pushConstantRangeCount = resources->push_constant_count,
            .pPushConstantRanges = resources->push_constants,
        };

        res = vkCreatePipelineLayout(dev->device, &pipeline_layout_info, NULL, &layout->pipeline_layout);
        vk_error_sub_set_vkresult(&retval, res);
        if (res) {
            free(set_layout_bindings);
            set_layout_bindings = NULL;
            continue;
        }
        
        free(set_layout_bindings);
        set_layout_bindings = NULL;
        ++successful;
    }

    vk_error_set_vkresult(&retval, successful == layout_count?VK_SUCCESS:VK_INCOMPLETE);
    return retval;
}

vk_error vk_make_graphics_pipelines(struct vk_device *dev, struct vk_pipeline *pipelines, uint32_t pipeline_count, bool is_blend)
{
    uint32_t successful = 0;
    vk_error retval = VK_ERROR_NONE;
    VkResult res;

    for (uint32_t i = 0; i < pipeline_count; ++i)
    {
        struct vk_pipeline* pipeline = &pipelines[i];
        struct vk_layout* layout = pipeline->layout;
        struct vk_resources* resources = layout->resources;

        pipeline->pipeline = NULL;
        pipeline->set_pool = NULL;

        VkGraphicsPipelineCreateInfo pipeline_info;

        bool has_tessellation_shader = false;
        VkPipelineShaderStageCreateInfo* stage_info;
        stage_info = malloc(resources->shader_count * sizeof(VkPipelineShaderStageCreateInfo));
        for (uint32_t j = 0; j < resources->shader_count; ++j)
        {
            struct vk_shader* shader = &resources->shaders[j];

            stage_info[j] = (VkPipelineShaderStageCreateInfo){
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = shader->stage,
                .module = shader->shader,
                .pName = "main",
            };
            if (shader->stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT || shader->stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)
                has_tessellation_shader = true;
        }

        VkPipelineViewportStateCreateInfo viewport_state = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .scissorCount = 1,
        };
        VkPipelineRasterizationStateCreateInfo rasterization_state = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_BACK_BIT,
            .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
            .lineWidth = 1,
        };
        VkPipelineMultisampleStateCreateInfo multisample_state = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        };
        VkPipelineDepthStencilStateCreateInfo depth_stencil_state = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .depthTestEnable = true,
            .depthWriteEnable = true,
            .depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL,
        };
        VkPipelineColorBlendAttachmentState color_blend_attachments[1] = {
            [0] = {
                .blendEnable = is_blend,
                .colorWriteMask = VK_COLOR_COMPONENT_R_BIT
                        | VK_COLOR_COMPONENT_G_BIT
                        | VK_COLOR_COMPONENT_B_BIT
                        | VK_COLOR_COMPONENT_A_BIT,
            },
        };
        if(is_blend){
            color_blend_attachments[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            color_blend_attachments[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            color_blend_attachments[0].colorBlendOp = VK_BLEND_OP_ADD;
            color_blend_attachments[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            color_blend_attachments[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            color_blend_attachments[0].alphaBlendOp = VK_BLEND_OP_ADD;
        }
        VkPipelineColorBlendStateCreateInfo color_blend_state = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .attachmentCount = 1,
            .pAttachments = color_blend_attachments,
        };
        VkDynamicState dynamic_states[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        VkPipelineDynamicStateCreateInfo dynamic_state = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = 2,
            .pDynamicStates = dynamic_states,
        };

        pipeline_info = (VkGraphicsPipelineCreateInfo){
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .flags = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT,
            .stageCount = resources->shader_count,
            .pStages = stage_info,
            .pVertexInputState = &pipeline->vertex_input_state,
            .pInputAssemblyState = &pipeline->input_assembly_state,
            .pTessellationState = has_tessellation_shader ? &pipeline->tessellation_state : NULL,
            .pViewportState = &viewport_state,
            .pRasterizationState = &rasterization_state,
            .pMultisampleState = &multisample_state,
            .pDepthStencilState = &depth_stencil_state,
            .pColorBlendState = &color_blend_state,
            .pDynamicState = &dynamic_state,
            .layout = layout->pipeline_layout,
            .renderPass = resources->render_pass,
            .subpass = 0,
            .basePipelineIndex = 0,
        };

        res = vkCreateGraphicsPipelines(dev->device, NULL, 1, &pipeline_info, NULL, &pipeline->pipeline);
        vk_error_sub_set_vkresult(&retval, res);
        if (res) {
            free(stage_info);
            stage_info = NULL;
            continue;
        }

        uint32_t image_sampler_count = 0;
        uint32_t storage_image_count = 0;
        uint32_t uniform_buffer_count = 0;
        uint32_t storage_buffer_count = 0;

        for (uint32_t j = 0; j < resources->image_count; ++j)
        {
            if ((resources->images[j].usage & VK_IMAGE_USAGE_SAMPLED_BIT))
                ++image_sampler_count;
            else if ((resources->images[j].usage & VK_IMAGE_USAGE_SAMPLED_BIT))
                ++storage_image_count;
        }

        for (uint32_t j = 0; j < resources->buffer_count; ++j)
        {
            if ((resources->buffers[j].usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT))
                ++uniform_buffer_count;
            else if ((resources->buffers[j].usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT))
                ++storage_buffer_count;
        }

        uint32_t pool_size_count = 0;
        VkDescriptorPoolSize pool_sizes[4];
        if (image_sampler_count > 0)
            pool_sizes[pool_size_count++] = (VkDescriptorPoolSize){
                .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount = pipeline->thread_count * image_sampler_count,
            };

        if (storage_image_count > 0)
            pool_sizes[pool_size_count++] = (VkDescriptorPoolSize){
                .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                .descriptorCount = pipeline->thread_count * storage_image_count,
            };

        if (uniform_buffer_count > 0)
            pool_sizes[pool_size_count++] = (VkDescriptorPoolSize){
                .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = pipeline->thread_count * uniform_buffer_count,
            };

        if (storage_buffer_count > 0)
            pool_sizes[pool_size_count++] = (VkDescriptorPoolSize){
                .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = pipeline->thread_count * storage_image_count,
            };

        VkDescriptorPoolCreateInfo set_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = pipeline->thread_count,
            .poolSizeCount = pool_size_count,
            .pPoolSizes = pool_sizes,
        };

        res = vkCreateDescriptorPool(dev->device, &set_info, NULL, &pipeline->set_pool);
        vk_error_sub_set_vkresult(&retval, res);
        if (res) {
            free(stage_info);
            stage_info = NULL;
            continue;
        }
        
        free(stage_info);
        stage_info = NULL;
        ++successful;
    }

    vk_error_set_vkresult(&retval, successful == pipeline_count?VK_SUCCESS:VK_INCOMPLETE);
    return retval;
}

void vk_free_layouts(struct vk_device *dev, struct vk_layout *layouts, uint32_t layout_count)
{
    vkDeviceWaitIdle(dev->device);

    for (uint32_t i = 0; i < layout_count; ++i)
    {
        vkDestroyPipelineLayout(dev->device, layouts[i].pipeline_layout, NULL);
        vkDestroyDescriptorSetLayout(dev->device, layouts[i].set_layout, NULL);
    }
}

void vk_free_pipelines(struct vk_device *dev, struct vk_pipeline *pipelines, uint32_t pipeline_count)
{
    vkDeviceWaitIdle(dev->device);

    for (uint32_t i = 0; i < pipeline_count; ++i)
    {
        vkDestroyPipeline(dev->device, pipelines[i].pipeline, NULL);
        vkDestroyDescriptorPool(dev->device, pipelines[i].set_pool, NULL);
    }
}


static vk_error create_render_pass(struct vk_device *dev, VkFormat color_format, VkFormat depth_format, VkRenderPass *render_pass,
        enum vk_render_pass_load_op keeps_contents, enum vk_make_depth_buffer has_depth)
{
    vk_error retval = VK_ERROR_NONE;
    VkResult res;

    VkAttachmentDescription render_pass_attachments[2] = {
        [0] = {
            .format = color_format,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = keeps_contents?VK_ATTACHMENT_LOAD_OP_LOAD:VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        },
        [1] = {
            .format = depth_format,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = keeps_contents?VK_ATTACHMENT_LOAD_OP_LOAD:VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        },
    };
    VkAttachmentReference render_pass_attachment_references[2] = {
        [0] = {
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        },
        [1] = {
            .attachment = 1,
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        },
    };
    VkSubpassDescription render_pass_subpasses[1] = {
        [0] = {
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
            .pColorAttachments = &render_pass_attachment_references[0],
            .pDepthStencilAttachment = has_depth?&render_pass_attachment_references[1]:NULL,
        },
    };
    VkRenderPassCreateInfo render_pass_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = has_depth?2:1,
        .pAttachments = render_pass_attachments,
        .subpassCount = 1,
        .pSubpasses = render_pass_subpasses,
    };

    res = vkCreateRenderPass(dev->device, &render_pass_info, NULL, render_pass);
    vk_error_set_vkresult(&retval, res);

    return retval;
}

vk_error vk_create_offscreen_buffers(struct vk_physical_device *phy_dev, struct vk_device *dev, VkFormat format,
        struct vk_offscreen_buffers *offscreen_buffers, uint32_t offscreen_buffer_count, VkRenderPass *render_pass,
        enum vk_render_pass_load_op keeps_contents, enum vk_make_depth_buffer has_depth, bool linear)
{
    uint32_t successful = 0;
    vk_error retval = VK_ERROR_NONE;
    VkResult res;
    vk_error err;

    VkImageFormatProperties format_properties;
    res = vkGetPhysicalDeviceImageFormatProperties(phy_dev->physical_device, format, VK_IMAGE_TYPE_2D,
            VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 0, &format_properties);
    vk_error_sub_set_vkresult(&retval, res);
    if (res != VK_SUCCESS)return retval;

    for (uint32_t i = 0; i < offscreen_buffer_count; ++i)
    {
        offscreen_buffers[i].color= (struct vk_image){0};
        offscreen_buffers[i].depth = (struct vk_image){0};
        offscreen_buffers[i].framebuffer = NULL;
        if (format_properties.maxExtent.width < offscreen_buffers[i].surface_size.width){
          offscreen_buffers[i].surface_size.width=format_properties.maxExtent.width;
        }
        if (format_properties.maxExtent.height < offscreen_buffers[i].surface_size.height){
          offscreen_buffers[i].surface_size.height=format_properties.maxExtent.height;
        }
    }

    VkFormat depth_format = vk_get_supported_depth_stencil_format(phy_dev);

    retval = create_render_pass(dev, format, depth_format, render_pass, keeps_contents, has_depth);
    if (!vk_error_is_success(&retval))
        return retval;

    for (uint32_t i = 0; i < offscreen_buffer_count; ++i)
    {
        offscreen_buffers[i].color = (struct vk_image){
            .format = format,
            .extent = offscreen_buffers[i].surface_size,
            .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
                | VK_IMAGE_USAGE_SAMPLED_BIT,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .make_view = true,
            .anisotropyEnable = true,
            .repeat_mode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .mipmaps = false,
            .linear = linear,
        };
        err = vk_create_images(phy_dev, dev, &offscreen_buffers[i].color, 1);
        vk_error_sub_merge(&retval, &err);
        if (!vk_error_is_success(&err))
            continue;

        if (has_depth)
        {
            offscreen_buffers[i].depth = (struct vk_image){
                .format = depth_format,
                .extent = offscreen_buffers[i].surface_size,
                .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                .make_view = true,
                .anisotropyEnable = true,
                .repeat_mode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                .mipmaps = false,
                .linear = false,
            };

            err = vk_create_images(phy_dev, dev, &offscreen_buffers[i].depth, 1);
            vk_error_sub_merge(&retval, &err);
            if (!vk_error_is_success(&err))
                continue;
        }

        VkImageView framebuffer_attachments[2] = {
            offscreen_buffers[i].color.view,
            offscreen_buffers[i].depth.view,
        };
        VkFramebufferCreateInfo framebuffer_info = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = *render_pass,
            .attachmentCount = has_depth?2:1,
            .pAttachments = framebuffer_attachments,
            .width = offscreen_buffers[i].surface_size.width,
            .height = offscreen_buffers[i].surface_size.height,
            .layers = 1,
        };

        res = vkCreateFramebuffer(dev->device, &framebuffer_info, NULL, &offscreen_buffers[i].framebuffer);
        vk_error_sub_set_vkresult(&retval, res);
        if (res)
            continue;

        ++successful;
    }

    vk_error_set_vkresult(&retval, successful == offscreen_buffer_count?VK_SUCCESS:VK_INCOMPLETE);
    return retval;
}

vk_error vk_create_graphics_buffers(struct vk_physical_device *phy_dev, struct vk_device *dev, VkFormat format,
        struct vk_graphics_buffers *graphics_buffers, uint32_t graphics_buffer_count, VkRenderPass *render_pass,
        enum vk_render_pass_load_op keeps_contents, enum vk_make_depth_buffer has_depth)
{
    uint32_t successful = 0;
    vk_error retval = VK_ERROR_NONE;
    VkResult res;
    vk_error err;

    VkImageFormatProperties format_properties;
    res = vkGetPhysicalDeviceImageFormatProperties(phy_dev->physical_device, format, VK_IMAGE_TYPE_2D,
            VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 0, &format_properties);
    vk_error_sub_set_vkresult(&retval, res);
    if (res != VK_SUCCESS)return retval;

    for (uint32_t i = 0; i < graphics_buffer_count; ++i)
    {
        graphics_buffers[i].color_view = NULL;
        graphics_buffers[i].depth = (struct vk_image){0};
        graphics_buffers[i].framebuffer = NULL;
        if (format_properties.maxExtent.width < graphics_buffers[i].surface_size.width){
          graphics_buffers[i].surface_size.width=format_properties.maxExtent.width;
        }
        if (format_properties.maxExtent.height < graphics_buffers[i].surface_size.height){
          graphics_buffers[i].surface_size.height=format_properties.maxExtent.height;
        }
    }

    VkFormat depth_format = vk_get_supported_depth_stencil_format(phy_dev);;

    retval = create_render_pass(dev, format, depth_format, render_pass, keeps_contents, has_depth);
    if (!vk_error_is_success(&retval))
        return retval;

    for (uint32_t i = 0; i < graphics_buffer_count; ++i)
    {
        VkImageViewCreateInfo view_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = graphics_buffers[i].swapchain_image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = format,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = VK_REMAINING_MIP_LEVELS,
                .baseArrayLayer = 0,
                .layerCount = VK_REMAINING_ARRAY_LAYERS,
            },
        };

        res = vkCreateImageView(dev->device, &view_info, NULL, &graphics_buffers[i].color_view);
        vk_error_sub_set_vkresult(&retval, res);
        if (res)
            continue;

        if (has_depth)
        {
            graphics_buffers[i].depth = (struct vk_image){
                .format = depth_format,
                .extent = graphics_buffers[i].surface_size,
                .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                .make_view = true,
                .multisample = false,
                .will_be_initialized = false,
                .anisotropyEnable = true,
                .repeat_mode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                .mipmaps = false,
                .linear = false,
            };

            err = vk_create_images(phy_dev, dev, &graphics_buffers[i].depth, 1);
            vk_error_sub_merge(&retval, &err);
            if (!vk_error_is_success(&err))
                continue;
        }

        VkImageView framebuffer_attachments[2] = {
            graphics_buffers[i].color_view,
            graphics_buffers[i].depth.view,
        };
        VkFramebufferCreateInfo framebuffer_info = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = *render_pass,
            .attachmentCount = has_depth?2:1,
            .pAttachments = framebuffer_attachments,
            .width = graphics_buffers[i].surface_size.width,
            .height = graphics_buffers[i].surface_size.height,
            .layers = 1,
        };

        res = vkCreateFramebuffer(dev->device, &framebuffer_info, NULL, &graphics_buffers[i].framebuffer);
        vk_error_sub_set_vkresult(&retval, res);
        if (res)
            continue;

        ++successful;
    }

    vk_error_set_vkresult(&retval, successful == graphics_buffer_count?VK_SUCCESS:VK_INCOMPLETE);
    return retval;
}

void vk_free_offscreen_buffers(struct vk_device *dev, struct vk_offscreen_buffers *offscreen_buffers, uint32_t graphics_buffer_count,
        VkRenderPass render_pass)
{
    vkDeviceWaitIdle(dev->device);

    for (uint32_t i = 0; i < graphics_buffer_count; ++i)
    {
        vk_free_images(dev, &offscreen_buffers[i].color, 1);
        vk_free_images(dev, &offscreen_buffers[i].depth, 1);

        vkDestroyFramebuffer(dev->device, offscreen_buffers[i].framebuffer, NULL);
    }

    vkDestroyRenderPass(dev->device, render_pass, NULL);
}








