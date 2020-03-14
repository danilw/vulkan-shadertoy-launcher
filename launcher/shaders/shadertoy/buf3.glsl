
// using https://www.shadertoy.com/view/lsjBWy

float lineforcorner(vec2 p, float rot, float size) {
    return length(max(abs(p)-vec2(size*2.0-rot,rot),vec2(0.0)));
}

float corner (vec2 p,float size) {
    return lineforcorner(p-size*sign(p.x+p.y+0.0001),sign(p.x+p.y+0.0001)*size+size,size);
}

//line df by iq http://iquilezles.org/www/articles/distfunctions/distfunctions.htm
float line( vec2 p, vec2 a, vec2 b ) 
{
    vec2 pa = p-a, ba = b-a;
    float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
    return length( pa - ba*h );
}

float halfdonut(vec2 p, float size) {
    vec2 p2 = p;
    p2.y = max(p.y,0.0);
    return length(vec2(abs(length(p2)-size),p.y-p2.y));
}

float quarterdonut(vec2 p, float size) {
    float len = length(max(p,vec2(0.0)))-size+min(max(p.x,p.y),0.0);
    return length(vec2(len,min(min(p.x,p.y),0.0)));
}

float halfdonutfor3(vec2 p, float size) {
    vec2 p2 = p;
    p2.y = max(p.y,0.0);
    return length(vec2(abs(length(p2)-size),min(p.y,-size/2.0)+size/2.0));
}


float num1(vec2 p, float size) {
    return length(max(abs(p)-vec2(0.0,size),vec2(0.0)));
}
float num0(vec2 p, float size) {
    return abs(num1(p,size/2.0)-size/2.0);
}
float num2(vec2 p, float size) {
    return min(min(
        halfdonut(p-vec2(0.0,size/2.0),size/2.0),
        line(p,vec2(size/2.0),vec2(-size/2.0,-size))),
        length(max(abs(p-vec2(0.0,-size))-vec2(size/2.0,0.0),vec2(0.0))));
}
float num3(vec2 p, float size) {
    return halfdonutfor3(vec2(abs(p.y)-size/2.0,p.x),size/2.0);
}
float num4(vec2 p, float size) {
    return min(
        num1(p-vec2(size/2.0,0.0),size),
        corner(vec2(-p.x,p.y)-size/4.0,size/4.0));
}
float num5(vec2 p, float size) {
    return min(min(
        corner(-p-vec2(size)*vec2(0.5/4.0,-3.5/4.0),size/8.0),
        halfdonut(vec2(p.x,abs(p.y+size/8.0)-size*(0.5+1.0/8.0)),size/4.0)),
        num1(p-vec2(size/4.0,-size/8.0),size*(0.5+1.0/8.0)));
}
float num6(vec2 p, float size) {
    return min(min(
        num0(p-vec2(0.0,-size/2.0),size/2.0),
        halfdonut(p-vec2(0.0,size-size/4.0),size/4.0)),
        num1(p-vec2(-size/4.0,size/4.0),size/2.0));
}
float num7(vec2 p, float size) {
    return min(
        length(max(abs(p-vec2(0.0,size))-vec2(size/2.0,0.0),vec2(0.0))),
        line(p,vec2(size/2.0,size),vec2(-size/2.0,-size)));
}
float num8(vec2 p, float size) {
    return abs(length(abs(p)-vec2(0.0,size/2.0))-size/2.0);
}
float num9(vec2 p, float size) {
    return num6(-p,size);
}

float num(vec2 p, float size, float num) {
    float len = length(p-vec2(-0.75,-0.3))-0.01;
    num *= 100.0;
    
    while (num >= 1.0) {
        float len2;
        int tv=(int(num)%10);
            if(tv == 0) len2 = num0(p,size);
            if(tv == 1) len2 = num1(p,size);
            if(tv == 2) len2 = num2(p,size);
            if(tv == 3) len2 = num3(p,size);
            if(tv == 4) len2 = num4(p,size);
            if(tv == 5) len2 = num5(p,size);
            if(tv == 6) len2 = num6(p,size);
            if(tv == 7) len2 = num7(p,size);
            if(tv == 8) len2 = num8(p,size);
            if(tv == 9) len2 = num9(p,size);
        len = min(len,len2);
        num /= 10.0;
        p.x+=0.5;
    }
    
    return len;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord)
{
	vec2 uv = (fragCoord.xy * 2.0 - iResolution.xy) / iResolution.y*1.5;
    
    float len = num(uv-vec2(2.4,1.0),0.3,float(iFrame));
    len = min(len,num(uv-vec2(2.4,0.0),0.3,iTime));

    vec3 col = len*(0.5 + 0.5*sin(64.0*len))*vec3(1.0);
    col = mix( vec3(1.0,0.6,0.0), col, smoothstep( 0.01, 0.04, len ) );
    
    fragColor = vec4(col,1.0);
}
