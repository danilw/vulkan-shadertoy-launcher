
// Created by Danil (2021+) https://twitter.com/AruGL
// The MIT License

// self https://www.shadertoy.com/view/WlcBWr

void mainImage_keyboard( out vec4 fragColor, in vec2 fragCoord );
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv = fragCoord/iResolution.xy;
    int idx=int(uv.x*2.)+int(uv.y*2.)*2;
    uv=fract(uv*2.);
    vec4 col = vec4(0.);
    switch(idx){
        case 0:col=vec4(texture(iChannel0,uv).rgb,1.);break;
        case 1:
          col=vec4(vec3(1.),0.);
          float d=print_n(uv+vec2(-0.5,-0.5-0.3),iMouse.x).x;
          d+=print_n(uv+vec2(-0.5,-0.5-0.1),iMouse.y).x;
          d+=print_n(uv+vec2(-0.5,-0.5+0.1),iMouse.z).x;
          d+=print_n(uv+vec2(-0.5,-0.5+0.3),iMouse.w).x;
          col.rgb*=1.-d;col.a=d;
          vec4 tc;
          mainImage_keyboard(tc,(uv*iResolution.xy));
          col.rgb=col.rgb*0.35+tc.rgb;
          break;
        case 2:col=vec4(texture(iChannel2,uv).rgb,1.);break;
        case 3:col=vec4(texture(iChannel3,uv).rgb,1.);break;
    }
    
    
    vec2 tuv=fragCoord/iResolution.xy;
    tuv*=5.;
    if((abs(tuv.x-0.5)<0.5)&&(abs(tuv.y-0.5)<0.5))
    col.rgb = texture(iTextures[0], tuv).rgb;
    
    
    fragColor = col;
    
    if(iMouse.z>0.0){
        vec2 res=iResolution.xy/iResolution.y;
        uv=fragCoord/iResolution.y-0.5*res;
        vec2 im = iMouse.xy/iResolution.y-0.5*res;
        fragColor.g+=0.8*(1.-smoothstep(0.1,0.1+1.5/iResolution.y,length(uv-im)));
    }
    
}


// modified
// https://www.shadertoy.com/view/lsXGzf

# define keyboard_texture iKeyboard

const int KEY_LEFT  = 37;
const int KEY_UP    = 38;
const int KEY_RIGHT = 39;
const int KEY_DOWN  = 40;

void mainImage_keyboard( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv = (-iResolution.xy + 2.0*fragCoord) / iResolution.y;

    vec3 col = vec3(0.0);

    // state
    col = mix( col, vec3(1.0,0.0,0.0), 
        (1.0-smoothstep(0.3,0.31,length(uv-vec2(-0.75,0.0))))*
        (0.3+0.7*texelFetch( keyboard_texture, ivec2(KEY_LEFT,0), 0 ).x) );

    col = mix( col, vec3(1.0,1.0,0.0), 
        (1.0-smoothstep(0.3,0.31,length(uv-vec2(0.0,0.5))))*
        (0.3+0.7*texelFetch( keyboard_texture, ivec2(KEY_UP,0), 0 ).x));
  
    col = mix( col, vec3(0.0,1.0,0.0),
        (1.0-smoothstep(0.3,0.31,length(uv-vec2(0.75,0.0))))*
        (0.3+0.7*texelFetch( keyboard_texture, ivec2(KEY_RIGHT,0), 0 ).x));

    col = mix( col, vec3(0.0,0.0,1.0),
        (1.0-smoothstep(0.3,0.31,length(uv-vec2(0.0,-0.5))))*
        (0.3+0.7*texelFetch( keyboard_texture, ivec2(KEY_DOWN,0), 0 ).x));


    // keypress 
    col = mix( col, vec3(1.0,0.0,0.0), 
        (1.0-smoothstep(0.0,0.01,abs(length(uv-vec2(-0.75,0.0))-0.35)))*
        texelFetch( keyboard_texture, ivec2(KEY_LEFT,1),0 ).x);
  
    col = mix( col, vec3(1.0,1.0,0.0),
        (1.0-smoothstep(0.0,0.01,abs(length(uv-vec2(0.0,0.5))-0.35)))*
        texelFetch( keyboard_texture, ivec2(KEY_UP,1),0 ).x);

    col = mix( col, vec3(0.0,1.0,0.0),
        (1.0-smoothstep(0.0,0.01,abs(length(uv-vec2(0.75,0.0))-0.35)))*
        texelFetch( keyboard_texture, ivec2(KEY_RIGHT,1),0 ).x);
  
    col = mix( col, vec3(0.0,0.0,1.0),
        (1.0-smoothstep(0.0,0.01,abs(length(uv-vec2(0.0,-0.5))-0.35)))*
        texelFetch( keyboard_texture, ivec2(KEY_DOWN,1),0 ).x);
    
    
    // toggle 
    col = mix( col, vec3(1.0,0.0,0.0), 
        (1.0-smoothstep(0.0,0.01,abs(length(uv-vec2(-0.75,0.0))-0.3)))*
        texelFetch( keyboard_texture, ivec2(KEY_LEFT,2),0 ).x);
  
    col = mix( col, vec3(1.0,1.0,0.0),
        (1.0-smoothstep(0.0,0.01,abs(length(uv-vec2(0.0,0.5))-0.3)))*
        texelFetch( keyboard_texture, ivec2(KEY_UP,2),0 ).x);

    col = mix( col, vec3(0.0,1.0,0.0),
        (1.0-smoothstep(0.0,0.01,abs(length(uv-vec2(0.75,0.0))-0.3)))*
        texelFetch( keyboard_texture, ivec2(KEY_RIGHT,2),0 ).x);
  
    col = mix( col, vec3(0.0,0.0,1.0),
        (1.0-smoothstep(0.0,0.01,abs(length(uv-vec2(0.0,-0.5))-0.3)))*
        texelFetch( keyboard_texture, ivec2(KEY_DOWN,2),0 ).x);

    fragColor = vec4(col,1.0);
}
