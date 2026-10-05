#version 330 core
in vec2 uv;
uniform sampler2D picture;
uniform sampler2D chromaPicture;
uniform bool yuvVideo;
uniform mat3 yuvToRgb;
uniform vec3 yuvOffset;
uniform vec2 uvScale;
uniform vec2 uvOffset;
uniform int pattern;
uniform int blendMode;
uniform float phase;
uniform float dotRadius;
uniform float surfaceAspect;
uniform float brightness;
uniform float opacity;
uniform float density;
uniform float flow;
uniform float angle;
uniform int palette;
uniform float edgeFade;
out vec4 color;
vec2 rotatePoint(vec2 p,float a){float c=cos(a),s=sin(a);return vec2(c*p.x-s*p.y,s*p.x+c*p.y);}
float boxDistance(vec2 p,vec2 halfSize){vec2 d=abs(p)-halfSize;return length(max(d,0.0))+min(max(d.x,d.y),0.0);}
float shapeMask(float distance){float pixel=max(fwidth(distance),.002);return 1.0-smoothstep(-pixel,pixel,distance);}
// Continuous harmonics deform a shared wave field. Sampling at cell centres
// keeps each element coherent; time changes smoothly rather than reseeding it.
float movingField(vec2 p,float t){
    p+=flow*.7*vec2(sin(p.y*.83+t*.29),cos(p.x*.67-t*.23));
    float v=.5+.23*sin(p.x*1.63+p.y*.57-t*.81)+.18*cos(p.y*1.37-p.x*.43+t*.61)+flow*.09*sin(p.x*2.71+p.y*2.13-t*.37);
    return smoothstep(.12,.88,v);
}
vec3 selectedTint(vec3 original){
    if(palette==1)return vec3(1.0);
    if(palette==2)return vec3(.22,.85,1.0);
    if(palette==3)return vec3(.72,.4,1.0);
    if(palette==4)return vec3(1.0,.65,.2);
    if(palette==5)return vec3(.25,1.0,.65);
    return original;
}
void finishColor(){
    if(pattern>0){
        color.rgb=selectedTint(color.rgb);
        if(edgeFade>0.0){float edge=min(min(uv.x,1.0-uv.x),min(uv.y,1.0-uv.y));color.a*=smoothstep(0.0,edgeFade,edge);}
    }
    color.rgb*=brightness;color.a*=opacity;
    // Screen uses the effective (alpha-weighted) source colour. Transparent
    // pixels must neither brighten nor darken the layer below.
    if(blendMode==1)color.rgb*=color.a;
}
void main(){
    vec2 metric=rotatePoint((uv-.5)*vec2(1.0,1.0/surfaceAspect),angle);
    vec2 patternUv=metric*vec2(1.0,surfaceAspect)+.5;
    if(pattern>=5){
        vec2 field=metric*density;
        if(pattern==8){
            // Rows drift in different directions without resetting at a tile.
            float row=floor(field.y);field.x+=sin(row*.47+phase*.31)*flow*.6;
        }
        vec2 cell=floor(field),local=fract(field)-.5;
        vec2 p=(cell+.5)/density*7.0;
        float wave=movingField(p,phase);
        float distance;
        float hollow=0.0;
        float light=smoothstep(.04,.45,wave);
        if(pattern==5){
            // Dense square cells stretch through travelling wave crests.
            vec2 halfSize=vec2(dotRadius*(.04+.7*wave*wave*wave),dotRadius*(.15+.85*wave));
            distance=boxDistance(local,halfSize);
        }else if(pattern==6){
            float direction=.7+flow*(.5*sin(p.x*.53+p.y*.37+phase*.24)+.45*sin(phase*.35));
            vec2 stroke=rotatePoint(local,direction);
            float width=dotRadius*.26*(.35+.65*wave);
            distance=boxDistance(stroke,vec2(width,.07+.32*wave));
        }else if(pattern==7){
            // Squares become circles, open into rings, then contract again.
            float morph=.5+.5*sin(wave*5.0+phase*.34);
            float radius=dotRadius*(.16+.84*wave);
            float shape=mix(max(abs(local.x),abs(local.y)),length(local),morph);
            float opening=smoothstep(.52,.94,wave)*(.5+.5*sin(phase*.23+p.x*.43));
            distance=shape-radius;
            hollow=shapeMask(shape-radius*opening*.78)*smoothstep(.01,.08,opening);
        }else{
            float breathing=.5+.5*sin(p.x*.72-p.y*.61+phase*.53);
            vec2 halfSize=vec2(dotRadius*(.18+.82*wave),dotRadius*(.18+.82*mix(wave,breathing,flow*.6)));
            distance=boxDistance(local,halfSize);
        }
        color=vec4(vec3(1.0),shapeMask(distance)*light*(1.0-hollow));finishColor();return;
    }
    if(pattern==1){
        vec2 field=patternUv*vec2(density,density/surfaceAspect)+vec2(-phase*.8,phase*.4);
        vec2 cell=floor(field);
        float wave=.5+.5*sin(cell.x*.65+cell.y*.5-phase*2.0);
        float radius=dotRadius*(.65+.35*wave);
        float distanceToDot=length(fract(field)-.5);
        float aa=max(fwidth(distanceToDot),.002);
        float alpha=1.0-smoothstep(radius-aa,radius+aa,distanceToDot);
        vec3 tint=mix(vec3(.18,.75,1.0),vec3(.9,1.0,1.0),wave);
        color=vec4(tint,alpha);finishColor();return;
    }
    if(pattern>1){
        vec2 field=patternUv*vec2(density,density/surfaceAspect);
        float distanceToLine;
        vec3 tint;
        if(pattern==2){
            distanceToLine=abs(fract(field.x+field.y-phase*.8)-.5);
            tint=mix(vec3(.15,.65,1.0),vec3(.7,1.0,1.0),.5+.5*sin((field.x-field.y)*.3-phase));
        }else if(pattern==3){
            vec2 centred=metric*density;
            float radius=length(centred);
            distanceToLine=abs(fract(radius-phase*.8)-.5);
            tint=mix(vec3(.45,.22,1.0),vec3(1.0,.5,.85),.5+.5*sin(radius-phase));
        }else{
            // Distance to a repeating stepped polyline: horizontal plateaus
            // and vertical rises are joined, rather than disconnected bars.
            float x=mod(field.x-phase*.7,2.0);
            float y=mod(field.y+1.0,3.0)-1.0;
            float upperX=min(max(x-1.0,0.0),2.0-x);
            float lowerX=min(max(1.0-x,0.0),x);
            float horizontal=min(length(vec2(upperX,y)),length(vec2(lowerX,y-1.0)));
            float vertical=length(vec2(min(min(x,2.0-x),abs(x-1.0)),y-clamp(y,0.0,1.0)));
            distanceToLine=min(horizontal,vertical);
            tint=mix(vec3(.1,1.0,.65),vec3(.8,1.0,.2),.5+.5*sin(field.y*.4-phase));
        }
        float aa=max(fwidth(distanceToLine),.002);
        float alpha=1.0-smoothstep(dotRadius-aa,dotRadius+aa,distanceToLine);
        color=vec4(tint,alpha);finishColor();return;
    }
    vec2 p=uv*uvScale+uvOffset;if(any(lessThan(p,vec2(0.0)))||any(greaterThan(p,vec2(1.0))))discard;
    if(yuvVideo){vec3 yuv=vec3(texture(picture,p).r,texture(chromaPicture,p).rg);color=vec4(clamp(yuvToRgb*(yuv-yuvOffset),0.0,1.0),1.0);}
    else color=texture(picture,p);
    finishColor();
}
