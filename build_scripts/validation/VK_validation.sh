#!/bin/sh

## this script DOES NOT WORK in the Ubuntu 21.04, copy paste these commands to the console
## and launch Vulkan app manually from console to have Validation layers work

## path example /home/user/vulkan_sdk/1.2.131.2/
VULKAN_SDK="<path to Vulkan SDK/version>"
VK_APP=$1

. $VULKAN_SDK"/setup-env.sh"

export VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation
./$VK_APP
