#!/bin/sh

gcc -DVK_USE_PLATFORM_WAYLAND_KHR -DUSE_stb_image -Os -s -I. -I/usr/include/wayland ../../vk_utils/vk_utils.c ../../vk_utils/vk_error_print.c ../../vk_utils/vk_render_helper.c xdg-shell-client-protocol.c ../../launcher/main.c -o VK_shadertoy_wayland  -lm -lxcb -lvulkan -lwayland-client

#clang 

strip VK_shadertoy_wayland
#upx VK_shadertoy_wayland

rm -rf shaders
rm -rf textures

cp -r ../../launcher/shaders shaders
cp -r ../../launcher/textures textures
