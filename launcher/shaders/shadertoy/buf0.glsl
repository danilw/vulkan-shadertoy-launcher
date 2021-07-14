void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec4 data1=texelFetch(iChannel1,ivec2(0,0),0);
    vec4 data2=texelFetch(iChannel1,ivec2(int(iResolution.x-1.),0),0);
    vec4 data3=texelFetch(iChannel1,ivec2(0,int(iResolution.y-1.)),0);
    vec4 data4=texelFetch(iChannel1,ivec2(int(iResolution.x-1.),int(iResolution.y-1.)),0);
    
    vec2 res=iResolution.xy/iResolution.y;
    vec2 uv=fragCoord/iResolution.y-0.5*res;
    
    vec3 col=vec3(0.);
    if((data1==data2)&&(data1==data3)&&(data1==data4))col=vec3(1.);
    else col=vec3(1.,0.,0.);
    
    // expected data1.y==iFrame -1 and data1.z==iFrame
    if((int(data1.y)!=iFrame-1)||(int(data1.z)!=iFrame))col=vec3(1.,0.,1.);
    if((int(data4.z)!=iFrame))col=vec3(col.r,1.,col.b*0.5);
    
    vec3 c1=print_n(uv+vec2(0.,-0.2),data1.x);
    c1+=print_n(uv+vec2(0.,0.0),data1.y);
    c1+=print_n(uv+vec2(0.,0.2),data1.z);
    
    fragColor=vec4(c1*col,1.);
    
    if(iMouse.z>0.0){
        vec2 im = iMouse.xy/iResolution.y-0.5*res;
        fragColor.r+=0.8*(1.-smoothstep(0.1,0.1+1.5/iResolution.y,length(uv-im)));
    }
}
