
// Danil, 2020 Vulkan shader launcher, self https://github.com/danilw/vulkan-shader-launcher

#ifndef vk_struct_H
#define vk_struct_H

#define VK_MAX_QUEUE_FAMILY 10

struct vk_physical_device
{
    VkPhysicalDevice physical_device;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memories;

    VkQueueFamilyProperties queue_families[VK_MAX_QUEUE_FAMILY];
    uint32_t queue_family_count;
    bool queue_families_incomplete;
};

struct vk_commands
{
    VkQueueFlags qflags;

    VkCommandPool pool;
    VkQueue *queues;
    uint32_t queue_count;
    VkCommandBuffer *buffers;
    uint32_t buffer_count;
};

struct vk_device
{
    VkDevice device;
    struct vk_commands *command_pools;
    uint32_t command_pool_count;
};

#define VK_MAX_PRESENT_MODES 4

struct vk_swapchain
{
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    VkSurfaceFormatKHR surface_format;
    VkSurfaceCapabilitiesKHR surface_caps;
    VkPresentModeKHR present_modes[VK_MAX_PRESENT_MODES];
    uint32_t present_modes_count;
};

struct vk_image
{
    VkFormat format;
    VkExtent2D extent;
    VkImageUsageFlagBits usage;
    VkShaderStageFlagBits stage;
    bool make_view;
    bool will_be_initialized;
    bool host_visible;
    bool multisample;
    uint32_t *sharing_queues;
    uint32_t sharing_queue_count;
    VkImage image;
    VkDeviceMemory image_mem;
    VkImageView view;
    VkSampler sampler;
    bool anisotropyEnable;
    VkSamplerAddressMode repeat_mode;
    bool mipmaps;
    bool linear;
};

struct vk_buffer
{
    VkFormat format;
    uint32_t size;
    VkBufferUsageFlagBits usage;
    VkShaderStageFlagBits stage;
    bool make_view;
    bool host_visible;
    uint32_t *sharing_queues;
    uint32_t sharing_queue_count;
    VkBuffer buffer;
    VkDeviceMemory buffer_mem;
    VkBufferView view;
};

struct vk_shader
{
    const char *spirv_file;
    VkShaderStageFlagBits stage;
    VkShaderModule shader;
};

struct vk_graphics_buffers
{
    VkExtent2D surface_size;
    VkImage swapchain_image;
    VkImageView color_view;
    struct vk_image depth;
    VkFramebuffer framebuffer;
};

struct vk_render_essentials
{
    VkImage *images;
    uint32_t image_count;
    VkQueue present_queue;
    VkCommandBuffer cmd_buffer;

    VkSemaphore sem_post_acquire;
    VkSemaphore sem_pre_submit;

    VkFence exec_fence;
    bool first_render;
};

struct vk_resources
{
    struct vk_image *images;
    uint32_t image_count;
    struct vk_buffer *buffers;
    uint32_t buffer_count;
    struct vk_shader *shaders;
    uint32_t shader_count;
    VkPushConstantRange *push_constants;
    uint32_t push_constant_count;
    VkRenderPass render_pass;
};

struct vk_layout
{
    struct vk_resources *resources;
    VkDescriptorSetLayout set_layout;
    VkPipelineLayout pipeline_layout;
};

struct vk_pipeline
{
    struct vk_layout *layout;
    VkPipelineVertexInputStateCreateInfo vertex_input_state;
    VkPipelineInputAssemblyStateCreateInfo input_assembly_state;
    VkPipelineTessellationStateCreateInfo tessellation_state;
    size_t thread_count;
    VkPipeline pipeline;
    VkDescriptorPool set_pool;
};

enum vk_render_pass_load_op
{
    VK_C_CLEAR = 0,
    VK_KEEP = 1,
};

enum vk_make_depth_buffer
{
    VK_WITHOUT_DEPTH = 0,
    VK_WITH_DEPTH = 1,
};

struct vk_offscreen_buffers
{
    VkExtent2D surface_size;
    struct vk_image color;
    struct vk_image depth;
    VkFramebuffer framebuffer;
};


struct app_data_struct{
    int iResolution[2]; //resolution
    int iMouse[2]; //mouse in window, it always updated (not like iMouse on shadertoy)
    int iMouse_lclick[2]; //mouse left click pos (its -[last pos] when left mosue not clicked)
    int iMouse_rclick[2]; //mouse right click pos (its -[last pos] when right mosue not clicked)
    bool iMouse_click[2]; //is mouse button clicked(left/right)
    float iTime; //time
    float iTimeDelta; //time delta
    int iFrame; //frames
    
    bool pause; //pause clicked
    bool quit; //quit clicked/happend
    bool drawdebug; //draw debug info, key press
};

#define APP_NAME_STR_LEN 80
struct app_os_window {
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    HINSTANCE connection;
    HWND window;
    POINT minsize;
#elif defined(VK_USE_PLATFORM_XCB_KHR)
    Display *display;
    xcb_connection_t *connection;
    xcb_screen_t *screen;
    xcb_window_t xcb_window;
    xcb_intern_atom_reply_t *atom_wm_delete_window;
#endif
    char name[APP_NAME_STR_LEN];
    
    bool prepared; // is vk setup prepared
    bool is_minimized; //window controled events
    bool resize_event; //window controled events
    bool fps_lock; //key pressed event
    bool reload_shaders_on_resize; //launch option
    bool print_debug; //launch option
    
    bool pause_refresh; //used only in Windows, on pause when key pressed refresh
    
    VkPresentModeKHR present_mode;
    struct app_data_struct app_data;
};


#endif
