# vulkan-shadertoy-launcher

**what is it** laucnher for **shadertoy shaders** in Vulkan.

**Use case** - launch your own shaders outside of shadertoy, or create a small single-bin(exe) file with built-in shaders, for demo-like apps. This code does not have Vulkan validation errors (and tested on Nvidia and AMD GPU-s).

*Minimal size* after upx compression - 30Kb, and using less than 1k lines of shader code building with yariv-shaders bin(exe)-size will be ~50Kb.

**Dependencies** - Vulkan library(sdk). MSVS or MinGW compiler for Windows. Linux requires *xcb* and *xcb-keysyms* library or *wayland-devel*.

Used Vulkan 1.0 and only single extension [VK_KHR_swapchain](https://vulkan.gpuinfo.org/listextensions.php) `VK_KHR_SWAPCHAIN_EXTENSION_NAME` that on 100% devices with Vulkan.

**Compiler/platform support** - VisualStudio, GCC/Clang, Mingw. OS Windows and Linux(X11/Wayland)

### **Bin builds** download:

**Download [from releases](https://github.com/danilw/vulkan-shadertoy-launcher/releases)**.

**Example shader in this download** - [shadertoy link](https://www.shadertoy.com/view/WlcBWr) shader use buffers cross-reading, feedback logic, and example for texture and keyboard support, only as an example that logic work same as on shadertoy.

### Contact: [**Join discord server**](https://discord.gg/JKyqWgt)

**Apps that I made using this code:** [glsl auto tetris](https://www.pouet.net/prod.php?which=85052), [glsl card game](https://www.pouet.net/prod.php?which=84806), [likes (glsl game)](https://www.pouet.net/prod.php?which=84805)

___
# How to use:

1. **Download** *Bin build* or build the project.

2. **Open** your shadertoy shader and copy-paste its code to `shaders/shadertoy/*.glsl` files.

3. **Small setup.** If buffers not needed, can be used simple dummy code for unused buffers:
```
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    discard;
}
```
*shadertoy textures* can be found on [Shadertoy Unofficial](https://shadertoyunofficial.wordpress.com/2019/07/23/shadertoy-media-files/)

### Binding:

**buffers** - `iChannels[0-3]` and `iChannel0-3` defined in the `shaders/src/*.frag` files.

**textures** - `iTextures[0-3]` is images from `*.png` files.

**keyboard** - `iKeyboard` shadertoy keyboard.

4. **Compile shaders** to `*.spv` using *glslangValidator* commands in the *shaders/build_shaders.cmd* or *shaders/build_shaders.sh*

5. **Launch** `VK_shadertoy_launcher.exe` and shader should work the same as on shadertoy

___
# Supported:

Buffers *(no mipmap)*, textures *(mipmap supported)*, keyboard, and mouse *(same as on shadertoy)*.

**Not supported** - audio/video, cubemaps, 3d texture.

Not implemented Uniforms - `iSampleRate` does not exist, `iChannelTime` does not exist, `iFrameRate` does not exist, `iChannelResolution` use in shader [textureSize](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/textureSize.xhtml) instead.

___
# Configuration:

To build *minimal bin(exe)* - a number of used buffers can be configured, stb_image can be disabled and yariv-shader format can be used to compress shaders. Look for `USEFUL defines` in the *main.c* file.

When you have just a *single shader*, and buffers not needed - use [vulkan-shader-launcher](https://github.com/danilw/vulkan-shader-launcher)

___
# Building:

`git clone https://github.com/danilw/vulkan-shadertoy-launcher`

**Use cmake to build.** 

Using VS2019 in the Windows (open links to see tutorial screenshot):

0. Download and install [Vulkan SDK](https://vulkan.lunarg.com/sdk/home#windows).
1. Launch VS press [Continue without code](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/vs_cmake_tut/1.png) and [File-Open-CMake](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/vs_cmake_tut/2.png) select [CMakeLists.txt file](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/vs_cmake_tut/3.png).
2. Press [Manage Configureations](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/vs_cmake_tut/4.png) and [Add new configuration](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/vs_cmake_tut/5.png) select [x64 Release](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/vs_cmake_tut/6.png).
3. In General [Configuration type set MinSizeRel](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/vs_cmake_tut/7.png) and press Ctrl+S to save.
4. Then [select created configuration](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/vs_cmake_tut/8.png) and press [Build-Build All](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/vs_cmake_tut/9.png).
5. Compiled build location *launcher\out\build\x64-Release\Vk_shadertoy.exe*, to launch required folder *shaders* and *textures*.


In the Linux use command:
```
cd launcher
mkdir build
cd build
cmake ..
make
```
To build with Wayland in *CMakeLists.txt* set `OPTION(USE_WAYLAND_LINUX "use Wayland for Linux" ON)` by default used X11. If your system missing *Wayland xdg-shell*, then look on Building without cmake below.

### Building without cmake:

First go to one of folders - **Wayland** `cd build_scripts/build_linux_wayland` or **x11** `cd build_scripts/build_linux_x11` or **mingw** `cd build_scripts/build_mingw_win`

Launch `sh build.sh`, or `sh build_yariv.sh` to have compressed single bin-file with buildin-shaders.

To build *yariv-shaders* `cd build_scripts/yariv_shaders` and `sh build_shaders_yariv.sh`

To generate *Wayland xdg-shel headers* use`sh gen_wayland_header.sh` in the *build_linux_wayland* folder.

___
This project includes **prebuild files:**

*build_scripts/win_vk_dll/vulkan-1.dll* file from Vulkan SDK needed to build using mingw.

*build_scripts/yariv_shaders/yariv_pack* this is compiled file, its source `build_scripts/yariv/main.c`, needed to generate yariv shaders.

*build_scripts/yariv_shaders/bin* folder with pre-build yariv shaders.

___
**Images:** 

![img](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/launcher_v1_1.png)


