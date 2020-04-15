
// Danil, 2020 Vulkan shader launcher, self https://github.com/danilw/vulkan-shader-launcher
// code based on Shabi's Vulkan Tutorials https://github.com/ShabbyX/vktut

// original license
/*
 * Copyright (C) 2016 Shahbaz Youssefi <ShabbyX@gmail.com>
 *
 * This file is part of Shabi's Vulkan Tutorials.
 *
 * Shabi's Vulkan Tutorials is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Shabi's Vulkan Tutorials is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Shabi's Vulkan Tutorials.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef vk_utils_H
#define vk_utils_H

#include <vulkan/vulkan.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <math.h>
#include "vk_error_print.h"
#if defined(VK_USE_PLATFORM_XCB_KHR)
#include <X11/Xutil.h>
#include <X11/Xlib-xcb.h>
#endif
#include "vk_struct.h"

#ifdef YARIV_SHADER
#include "../yariv/yariv.h"
#endif

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

void vk_exit(VkInstance vk);

vk_error vk_enumerate_devices(VkInstance vk, struct vk_physical_device *devs, uint32_t *count);

vk_error vk_get_commands(struct vk_physical_device *phy_dev, struct vk_device *dev, VkDeviceQueueCreateInfo queue_info[], uint32_t queue_info_count, uint32_t create_count);

void vk_cleanup(struct vk_device *dev);

vk_error vk_load_shader(struct vk_device *dev, const uint32_t *code, VkShaderModule *shader, size_t size);
#ifdef YARIV_SHADER
vk_error vk_load_shader_yariv(struct vk_device *dev, const uint32_t *yariv_code, VkShaderModule *shader, size_t in_yariv_size);
#endif
vk_error vk_load_shader_spirv_file(struct vk_device *dev, const char *spirv_file, VkShaderModule *shader);
void vk_free_shader(struct vk_device *dev, VkShaderModule shader);

uint32_t vk_find_suitable_memory(struct vk_physical_device *phy_dev, struct vk_device *dev,
        VkMemoryRequirements *mem_req, VkMemoryPropertyFlags properties);

vk_error vk_init_ext(VkInstance *vk, const char *ext_names[], uint32_t ext_count);
vk_error vk_get_dev_ext(struct vk_physical_device *phy_dev, struct vk_device *dev, VkQueueFlags qflags,
        VkDeviceQueueCreateInfo queue_info[], uint32_t *queue_info_count,
        const char *ext_names[], uint32_t ext_count);

static inline vk_error vk_init(VkInstance *vk)
{
    const char *extension_names[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
#if defined(VK_USE_PLATFORM_WIN32_KHR)
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif defined(VK_USE_PLATFORM_XCB_KHR)
        VK_KHR_XCB_SURFACE_EXTENSION_NAME,
#endif
    };
    return vk_init_ext(vk, extension_names, sizeof extension_names / sizeof *extension_names);
}
static inline vk_error vk_get_dev(struct vk_physical_device *phy_dev, struct vk_device *dev, VkQueueFlags qflags,
        VkDeviceQueueCreateInfo queue_info[], uint32_t *queue_info_count)
{
    const char *extension_names[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };
    return vk_get_dev_ext(phy_dev, dev, qflags, queue_info, queue_info_count, extension_names,
            sizeof extension_names / sizeof *extension_names);
}

static vk_error vk_setup(struct vk_physical_device *phy_dev, struct vk_device *dev, VkQueueFlags qflags, uint32_t create_count)
{
    VkDeviceQueueCreateInfo queue_info[VK_MAX_QUEUE_FAMILY];
    uint32_t queue_info_count = 0;
    
    queue_info_count = phy_dev->queue_family_count;
    vk_error res = vk_get_dev(phy_dev, dev, qflags, queue_info, &queue_info_count);
    if (vk_error_is_success(&res))
    {
        if(create_count<=queue_info[0].queueCount)create_count=0; //0=create one cmd_buffer per Queue
        res = vk_get_commands(phy_dev, dev, queue_info, queue_info_count, create_count);
    }
    return res;

}

VkResult vk_create_surface(VkInstance vk, struct vk_swapchain *swapchain, struct app_os_window *os_window);

vk_error vk_get_swapchain(VkInstance vk, struct vk_physical_device *phy_dev, struct vk_device *dev,
        struct vk_swapchain *swapchain, struct app_os_window *os_window, uint32_t thread_count, VkPresentModeKHR *present_mode);
void vk_free_swapchain(VkInstance vk, struct vk_device *dev, struct vk_swapchain *swapchain);

VkImage *vk_get_swapchain_images(struct vk_device *dev, struct vk_swapchain *swapchain, uint32_t *count);

vk_error vk_create_images(struct vk_physical_device *phy_dev, struct vk_device *dev,
        struct vk_image *images, uint32_t image_count);
vk_error vk_create_buffers(struct vk_physical_device *phy_dev, struct vk_device *dev,
        struct vk_buffer *buffers, uint32_t buffer_count);
vk_error vk_load_shaders(struct vk_device *dev,
        struct vk_shader *shaders, uint32_t shader_count);
vk_error vk_get_presentable_queues(struct vk_physical_device *phy_dev, struct vk_device *dev,
        VkSurfaceKHR surface, uint32_t **presentable_queues, uint32_t *presentable_queue_count);
VkFormat vk_get_supported_depth_stencil_format(struct vk_physical_device *phy_dev);

void vk_free_images(struct vk_device *dev, struct vk_image *images, uint32_t image_count);
void vk_free_buffers(struct vk_device *dev, struct vk_buffer *buffers, uint32_t buffer_count);
void vk_free_shaders(struct vk_device *dev, struct vk_shader *shaders, uint32_t shader_count);
void vk_free_graphics_buffers(struct vk_device *dev, struct vk_graphics_buffers *graphics_buffers, uint32_t graphics_buffer_count,
        VkRenderPass render_pass);

vk_error vk_make_graphics_layouts(struct vk_device *dev, struct vk_layout *layouts, uint32_t layout_count, bool w_img_pattern, uint32_t *img_pattern, uint32_t img_pattern_size);
vk_error vk_make_graphics_pipelines(struct vk_device *dev, struct vk_pipeline *pipelines, uint32_t pipeline_count, bool is_blend);

void vk_free_layouts(struct vk_device *dev, struct vk_layout *layouts, uint32_t layout_count);
void vk_free_pipelines(struct vk_device *dev, struct vk_pipeline *pipelines, uint32_t pipeline_count);

vk_error vk_create_offscreen_buffers(struct vk_physical_device *phy_dev, struct vk_device *dev, VkFormat format,
        struct vk_offscreen_buffers *offscreen_buffers, uint32_t offscreen_buffer_count, VkRenderPass *render_pass,
        enum vk_render_pass_load_op keeps_contents, enum vk_make_depth_buffer has_depth, bool linear);
vk_error vk_create_graphics_buffers(struct vk_physical_device *phy_dev, struct vk_device *dev, VkFormat format,
        struct vk_graphics_buffers *graphics_buffers, uint32_t graphics_buffer_count, VkRenderPass *render_pass,
        enum vk_render_pass_load_op keeps_contents, enum vk_make_depth_buffer has_depth);

void vk_free_offscreen_buffers(struct vk_device *dev, struct vk_offscreen_buffers *offscreen_buffers, uint32_t offscreen_buffer_count,
        VkRenderPass render_pass);

#endif
