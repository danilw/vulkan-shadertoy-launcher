
vk_error vk_render_transition_images_screenshot_swapchain_begin(struct vk_device *dev, struct vk_render_essentials *essentials,
                        struct vk_image *srcImage, struct vk_image *dstImage){
    vk_error retval = VK_ERROR_NONE;
    VkResult res;
    
    vkResetCommandBuffer(essentials->cmd_buffer, 0);
    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    res = vkBeginCommandBuffer(essentials->cmd_buffer, &begin_info);
    vk_error_set_vkresult(&retval, res);
    if (res)
    {
        vk_error_printf(&retval, "Couldn't begin recording a command buffer to screenshot image\n");
        return retval;
    }

    VkImageMemoryBarrier image_barrier_dstImage = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .image = dstImage->image,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = VK_REMAINING_MIP_LEVELS,
            .baseArrayLayer = 0,
            .layerCount = VK_REMAINING_ARRAY_LAYERS,
        },
    };
    
    vkCmdPipelineBarrier(essentials->cmd_buffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, NULL,
            0, NULL,
            1, &image_barrier_dstImage);
    
    VkImageMemoryBarrier image_barrier_srcImage = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .image = srcImage->image,
        .srcAccessMask = VK_ACCESS_MEMORY_READ_BIT,
        .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = VK_REMAINING_MIP_LEVELS,
            .baseArrayLayer = 0,
            .layerCount = VK_REMAINING_ARRAY_LAYERS,
        },
    };
    
    vkCmdPipelineBarrier(essentials->cmd_buffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, NULL,
            0, NULL,
            1, &image_barrier_srcImage);
    
    vkEndCommandBuffer(essentials->cmd_buffer);

    res = vkResetFences(dev->device, 1, &essentials->exec_fence);
    vk_error_set_vkresult(&retval, res);
    if (res)
    {
        vk_error_printf(&retval, "Failed to reset fence\n");
        return retval;
    }

    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &essentials->cmd_buffer,
    };

    vkQueueSubmit(essentials->present_queue, 1, &submit_info, essentials->exec_fence);
    res = vkWaitForFences(dev->device, 1, &essentials->exec_fence, true, 1000000000);
    vk_error_set_vkresult(&retval, res);
    if (res)
    {
        vk_error_printf(&retval, "Failed to reset fence\n");
        return retval;
    }
    
    return retval;
}

vk_error vk_render_transition_images_screenshot_swapchain_end(struct vk_device *dev, struct vk_render_essentials *essentials,
                        struct vk_image *srcImage, struct vk_image *dstImage){
    vk_error retval = VK_ERROR_NONE;
    VkResult res;
    
    vkResetCommandBuffer(essentials->cmd_buffer, 0);
    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    res = vkBeginCommandBuffer(essentials->cmd_buffer, &begin_info);
    vk_error_set_vkresult(&retval, res);
    if (res)
    {
        vk_error_printf(&retval, "Couldn't begin recording a command buffer to screenshot image\n");
        return retval;
    }

    VkImageMemoryBarrier image_barrier_dstImage = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .image = dstImage->image,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_GENERAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = VK_REMAINING_MIP_LEVELS,
            .baseArrayLayer = 0,
            .layerCount = VK_REMAINING_ARRAY_LAYERS,
        },
    };
    
    vkCmdPipelineBarrier(essentials->cmd_buffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, NULL,
            0, NULL,
            1, &image_barrier_dstImage);
    
    VkImageMemoryBarrier image_barrier_srcImage = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .image = srcImage->image,
        .srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
        .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = VK_REMAINING_MIP_LEVELS,
            .baseArrayLayer = 0,
            .layerCount = VK_REMAINING_ARRAY_LAYERS,
        },
    };
    
    vkCmdPipelineBarrier(essentials->cmd_buffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, NULL,
            0, NULL,
            1, &image_barrier_srcImage);
    
    vkEndCommandBuffer(essentials->cmd_buffer);

    res = vkResetFences(dev->device, 1, &essentials->exec_fence);
    vk_error_set_vkresult(&retval, res);
    if (res)
    {
        vk_error_printf(&retval, "Failed to reset fence\n");
        return retval;
    }

    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &essentials->cmd_buffer,
    };

    vkQueueSubmit(essentials->present_queue, 1, &submit_info, essentials->exec_fence);
    res = vkWaitForFences(dev->device, 1, &essentials->exec_fence, true, 1000000000);
    vk_error_set_vkresult(&retval, res);
    if (res)
    {
        vk_error_printf(&retval, "Failed to reset fence\n");
        return retval;
    }
    
    return retval;
}

// RGBA BMP from https://en.wikipedia.org/wiki/BMP_file_format
unsigned char ev(uint32_t v) {
  static uint32_t counter = 0;
  return (unsigned char)((v) >> ((8*(counter++))%32));
}
void write_bmp(uint32_t w, uint32_t h, uint8_t *rgba) {
    FILE *f;
    unsigned char *img = NULL;
    uint32_t x, y;
    uint32_t filesize = 108 + 14 + 4 * w*h;
    static int scr_id = 0;

    img = (unsigned char *) malloc(4 * w * h);
    memset(img, 0, 4 * w * h);
    
    for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
            img[(x + y * w)*4 + 2] = rgba[(x+y*w)*4+0];
            img[(x + y * w)*4 + 1] = rgba[(x+y*w)*4+1];
            img[(x + y * w)*4 + 0] = rgba[(x+y*w)*4+2];
            img[(x + y * w)*4 + 3] = rgba[(x+y*w)*4+3];
        }
    }
    
    unsigned char bmpfileheader[14] = {'B','M', ev(filesize),ev(filesize),ev(filesize),ev(filesize), 0,0,0,0, 108+14, 0, 0, 0};

    unsigned char bmpinfoheader[108] = {108,0,0,0, 
                                        ev(w),ev(w),ev(w),ev(w), ev(h),ev(h),ev(h),ev(h), 1,0, 32,0, 3,0,0,0, ev(w*h*4),ev(w*h*4),ev(w*h*4),ev(w*h*4),
                                        ev(0x0b13),ev(0x0b13),ev(0x0b13),ev(0x0b13), ev(0x0b13),ev(0x0b13),ev(0x0b13),ev(0x0b13),
                                        0,0,0,0, 0,0,0,0, 
                                        0,0,0,0, 0,0,0,0, 
                                        0,0,0,0xff, 0,0,0xff,0, 0,0xff,0,0, 0xff,0,0,0,
                                        ev(0x57696E20),ev(0x57696E20),ev(0x57696E20),ev(0x57696E20), 
                                        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 
      };
    
    char file_name[80] = {0};
    sprintf(file_name, "screenshot_%d.png", scr_id++);
    
    f = fopen(file_name, "wb");
    fwrite(bmpfileheader, 1, 14, f);
    fwrite(bmpinfoheader, 1, 108, f);
    for (int i = 0; i < h; i++) {
        fwrite(img + (w * (h - i - 1) * 4), 4, w, f);
    }

    free(img);
    fclose(f);
}

vk_error make_screenshot(struct vk_physical_device *phy_dev, struct vk_device *dev, struct vk_swapchain *swapchain, struct vk_render_essentials *essentials, uint32_t image_index)
{
    vk_error retval = VK_ERROR_NONE;
    VkResult res;
    
    if (!essentials->first_render)
    {
        res = vkWaitForFences(dev->device, 1, &essentials->exec_fence, true, 1000000000);
        vk_error_set_vkresult(&retval, res);
        if (res)
        {
            vk_error_printf(&retval, "Wait for fence failed\n");
            return retval;
        }
    }
    
    uint32_t support_format_list[4] = {VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_SRGB, VK_FORMAT_R8G8B8A8_UNORM};
    bool supported=false;
    for (int i = 0; (i < 4)&&(!supported); i++){
        if(swapchain->surface_format.format==support_format_list[i])supported=true;
    }
    supported &= swapchain->surface_caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    
    if(!supported){
        vk_error_printf(&retval, "Can not save screenshot, surface has unique format or not supported transfer %lu\n", 
                                (unsigned long)swapchain->surface_format.format);
        return retval;
    }
    
    struct vk_image srcImage = {
        .image = essentials->images[image_index],
    };
    
    VkFormat img_format=VK_FORMAT_R8G8B8A8_UNORM; //VK_FORMAT_R8G8B8A8_SRGB
    struct vk_image dstImage = {
        .format = img_format,
        .extent = render_data.main_gbuffers[image_index].surface_size,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .make_view = false,
        .host_visible = true,
        .anisotropyEnable = true,
        .repeat_mode = VK_SAMPLER_ADDRESS_MODE_REPEAT, //VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER //VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
        .mipmaps = false,
        .linear = true,
    };

    retval = vk_create_images(phy_dev, dev, &dstImage, 1);
    if (!vk_error_is_success(&retval))
    {
        vk_error_printf(&retval, "Failed to create dstImage for screenshot\n");
        return retval;
    }
    
    retval = vk_render_transition_images_screenshot_swapchain_begin(dev, essentials, &srcImage, &dstImage);
    if (!vk_error_is_success(&retval))
    {
        return retval;
    }
    
    VkImageCopy imageCopyRegion = {
      .srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .srcSubresource.layerCount = 1,
      .dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .dstSubresource.layerCount = 1,
      .extent.width = dstImage.extent.width,
      .extent.height = dstImage.extent.height,
      .extent.depth = 1,
    };
    
    retval = vk_render_copy_image(dev, essentials, &dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, &imageCopyRegion, "screenshot");
    if (!vk_error_is_success(&retval))
    {
        vk_error_printf(&retval, "Failed to copy image for screenshot\n");
        return retval;
    }
    
    retval = vk_render_transition_images_screenshot_swapchain_end(dev, essentials, &srcImage, &dstImage);
    if (!vk_error_is_success(&retval))
    {
        return retval;
    }
    
    VkImageSubresource subResource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0 };
    VkSubresourceLayout subResourceLayout;
    vkGetImageSubresourceLayout(dev->device, dstImage.image, &subResource, &subResourceLayout);
    
    uint8_t* data;
    vkMapMemory(dev->device, dstImage.image_mem, 0, VK_WHOLE_SIZE, 0, (void**)&data);
    data += subResourceLayout.offset;
    
    int color_order[3] = {0, 1, 2};
    
    if(swapchain->surface_format.format==VK_FORMAT_B8G8R8A8_SRGB || swapchain->surface_format.format==VK_FORMAT_B8G8R8A8_UNORM){
        color_order[0]=2; color_order[1]=1; color_order[2]=0; 
    }
    
    uint8_t *data_rgba;
    data_rgba = (uint8_t*) malloc(4 * dstImage.extent.width * dstImage.extent.height);
    for (uint32_t y = 0; y < dstImage.extent.height; y++)
    {
        uint8_t *row = (uint8_t*)data;
        for (uint32_t x = 0; x < dstImage.extent.width; x++)
        {
            data_rgba[(x+y*dstImage.extent.width)*4+0] = (uint8_t)row[x*4+color_order[0]];
            data_rgba[(x+y*dstImage.extent.width)*4+1] = (uint8_t)row[x*4+color_order[1]];
            data_rgba[(x+y*dstImage.extent.width)*4+2] = (uint8_t)row[x*4+color_order[2]];
            data_rgba[(x+y*dstImage.extent.width)*4+3] = (uint8_t)row[x*4+3];
        }
        data += subResourceLayout.rowPitch;
    }
    
    write_bmp(dstImage.extent.width, dstImage.extent.height, data_rgba);
    
    printf("screenshot done\n");
    
    free(data_rgba);
    vkUnmapMemory(dev->device, dstImage.image_mem);
    vk_free_images(dev, &dstImage, 1);
    
    return retval;
}
