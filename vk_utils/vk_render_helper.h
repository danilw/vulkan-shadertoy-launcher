
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

#ifndef vk_render_helper_H
#define vk_render_helper_H

#include "vk_utils.h"

int vk_render_get_essentials(struct vk_render_essentials *essentials, struct vk_physical_device *phy_dev,
        struct vk_device *dev, struct vk_swapchain *swapchain);
void vk_render_cleanup_essentials(struct vk_render_essentials *essentials, struct vk_device *dev);

/*
 * Acquire an image from the swapchain, reset the command buffer, start recording, perform layout transition from
 * undefined to to_layout.
 */
int vk_render_start(struct vk_render_essentials *essentials, struct vk_device *dev,
        struct vk_swapchain *swapchain, VkImageLayout to_layout, uint32_t *image_index);


/* Fill the contents of a host-visible buffer/image with arbitrary data */
vk_error vk_render_fill_buffer(struct vk_device *dev, struct vk_buffer *to, void *from, size_t size, const char *name);
vk_error vk_render_fill_image(struct vk_device *dev, struct vk_image *to, void *from, size_t size, const char *name);

/*
 * Copy a buffer/image to another, for example from a host-visible one to a device-local one.  This uses a command
 * buffer, submits it, and waits for it to finish, so it's not supposed to be used while recording a command buffer.
 */
vk_error vk_render_copy_buffer(struct vk_device *dev, struct vk_render_essentials *essentials,
        struct vk_buffer *to, struct vk_buffer *from, size_t size, const char *name);
vk_error vk_render_copy_image(struct vk_device *dev, struct vk_render_essentials *essentials,
        struct vk_image *to, VkImageLayout to_layout, struct vk_image *from, VkImageLayout from_layout,
        VkImageCopy *region, const char *name);
vk_error vk_render_copy_buffer_to_image(struct vk_device *dev, struct vk_render_essentials *essentials,
        struct vk_image *to, VkImageLayout to_layout, struct vk_buffer *from,
        VkBufferImageCopy *region, const char *name);
vk_error vk_render_copy_image_to_buffer(struct vk_device *dev, struct vk_render_essentials *essentials,
        struct vk_buffer *to, struct vk_image *from, VkImageLayout from_layout,
        VkBufferImageCopy *region, const char *name);

vk_error create_staging_buffer(struct vk_physical_device *phy_dev, struct vk_device *dev, struct vk_render_essentials *essentials,
        struct vk_buffer *staging, uint8_t *contents, size_t size, const char *name);

// use map and unmap memory every time, not optimized
vk_error vk_render_update_texture(struct vk_physical_device *phy_dev, struct vk_device *dev, struct vk_render_essentials *essentials,
        struct vk_image *image, VkImageLayout base_layout, uint8_t *contents, const char *name);

/*
 * Transition an image to a new layout.  This uses a command buffer, submits it, and waits for it to finish, so it's
 * not supposed to be used while recording a command buffer.
 */
vk_error vk_render_transition_images(struct vk_device *dev, struct vk_render_essentials *essentials,
        struct vk_image *images, uint32_t image_count,
        VkImageLayout from, VkImageLayout to, VkImageAspectFlags aspect, const char *name);

vk_error vk_render_transition_images_mipmaps(struct vk_physical_device *phy_dev, struct vk_device *dev, struct vk_render_essentials *essentials,
        struct vk_image *image, VkImageAspectFlags aspect, const char *name);

/*
 * Create a texture image.  This uses a command buffer, submits it, and waits for it to finish,
 * so it's not supposed to be used while recording a command buffer.  It creates and destroys a staging buffer in the
 * process.  In the end, it transitions the image to the desired layout.
 */
vk_error vk_render_init_texture(struct vk_physical_device *phy_dev, struct vk_device *dev, struct vk_render_essentials *essentials,
        struct vk_image *image, VkImageLayout layout, uint8_t *contents, const char *name);

/*
 * Copy over arbitrary data to the buffer.  This uses a command buffer, submits it, and waits for it to finish, so it's
 * not supposed to be used while recording a command buffer.  It creates and destroys a staging buffer in the process.
 */
vk_error vk_render_init_buffer(struct vk_physical_device *phy_dev, struct vk_device *dev, struct vk_render_essentials *essentials,
        struct vk_buffer *buffer, void *contents, const char *name);


/*
 * allow additional wait and signal semaphores so the submission will be
 * synchronized with off-screen renders as well.
 */
int vk_render_finish(struct vk_render_essentials *essentials, struct vk_device *dev,
        struct vk_swapchain *swapchain, VkImageLayout from_layout, uint32_t image_index,
        VkSemaphore wait_sem, VkSemaphore signal_sem);
        

#endif
