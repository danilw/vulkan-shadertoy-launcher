// self https://www.shadertoy.com/view/WlcBWr

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
          break;
        case 2:col=vec4(texture(iChannel2,uv).rgb,1.);break;
        case 3:col=vec4(texture(iChannel3,uv).rgb,1.);break;
    }
    fragColor = col;
}
