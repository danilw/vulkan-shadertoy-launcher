// Danil, 2020 Vulkan shadertoy launcher, self https://github.com/danilw/vulkan-shadertoy-launcher

#include <stdio.h>
#include <stdlib.h>

//define to not resize offscreen-buf on resize
//#define NO_RESIZE_BUF

//define to use one CommandBuffer for all offscreen rendering
//#define ONE_CMD

#if defined(VK_USE_PLATFORM_XCB_KHR)
#include <unistd.h>
#endif

#ifdef _WIN32
#pragma comment(linker, "/subsystem:windows")
#endif

#include <time.h>
#include "../vk_utils/vk_utils.h"
#include "../vk_utils/vk_render_helper.h"
#include "../os_utils/utils.h"

//using stb_image
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_THREAD_LOCALS
#include "stb_image.h"

//same numbers in <*.frag> files
//number of buffers to create, any number(>0), if you need 0 use https://github.com/danilw/vulkan-shader-launcher
//names shaders/spv/<file>.spv look files names in that foldred
#define OFFSCREEN_BUFFERS 4

//number of images(>0)
//names textures/<X>.png X start from 1
#define IMAGE_TEXTURES 4

//linear or mipmap for textures
#define USE_MIPMAPS true

struct shaders_uniforms {
    float iMouse[4];
    float iDate[4];
    int iMouse_lr[2];
    float iResolution[2];
    int debugdraw;
    int pause;
    float iTime;
    float iTimeDelta;
    int iFrame;
};

enum
{
    BUFFER_VERTICES = 0,
    BUFFER_INDICES = 1,
};
enum
{
    SHADER_MAIN_VERTEX = 0,
    SHADER_MAIN_FRAGMENT = 1,
};

struct render_data
{
    struct objects
    {
        struct vertex
        {
            float pos[3];
        } vertices[4];

        uint16_t indices[4];
    } objects;

    struct shaders_uniforms push_constants;

    struct vk_image images[IMAGE_TEXTURES+OFFSCREEN_BUFFERS];
    struct vk_buffer buffers[2];
    struct vk_shader shaders[2+OFFSCREEN_BUFFERS*2];
    struct vk_graphics_buffers *main_gbuffers;
    struct vk_offscreen_buffers *buf_obuffers;

    VkRenderPass buf_render_pass[OFFSCREEN_BUFFERS];
    struct vk_layout buf_layout[OFFSCREEN_BUFFERS];
    struct vk_pipeline buf_pipeline[OFFSCREEN_BUFFERS];
    VkDescriptorSet buf_desc_set[OFFSCREEN_BUFFERS];

    VkRenderPass main_render_pass;
    struct vk_layout main_layout;
    struct vk_pipeline main_pipeline;
    VkDescriptorSet main_desc_set;

};

struct render_data render_data = { .main_gbuffers = NULL, .buf_obuffers = NULL, };
VkInstance vk;
struct vk_physical_device phy_dev;
struct vk_device dev;
struct vk_swapchain swapchain = {0};
struct app_os_window os_window;
struct vk_render_essentials essentials;

VkFence offscreen_fence = VK_NULL_HANDLE;
VkQueue offscreen_queue[OFFSCREEN_BUFFERS] = {VK_NULL_HANDLE};
bool use_one_VkQueue=false; //I check if more then 1 VK_QUEUE_GRAPHICS_BIT suported, then it set to true <-!!!
VkCommandBuffer offscreen_cmd_buffer[OFFSCREEN_BUFFERS] = {VK_NULL_HANDLE};
VkSemaphore wait_buf_sem = VK_NULL_HANDLE, wait_main_sem = VK_NULL_HANDLE;
bool first_submission = true;

#if defined(VK_USE_PLATFORM_WIN32_KHR)
#include <shellapi.h>

static bool render_loop_draw(struct vk_physical_device *phy_dev, struct vk_device *dev, struct vk_swapchain *swapchain, struct app_os_window *os_window);
static bool on_window_resize(struct vk_physical_device *phy_dev, struct vk_device *dev, struct vk_render_essentials *essentials,
                             struct vk_swapchain *swapchain, struct render_data *render_data, struct app_os_window *os_window);

#include "../os_utils/os_win_utils.h"
#elif defined(VK_USE_PLATFORM_XCB_KHR)
#include "../os_utils/xcb_x11_utils.h"
#endif

static vk_error init_texture(struct vk_physical_device *phy_dev, struct vk_device *dev, struct vk_render_essentials *essentials,
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

    VkFormat img_format=VK_FORMAT_R8G8B8A8_UNORM; //VK_FORMAT_R8G8B8A8_SRGB

    *image = (struct vk_image) {
        .format = img_format,
        .extent = { .width = width, .height = height },
        .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .make_view = true,
        .host_visible = false,
        .anisotropyEnable = true,
        .repeat_mode = VK_SAMPLER_ADDRESS_MODE_REPEAT, //VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER //VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
        .mipmaps = mipmaps,
    };

    retval = vk_create_images(phy_dev, dev, image, 1);
    if (!vk_error_is_success(&retval))
    {
        retval.error.type=VK_ERROR_ERRNO;
        vk_error_printf(&retval, "Failed to create texture images\n");
        stbi_image_free(generated_texture);
        return retval;
    }

    retval = vk_render_init_texture(phy_dev, dev, essentials, image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, generated_texture, name);

    stbi_image_free(generated_texture);
    return retval;
}

static vk_error allocate_render_data(struct vk_physical_device *phy_dev, struct vk_device *dev,
                                     struct vk_swapchain *swapchain, struct vk_render_essentials *essentials, struct render_data *render_data, bool reload_shaders)
{

    static bool load_once=false;
    vk_error retval = VK_ERROR_NONE;
    VkResult res;
    if(!load_once) {
        render_data->buffers[BUFFER_VERTICES] = (struct vk_buffer) {
            .size = sizeof render_data->objects.vertices,
             .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
              .host_visible = false,
        };

        render_data->buffers[BUFFER_INDICES] = (struct vk_buffer) {
            .size = sizeof render_data->objects.indices,
             .usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
              .host_visible = false,
        };

        retval = vk_create_buffers(phy_dev, dev, render_data->buffers, 2);
        if (!vk_error_is_success(&retval))
        {
            vk_error_printf(&retval, "Failed to create vertex, index and transformation buffers\n");
            return retval;
        }
    }
    if(!load_once) {
        render_data->objects = (struct objects) {
            .vertices = {
                [0] = (struct vertex){ .pos = { 1.0,  1.0, 0.0}, },
                [1] = (struct vertex){ .pos = { 1.0, -1.0, 0.0}, },
                [2] = (struct vertex){ .pos = {-1.0,  1.0, 0.0}, },
                [3] = (struct vertex){ .pos = {-1.0, -1.0, 0.0}, },
            },
            .indices = {
                0, 1, 2, 3,
            },
        };
        retval = vk_render_init_buffer(phy_dev, dev, essentials, &render_data->buffers[BUFFER_VERTICES], render_data->objects.vertices, "vertex");
        if (!vk_error_is_success(&retval))
            return retval;
        retval = vk_render_init_buffer(phy_dev, dev, essentials, &render_data->buffers[BUFFER_INDICES], render_data->objects.indices, "index");
        if (!vk_error_is_success(&retval))
            return retval;

        for(uint32_t i=0; i<IMAGE_TEXTURES; i++) {
            char txt[255]= {0};
            sprintf(txt,"textures/%d.png",i+1);
            retval = init_texture(phy_dev, dev, essentials, &render_data->images[i], txt, USE_MIPMAPS);
            if (!vk_error_is_success(&retval))
                return retval;
        }
        for(uint32_t i=IMAGE_TEXTURES; i<IMAGE_TEXTURES+OFFSCREEN_BUFFERS; i++) {
            retval = init_texture(phy_dev, dev, essentials, &render_data->images[i], "textures/empty.png", false);
            if (!vk_error_is_success(&retval))
                return retval;
        }
    }
    if((!load_once)||(reload_shaders)) {
        render_data->shaders[SHADER_MAIN_VERTEX] = (struct vk_shader) {
            .spirv_file = "shaders/spv/main.vert.spv",
             .stage = VK_SHADER_STAGE_VERTEX_BIT,
        };
        render_data->shaders[SHADER_MAIN_FRAGMENT] = (struct vk_shader) {
            .spirv_file = "shaders/spv/main.frag.spv",
             .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        };
        char txt[OFFSCREEN_BUFFERS][255]= {0};
        for(uint32_t i=0; i<OFFSCREEN_BUFFERS*2; i+=2) {
            render_data->shaders[i+2] = (struct vk_shader) {
                .spirv_file = "shaders/spv/buf.vert.spv",
                 .stage = VK_SHADER_STAGE_VERTEX_BIT,
            };
            //txt[i]=char[255]{0};
            if(i>0) {
                sprintf(txt[i/2],"shaders/spv/buf%d.frag.spv",i/2);
            } else {
                sprintf(txt[i/2],"shaders/spv/buf.frag.spv");
            }
            render_data->shaders[i+2+1] = (struct vk_shader) {
                .spirv_file = txt[i/2],
                 .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            };
        }
        retval = vk_load_shaders(dev, render_data->shaders, 2+OFFSCREEN_BUFFERS*2);
        if (!vk_error_is_success(&retval))
        {
            vk_error_printf(&retval, "Could not load the shaders\n");
            return retval;
        }

    }
    render_data->main_gbuffers = malloc(essentials->image_count * sizeof *render_data->main_gbuffers);
    for (uint32_t i = 0; i < essentials->image_count; ++i)
        render_data->main_gbuffers[i] = (struct vk_graphics_buffers) {
        .surface_size = swapchain->surface_caps.currentExtent,
         .swapchain_image = essentials->images[i],
    };

#ifdef NO_RESIZE_BUF
    if(!load_once) {
#endif
        render_data->buf_obuffers = malloc(2 * (sizeof (*render_data->buf_obuffers)) * OFFSCREEN_BUFFERS);
        for (uint32_t i = 0; i < 2*OFFSCREEN_BUFFERS; i++)
            render_data->buf_obuffers[i] = (struct vk_offscreen_buffers) {
            .surface_size = swapchain->surface_caps.currentExtent,
        };
#ifdef NO_RESIZE_BUF
    }
#endif

    // 8bit BGRA for main_image VK_FORMAT_B8G8R8A8_UNORM
    retval = vk_create_graphics_buffers(phy_dev, dev, swapchain->surface_format.format, render_data->main_gbuffers, essentials->image_count,
                                        &render_data->main_render_pass, VK_C_CLEAR, VK_WITHOUT_DEPTH);
    if (!vk_error_is_success(&retval))
    {
        vk_error_printf(&retval, "Could not create graphics buffers\n");
        return retval;
    }

#ifdef NO_RESIZE_BUF
    if(!load_once) {
#endif
        for (uint32_t i = 0; i < OFFSCREEN_BUFFERS; i++) {
            // 32 bit format RGBA for buffers VK_FORMAT_R32G32B32A32_SFLOAT
            retval = vk_create_offscreen_buffers(phy_dev, dev, VK_FORMAT_R32G32B32A32_SFLOAT, &render_data->buf_obuffers[i*2], 2,
                                                 &render_data->buf_render_pass[i], VK_C_CLEAR, VK_WITHOUT_DEPTH);
            if (!vk_error_is_success(&retval))
            {
                vk_error_printf(&retval, "Could not create off-screen buffers\n");
                return retval;
            }
        }
#ifdef NO_RESIZE_BUF
    }
#endif

    struct vk_image **image_pointer; //&render_data->buf_obuffers[2].color
    image_pointer = malloc(1 * sizeof(struct vk_image*) * (IMAGE_TEXTURES+OFFSCREEN_BUFFERS));
    for(uint32_t i=0; i<IMAGE_TEXTURES+OFFSCREEN_BUFFERS; i++) {
        image_pointer[i]=&render_data->images[i];
    }

    VkPushConstantRange push_constant_range = {
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .offset = 0,
        .size = sizeof render_data->push_constants,
    };


        /*******************
            * BUF PART *
         *******************/
#ifdef NO_RESIZE_BUF
    if(!load_once) {
#endif
        for (int i=0; i<OFFSCREEN_BUFFERS; i++)
        {
            /* Layouts */

            struct vk_resources resources = {
                .images = *image_pointer,
                .image_count = IMAGE_TEXTURES+OFFSCREEN_BUFFERS,
                .buffers = render_data->buffers,
                .buffer_count = 2,
                .shaders = &render_data->shaders[SHADER_MAIN_FRAGMENT+1+i*2],
                .shader_count = 2,
                .push_constants = &push_constant_range,
                .push_constant_count = 1,
                .render_pass = render_data->buf_render_pass[i],
            };
            render_data->buf_layout[i] = (struct vk_layout) {
                .resources = &resources,
            };
            retval = vk_make_graphics_layouts(dev, &render_data->buf_layout[i], 1);
            if (!vk_error_is_success(&retval))
            {
                vk_error_printf(&retval, "BUF: Could not create descriptor set or pipeline layouts\n");
                return retval;
            }

            /* Pipeline */
            VkVertexInputBindingDescription vertex_binding = {
                .binding = 0,
                .stride = sizeof *render_data->objects.vertices,
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
            };

            VkVertexInputAttributeDescription vertex_attributes[1] = {
                [0] = {
                    .location = 0,
                    .binding = 0,
                    .format = VK_FORMAT_R32G32_SFLOAT,
                    .offset = 0,
                },
            };
            render_data->buf_pipeline[i] = (struct vk_pipeline) {
                .layout = &render_data->buf_layout[i],
                .vertex_input_state = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                    .vertexBindingDescriptionCount = 1,
                    .pVertexBindingDescriptions = &vertex_binding,
                    .vertexAttributeDescriptionCount = 1,
                    .pVertexAttributeDescriptions = vertex_attributes,
                },
                .input_assembly_state = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
                },
                .tessellation_state = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
                },
                .thread_count = 1,
            };

            retval = vk_make_graphics_pipelines(dev, &render_data->buf_pipeline[i], 1, false);
            if (!vk_error_is_success(&retval))
            {
                vk_error_printf(&retval, "BUF: Could not create graphics pipeline\n");
                return retval;
            }

            /* Descriptor Set */
            VkDescriptorSetAllocateInfo set_info = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .descriptorPool = render_data->buf_pipeline[i].set_pool,
                .descriptorSetCount = 1,
                .pSetLayouts = &render_data->buf_layout[i].set_layout,
            };
            res = vkAllocateDescriptorSets(dev->device, &set_info, &render_data->buf_desc_set[i]);
            retval = VK_ERROR_NONE;
            vk_error_set_vkresult(&retval, res);
            if (res)
            {
                vk_error_printf(&retval, "BUF: Could not allocate descriptor set from pool\n");
                return retval;
            }
        }

#ifdef NO_RESIZE_BUF
    }
#endif

    /*******************
     * MAIN_IMAGE PART *
     *******************/
    {
        /* Layouts */

        struct vk_resources resources = {
            .images = *image_pointer,
            .image_count = IMAGE_TEXTURES+OFFSCREEN_BUFFERS,
            .buffers = render_data->buffers,
            .buffer_count = 2,
            .shaders = &render_data->shaders[SHADER_MAIN_VERTEX],
            .shader_count = 2,
            .push_constants = &push_constant_range,
            .push_constant_count = 1,
            .render_pass = render_data->main_render_pass,
        };
        render_data->main_layout = (struct vk_layout) {
            .resources = &resources,
        };
        retval = vk_make_graphics_layouts(dev, &render_data->main_layout, 1);
        if (!vk_error_is_success(&retval))
        {
            vk_error_printf(&retval, "Could not create descriptor set or pipeline layouts\n");
            return retval;
        }

        /* Pipeline */
        VkVertexInputBindingDescription vertex_binding = {
            .binding = 0,
            .stride = sizeof *render_data->objects.vertices,
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        };

        VkVertexInputAttributeDescription vertex_attributes[1] = {
            [0] = {
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32_SFLOAT,
                .offset = 0,
            },
        };
        render_data->main_pipeline = (struct vk_pipeline) {
            .layout = &render_data->main_layout,
            .vertex_input_state = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                .vertexBindingDescriptionCount = 1,
                .pVertexBindingDescriptions = &vertex_binding,
                .vertexAttributeDescriptionCount = 1,
                .pVertexAttributeDescriptions = vertex_attributes,
            },
            .input_assembly_state = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
            },
            .tessellation_state = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
            },
            .thread_count = 1,
        };

        retval = vk_make_graphics_pipelines(dev, &render_data->main_pipeline, 1, false);
        if (!vk_error_is_success(&retval))
        {
            vk_error_printf(&retval, "Could not create graphics pipeline\n");
            return retval;
        }

        /* Descriptor Set */
        VkDescriptorSetAllocateInfo set_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = render_data->main_pipeline.set_pool,
            .descriptorSetCount = 1,
            .pSetLayouts = &render_data->main_layout.set_layout,
        };
        res = vkAllocateDescriptorSets(dev->device, &set_info, &render_data->main_desc_set);
        retval = VK_ERROR_NONE;
        vk_error_set_vkresult(&retval, res);
        if (res)
        {
            vk_error_printf(&retval, "Could not allocate descriptor set from pool\n");
            return retval;
        }
    }

    load_once=true;
    free(image_pointer);


    return retval;
}

static void free_render_data(struct vk_device *dev, struct vk_render_essentials *essentials, struct render_data *render_data)
{
    vkDeviceWaitIdle(dev->device);

    vk_free_pipelines(dev, &render_data->main_pipeline, 1);
    vk_free_layouts(dev, &render_data->main_layout, 1);
    vk_free_pipelines(dev, render_data->buf_pipeline, OFFSCREEN_BUFFERS);
    vk_free_layouts(dev, render_data->buf_layout, OFFSCREEN_BUFFERS);
    vk_free_images(dev, render_data->images, IMAGE_TEXTURES+OFFSCREEN_BUFFERS);

    vk_free_buffers(dev, render_data->buffers, 2);
    vk_free_shaders(dev, render_data->shaders, 2+OFFSCREEN_BUFFERS*2);

    for (int i=0; i<OFFSCREEN_BUFFERS; i++) {
        vk_free_offscreen_buffers(dev, &render_data->buf_obuffers[i*2], 2, render_data->buf_render_pass[i]);
    }
    vk_free_graphics_buffers(dev, render_data->main_gbuffers, essentials->image_count, render_data->main_render_pass);

    free(render_data->main_gbuffers);
    free(render_data->buf_obuffers);
}

// TO DO FREE BZUF FENCE LOOP
static void exit_cleanup_render_loop(struct vk_device *dev, struct vk_render_essentials *essentials, struct render_data *render_data,
                                     VkSemaphore wait_buf_sem, VkSemaphore wait_main_sem, VkFence offscreen_fence)
{
    vkDeviceWaitIdle(dev->device);
    if(offscreen_fence!= VK_NULL_HANDLE)vkDestroyFence(dev->device, offscreen_fence, NULL);
    if(wait_main_sem!= VK_NULL_HANDLE)vkDestroySemaphore(dev->device, wait_main_sem, NULL);
    if(wait_buf_sem!= VK_NULL_HANDLE)vkDestroySemaphore(dev->device, wait_buf_sem, NULL);
    free_render_data(dev, essentials, render_data);
    vk_render_cleanup_essentials(essentials, dev);
}

static void render_loop_init(struct vk_physical_device *phy_dev, struct vk_device *dev, struct vk_swapchain *swapchain, struct app_os_window *os_window)
{
    int res;
    vk_error retval = VK_ERROR_NONE;
    static bool once=false;

    res = vk_render_get_essentials(&essentials, phy_dev, dev, swapchain);
    if (res) {
        vk_render_cleanup_essentials(&essentials, dev);
        return;
    }

    if(!once) {
        uint32_t *presentable_queues = NULL;
        uint32_t presentable_queue_count = 0;

        retval = vk_get_presentable_queues(phy_dev, dev, swapchain->surface, &presentable_queues, &presentable_queue_count);
        if (!vk_error_is_success(&retval) || presentable_queue_count == 0)
        {
            printf("No presentable queue families.  You should have got this error in vk_render_get_essentials before.\n");
            free(presentable_queues);
            vk_render_cleanup_essentials(&essentials, dev);
            return;
        }

#if !defined(ONE_CMD)
        //check to use 1 cmd_buffer per Queues or 1 Queue for many cmd_buffers
        if (dev->command_pools[presentable_queues[0]].queue_count < 1+OFFSCREEN_BUFFERS)
        {
            retval.error.type=VK_ERROR_ERRNO;
            printf("Not enough queues in the presentable queue family %u, used only one queue.\n", presentable_queues[0]);
            use_one_VkQueue = false;
        } else {
            use_one_VkQueue = true;
        }
#endif
        if(use_one_VkQueue)
        {
            for(uint32_t i=0; i<OFFSCREEN_BUFFERS; i++) {
                offscreen_queue[i] = dev->command_pools[presentable_queues[0]].queues[1+i];
                offscreen_cmd_buffer[i] = dev->command_pools[presentable_queues[0]].buffers[1+i];
            }
        } else {
            for(uint32_t i=0; i<OFFSCREEN_BUFFERS; i++) {
                offscreen_queue[i] = dev->command_pools[presentable_queues[0]].queues[0];
#if defined(ONE_CMD)
                offscreen_cmd_buffer[i] = dev->command_pools[presentable_queues[0]].buffers[1];
#else
                offscreen_cmd_buffer[i] = dev->command_pools[presentable_queues[0]].buffers[1+i];
#endif
            }
        }

        free(presentable_queues);
    }

    retval = allocate_render_data(phy_dev, dev, swapchain, &essentials, &render_data, os_window->reload_shaders_on_resize);
    if (!vk_error_is_success(&retval))
    {
        free_render_data(dev, &essentials, &render_data);
        vk_render_cleanup_essentials(&essentials, dev);
        return;
    }
#ifdef NO_RESIZE_BUF
    if(!once) {
#endif
        for (int i=0; i<OFFSCREEN_BUFFERS*2; i++) {
            retval = vk_render_transition_images(dev, &essentials, &render_data.buf_obuffers[i].color, 1,
                                                 VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                 VK_IMAGE_ASPECT_COLOR_BIT, "off-screen color");
            if (!vk_error_is_success(&retval)) {
                free_render_data(dev, &essentials, &render_data);
                vk_render_cleanup_essentials(&essentials, dev);
                return;
            }
        }
#ifdef NO_RESIZE_BUF
    }
#endif
    if(!once) {
        VkSemaphoreCreateInfo sem_info = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };
        res = vkCreateSemaphore(dev->device, &sem_info, NULL, &wait_buf_sem);
        vk_error_set_vkresult(&retval, res);
        if (res)
        {
            vk_error_printf(&retval, "Failed to create wait-render semaphore\n");
            exit_cleanup_render_loop(dev, &essentials, &render_data, wait_buf_sem, wait_main_sem, offscreen_fence);
            return;
        }
        res = vkCreateSemaphore(dev->device, &sem_info, NULL, &wait_main_sem);
        vk_error_set_vkresult(&retval, res);
        if (res)
        {
            vk_error_printf(&retval, "Failed to create wait-post-process semaphore\n");
            exit_cleanup_render_loop(dev, &essentials, &render_data, wait_buf_sem, wait_main_sem, offscreen_fence);
            return;
        }

        VkFenceCreateInfo fence_info = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        };
        res = vkCreateFence(dev->device, &fence_info, NULL, &offscreen_fence);
        vk_error_set_vkresult(&retval, res);
        if (res)
        {
            vk_error_printf(&retval, "Failed to create fence\n");
            exit_cleanup_render_loop(dev, &essentials, &render_data, wait_buf_sem, wait_main_sem, offscreen_fence);
            return;
        }

    }
    once=true;
    os_window->prepared = true;
    os_window->resize_event=false;

    return;

}

static void exit_cleanup(VkInstance vk, struct vk_device *dev, struct vk_swapchain *swapchain, struct app_os_window *os_window)
{
    vk_free_swapchain(vk, dev, swapchain);
    vk_cleanup(dev);
#if defined(VK_USE_PLATFORM_XCB_KHR)
    xcb_destroy_window(os_window->connection, os_window->xcb_window);
    xcb_disconnect(os_window->connection);
    free(os_window->atom_wm_delete_window);
#endif
    vk_exit(vk);
}


static bool on_window_resize(struct vk_physical_device *phy_dev, struct vk_device *dev, struct vk_render_essentials *essentials,
                             struct vk_swapchain *swapchain, struct render_data *render_data, struct app_os_window *os_window)
{
    vk_error res = VK_ERROR_NONE;

    if(!os_window->prepared)return true;

    vkDeviceWaitIdle(dev->device);
    os_window->prepared = false;

    vk_free_pipelines(dev, &render_data->main_pipeline, 1);
    vk_free_graphics_buffers(dev, render_data->main_gbuffers, essentials->image_count, render_data->main_render_pass);
    vk_free_layouts(dev, &render_data->main_layout, 1);

#ifndef NO_RESIZE_BUF
    vk_free_pipelines(dev, render_data->buf_pipeline, OFFSCREEN_BUFFERS);
    for (int i=0; i<OFFSCREEN_BUFFERS; i++) {
        vk_free_offscreen_buffers(dev, &render_data->buf_obuffers[i*2], 2, render_data->buf_render_pass[i]);
    }
    vk_free_layouts(dev, render_data->buf_layout, OFFSCREEN_BUFFERS);
#endif

    if(os_window->reload_shaders_on_resize)
        vk_free_shaders(dev, render_data->shaders, 2+OFFSCREEN_BUFFERS*2);

    vk_render_cleanup_essentials(essentials, dev);

    free(render_data->main_gbuffers);

#ifndef NO_RESIZE_BUF
    free(render_data->buf_obuffers);
#endif

    res = vk_get_swapchain(vk, phy_dev, dev, swapchain, os_window, 1, &os_window->present_mode);
    if (vk_error_is_error(&res))
    {
        vk_error_printf(&res, "Could not create surface and swapchain\n");
        exit_cleanup(vk, dev, swapchain, os_window);
        return false;
    }

    render_loop_init(phy_dev, dev, swapchain, os_window);

    return true;
}

void update_params(struct app_data_struct *app_data, bool fps_lock) {
    float rdelta=0;
    if(fps_lock)FPS_LOCK(60);
    float delta=update_fps_delta();
    if(!app_data->pause) {
        app_data->iFrame++;
        app_data->iTime+=delta;
    }
    app_data->iTimeDelta=delta;
    float pause_time=pres_pause(app_data->pause);
}

void set_push_constants(struct app_os_window *os_window) {
    struct my_time_struct my_time;
    get_local_time(&my_time);
    float day_sec=((float)my_time.msec)/1000.0+my_time.sec+my_time.min*60+my_time.hour*3600;

    render_data.push_constants = (struct shaders_uniforms) {
        .iResolution[0]=os_window->app_data.iResolution[0],
        .iResolution[1]=os_window->app_data.iResolution[1],
        .iTime=os_window->app_data.iTime,
        .iTimeDelta=os_window->app_data.iTimeDelta,
        .iFrame=os_window->app_data.iFrame,
        .iMouse[0]=os_window->app_data.iMouse[0],
        .iMouse[1]=os_window->app_data.iMouse[1],
        .iMouse[2]=os_window->app_data.iMouse_lclick[0],
        .iMouse[3]=os_window->app_data.iMouse_lclick[1],
        .iMouse_lr[0]=(int)os_window->app_data.iMouse_click[0],
        .iMouse_lr[1]=(int)os_window->app_data.iMouse_click[1],
        .iDate[0]=my_time.year,
        .iDate[1]=my_time.month,
        .iDate[2]=my_time.day,
        .iDate[3]=day_sec,
        .debugdraw=(int)os_window->app_data.drawdebug,
        .pause=(int)os_window->app_data.pause,
    };
}

void update_push_constants_window_size(struct app_os_window *os_window) {
    render_data.push_constants.iMouse[0]=os_window->app_data.iMouse[0];
    render_data.push_constants.iMouse[1]=os_window->app_data.iMouse[1];
    render_data.push_constants.iMouse[2]=os_window->app_data.iMouse_lclick[0];
    render_data.push_constants.iMouse[3]=os_window->app_data.iMouse_lclick[1];
    render_data.push_constants.iResolution[0]=os_window->app_data.iResolution[0];
    render_data.push_constants.iResolution[1]=os_window->app_data.iResolution[1];
}

#define sign(x) ((x > 0) ? 1 : ((x < 0) ? -1 : 0))
void update_push_constants_local_size(float width, float height) {
    render_data.push_constants.iMouse[0]=((render_data.push_constants.iMouse[0]/render_data.push_constants.iResolution[1])-0.5*
                                          (render_data.push_constants.iResolution[0]/render_data.push_constants.iResolution[1]))*height+0.5*width;
    render_data.push_constants.iMouse[1]=((render_data.push_constants.iMouse[1]/render_data.push_constants.iResolution[1])-0.5)*height+0.5*height;
    render_data.push_constants.iMouse[2]=sign(render_data.push_constants.iMouse[2])*(((abs(render_data.push_constants.iMouse[2])
                                         /render_data.push_constants.iResolution[1])-0.5*
                                         (render_data.push_constants.iResolution[0]/render_data.push_constants.iResolution[1]))*height+0.5*width);
    render_data.push_constants.iMouse[3]=sign(render_data.push_constants.iMouse[3])*(((abs(render_data.push_constants.iMouse[3])
                                         /render_data.push_constants.iResolution[1])-0.5)*height+0.5*height);
    render_data.push_constants.iResolution[0]=width;
    render_data.push_constants.iResolution[1]=height;
}

static bool render_loop_buf(struct vk_physical_device *phy_dev, struct vk_device *dev, struct vk_render_essentials *essentials,
                            struct render_data *render_data, VkCommandBuffer cmd_buffer, int render_index, int buffer_index, struct app_data_struct *app_data)
{
    vk_error retval = VK_ERROR_NONE;
    VkResult res;

    if ((!essentials->first_render)&&(buffer_index==0))
    {
        res = vkWaitForFences(dev->device, 1, &essentials->exec_fence, true, 1000000000);
        vk_error_set_vkresult(&retval, res);
        if (res)
        {
            vk_error_printf(&retval, "Wait for fence failed\n");
            return false;
        }
    }
#ifdef NO_RESIZE_BUF
    update_push_constants_local_size(render_data->buf_obuffers[render_index+buffer_index*2].surface_size.width, render_data->buf_obuffers[render_index+buffer_index*2].surface_size.height);
#endif
#if defined(ONE_CMD)
    if((use_one_VkQueue)||(buffer_index==0))
#endif
    {
        vkResetCommandBuffer(cmd_buffer, 0);
        VkCommandBufferBeginInfo begin_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        };
        res = vkBeginCommandBuffer(cmd_buffer, &begin_info);
        vk_error_set_vkresult(&retval, res);
        if (res)
        {
            vk_error_printf(&retval, "BUF: Couldn't even begin recording a command buffer\n");
            return false;
        }
    }
    VkImageMemoryBarrier image_barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_MEMORY_READ_BIT,
        .dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = render_data->buf_obuffers[render_index+buffer_index*2].color.image,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };

    vkCmdPipelineBarrier(cmd_buffer,
                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                         0,
                         0, NULL,
                         0, NULL,
                         1, &image_barrier);

    VkClearValue clear_values = {
        .color = { .float32 = {0.0, 0.0, 0.0, 1.0}, },
    };
    VkRenderPassBeginInfo pass_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = render_data->buf_render_pass[buffer_index],
        .framebuffer = render_data->buf_obuffers[render_index+buffer_index*2].framebuffer,
        .renderArea = {
            .offset = { .x = 0, .y = 0, },
            .extent = render_data->buf_obuffers[render_index+buffer_index*2].surface_size,
        },
        .clearValueCount = 1,
        .pClearValues = &clear_values,
    };

    vkCmdBeginRenderPass(cmd_buffer, &pass_info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, render_data->buf_pipeline[buffer_index].pipeline);

    int render_index_t[OFFSCREEN_BUFFERS];
    for(int i=0; i<OFFSCREEN_BUFFERS; i++) {
        if(i<buffer_index) {
            render_index_t[i]=render_index+i*2;
        } else {
            render_index_t[i]=render_index-1+i*2;
            if(render_index_t[i]<i*2)render_index_t[i]=1+i*2;
        }
    }

    VkDescriptorImageInfo set_write_image_info[IMAGE_TEXTURES+OFFSCREEN_BUFFERS] = {0};
    for(uint32_t i=0; i<IMAGE_TEXTURES; i++) {
        set_write_image_info[i]=(VkDescriptorImageInfo) {
            .sampler = render_data->images[i].sampler,
            .imageView = render_data->images[i].view,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };
    }
    for(uint32_t i=0; i<OFFSCREEN_BUFFERS; i++) {
        set_write_image_info[IMAGE_TEXTURES+i]=(VkDescriptorImageInfo) {
            .sampler = render_data->buf_obuffers[render_index_t[i]].color.sampler,
            .imageView = render_data->buf_obuffers[render_index_t[i]].color.view,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };
    }

    VkWriteDescriptorSet set_write[2] = {
        [0]={
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = render_data->buf_desc_set[buffer_index],
            .dstBinding = 0,
            .descriptorCount = IMAGE_TEXTURES,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &set_write_image_info[0],
        },
        [1]={
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = render_data->buf_desc_set[buffer_index],
            .dstBinding = IMAGE_TEXTURES,
            .descriptorCount = OFFSCREEN_BUFFERS,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &set_write_image_info[IMAGE_TEXTURES],
        },
    };
    vkUpdateDescriptorSets(dev->device, 2, set_write, 0, NULL);

    vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            render_data->buf_layout[buffer_index].pipeline_layout, 0, 1, &render_data->buf_desc_set[buffer_index], 0, NULL);
    VkDeviceSize vertices_offset = 0;
    vkCmdBindVertexBuffers(cmd_buffer, 0, 1, &render_data->buffers[BUFFER_VERTICES].buffer, &vertices_offset);
    vkCmdBindIndexBuffer(cmd_buffer, render_data->buffers[BUFFER_INDICES].buffer, 0, VK_INDEX_TYPE_UINT16);

    VkViewport viewport = {
        .x = 0,
        .y = 0,
        .width = render_data->buf_obuffers[render_index+buffer_index*2].surface_size.width,
        .height = render_data->buf_obuffers[render_index+buffer_index*2].surface_size.height,
        .minDepth = 0,
        .maxDepth = 1,
    };
    vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);

    VkRect2D scissor = {
        .offset = { .x = 0, .y = 0, },
        .extent = render_data->buf_obuffers[render_index+buffer_index*2].surface_size,
    };
    vkCmdSetScissor(cmd_buffer, 0, 1, &scissor);

    vkCmdPushConstants(cmd_buffer, render_data->buf_layout[buffer_index].pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                       sizeof render_data->push_constants, &render_data->push_constants);

    vkCmdDrawIndexed(cmd_buffer, 4, 1, 0, 0, 0);
    vkCmdEndRenderPass(cmd_buffer);

    image_barrier = (VkImageMemoryBarrier) {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = render_data->buf_obuffers[render_index+buffer_index*2].color.image,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };

    vkCmdPipelineBarrier(cmd_buffer,
                         VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                         VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                         0,
                         0, NULL,
                         0, NULL,
                         1, &image_barrier);
#if defined(ONE_CMD)
    if((use_one_VkQueue)||((!use_one_VkQueue)&&(buffer_index==OFFSCREEN_BUFFERS-1)))
#endif
    {
        vkEndCommandBuffer(cmd_buffer);
    }
    return true;
}

static bool render_loop_draw(struct vk_physical_device *phy_dev, struct vk_device *dev, struct vk_swapchain *swapchain, struct app_os_window *os_window)
{
    static int render_index=0;
    int res;
    vk_error retval = VK_ERROR_NONE;

    set_push_constants(os_window);

    for(int i=0; i<OFFSCREEN_BUFFERS; i++) {
        if (!render_loop_buf(phy_dev, dev, &essentials, &render_data, offscreen_cmd_buffer[i], render_index, i, &os_window->app_data)) {
            printf("Error on rendering buffers \n");
            return false;
        }
        update_push_constants_window_size(os_window);

#if defined(ONE_CMD)
        if(((i==0)&&(use_one_VkQueue))||(((!use_one_VkQueue)&&(i==OFFSCREEN_BUFFERS-1)))) { //wait main screen, or Queue all if only one VKQueue
#else
        if(i==0) { //wait main screen, or Queue all if only one VKQueue
#endif
            if (!first_submission)
            {
                res = vkWaitForFences(dev->device, 1, &offscreen_fence, true, 1000000000);
                vk_error_set_vkresult(&retval, res);
                if (res)
                {
                    vk_error_printf(&retval, "Wait for main fence failed\n");
                    return false;
                }
            }

            VkPipelineStageFlags wait_sem_stages[1] = {VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT};
            VkSubmitInfo submit_info = {
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .waitSemaphoreCount = first_submission?0:1,
                .pWaitSemaphores = &wait_main_sem,
                .pWaitDstStageMask = wait_sem_stages,
                .commandBufferCount = 1,
                .pCommandBuffers = &offscreen_cmd_buffer[i],
                .signalSemaphoreCount = 1,
                .pSignalSemaphores = &wait_buf_sem,
            };
            res = vkResetFences(dev->device, 1, &offscreen_fence);
            vk_error_set_vkresult(&retval, res);
            if (res)
            {
                vk_error_printf(&retval, "Failed to reset fence\n");
                return false;
            }
            vkQueueSubmit(offscreen_queue[i], 1, &submit_info, offscreen_fence);
            first_submission = false;
        }
        else 
#if defined(ONE_CMD)
        if(use_one_VkQueue)
#endif
        { //wait last buf/shader in loop, if multi VkQueue supported
            res = vkWaitForFences(dev->device, 1, &offscreen_fence, true, 1000000000);
            vk_error_set_vkresult(&retval, res);
            if (res)
            {
                vk_error_printf(&retval, "Wait for buf fence failed\n");
                return false;
            }

            VkPipelineStageFlags wait_sem_stages[1] = {VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT};
            VkSubmitInfo submit_info = {
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = &wait_buf_sem,
                .pWaitDstStageMask = wait_sem_stages,
                .commandBufferCount = 1,
                .pCommandBuffers = &offscreen_cmd_buffer[i],
                .signalSemaphoreCount = 1,
                .pSignalSemaphores = &wait_main_sem, //used main sem
            };
            res = vkResetFences(dev->device, 1, &offscreen_fence);
            vk_error_set_vkresult(&retval, res);
            if (res)
            {
                vk_error_printf(&retval, "Failed to reset fence\n");
                return false;
            }
            vkQueueSubmit(offscreen_queue[i], 1, &submit_info, offscreen_fence);
            VkSemaphore tmp_sem=wait_buf_sem;
            wait_buf_sem=wait_main_sem;
            wait_main_sem=tmp_sem;
        }
    }

    uint32_t image_index;

    res = vk_render_start(&essentials, dev, swapchain, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, &image_index);
    if (res)
        return false;

    VkClearValue clear_values = {
        .color = { .float32 = {0.0, 0.0, 0.0, 1.0}, },
    };
    VkRenderPassBeginInfo pass_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = render_data.main_render_pass,
        .framebuffer = render_data.main_gbuffers[image_index].framebuffer,
        .renderArea = {
            .offset = { .x = 0, .y = 0, },
            .extent = render_data.main_gbuffers[image_index].surface_size,
        },
        .clearValueCount = 1,
        .pClearValues = &clear_values,
    };

    vkCmdBeginRenderPass(essentials.cmd_buffer, &pass_info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(essentials.cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, render_data.main_pipeline.pipeline);

    VkDescriptorImageInfo set_write_image_info[IMAGE_TEXTURES+OFFSCREEN_BUFFERS] = {0};
    for(uint32_t i=0; i<IMAGE_TEXTURES; i++) {
        set_write_image_info[i]=(VkDescriptorImageInfo) {
            .sampler = render_data.images[i].sampler,
             .imageView = render_data.images[i].view,
              .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };
    }
    for(uint32_t i=0; i<OFFSCREEN_BUFFERS; i++) {
        set_write_image_info[IMAGE_TEXTURES+i]=(VkDescriptorImageInfo) {
            .sampler = render_data.buf_obuffers[i*2+render_index].color.sampler,
             .imageView = render_data.buf_obuffers[i*2+render_index].color.view,
              .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };
    }

    VkWriteDescriptorSet set_write[2] = {
        [0]={
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = render_data.main_desc_set,
            .dstBinding = 0,
            .descriptorCount = IMAGE_TEXTURES,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &set_write_image_info[0],
        },
        [1]={
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = render_data.main_desc_set,
            .dstBinding = IMAGE_TEXTURES,
            .descriptorCount = OFFSCREEN_BUFFERS,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &set_write_image_info[IMAGE_TEXTURES],
        },
    };
    vkUpdateDescriptorSets(dev->device, 2, set_write, 0, NULL);

    vkCmdBindDescriptorSets(essentials.cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            render_data.main_layout.pipeline_layout, 0, 1, &render_data.main_desc_set, 0, NULL);


    VkDeviceSize vertices_offset = 0;
    vkCmdBindVertexBuffers(essentials.cmd_buffer, 0, 1, &render_data.buffers[BUFFER_VERTICES].buffer, &vertices_offset);
    vkCmdBindIndexBuffer(essentials.cmd_buffer, render_data.buffers[BUFFER_INDICES].buffer, 0, VK_INDEX_TYPE_UINT16);

    VkViewport viewport = {
        .x = 0,
        .y = 0,
        .width = render_data.main_gbuffers[image_index].surface_size.width,
        .height = render_data.main_gbuffers[image_index].surface_size.height,
        .minDepth = 0,
        .maxDepth = 1,
    };
    vkCmdSetViewport(essentials.cmd_buffer, 0, 1, &viewport);

    VkRect2D scissor = {
        .offset = { .x = 0, .y = 0, },
        .extent = render_data.main_gbuffers[image_index].surface_size,
    };
    vkCmdSetScissor(essentials.cmd_buffer, 0, 1, &scissor);

    vkCmdPushConstants(essentials.cmd_buffer, render_data.main_layout.pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                       sizeof render_data.push_constants, &render_data.push_constants);

    //vkCmdDraw(essentials.cmd_buffer, 4, 1, 0, 0);
    vkCmdDrawIndexed(essentials.cmd_buffer, 4, 1, 0, 0, 0);

    vkCmdEndRenderPass(essentials.cmd_buffer);

    res = vk_render_finish(&essentials, dev, swapchain, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, image_index, wait_buf_sem, wait_main_sem);

    if (res == VK_ERROR_OUT_OF_DATE_KHR) {
        os_window->resize_event=true;
        res = 0;
    } else if (res == VK_ERROR_SURFACE_LOST_KHR) {
        vkDestroySurfaceKHR(vk, swapchain->surface, NULL);
        VkResult tres;
        tres=vk_create_surface(vk, swapchain, os_window);
        vk_error_set_vkresult(&retval, tres);
        if (tres)
            return false;
        os_window->resize_event=true;
        res = 0;
    }

    if (res)
        return false;

    update_params(&os_window->app_data,os_window->fps_lock);
    if(render_index>=1)render_index=0;
    else render_index+=1;
    return true;

}

void init_win_params(struct app_os_window *os_window) {
    os_window->app_data.iResolution[0]=1280;
    os_window->app_data.iResolution[1]=720;
    os_window->app_data.iFrame=0;
    os_window->app_data.iMouse[0]=0;
    os_window->app_data.iMouse[1]=0;
    os_window->app_data.iMouse_click[0]=false;
    os_window->app_data.iMouse_click[1]=false;
    os_window->app_data.iMouse_lclick[0]=0;
    os_window->app_data.iMouse_lclick[1]=0;
    os_window->app_data.iTime=0;
    os_window->app_data.pause=false;
    os_window->app_data.quit=false;
    os_window->app_data.drawdebug=false;
    os_window->fps_lock=false;
    os_window->is_minimized=false;
    os_window->prepared=false;
    os_window->resize_event=false;
    os_window->reload_shaders_on_resize=false;
    os_window->print_debug=false;
    strncpy(os_window->name, "Vulkan Shader launcher | twitter.com/AruGL", APP_NAME_STR_LEN);
}

#if defined(VK_USE_PLATFORM_XCB_KHR)
static void render_loop_xcb(struct vk_physical_device *phy_dev, struct vk_device *dev, struct vk_swapchain *swapchain, struct app_os_window *os_window)
{
    while (!os_window->app_data.quit)
    {
        xcb_generic_event_t *event;

        if (os_window->app_data.pause) {
            event = xcb_wait_for_event(os_window->connection);
        }
        else {
            event = xcb_poll_for_event(os_window->connection);
        }
        while (event) {
            app_handle_xcb_event(os_window, event);
            free(event);
            event = xcb_poll_for_event(os_window->connection);
        }
        if((!os_window->is_minimized)&&(!os_window->resize_event)) {
            if(!os_window->app_data.quit) {
                os_window->app_data.quit=!render_loop_draw(phy_dev, dev, swapchain, os_window);
            }
            else break;
        }
        else {
            if((!os_window->is_minimized)&&os_window->resize_event) {
                on_window_resize(phy_dev, dev, &essentials, swapchain, &render_data, os_window); //execute draw or resize per frame, not together
            }
        }
        if(os_window->is_minimized) { //I do not delete everything on minimize, only stop rendering
            sleep_ms(10);
        }
    }
    exit_cleanup_render_loop(dev, &essentials, &render_data, wait_buf_sem, wait_main_sem, offscreen_fence);
}
#endif

void print_usage(char *name) {
    printf("Usage: %s \n"
           "\t[--present_mode <present mode enum>]\n"
           "\t <present_mode_enum>\tVK_PRESENT_MODE_IMMEDIATE_KHR = %d\n"
           "\t\t\t\tVK_PRESENT_MODE_MAILBOX_KHR = %d\n"
           "\t\t\t\tVK_PRESENT_MODE_FIFO_KHR = %d\n"
           "\t\t\t\tVK_PRESENT_MODE_FIFO_RELAXED_KHR = %d\n"
           "\t[--debug]\n"
           "\t[--reload_shaders] will reload shaders form file on resize\n"
           "Control: Keyboard 1-debug, 2-vsynk 60fps, Space-pause\n", name,
           VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_MAILBOX_KHR,
           VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_FIFO_RELAXED_KHR);
}

#if defined(VK_USE_PLATFORM_WIN32_KHR)

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
    MSG msg;
    bool done;
    int argc;
    char **argv;

    msg.wParam = 0;

    LPWSTR *commandLineArgs = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (NULL == commandLineArgs) {
        argc = 0;
    }

    if (argc > 0) {
        argv = (char **)malloc(sizeof(char *) * argc);
        if (argv == NULL) {
            argc = 0;
        }
        else {
            for (int iii = 0; iii < argc; iii++) {
                size_t wideCharLen = wcslen(commandLineArgs[iii]);
                size_t numConverted = 0;

                argv[iii] = (char *)malloc(sizeof(char) * (wideCharLen + 1));
                if (argv[iii] != NULL) {
                    wcstombs_s(&numConverted, argv[iii], wideCharLen + 1, commandLineArgs[iii], wideCharLen + 1);
                }
            }
        }
    }
    else {
        argv = NULL;
    }

    vk_error res =VK_ERROR_NONE;
    int retval = EXIT_FAILURE;

    init_win_params(&os_window);
    uint32_t dev_count = 1;
    os_window.present_mode = VK_PRESENT_MODE_FIFO_KHR;

    if(argc>1) {
        if (strcmp(argv[1], "--help") == 0)
        {
            SetStdOutToNewConsole();
            print_usage(argv[0]);
            Sleep(44000);
            return 0;
        }
    }

    for (int i = 1; i < argc; i++) {
        if ((strcmp(argv[i], "--present_mode") == 0) && (i < argc - 1)) {
            os_window.present_mode = atoi(argv[i + 1]);
            i++;
            continue;
        }
        if (strcmp(argv[i], "--debug") == 0)
        {
            os_window.print_debug=true;
            continue;
        }
        if (strcmp(argv[i], "--reload_shaders") == 0)
        {
            os_window.reload_shaders_on_resize=true;
            continue;
        }
    }

    if (argc > 0 && argv != NULL) {
        for (int iii = 0; iii < argc; iii++) {
            if (argv[iii] != NULL) {
                free(argv[iii]);
            }
        }
        free(argv);
    }

    if(os_window.print_debug) {
        SetStdOutToNewConsole();
    }

    os_window.connection = hInstance;

    res = vk_init(&vk);
    if (!vk_error_is_success(&res))
    {
        vk_error_printf(&res, "Could not initialize Vulkan\n");
        return retval;
    }

    res = vk_enumerate_devices(vk, &phy_dev, &dev_count);
    if (vk_error_is_error(&res))
    {
        vk_error_printf(&res, "Could not enumerate devices\n");
        vk_exit(vk);
        return retval;
    }

    if (dev_count < 1)
    {
        printf("No graphics card? Shame on you\n");
        vk_exit(vk);
        return retval;
    }

#if defined(ONE_CMD)
    res = vk_setup(&phy_dev, &dev, VK_QUEUE_GRAPHICS_BIT, 2); //1 main 1 offscreen cmd
#else
    res = vk_setup(&phy_dev, &dev, VK_QUEUE_GRAPHICS_BIT, 1+OFFSCREEN_BUFFERS); //cmd buffers alloc
#endif
    if (vk_error_is_error(&res))
    {
        vk_error_printf(&res, "Could not setup logical device, command pools and queues\n");
        vk_cleanup(&dev);
        vk_exit(vk);
        return retval;
    }

    app_create_window(&os_window);

    swapchain.swapchain = VK_NULL_HANDLE;
    res = vk_get_swapchain(vk, &phy_dev, &dev, &swapchain, &os_window, 1, &os_window.present_mode);
    if (vk_error_is_error(&res))
    {
        vk_error_printf(&res, "Could not create surface and swapchain\n");
        exit_cleanup(vk, &dev, &swapchain, &os_window);
        return retval;
    }

    render_loop_init(&phy_dev, &dev, &swapchain, &os_window);
    done = false;
    while (!done) {
        PeekMessage(&msg,0,0,0,PM_REMOVE);
        process_msg(&msg,&done);
        RedrawWindow(os_window.window, NULL, NULL, RDW_INTERNALPAINT);
    }

    exit_cleanup_render_loop(&dev, &essentials, &render_data, wait_buf_sem, wait_main_sem, offscreen_fence);
    exit_cleanup(vk, &dev, &swapchain, &os_window);

    return (int)msg.wParam;
}

#elif defined(VK_USE_PLATFORM_XCB_KHR)

int main(int argc, char **argv)
{
    vk_error res;
    int retval = EXIT_FAILURE;
    VkInstance vk;
    struct vk_physical_device phy_dev;
    struct vk_device dev;
    struct vk_swapchain swapchain = {0};
    struct app_os_window os_window;
    init_win_params(&os_window);
    uint32_t dev_count = 1;
    os_window.present_mode = VK_PRESENT_MODE_FIFO_KHR;

    if(argc>1) {
        if (strcmp(argv[1], "--help") == 0)
        {
            print_usage(argv[0]);
            return 0;
        }
    }

    for (int i = 1; i < argc; i++) {
        if ((strcmp(argv[i], "--present_mode") == 0) && (i < argc - 1)) {
            os_window.present_mode = atoi(argv[i + 1]);
            i++;
            continue;
        }
        if (strcmp(argv[i], "--debug") == 0)
        {
            os_window.print_debug=true;
            continue;
        }
        if (strcmp(argv[i], "--reload_shaders") == 0)
        {
            os_window.reload_shaders_on_resize=true;
            continue;
        }
    }

    srand(time(NULL));

    res = vk_init(&vk);
    if (!vk_error_is_success(&res))
    {
        vk_error_printf(&res, "Could not initialize Vulkan\n");
        return retval;
    }

    res = vk_enumerate_devices(vk, &phy_dev, &dev_count);
    if (vk_error_is_error(&res))
    {
        vk_error_printf(&res, "Could not enumerate devices\n");
        vk_exit(vk);
        return retval;
    }

    if (dev_count < 1)
    {
        printf("No graphics card? Shame on you\n");
        vk_exit(vk);
        return retval;
    }

#if defined(ONE_CMD)
    res = vk_setup(&phy_dev, &dev, VK_QUEUE_GRAPHICS_BIT, 2); //1 main 1 offscreen cmd
#else
    res = vk_setup(&phy_dev, &dev, VK_QUEUE_GRAPHICS_BIT, 1+OFFSCREEN_BUFFERS); //cmd buffers alloc
#endif
    if (vk_error_is_error(&res))
    {
        vk_error_printf(&res, "Could not setup logical device, command pools and queues\n");
        vk_cleanup(&dev);
        vk_exit(vk);
        return retval;
    }

    printf("Init XCB\n");
    app_init_connection(&os_window);
    app_create_xcb_window(&os_window);


    swapchain.swapchain = VK_NULL_HANDLE;
    res = vk_get_swapchain(vk, &phy_dev, &dev, &swapchain, &os_window, 1, &os_window.present_mode);
    if (vk_error_is_error(&res))
    {
        vk_error_printf(&res, "Could not create surface and swapchain\n");
        exit_cleanup(vk, &dev, &swapchain, &os_window);
        return retval;
    }

    render_loop_init(&phy_dev, &dev, &swapchain, &os_window);
    render_loop_xcb(&phy_dev, &dev, &swapchain, &os_window);

    retval = 0;

    exit_cleanup(vk, &dev, &swapchain, &os_window);
    return retval;
}

#endif
