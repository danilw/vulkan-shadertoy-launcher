void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    fragColor = vec4(0.0,0.0,0.0,1.0);
    
    ivec2 ipx=ivec2(fragCoord);
    if((ipx==ivec2(0,0))||(ipx==ivec2(int(iResolution.x-1.),0))
    ||(ipx==ivec2(0,int(iResolution.y-1.)))||(ipx==ivec2(int(iResolution.x-1.),int(iResolution.y-1.)))){
        vec4 data=texelFetch(iChannel1,ipx,0);
        fragColor=vec4(iTime,float(iFrame),data.z+1.,0.);
    }
}
