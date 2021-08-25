#!/bin/sh

x86_64-w64-mingw32-gcc -DVK_USE_PLATFORM_WIN32_KHR -DUSE_stb_image -Os -s -lm -mwindows -fdata-sections -ffunction-sections -Wl,--gc-sections ../../vk_utils/vk_utils.c ../../vk_utils/vk_error_print.c ../../vk_utils/vk_render_helper.c ../../launcher/main.c -o VK_shadertoy.exe ../win_vk_dll/vulkan-1.dll

#upx VK_shadertoy.exe

rm -rf shaders
rm -rf textures

cp -r ../../launcher/shaders shaders
cp -r ../../launcher/textures textures
