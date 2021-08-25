#!/bin/sh

gcc -DVK_USE_PLATFORM_XCB_KHR -DUSE_stb_image -Os -s -fdata-sections -ffunction-sections -Wl,--gc-sections ../../vk_utils/vk_utils.c ../../vk_utils/vk_error_print.c ../../vk_utils/vk_render_helper.c ../../launcher/main.c -o VK_shadertoy -lm -lxcb -lxcb-keysyms -lvulkan

#clang -DVK_USE_PLATFORM_XCB_KHR -DUSE_stb_image -Os -s ../../vk_utils/vk_utils.c ../../vk_utils/vk_error_print.c ../../vk_utils/vk_render_helper.c ../../launcher/main.c -o VK_shadertoy -lm -lxcb -lxcb-keysyms -lvulkan

strip VK_shadertoy
#upx VK_shadertoy

rm -rf shaders
rm -rf textures

cp -r ../../launcher/shaders shaders
cp -r ../../launcher/textures textures
