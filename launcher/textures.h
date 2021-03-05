
static vk_error init_texture_mem(struct vk_physical_device *phy_dev, struct vk_device *dev, struct vk_render_essentials *essentials,
                             struct vk_image *image, uint8_t *texture, int width, int height, const char *name, bool mipmaps, bool linear)
{
    vk_error retval = VK_ERROR_NONE;
    VkFormat img_format=VK_FORMAT_R8G8B8A8_UNORM; //VK_FORMAT_R8G8B8A8_SRGB
    *image = (struct vk_image) {
        .format = img_format,
        .extent = { .width = width, .height = height },
        .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .make_view = true,
        .host_visible = false,
        .anisotropyEnable = true,
        .repeat_mode = VK_SAMPLER_ADDRESS_MODE_REPEAT, //VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER //VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
        .mipmaps = mipmaps,
        .linear = linear||mipmaps,
    };

    retval = vk_create_images(phy_dev, dev, image, 1);
    if (!vk_error_is_success(&retval))
    {
        retval.error.type=VK_ERROR_ERRNO;
        vk_error_printf(&retval, "Failed to create texture images\n");
        return retval;
    }
    retval = vk_render_init_texture(phy_dev, dev, essentials, image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, texture, name);
    return retval;
}

#ifdef USE_stb_image
static vk_error init_texture_file(struct vk_physical_device *phy_dev, struct vk_device *dev, struct vk_render_essentials *essentials,
                             struct vk_image *image, const char *name, bool mipmaps)
{
    vk_error retval = VK_ERROR_NONE;
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true); //flip image Y
    uint8_t *generated_texture = stbi_load(name, &width, &height, &channels, STBI_rgb_alpha);
    if(generated_texture == NULL) {
        retval.error.type=VK_ERROR_ERRNO;
        printf("Error in loading image\n");
        return retval;
    }
    
    retval = init_texture_mem(phy_dev, dev, essentials, image, generated_texture, width, height, name, mipmaps, true);
    stbi_image_free(generated_texture);
    return retval;
}
#endif

static vk_error texture_empty(struct vk_physical_device *phy_dev, struct vk_device *dev, struct vk_render_essentials *essentials, struct vk_image *image, int width, int height)
{
    vk_error retval = VK_ERROR_NONE;
    size_t texture_size = width * height * 4 * sizeof(uint8_t);
    uint8_t *generated_texture = malloc(texture_size);
    if(generated_texture == NULL) {
        retval.error.type=VK_ERROR_ERRNO;
        printf("Error in allocating memory\n");
        return retval;
    }
    for (unsigned int i = 0; i < height; ++i){
        for (unsigned int j = 0; j < width; ++j){
            size_t pixel = (i * width + j) * 4 * sizeof(uint8_t);
            generated_texture[pixel + 0] = 0x00;
            generated_texture[pixel + 1] = 0x00;
            generated_texture[pixel + 2] = 0x00;
            generated_texture[pixel + 3] = 0x00;
        }
    }
    retval = init_texture_mem(phy_dev, dev, essentials, image, generated_texture, width, height, "empty", false, false);
    free(generated_texture);
    return retval;
}
