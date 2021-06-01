#version 450

#extension GL_ARB_separate_shader_objects: enable
#extension GL_ARB_shading_language_420pack: enable
#extension GL_GOOGLE_include_directive : enable

layout (location = 0) in vec2 frag_pos;

#define OFFSCREEN_BUFFERS 4
#define IMAGE_TEXTURES 4

layout (set = 0, binding = 0) uniform sampler2D iTextures[IMAGE_TEXTURES]; //textures
layout (set = 0, binding = IMAGE_TEXTURES) uniform sampler2D iChannels[OFFSCREEN_BUFFERS]; //buffers
layout (set = 0, binding = IMAGE_TEXTURES+OFFSCREEN_BUFFERS) uniform sampler2D iKeyboard; //keyboard

//define in any order
#define iChannel0 iChannels[0]
#define iChannel1 iChannels[1]
#define iChannel2 iChannels[2]
#define iChannel3 iChannels[3]

layout (push_constant) uniform push_constants
{
    vec4 u_Mouse;
    vec4 u_Date;
    bvec2 u_Mouse_lr;
    vec2 u_Resolution;
    bool u_debugdraw;
    int pCustom;
    float u_Time;
    float u_TimeDelta;
    int u_Frame;
} constants;

vec3 iResolution=vec3(constants.u_Resolution,1.);
float iTime=constants.u_Time;
float iTimeDelta=constants.u_TimeDelta;
int iFrame=constants.u_Frame;
vec4 iMouse=constants.u_Mouse;
vec4 iDate=constants.u_Date;
bool is_debugdraw=constants.u_debugdraw;
bool is_pause=bool(constants.pCustom-(constants.pCustom/10)*10);
bool main_image_srgb=bool((constants.pCustom/10)*10-(constants.pCustom/100)*100);

layout (location = 0) out vec4 out_color;

#include "../shadertoy/common.glsl"
#include "../shadertoy/main_image.glsl"

void main()
{
    vec4 uFragColor=vec4(0.);
    vec2 fragCoord=gl_FragCoord.xy;
    fragCoord.y=iResolution.y-fragCoord.y; // shadertoy v(y)-flip main_image
    
    mainImage(uFragColor,fragCoord);
    out_color=uFragColor;
    //if(is_pause)out_color=vec4(vec3(dot(clamp(out_color.rgb,0.,1.),vec3(1.))/3.),1.);
    if(main_image_srgb)out_color.rgb = ((exp2(out_color.rgb)-1.0)-out_color.rgb*0.693147)*3.258891;
}
