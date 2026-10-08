#version 330 core
in vec2 uv;
uniform float seconds;
uniform float reveal;
uniform float rank;
uniform vec2 surfacePixels;
uniform float surfaceOpacity;
uniform float brightness;
out vec4 color;

float stroke(float distance,float width){
    float aa=max(fwidth(distance),.35);
    return 1.0-smoothstep(width-aa,width+aa,distance);
}
float glow(float distance,float scale){return exp(-distance/max(scale,.01));}
void main(){
    vec2 size=max(surfacePixels,vec2(8.0));
    vec2 p=uv*size;
    float t=seconds;
    float envelope=1.0-reveal;
    float edge=min(min(p.x,size.x-p.x),min(p.y,size.y-p.y));
    // A continuous perimeter coordinate sends the trace around the actual mesh.
    float along;
    if(edge==p.y)along=uv.x*.25;
    else if(edge==size.x-p.x)along=.25+uv.y*.25;
    else if(edge==size.y-p.y)along=.5+(1.0-uv.x)*.25;
    else along=.75+(1.0-uv.y)*.25;
    float trace=smoothstep(.3+rank*.25,2.7+rank*.25,t);
    float trail=1.0-smoothstep(trace-.008,trace+.008,along);
    float head=glow(abs(along-trace),.018)*(1.0-smoothstep(2.65,3.1,t));
    float border=(stroke(edge,1.0)*(.42*trail+head)+glow(edge,5.0)*head*.3);

    // Cells use approximately fixed projector pixels, not a fixed count per panel.
    float cell=64.0;
    float solve=smoothstep(1.9+rank*.18,4.6,t);
    float lock=smoothstep(4.5,5.45,t);
    vec2 metric=p+(1.0-lock)*vec2(sin(p.y/cell+t)*1.6,cos(p.x/cell-t)*1.6)*solve;
    vec2 nearest=mod(metric+cell*.5,cell)-cell*.5;
    float grid=stroke(min(abs(nearest.x),abs(nearest.y)),.65);
    float diagonal=stroke(abs(mod(metric.x-metric.y+cell*.5,cell)-cell*.5)*.7071,.55);
    float assembled=1.0-smoothstep(solve-.04,solve+.04,(uv.x+uv.y)*.5);
    float weave=(grid*.30+diagonal*.13)*assembled*solve;

    float scan=(t-1.7-rank*.12)/3.4;
    float beamDistance=abs(uv.x-scan)*size.x;
    float beam=(stroke(beamDistance,1.0)*.85+glow(beamDistance,17.0)*.18)*smoothstep(.8,1.6,t)*(1.0-smoothstep(5.0,5.5,t));
    float nodes=(stroke(length(nearest),1.5)*.60+glow(length(nearest),4.0)*.16)*assembled*solve;
    float lockRing=glow(abs(length((uv-.5)*size)-min(size.x,size.y)*(t-4.65)*.72),5.0)*smoothstep(4.6,4.8,t)*(1.0-smoothstep(5.25,5.6,t));

    // Corner brackets acquire and settle. No changing random seeds or hard flashes.
    vec2 corner=min(p,size-p);
    float bracket=(stroke(corner.x,1.1)*(1.0-smoothstep(13.0,17.0,corner.y))+stroke(corner.y,1.1)*(1.0-smoothstep(13.0,17.0,corner.x)))*smoothstep(.0,.55,t);
    float breathing=.85+.15*sin(t*3.2-rank);
    float ink=clamp((border+weave+beam+nodes+bracket*.6+lockRing*.6)*envelope*breathing,0.0,1.0);
    vec3 tint=mix(vec3(.20,.72,.83),vec3(.83,1.0,.98),clamp(beam+bracket+border,0.0,1.0));
    // Content fades in separately. Transparent ink also keeps translucent panels continuous.
    color=vec4(tint*brightness,ink*surfaceOpacity);
}
