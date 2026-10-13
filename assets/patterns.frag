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
        if((pattern==14||pattern==15||pattern==16||pattern==17||pattern==18)&&palette!=0)
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
// One circular clock drives every October pose. Envelopes have zero slope
// outside their event, so neither forward nor negative playback snaps at 12 s.
float octoberBeat(float seconds,float first,float last){
    float ramp=(last-first)*.27;
    return smoothstep(first,first+ramp,seconds)*(1.0-smoothstep(last-ramp,last,seconds));
}
float octoberEye(vec2 p){
    // A broad triangular carved opening, with a horizontal base.
    return max(abs(p.x)*.92-p.y*.65-.09,p.y-.14);
}
void octoberPumpkin(inout vec3 ink,vec2 p,vec2 centre,float radius,
                    float tilt,float bounce,float gaze,float wink,float hiccup,float cycle){
    vec2 local=rotatePoint(p-centre+vec2(0.0,bounce*radius),tilt);
    local/=vec2(1.0+.035*bounce,1.0-.055*bounce);
    harvestPumpkin(ink,local,vec2(0.0),radius,.94+.035*sin(cycle));
    vec2 q=local/radius;
    float body=shapeMask(harvestEllipse(q,vec2(.97,.73))*radius);
    vec2 left=q-vec2(-.33+gaze,-.15),right=q-vec2(.33+gaze,-.15);
    left.y/=mix(1.0,.12,wink);
    float eyes=min(octoberEye(left),octoberEye(right));
    float grin=max(harvestEllipse(q-vec2(0.0,.19),vec2(.48,.25)),.13-q.y);
    float teeth=min(boxDistance(q-vec2(-.16,.16),vec2(.045,.075)),
                    boxDistance(q-vec2(.16,.16),vec2(.045,.075)));
    grin=max(grin,-teeth);
    float roundMouth=harvestEllipse(q-vec2(0.0,.25),vec2(.17,.20));
    float mouth=mix(grin,roundMouth,hiccup);
    float carved=min(eyes,mouth);
    float rim=shapeMask((carved-.035)*radius)*body;
    float openings=shapeMask(carved*radius)*body;
    ink=mix(ink,vec3(.075,.023,.009),rim);
    vec3 candle=vec3(1.0,.62,.13)*(.88+.045*sin(cycle*2.0));
    candle+=vec3(.0,.075,.045)*clamp(-q.y+.30,0.0,.70);
    ink=mix(ink,candle,openings);
    // Moving dark pupils make the glance readable within the carved light.
    float pupils=min(harvestEllipse(q-vec2(-.33+gaze*1.5,-.12),vec2(.043,.060)),
                     harvestEllipse(q-vec2(.33+gaze*1.5,-.12),vec2(.043,.060)));
    ink=mix(ink,vec3(.11,.035,.008),shapeMask(pupils*radius)*openings*(1.0-wink*.8));
}
vec2 octoberGhostRoute(float turn,vec2 extent){
    float a=turn*6.2831853;
    // A closed oval with a slight bend: position and tangent both match.
    return extent*vec2(.235*cos(a)+.028*sin(a*2.0),.20*sin(a));
}
void octoberGhost(inout vec3 ink,vec2 p,vec2 centre,float radius,
                  float index,float seconds,float cycle,bool guide){
    float chase=index>=2.0?octoberBeat(seconds,6.0,8.7):0.0;
    float stretch=1.0+.18*flow*chase;
    float tilt=.055*flow*sin(cycle+index*1.7);
    vec2 q=rotatePoint((p-centre)/radius,tilt);
    q*=vec2(sqrt(stretch),1.0/stretch);
    float wave=guide?flow*octoberBeat(seconds,8.0,10.2):.0;
    vec2 leftHand=vec2(-.91,-.02-.48*wave);
    vec2 rightHand=guide?vec2(.99,.16):vec2(.88,.16+.08*flow*sin(cycle+index));
    float arms=min(harvestSegment(q,vec2(-.57,-.03),leftHand),
                   harvestSegment(q,vec2(.57,.01),rightHand))-.102;
    float cap=max(harvestEllipse(q-vec2(0.0,-.28),vec2(.73,.77)),q.y-.08);
    float hem=.70+.10*cos(q.x*13.0)+.030*flow*sin(cycle*2.0+index+q.x*4.0);
    float width=.66-.075*clamp(q.y,0.0,1.0);
    float cloth=max(max(abs(q.x)-width,q.y-hem),-.13-q.y);
    float distance=min(min(cap,cloth),arms);
    float mask=shapeMask(distance*radius);
    float halo=exp(-max(distance,0.0)*16.0)*(1.0-mask);
    ink+=vec3(.022,.040,.066)*halo;
    float light=.65+.29*clamp(1.0-length((q-vec2(-.27,-.40))/vec2(1.6,1.8)),0.0,1.0);
    float folds=.045*(.5+.5*cos(q.x*15.0+.55*flow*sin(cycle+index)))*smoothstep(-.05,.55,q.y);
    vec3 sheet=vec3(.83,.89,.92)*(light-folds);
    sheet=mix(sheet,vec3(.24,.37,.50),smoothstep(.12,.84,q.y)*.20);
    ink=mix(ink,sheet,mask);
    float blink=octoberBeat(seconds,1.6+index*3.1,2.12+index*3.1);
    float eyeHeight=(index>=2.0?.17:.135)*mix(1.0,.14,blink);
    float look=.025*sin(cycle+index*.9)+.025;
    float eyes=min(harvestEllipse(q-vec2(-.235+look,-.27),vec2(.092,eyeHeight)),
                   harvestEllipse(q-vec2(.235+look,-.27),vec2(.092,eyeHeight)));
    float eyeMask=shapeMask(eyes*radius)*mask;
    ink=mix(ink,vec3(.020,.033,.050),eyeMask);
    float glints=min(length(q-vec2(-.253+look,-.304)),length(q-vec2(.217+look,-.304)))-.024;
    ink=mix(ink,vec3(.94,.99,1.0),shapeMask(glints*radius)*eyeMask*(1.0-blink));
    float smile=max(abs(harvestEllipse(q-vec2(0.0,-.012),vec2(.18,.14)))-.022,.02-q.y);
    float shy=abs(harvestEllipse(q-vec2(.015,.08),vec2(.062,.085)))-.022;
    float mouth=index>=2.0?mix(shy,smile,.35*chase):smile;
    ink=mix(ink,vec3(.028,.044,.066),shapeMask(mouth*radius)*mask);
    if(guide){
        vec2 lantern=centre+rotatePoint(vec2(.99,.50)*radius,-tilt);
        harvestLantern(ink,p,lantern,radius*.26,.90+.04*sin(cycle));
    }
}
// Four nearby candidates per rain layer. The seed lattice is periodic, and
// each layer translates exactly its integer vertical period in twelve seconds.
// No spatial culls: derivative AA runs for every candidate in every fragment.
void octoberRain(inout vec3 ink,vec2 p,float turn,float cycle,float layer){
    float grid=(4.5+density*.18)*(layer>.5?1.45:1.0);
    float gust=.08+.28*flow*sin(cycle);
    vec2 field=vec2(p.x-gust*p.y,p.y)*grid;
    float period=layer>.5?13.0:11.0;
    field.y-=turn*period;
    vec2 base=floor(field);
    float lengthInCell=(.16+.52*dotRadius)*(layer>.5?.80:1.0);
    float width=(.0022+.0095*dotRadius)*(layer>.5?.72:1.0);
    // Candidate identities change at tile boundaries. Derive the footprint
    // from continuous panel coordinates, not a discontinuous candidate SDF.
    float pixel=max(length(fwidth(p)),.001);
    for(int i=0;i<4;++i){
        vec2 cell=base+vec2(float(i%2),float(i/2));
        vec2 id=mod(cell,vec2(7.0,period));
        float seed=cellSeed(id+vec2(layer*17.0,layer*9.0));
        float other=cellSeed(id.yx+vec2(5.3+layer*11.0,2.7));
        vec2 centre=cell+vec2(seed-.5,other-.5)*.30;
        vec2 local=(field-centre)/grid;
        local.x+=gust*local.y;
        float halfLength=lengthInCell*(.76+.24*seed)/grid;
        vec2 along=vec2(gust*halfLength,halfLength);
        float distance=harvestSegment(local,-along,along)-width;
        float mask=1.0-smoothstep(-pixel,pixel,distance);
        vec3 rain=layer>.5?vec3(.20,.32,.45):vec3(.38,.54,.67);
        rain*=.82+.18*seed;
        ink=mix(ink,rain,mask*(layer>.5?.38:.64));
    }
}
void octoberWindowLeaf(inout vec3 ink,vec2 p,vec2 centre,float radius,float index,float seconds,float cycle){
    float lift=flow*octoberBeat(seconds,4.0,7.8)*.028;
    centre+=vec2(flow*.012*sin(cycle+index),-lift);
    vec2 q=rotatePoint((p-centre)/radius,(-.48+index*1.10)+flow*.24*sin(cycle+index*1.4));
    q.x/=1.0-.10*flow*(1.0-cos(cycle+index))*.5;
    float distance=harvestMaple(q);
    float mask=shapeMask(distance*radius);
    float veins=min(harvestSegment(q,vec2(0.0,.44),vec2(0.0,-.83)),
                    min(harvestSegment(q,vec2(0.0,.08),vec2(.76,-.14)),
                        harvestSegment(q,vec2(0.0,.08),vec2(-.76,-.14))));
    vec3 leaf=vec3(.69,.275,.075)*(.67+.26*clamp(.5-q.y*.38,0.0,1.0));
    leaf=mix(leaf,vec3(.25,.080,.025),shapeMask((veins-.024)*radius)*.75);
    ink=mix(ink,leaf,mask);
}
void main(){
    vec2 metric=rotatePoint((uv-.5)*vec2(1.0,1.0/surfaceAspect),angle);
    vec2 patternUv=metric*vec2(1.0,surfaceAspect)+.5;
    if(pattern==16){
        float turn=fract(phase/12.0),seconds=turn*12.0,cycle=turn*6.2831853;
        float shortAxis=min(1.0,1.0/surfaceAspect);
        vec2 p=metric/shortAxis,extent=vec2(1.0,1.0/surfaceAspect)/shortAxis;
        float radius=.086+.245*dotRadius;
        float ground=.32*extent.y;
        float floorLight=exp(-pow((p.y-ground)/(.17*extent.y),2.0));
        vec3 ink=vec3(.004,.003,.008)+vec3(.036,.015,.055)*floorLight;
        float hiccup=octoberBeat(seconds,3.25,5.65);
        float answer=octoberBeat(seconds,6.5,8.5);
        float tilt=(.035+.085*flow)*sin(cycle)+.075*flow*octoberBeat(seconds,4.1,6.35);
        vec2 little=vec2(-.235*extent.x,.215*extent.y);
        // Draw the smaller cast first, so the large face stays unobstructed.
        octoberPumpkin(ink,p,little,radius*.70,-.045*flow*sin(cycle),
                       .055*flow*hiccup,-.025*hiccup,.0,hiccup,cycle+.5);
        octoberPumpkin(ink,p,vec2(.235*extent.x,.225*extent.y),radius*.62,
                       .055*flow*sin(cycle+.8),.050*flow*answer,.035*answer,
                       octoberBeat(seconds,9.0,9.6),.0,cycle+1.1);
        octoberPumpkin(ink,p,vec2(.0,.135*extent.y),radius,tilt,
                       .045*flow*answer,-.055*octoberBeat(seconds,4.0,6.6),
                       octoberBeat(seconds,7.0,7.75),.0,cycle);
        float count=clamp(2.0+(density-4.0)*(4.0/76.0),2.0,6.0);
        for(int i=0;i<6;++i){
            float index=float(i),presence=clamp(count-index,0.0,1.0);
            if(presence<=0.0)continue; // Uniform setting/index skip only.
            float travel=clamp((seconds-3.35-index*.12)/2.7,0.0,1.0);
            float alive=smoothstep(.0,.16,travel)*(1.0-smoothstep(.65,1.0,travel));
            float seed=fract(index*.618034+.21);
            vec2 centre=little+vec2((seed-.5)*radius*.42+flow*.035*sin(travel*3.1415927+index),
                                    radius*.19-travel*(.13+.11*flow));
            float emberRadius=(.0023+.011*dotRadius)*(.76+.35*seed);
            float distance=length(p-centre)-emberRadius;
            float mask=shapeMask(distance)*alive*presence;
            ink+=vec3(.72,.30,.055)*mask;
        }
        color=vec4(clamp(ink,0.0,1.0),1.0);finishColor();return;
    }
    if(pattern==17){
        float turn=fract(phase/12.0),seconds=turn*12.0,cycle=turn*6.2831853;
        float shortAxis=min(1.0,1.0/surfaceAspect);
        vec2 p=metric/shortAxis,extent=vec2(1.0,1.0/surfaceAspect)/shortAxis;
        vec3 ink=vec3(.003,.007,.015);
        float radius=.074+.19*dotRadius;
        int count=int(clamp(2.0+floor(density/26.0),2.0,4.0));
        for(int i=3;i>=0;--i){
            if(i>=count)continue; // Same branch in every fragment.
            float index=float(i);
            float lag=index>=2.0?.060*flow*octoberBeat(seconds,3.0,7.9):0.0;
            float tour=turn-index*.21-lag;
            vec2 centre=octoberGhostRoute(tour,extent);
            float size=radius*(i==0?1.0:i==1?.85:.91);
            octoberGhost(ink,p,centre,size,index,seconds,cycle,i==0);
        }
        color=vec4(clamp(ink,0.0,1.0),1.0);finishColor();return;
    }
    if(pattern==18){
        float turn=fract(phase/12.0),seconds=turn*12.0,cycle=turn*6.2831853;
        float shortAxis=min(1.0,1.0/surfaceAspect);
        vec2 p=metric/shortAxis,extent=vec2(1.0,1.0/surfaceAspect)/shortAxis;
        vec2 panel=p/extent;
        float glass=.86+.07*sin(panel.x*29.0+panel.y*17.0)+.07*cos(panel.y*37.0-panel.x*13.0);
        vec3 ink=vec3(.008,.016,.030)*glass;
        float gust=flow*sin(cycle);
        vec2 reflected=p-vec2(.19*extent.x+gust*.020*cos(panel.y*15.0+cycle),.16*extent.y);
        vec2 reflection=reflected/vec2(.055+.20*dotRadius,.29*extent.y);
        float amber=exp(-dot(reflection,reflection)*.70);
        float ripples=.80+.12*flow*cos(panel.y*33.0+cycle*2.0)+.08*flow*cos(panel.y*51.0-cycle);
        ink+=vec3(.19,.091,.026)*amber*ripples;
        octoberRain(ink,p,turn,cycle,1.0);
        octoberRain(ink,p,turn,cycle,0.0);
        float frameWidth=.006+.012*dotRadius;
        float frame=abs(boxDistance(p,extent*vec2(.455,.435)))-frameWidth;
        float mullion=boxDistance(p-vec2(-.095*extent.x,0.0),vec2(frameWidth,.435*extent.y));
        float sill=boxDistance(p-vec2(.0,.36*extent.y),vec2(.465*extent.x,.012+.024*dotRadius));
        float bronze=shapeMask(min(min(frame,mullion),sill));
        ink=mix(ink,vec3(.155,.082,.033)*(.82+.18*clamp(-panel.y+.50,0.0,1.0)),bronze);
        float leafSize=.026+.09*dotRadius;
        for(int i=0;i<2;++i){
            float index=float(i);
            vec2 centre=vec2((i==0?-.23:.27)*extent.x,.36*extent.y-leafSize*.40);
            octoberWindowLeaf(ink,p,centre,leafSize,index,seconds,cycle);
        }
        color=vec4(clamp(ink,0.0,1.0),1.0);finishColor();return;
    }
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
