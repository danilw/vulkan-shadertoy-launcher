#!/bin/sh

gcc -DVK_USE_PLATFORM_XCB_KHR -O2 -s -lm -lxcb -lxcb-keysyms -lvulkan ../vk_utils/vk_utils.c ../vk_utils/vk_error_print.c ../vk_utils/vk_render_helper.c main.c -o VKexample
