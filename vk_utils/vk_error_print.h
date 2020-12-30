
// Danil, 2020 Vulkan shader launcher, self https://github.com/danilw/vulkan-shader-launcher
// The MIT License

#ifndef vk_utils_printf_H
#define vk_utils_printf_H

#include <vulkan/vulkan.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#ifdef _GNUC
#define ATTR_UNUSED __attribute__((format(printf, 3, 4)))
#else
#define ATTR_UNUSED
#endif

enum vk_error_type
{
    VK_ERROR_SUCCESS = 0,
    VK_ERROR_VKRESULT,
    VK_ERROR_VKRESULT_WARNING,  /* VK_INCOMPLETE for example */
    VK_ERROR_ERRNO,
};

typedef struct vk_error_data
{
    enum vk_error_type type;
    union {
        VkResult vkresult;
        int err_no;
    };
    const char *file;
    unsigned int line;
} vk_error_data;

typedef struct vk_error
{
    struct vk_error_data error;
    struct vk_error_data sub_error; /*
                         * Used in cases where error is e.g. "VK_INCOMPLETE", and it is due to
                         * another error.
                         */
} vk_error;

#define VK_ERROR_NONE (struct vk_error){ .error = { .type = VK_ERROR_SUCCESS,}, .sub_error = { .type = VK_ERROR_SUCCESS,}, }

#define vk_error_set_vkresult(es, e)     vk_error_data_set_vkresult(&(es)->error,     (e), __FILE__, __LINE__)
#define vk_error_set_errno(es, e)        vk_error_data_set_errno   (&(es)->error,     (e), __FILE__, __LINE__)
#define vk_error_sub_set_vkresult(es, e) vk_error_data_set_vkresult(&(es)->sub_error, (e), __FILE__, __LINE__)
#define vk_error_sub_set_errno(es, e)    vk_error_data_set_errno   (&(es)->sub_error, (e), __FILE__, __LINE__)
#define vk_error_merge(es, os)                                \
do {                                                            \
    if (vk_error_data_merge(&(es)->error, &(os)->error))  \
        (es)->sub_error = (os)->sub_error;              \
} while (0)
#define vk_error_sub_merge(es, os)       vk_error_data_merge(&(es)->sub_error, &(os)->error)

void vk_error_data_set_vkresult(struct vk_error_data *error, VkResult vkresult, const char *file, unsigned int line);
void vk_error_data_set_errno(struct vk_error_data *error, int err_no, const char *file, unsigned int line);
bool vk_error_data_merge(struct vk_error_data *error, struct vk_error_data *other);

bool vk_error_is_success(struct vk_error *error);
bool vk_error_is_warning(struct vk_error *error);
bool vk_error_is_error(struct vk_error *error);
#if defined(VK_USE_PLATFORM_WIN32_KHR)
void win_error(char *iout, char *iout2);
#endif
#define vk_error_printf(es, ...) vk_error_fprintf(stdout, (es), __VA_ARGS__)
void vk_error_fprintf(FILE *fout, struct vk_error *error, const char *fmt, ...) ATTR_UNUSED;


const char *vk_VkPhysicalDeviceType_string(VkPhysicalDeviceType type);


#endif
