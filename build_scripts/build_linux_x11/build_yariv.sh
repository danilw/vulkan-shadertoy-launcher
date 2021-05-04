#!/bin/sh

gcc -DVK_USE_PLATFORM_XCB_KHR -DYARIV_SHADER -Os -s -I. -fdata-sections -ffunction-sections -Wl,--gc-sections ../../vk_utils/vk_utils.c ../../vk_utils/vk_error_print.c ../../vk_utils/vk_render_helper.c ../../launcher/main.c -o VK_shadertoy_compressed -lm -lxcb -lxcb-keysyms -lvulkan

#clang 

strip VK_shadertoy_compressed
upx VK_shadertoy_compressed
