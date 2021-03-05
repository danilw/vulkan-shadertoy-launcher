// https://www.shadertoy.com/view/WdBGzc

// Created by Danil (2019+) https://github.com/danilw
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

//short version
#define gc(x) vec3( .6 + .6 * cos( 6.3 *  x/s  + vec4(0,23,21,0)  ))

void mainImage( out vec4 O, in vec2 u ){
    O = vec4(vec3(0x1a,0x13,0x21)/float(0xff),1.);
    vec2 R = iResolution.xy,
    U = (u -.5*R) / R.y;
    if(max(abs(U.x),U.y) > .5){O=vec4(0.); return;}
    float s=1./8., //8. it number of tiles, set to any
    t = iTime,
    z = 1./ R.y;
    vec2 p=(fract(U/s)-.5),
    v=floor(U/s)+.5/s;
    s=1./s;
    bvec2 gv=bvec2(v.x==0.,v.y==s-1.);
    float r=(v.x+1.)/2.-.5,r1=(s-1.-v.y+1.)/2.-.5;
    if(gv.x)r=r1;if(gv.y)r1=r;
    vec3 ccx=gc(v.x);
    vec3 ccy=gc(s-1.-v.y);
    float dc=0.;
    for(float i=0.;i<2.;i++)
    dc=max(dc,smoothstep(z*s,-z*s,length(p+.45*vec2(sin((t+1./120.*i)*r-1.57),cos((t+1./120.*i)*r1-1.57))) - .015));
    O.rgb = mix(O.rgb,ccy+ccx-ccy*ccx,dc);
    vec4 yc=texture(iChannel3,u/R)*(1.-0.25*smoothstep(24.3,25.132,mod(t,25.132)));
    O=(gv.x||gv.y)?max(O,yc/(1.+.025*max(r,r1)/s)):max(O,yc/(1.+0.02*max(mod(t,25.132)/13.-1.,0.)));
}

