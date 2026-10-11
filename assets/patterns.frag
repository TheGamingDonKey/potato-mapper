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
// Fixed spatial seeds interpolate continuously; animation moves coordinates.
float cellSeed(vec2 p){return fract(sin(dot(p,vec2(127.1,311.7)))*43758.5453);}
float smoothNoise(vec2 p){
    vec2 i=floor(p),f=fract(p);f=f*f*f*(f*(f*6.0-15.0)+10.0);
    return mix(mix(cellSeed(i),cellSeed(i+vec2(1,0)),f.x),mix(cellSeed(i+vec2(0,1)),cellSeed(i+vec2(1)),f.x),f.y);
}
vec2 organicWarp(vec2 p,float t,float amount){
    return p+amount*vec2(sin(p.y*1.3+t*.31)+.35*cos(p.x*.9-t*.23),cos(p.x*1.1-t*.27)+.35*sin(p.y*.8+t*.19));
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
        // Seasonal illustrations keep their shadows, ribs and facial detail
        // when a colour preset is selected. Older effects retain their tint.
        if((pattern==14||pattern==15)&&palette!=0)
            color.rgb=selectedTint(vec3(1.0))*dot(color.rgb,vec3(.25,.60,.15));
        else color.rgb=selectedTint(color.rgb);
        if(edgeFade>0.0){float edge=min(min(uv.x,1.0-uv.x),min(uv.y,1.0-uv.y));color.a*=smoothstep(0.0,edgeFade,edge);}
    }
    color.rgb*=brightness;color.a*=opacity;
    // Screen uses the effective (alpha-weighted) source colour. Transparent
    // pixels must neither brighten nor darken the layer below.
    if(blendMode==1)color.rgb*=color.a;
}
float harvestSegment(vec2 p,vec2 a,vec2 b){
    vec2 edge=b-a;
    return length(p-a-edge*clamp(dot(p-a,edge)/dot(edge,edge),0.0,1.0));
}
float harvestEllipse(vec2 p,vec2 radii){
    return (length(p/radii)-1.0)*min(radii.x,radii.y);
}
// Five pointed lobes and deep notches, rather than an oval autumn leaf.
// Six instances below bound the work to 72 outline edges per fragment.
float harvestMaple(vec2 p){
    const vec2 outline[12]=vec2[12](
        vec2(0.0,-1.0),vec2(.22,-.43),vec2(.55,-.65),vec2(.52,-.08),
        vec2(1.0,-.15),vec2(.55,.35),vec2(0.0,.44),vec2(-.55,.35),
        vec2(-1.0,-.15),vec2(-.52,-.08),vec2(-.55,-.65),vec2(-.22,-.43));
    float distance=10.0,side=1.0;
    for(int i=0;i<12;++i){
        vec2 a=outline[i],b=outline[(i+1)%12],edge=b-a;
        vec2 nearest=p-a-edge*clamp(dot(p-a,edge)/dot(edge,edge),0.0,1.0);
        distance=min(distance,dot(nearest,nearest));
        float safeY=edge.y+(edge.y>=0.0?.00001:-.00001);
        if((a.y>p.y)!=(b.y>p.y))
            if(p.x<a.x+edge.x*(p.y-a.y)/safeY)side=-side;
    }
    return min(sqrt(distance)*side,harvestSegment(p,vec2(0.0,.36),vec2(0.0,.74))-.045);
}
void harvestLeaves(inout vec3 ink,vec2 p,float cycle,bool silhouettes){
    float count=clamp(2.0+density*.055,2.0,6.0);
    for(int i=0;i<6;++i){
        float index=float(i),seed=fract(index*.618034+.21);
        float presence=1.0-smoothstep(count-.35,count+.35,index+.5);
        if(presence<=0.0)continue;
        // The wrap happens beyond the panel, so falling leaves never pop in
        // its visible area. Every movement repeats at phase + 12 seconds.
        float fall=fract(phase/12.0+index*.173+.08);
        float radius=dotRadius*(.19+.085*seed);
        float panelExtent=.5*abs(sin(angle))+.5*abs(cos(angle))/surfaceAspect;
        float travelExtent=panelExtent+radius*1.3+.025;
        vec2 centre=vec2(seed*.98-.49+flow*.055*sin(cycle+index*1.7),
                         mix(-travelExtent,travelExtent,fall));
        vec2 q=rotatePoint((p-centre)/radius,seed*6.2831853+(.18+.65*flow)*sin(cycle+index));
        q.x/=mix(.68,1.0,.5+.5*cos(cycle+index*1.9));
        float distance=harvestMaple(q),mask=shapeMask(distance*radius)*presence;
        vec3 leaf=mix(vec3(.72,.065,.025),vec3(1.0,.54,.055),seed);
        leaf*=.65+.28*(1.0-smoothstep(-.8,.5,q.y))+.07*sin(q.x*5.0+q.y*3.0);
        float vein=min(harvestSegment(q,vec2(0.0,.45),vec2(0.0,-.86)),
                       min(harvestSegment(q,vec2(0.0,.12),vec2(.78,-.14)),
                           harvestSegment(q,vec2(0.0,.12),vec2(-.78,-.14))));
        leaf=mix(leaf,leaf*.38,shapeMask((vein-.024)*radius)*.75);
        if(silhouettes)leaf=vec3(.025,.012,.005);
        ink=mix(ink,leaf,mask*(silhouettes?.62:.94));
    }
}
void harvestPumpkin(inout vec3 ink,vec2 p,vec2 centre,float radius,float light){
    vec2 q=(p-centre)/radius;
    float shadow=shapeMask(harvestEllipse(q-vec2(.04,.76),vec2(1.15,.13))*radius);
    ink=mix(ink,ink*.35,shadow*.65);
    float distance=harvestEllipse(q,vec2(1.0,.75))-.025*cos(atan(q.y/.75,q.x)*10.0);
    float mask=shapeMask(distance*radius);
    float ribs=.68+.32*pow(.5+.5*cos(q.x*11.5+q.y*q.y*.9),2.0);
    float shade=.43+.48*clamp(1.0-length((q-vec2(-.34,-.28))/vec2(1.6,1.3)),0.0,1.0);
    vec3 pumpkin=vec3(.98,.34,.035)*ribs*shade*light;
    pumpkin+=vec3(.15,.075,.012)*exp(-dot(q-vec2(-.32,-.36),q-vec2(-.32,-.36))*14.0);
    ink=mix(ink,pumpkin,mask);
    vec2 stem=rotatePoint(q-vec2(.015,-.86),-.19);
    float stemMask=shapeMask(boxDistance(stem,vec2(.075,.18))*radius);
    ink=mix(ink,vec3(.22,.24,.075)*(.72+.28*cos(stem.x*31.0)),stemMask);
}
void harvestPotato(inout vec3 ink,vec2 p,vec2 centre,float radius,float cycle){
    vec2 q=rotatePoint((p-centre)/radius,.035*flow*sin(cycle));
    vec3 skin=vec3(.84,.62,.34),shadow=vec3(.29,.14,.055);
    float feet=min(harvestSegment(q,vec2(-.32,.88),vec2(-.53,1.06)),
                   harvestSegment(q,vec2(.32,.88),vec2(.53,1.06)));
    ink=mix(ink,shadow,shapeMask((feet-.105)*radius));
    vec2 shoulder=vec2(.61,.04);
    vec2 hand=shoulder+rotatePoint(vec2(.43,-.43),(.08+.28*flow)*sin(cycle*2.0));
    float arms=min(harvestSegment(q,shoulder,hand),
                   harvestSegment(q,vec2(-.61,.18),vec2(-.94,.42)));
    ink=mix(ink,skin*.82,shapeMask((arms-.095)*radius));
    ink=mix(ink,skin,shapeMask(harvestEllipse(q-hand,vec2(.12,.12))*radius));
    float width=.70+.035*sin(q.y*7.0)+.025*sin(q.y*11.0);
    float body=shapeMask(harvestEllipse(q,vec2(width,1.0))*radius);
    float shade=.58+.40*clamp(1.0-length((q-vec2(-.29,-.35))/vec2(1.4,1.8)),0.0,1.0);
    ink=mix(ink,skin*shade,body);
    float spots=min(harvestEllipse(q-vec2(-.37,.35),vec2(.052,.032)),
                    min(harvestEllipse(q-vec2(.37,.39),vec2(.045,.030)),
                        harvestEllipse(q-vec2(-.11,-.76),vec2(.032,.050))));
    ink=mix(ink,skin*.55,shapeMask(spots*radius)*body*.45);
    float cheeks=min(harvestEllipse(q-vec2(-.40,.035),vec2(.105,.065)),
                     harvestEllipse(q-vec2(.40,.035),vec2(.105,.065)));
    ink=mix(ink,vec3(.80,.30,.19),shapeMask(cheeks*radius)*body*.48);
    float eyes=min(length(q-vec2(-.24,-.20)),length(q-vec2(.24,-.20)))-.064;
    ink=mix(ink,vec3(.065,.030,.015),shapeMask(eyes*radius)*body);
    float glints=min(length(q-vec2(-.258,-.219)),length(q-vec2(.222,-.219)))-.017;
    ink=mix(ink,vec3(1.0,.92,.70),shapeMask(glints*radius)*body);
    float smile=abs(harvestEllipse(q-vec2(0.0,-.06),vec2(.25,.20)))-.019;
    ink=mix(ink,vec3(.17,.06,.028),shapeMask(smile*radius)*smoothstep(-.05,-.015,q.y)*body);
    float scarf=min(boxDistance(q-vec2(0.0,.56),vec2(.76,.135)),
                    boxDistance(rotatePoint(q-vec2(.40,.85),-.16),vec2(.12,.29)));
    vec3 cranberry=mix(vec3(.38,.025,.050),vec3(.68,.085,.13),.5+.5*sin(q.y*14.0+q.x*2.0));
    ink=mix(ink,cranberry,shapeMask(scarf*radius));
}
void harvestLantern(inout vec3 ink,vec2 p,vec2 centre,float radius,float breath){
    vec2 q=(p-centre)/radius;
    float halo=exp(-dot(q/vec2(2.7,2.5),q/vec2(2.7,2.5)));
    ink+=vec3(.25,.095,.018)*halo*breath;
    float glass=shapeMask(boxDistance(q,vec2(.64,.83))*radius);
    vec3 amber=vec3(.16,.055,.009)+vec3(.28,.13,.020)*exp(-dot(q,q)*1.8)*breath;
    ink=mix(ink,amber,glass*.72);
    float frame=abs(boxDistance(q,vec2(.68,.87)))-.045;
    float roof=min(harvestSegment(q,vec2(-.76,-.88),vec2(0.0,-1.15)),
                   harvestSegment(q,vec2(0.0,-1.15),vec2(.76,-.88)))-.045;
    float base=boxDistance(q-vec2(0.0,.90),vec2(.78,.075));
    float handle=abs(length(q-vec2(0.0,-1.29))-.18)-.035;
    float iron=shapeMask(min(min(frame,roof),min(base,handle))*radius);
    ink=mix(ink,vec3(.38,.20,.060)*(.80+.20*breath),iron);
    float candle=shapeMask(boxDistance(q-vec2(0.0,.40),vec2(.14,.32))*radius);
    ink=mix(ink,vec3(.86,.56,.24),candle);
    float flame=shapeMask(harvestEllipse(q-vec2(.025,-.055),vec2(.10,.21))*radius);
    ink=mix(ink,vec3(1.0,.77,.31)*(.92+.08*breath),flame);
    float core=shapeMask(harvestEllipse(q-vec2(.02,.0),vec2(.045,.105))*radius);
    ink=mix(ink,vec3(1.0,.93,.66),core);
}
void main(){
    vec2 metric=rotatePoint((uv-.5)*vec2(1.0,1.0/surfaceAspect),angle);
    vec2 patternUv=metric*vec2(1.0,surfaceAspect)+.5;
    if(pattern==14){
        float cycle=phase*(6.2831853/12.0),ground=.32/surfaceAspect;
        float texture=.80+.12*sin(metric.x*43.0+sin(metric.y*17.0))+.08*cos(metric.x*19.0-metric.y*31.0);
        float floorLight=smoothstep(.08/surfaceAspect,.44/surfaceAspect,metric.y);
        vec3 ink=vec3(.012,.006,.003)+vec3(.065,.024,.005)*floorLight*texture;
        vec2 glowPoint=metric-vec2(-.16,ground-.13);
        ink+=vec3(.060,.023,.004)*exp(-dot(glowPoint,glowPoint)*9.0)*(.92+.08*flow*sin(cycle));
        harvestLeaves(ink,metric,cycle,false);
        float pumpkinSize=.034+dotRadius*.25;
        harvestPumpkin(ink,metric,vec2(-.31,ground-pumpkinSize*.73),pumpkinSize,1.0);
        harvestPumpkin(ink,metric,vec2(-.105,ground-pumpkinSize*.49),pumpkinSize*.67,.94);
        harvestPumpkin(ink,metric,vec2(.36,ground-pumpkinSize*.54),pumpkinSize*.74,.92);
        float potatoSize=.045+dotRadius*.28;
        harvestPotato(ink,metric,vec2(.13,ground-potatoSize*1.08),potatoSize,cycle);
        color=vec4(clamp(ink,0.0,1.0),1.0);finishColor();return;
    }
    if(pattern==15){
        float cycle=phase*(6.2831853/12.0),ground=.34/surfaceAspect;
        float texture=.88+.07*sin(metric.x*37.0+metric.y*13.0)+.05*cos(metric.y*31.0);
        vec3 ink=vec3(.010,.005,.002)+vec3(.028,.011,.002)*smoothstep(0.0,ground,metric.y)*texture;
        float lanternSize=.035+dotRadius*.27;
        for(int i=0;i<3;++i){
            float index=float(i),breath=.88+(.04+.08*flow)*sin(cycle+index*1.5);
            vec2 centre=vec2((index-1.0)*.31+flow*.013*sin(cycle+index),-.035/surfaceAspect);
            harvestLantern(ink,metric,centre,lanternSize*(i==1?1.10:.88),breath);
        }
        harvestLeaves(ink,metric,cycle,true);
        float pumpkinSize=.030+dotRadius*.19;
        harvestPumpkin(ink,metric,vec2(-.30,ground-pumpkinSize*.73),pumpkinSize,.78);
        harvestPumpkin(ink,metric,vec2(.31,ground-pumpkinSize*.52),pumpkinSize*.72,.82);
        color=vec4(clamp(ink,0.0,1.0),1.0);finishColor();return;
    }
    if(pattern==13){
        // Two staggered lattices give the nearest hexagon centre. The shared
        // field bends the whole lattice gently and offsets neighbouring pulses.
        vec2 p=metric*density+vec2(-phase*.18,phase*.08);
        p+=flow*.28*vec2(sin(metric.y*5.0+phase*.23),cos(metric.x*4.0-phase*.19));
        const vec2 spacing=vec2(1.0,1.7320508);
        vec2 a=mod(p,spacing)-spacing*.5;
        vec2 b=mod(p-spacing*.5,spacing)-spacing*.5;
        vec2 local=dot(a,a)<dot(b,b)?a:b;
        vec2 centre=(p-local)/density*7.0;
        // Two broad harmonics coordinate the pulses. The lattice already
        // supplies the gentle warp, so avoid deforming the field a second time.
        float wave=smoothstep(.1,.9,.5+.33*sin(centre.x*1.63+centre.y*.57-phase*.53)
                                       +flow*.15*cos(centre.y*1.37-centre.x*.43+phase*.4));
        float radius=dotRadius*(.25+.75*wave);
        vec2 q=abs(local);
        float distance=max(q.x,dot(q,vec2(.5,.8660254)))-radius;
        color=vec4(vec3(1.0),shapeMask(distance)*(.3+.7*wave));
        finishColor();return;
    }
    if(pattern>=9){
        float alpha;
        if(pattern==9){
            // Curved interference ridges evoke pool-bottom light, without a
            // fluid simulation or tracing light rays every frame.
            vec2 p=organicWarp(metric*density*.55,phase,flow*.6);
            float a=sin(p.x*1.7+sin(p.y*1.3+phase*.43))-cos(p.y*1.6+sin(p.x*1.1-phase*.37));
            float b=sin(p.x*.8-p.y*1.2+flow*.8*sin(p.y*1.15+phase*.29)+phase*.29);
            float d=min(abs(a)*.35,abs(b)*.45);
            alpha=clamp(shapeMask(d-dotRadius*.16)+.12*max(0.0,1.0/(1.0+d*40.0)-1.0/11.0),0.0,1.0);
        }else if(pattern==10){
            // Two fixed noise scales form a moving height field. Contours
            // remain continuous instead of jumping to new random values.
            vec2 p=organicWarp(metric*5.0,phase,flow*.65)+vec2(phase*.06,-phase*.04);
            float height=.65*smoothNoise(p*.7)+.35*smoothNoise(p*1.55+vec2(8.2,3.7));
            float d=abs(fract(height*density*.6-phase*.12)-.5);
            alpha=shapeMask(d-dotRadius*.32);
        }else if(pattern==11){
            // Opposite quarter-circle pairs meet at tile-side midpoints.
            // Warping the whole coordinate field keeps adjacent paths joined.
            vec2 p=organicWarp(metric*density*.55+vec2(phase*.12,-phase*.08),phase,flow*.25);
            vec2 cell=floor(p),local=fract(p);if(cellSeed(cell)<.5)local.x=1.0-local.x;
            float d=min(abs(length(local)-.5),abs(length(local-vec2(1))-.5));
            float light=.7+.3*sin(p.x*.4+p.y*.37-phase*.6);
            alpha=shapeMask(d-dotRadius*.3)*light;
        }else{
            // Logarithmic rings and angular spokes create perspective depth.
            // A soft vanishing point hides subpixel detail at the centre.
            vec2 p=metric-flow*.045*vec2(sin(phase*.23),cos(phase*.19));
            float r=max(length(p),.001),depth=-log(r)*1.6+phase*.45;
            float rings=abs(fract(depth*density*.12)-.5);
            float spokes=abs(sin(atan(p.y,p.x)*max(4.0,floor(density*.25))+flow*.4*sin(depth*.6+phase*.2)));
            alpha=max(shapeMask(rings-dotRadius*.32),shapeMask(spokes-dotRadius*.4));
            alpha*=smoothstep(.018,.06,r)*(.25+.75*smoothstep(.04,.55,r));
        }
        color=vec4(vec3(1.0),alpha);finishColor();return;
    }
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
