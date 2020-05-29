# vulkan-shadertoy-launcher

base on [vulkan-shader-launcher](https://github.com/danilw/vulkan-shader-launcher)

**what is it** laucnher for **[shadertoy](https://www.shadertoy.com/) shaders** in Vulkan. Read **how to use**, support features and limitations below.

**Dependencies** - Vulkan library(sdk). MSVS or MinGW compiler for Windows. Linux require *xcb* and *xcb-keysyms* library.

**Compiler/platform support** - VisualStudio, GCC/Clang, Mingw. OS Windows and Linux(X11)

### **Bin builds** download:

Used shaders for examples: shadertoy links *[example1](https://www.shadertoy.com/view/wlX3zn)*, *[example2](https://www.shadertoy.com/view/MstGWX)*, *[example3](https://www.shadertoy.com/view/lsXGzf)*

**Download [from releases](https://github.com/danilw/vulkan-shadertoy-launcher/releases)**, or direct links latest build: Windows **[Download Win64](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/shatertoy_launcher/vulkan-shadertoy-launcher_win.zip)** and Linux **[Download Linux64](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/shatertoy_launcher/vulkan-shadertoy-launcher_linux.zip)**

### Contact: [**Join discord server**](https://discord.gg/JKyqWgt)

**Apps that I made using this code:** [glsl auto tetris](https://www.pouet.net/prod.php?which=85052), [glsl card game](https://www.pouet.net/prod.php?which=84806), [likes (glsl game)](https://www.pouet.net/prod.php?which=84805), 

___
TODO:

0. Wayland support and proper Linux build in Ubuntu LTS (current build use latest libraries, not stable).
1. video as texture, sound and 3d textures support, and mipmap for buffers
2. config file support and tile render
3. export shaders to video frame by frame (using ffmpeg)

___
# How to use

1. **Download** *Bin build* or build project yourself. as example launcher name `VK_shadertoy_launcher.exe`

2. **Open** any shadertoy shader and copy-paste its code to `shaders/shadertoy/*.glsl` files.

3. **Small setup.** If buffers not needed, can be used simple dummy code for unused buffers 
```
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    discard;
}
```
*shadertoy textures* can be found on [Shadertoy Unofficial](https://shadertoyunofficial.wordpress.com/2019/07/23/shadertoy-media-files/)

**binding rules/order:** look files `shaders/src/*.frag` example:
```
#define iChannel0 iChannels[0]
#define iChannel1 iChannels[1]
#define iChannel2 iChannels[2]
#define iChannel3 iChannels[3]
```
change in any order you need, `iTextures` is images from `*.png` files. Default order \<0-4\> by file name.

**textures** names in `textures/` folder \<1-4\>.png files

4. **Compile shaders** to `*.spv` format using *glslangValidator* command list in file *build_shaders.cmd* or *build_shaders.sh*

5. **Launch** `VK_shadertoy_launcher.exe` and it shoult work same as on shadertoy

___
# Support features and extra

0. **Input** *Mouse and Keyboard* same as on shadertoy.

1. **Textures** mipmaps enabled by default, edit in `main.c` option `USE_MIPMAPS`, all textures with VFlip to change remove `stbi_set_flip_vertically_on_load(true);`

2. **Number of Buf and textures** can be configured in `main.c`, set `OFFSCREEN_BUFFERS` and `IMAGE_TEXTURES` to any number you need, after changing its values also edit `shaders/src/*.frag` files, same values there. *For each buffer and image **required** its own `*.spv` or `*.png` file in those folders.*

3. **Static resolution** uncomment `#define NO_RESIZE_BUF` in `main.c` to make buffers not resize on window resize.

4. **Buffers format** all Buffers `VK_FORMAT_R32G32B32A32_SFLOAT` images `VK_FORMAT_R8G8B8A8_UNORM`, to change edit it in `main.c` code.

*extra VkQueue* - used *1 VkQueue per buffer* when available, or *1 VkQueue* for everything.

*extra CommandBuffer* - set `#define ONE_CMD` in `main.c`(line 10) to use only *2 CommandBuffers* for everything always, by default used *1 CommandBuffer* per VkQueue, or *all CommanBuffers in single VkQueue* when `VkQueue<Num of buffers`

**Not supported right now**

1. Only 2D textures(images), cubemaps, sound, and 3d textures not supported.\<TODO\>

2. Buffers only Linear filtering. to have `Nearest` fileter, `texelFetch` in shaders can be used instead of `texture`.

3. List of not implemented Uniforms - `iSampleRate` does not exist now, `iChannelTime` does not exist, `iFrameRate` does not exist, `iChannelResolution` use in shader [textureSize](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/textureSize.xhtml) instead.

Everything else work same as on shadertoy.

___
# Building

**Use cmake to build.** In Windows VS press *open Cmake*.
```
cd launcher
mkdir build
cd build
cmake ..
make
```

*Building without cmake:*

Build with **gcc** (linux): (to build with *clang* change *gcc* to *clang*)
```
gcc -DVK_USE_PLATFORM_XCB_KHR -O2 -s -lm -lxcb -lxcb-keysyms -lvulkan ../vk_utils/vk_utils.c ../vk_utils/vk_error_print.c ../vk_utils/vk_render_helper.c main.c -o VK_shadertoy_launcher
```
Build with **mingw64** (*vulkan-1.dll* from VulkanSDK, *vulkan.h* in system(cygwin or native) path):
```
x86_64-w64-mingw32-gcc -DVK_USE_PLATFORM_WIN32_KHR -O2 -s -lm -mwindows ../vk_utils/vk_utils.c ../vk_utils/vk_error_print.c ../vk_utils/vk_render_helper.c main.c -o VK_shadertoy_launcher.exe <path to>/vulkan-1.dll
```

**Images:** 

![img](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/shatertoy_launcher/img.png)


