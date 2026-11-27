// Original cut-paper illustrations for the shared mapped surface shader.
// Coordinates use the shortest surface axis so wheels, faces and dials stay round.
const float christmasTau=6.283185307;
const vec3 nightInk=vec3(.048,.087,.132);
const vec3 brass=vec3(.83,.55,.22);
const vec3 goldLeaf=vec3(1.0,.82,.43);
const vec3 ivory=vec3(1.0,.91,.72);
const vec3 cranberry=vec3(.72,.13,.19);

float christmasBox(vec2 p,vec2 halfSize,float radius){
    return boxDistance(p,max(halfSize-vec2(radius),vec2(0.0)))-radius;
}
float christmasTriangle(vec2 p,vec2 a,vec2 b,vec2 c){
    vec2 ab=normalize(b-a),bc=normalize(c-b),ca=normalize(a-c);
    float orientation=sign(ab.x*(c.y-a.y)-ab.y*(c.x-a.x));
    return max(max(dot(p-a,vec2(ab.y,-ab.x))*orientation,
                   dot(p-b,vec2(bc.y,-bc.x))*orientation),dot(p-c,vec2(ca.y,-ca.x))*orientation);
}
float christmasStar(vec2 p,float radius){
    const float wedge=.62831853;
    float a=abs(mod(atan(p.x,-p.y)+wedge,2.0*wedge)-wedge);
    vec2 folded=vec2(sin(a),cos(a))*length(p);
    vec2 edge=vec2(.245,-.663);
    return dot(folded-vec2(0.0,radius),normalize(vec2(-edge.y,edge.x)));
}
void paper(inout vec3 ink,float distance,vec3 pigment){ink=mix(ink,pigment,shapeMask(distance));}
void cutBox(inout vec3 ink,vec2 p,vec2 halfSize,float radius,vec3 pigment){paper(ink,christmasBox(p,halfSize,radius),pigment);}
void cutDisc(inout vec3 ink,vec2 p,float radius,vec3 pigment){paper(ink,length(p)-radius,pigment);}
void cutLine(inout vec3 ink,vec2 p,vec2 a,vec2 b,float width,vec3 pigment){paper(ink,harvestSegment(p,a,b)-width,pigment);}
float lightPool(vec2 p,vec2 radii){vec2 q=p/radii;return exp(-dot(q,q));}

// Each star stays within its cell. All twinkles use integer harmonics of the same loop.
void winterStars(inout vec3 ink,vec2 p,float cycle,float amount,vec3 pigment){
    vec2 grid=p*12.0,cell=floor(grid);
    float seed=cellSeed(cell+vec2(7,19));
    vec2 offset=vec2(seed,cellSeed(cell.yx+vec2(11,2)))*.56+.22;
    vec2 q=(fract(grid)-offset)/12.0;
    float pulse=.55+.45*cos(2.0*cycle+seed*christmasTau);
    float ray=min(abs(q.x),abs(q.y))*.80+max(abs(q.x),abs(q.y))*.20;
    float size=.0012+.0016*pulse;
    float mask=shapeMask(ray-size)*smoothstep(seed-.04,seed+.04,amount);
    ink=mix(ink,pigment,mask*(.48+.45*pulse));
}
void pineWood(inout vec3 ink,vec2 p,float spacing,float baseline,float height,vec3 pigment){
    vec2 q=vec2(mod(p.x+spacing*.5,spacing)-spacing*.5,p.y-baseline);
    float seed=cellSeed(vec2(floor((p.x+spacing*.5)/spacing),17.0));
    float h=height*(.72+.28*seed),w=spacing*(.46+.12*seed);
    float tree=min(christmasTriangle(q,vec2(0,-h),vec2(w*.65,-h*.34),vec2(-w*.65,-h*.34)),
                   christmasTriangle(q,vec2(0,-h*.83),vec2(w*.87,-h*.14),vec2(-w*.87,-h*.14)));
    tree=min(tree,christmasTriangle(q,vec2(0,-h*.62),vec2(w,0),vec2(-w,0)));
    paper(ink,tree,pigment);
}

void expressWheel(inout vec3 ink,vec2 q,float radius,float spin){
    cutDisc(ink,q,radius,nightInk);
    cutDisc(ink,q,radius*.86,brass);
    cutDisc(ink,q,radius*.72,vec3(.22,.09,.13));
    vec2 spoke=rotatePoint(q,spin);
    float spokes=min(min(abs(spoke.x),abs(spoke.y)),min(abs(spoke.x-spoke.y),abs(spoke.x+spoke.y))*.707);
    paper(ink,max(spokes-radius*.060,length(q)-radius*.72),goldLeaf);
    cutDisc(ink,q,radius*.20,brass);
}
void expressCoach(inout vec3 ink,vec2 q,float spin){
    if(abs(q.x)>.24||abs(q.y)>.16)return;
    expressWheel(ink,q-vec2(-.126,.100),.043,spin);
    expressWheel(ink,q-vec2(.126,.100),.043,spin);
    cutBox(ink,q,vec2(.199,.085),.012,vec3(.12,.24,.24));
    cutBox(ink,q-vec2(0,-.070),vec2(.210,.019),.012,vec3(.060,.15,.19));
    cutBox(ink,q-vec2(0,-.086),vec2(.197,.006),.004,vec3(.72,.83,.81));
    cutBox(ink,q-vec2(0,.053),vec2(.197,.005),.002,brass);
    float index=clamp(floor((q.x+.176)/.072),0.0,4.0);
    vec2 pane=q-vec2(-.142+.072*index,-.012);
    cutBox(ink,pane,vec2(.028,.034),.006,brass);
    cutBox(ink,pane,vec2(.022,.028),.004,mix(goldLeaf,ivory,.54-.32*pane.y/.028));
    paper(ink,max(christmasBox(pane,vec2(.023,.029),.004),.010-abs(pane.x)-pane.y*.22),cranberry);
    cutBox(ink,pane-vec2(0,-.024),vec2(.024,.004),.002,brass);
}
vec3 northPoleExpress(vec2 p,vec2 extent,float turn,float cycle){
    vec2 panel=p/extent;
    vec3 ink=mix(vec3(.055,.075,.166),vec3(.52,.46,.57),smoothstep(-.46,.27,panel.y));
    vec2 moon=p-vec2(.30*extent.x,-.24*extent.y);
    ink+=vec3(.16,.12,.046)*lightPool(moon,vec2(.24));
    cutDisc(ink,moon,.102,vec3(.95,.78,.53));
    cutDisc(ink,moon-vec2(-.020,.004),.075,vec3(.96,.82,.60));
    winterStars(ink,p+vec2(0,2),cycle,.34*(1.0-smoothstep(-.35,-.05,panel.y)),ivory);
    // The camera rides with the train. Mountain silhouettes, pines and viaduct
    // piers pass at different rates; each translation closes on a whole repeat.
    float farX=p.x+turn*1.12;
    float ridge=.005*extent.y-.13*cos(farX*christmasTau/1.12)-.043*cos(farX*christmasTau*3.0/1.12);
    paper(ink,ridge-p.y,vec3(.53,.56,.64));
    paper(ink,ridge+.014+.020*abs(sin(farX*christmasTau*4.0/1.12))-p.y,vec3(.32,.38,.50));
    float nearRidge=.115*extent.y-.050*cos(p.x*7.7+cycle)-.026*sin(p.x*13.0+cycle*2.0);
    paper(ink,nearRidge-p.y,vec3(.22,.35,.42));
    // Periodic pine heights keep the distant forest continuous at the cycle seam.
    pineWood(ink,vec2(mod(p.x+turn*.96+.48,.96)-.48,p.y),.12,.215*extent.y,.17,vec3(.12,.24,.29));
    float rail=.26;
    float bridgeY=p.y-rail,span=.31;
    vec2 arch=vec2(mod(p.x+turn*span*9.0+span*.5,span)-span*.5,bridgeY);
    float opening=harvestEllipse(arch-vec2(0,.174),vec2(.106,.123));
    float insideArch=min(opening,max(abs(arch.x)-.106,.174-arch.y));
    paper(ink,max(-bridgeY,-insideArch),vec3(.29,.25,.31));
    paper(ink,max(max(-bridgeY,-insideArch),insideArch-.014),vec3(.59,.46,.40));
    cutBox(ink,vec2(0,bridgeY-.018),vec2(1.0,.013),.002,vec3(.64,.55,.51));
    cutBox(ink,vec2(0,bridgeY+.002),vec2(1.0,.010),.002,vec3(.16,.18,.24));
    cutBox(ink,vec2(0,bridgeY+.012),vec2(1.0,.004),.001,vec3(.88,.81,.67));
    float size=(.73+.93*dotRadius)*min(1.0,extent.x*.89);
    vec2 origin=vec2(.18*extent.x+.12*sin(cycle),rail-.016);
    vec2 q=(p-origin)/size;
    float spin=cycle*12.0;
    // Steam curls expand and dissolve before an individual puff wraps.
    for(int i=0;i<9;++i){
        float n=float(i),count=3.0+density*.074;
        float alive=clamp(count-n,0.0,1.0);
        if(alive<=0.0)continue;
        float age=fract(turn*3.0+n*.236068);
        vec2 puff=vec2(.179-age*(.32+.46*flow),-.268-age*(.12+.13*flow)-.024*sin(age*5.8+n));
        vec2 vapour=q-puff;float r=.014+.041*age;
        float cloud=min(length(vapour)-r,length(vapour-vec2(-r*.73,r*.20))-r*.68);
        ink=mix(ink,vec3(.91,.84,.75),shapeMask(cloud)*alive*smoothstep(.0,.10,age)*(1.0-smoothstep(.62,1.0,age))*.73);
    }
    cutLine(ink,q,vec2(-1.14,-.05),vec2(-.10,-.05),.012,nightInk);
    expressCoach(ink,q-vec2(-.898,-.142),spin);
    expressCoach(ink,q-vec2(-.472,-.142),spin);
    if(abs(q.x)<.32&&q.y<.016&&q.y>-.345){
        cutBox(ink,q-vec2(.005,-.089),vec2(.235,.027),.009,vec3(.24,.10,.14));
        cutBox(ink,q-vec2(.066,-.144),vec2(.160,.058),.047,cranberry);
        cutBox(ink,q-vec2(.070,-.164),vec2(.132,.012),.009,vec3(.88,.28,.22));
        for(int i=0;i<3;++i)cutBox(ink,q-vec2(-.020+float(i)*.084,-.144),vec2(.006,.054),.004,brass);
        cutBox(ink,q-vec2(-.153,-.177),vec2(.079,.084),.009,vec3(.58,.11,.17));
        cutBox(ink,q-vec2(-.15,-.206),vec2(.051,.033),.007,brass);
        cutBox(ink,q-vec2(-.15,-.206),vec2(.041,.026),.004,goldLeaf);
        cutBox(ink,q-vec2(-.15,-.206),vec2(.004,.026),.002,cranberry);
        cutBox(ink,q-vec2(-.151,-.264),vec2(.099,.014),.008,nightInk);
        cutBox(ink,q-vec2(-.151,-.278),vec2(.093,.004),.002,vec3(.80,.77,.68));
        cutBox(ink,q-vec2(.176,-.223),vec2(.021,.041),.003,nightInk);
        cutBox(ink,q-vec2(.176,-.264),vec2(.034,.012),.005,brass);
        cutBox(ink,q-vec2(.071,-.204),vec2(.022,.018),.012,brass);
        cutDisc(ink,q-vec2(.231,-.177),.023,brass);
        cutDisc(ink,q-vec2(.239,-.177),.015,ivory);
        paper(ink,christmasTriangle(q,vec2(.203,-.090),vec2(.286,-.011),vec2(.184,-.028)),cranberry);
        cutLine(ink,q,vec2(.209,-.081),vec2(.267,-.020),.004,brass);
        cutBox(ink,q-vec2(-.007,-.089),vec2(.235,.006),.003,goldLeaf);
        for(int i=0;i<3;++i)expressWheel(ink,q-vec2(-.152+float(i)*.134,-.051),.053,spin);
        vec2 crank=vec2(cos(spin),-sin(spin))*.022;
        cutLine(ink,q,vec2(-.152,-.051)+crank,vec2(.116,-.051)+crank,.007,brass);
    }
    vec2 headlight=q-vec2(.245,-.178);
    ink+=vec3(.28,.19,.055)*lightPool(headlight-vec2(.07,0),vec2(.14,.027))*smoothstep(-.015,.025,headlight.x);
    return ink;
}

void wrappedPresent(inout vec3 ink,vec2 p,vec2 halfSize,vec3 wrap,vec3 ribbon){
    cutBox(ink,p,halfSize,.005,wrap);
    cutBox(ink,p-vec2(0,-halfSize.y*.67),vec2(halfSize.x*1.05,halfSize.y*.24),.003,mix(wrap,ivory,.19));
    cutBox(ink,p-vec2(-halfSize.x*.18,0),vec2(halfSize.x*.12,halfSize.y),.001,ribbon);
    vec2 knot=p-vec2(-halfSize.x*.18,-halfSize.y);
    for(int i=0;i<2;++i){
        float side=i==0?-1.0:1.0;
        vec2 loop=rotatePoint(knot-vec2(side*halfSize.x*.25,-halfSize.y*.19),side*.42);
        paper(ink,abs(harvestEllipse(loop,vec2(halfSize.x*.27,halfSize.y*.17)))-.0025,ribbon);
    }
    cutDisc(ink,knot,halfSize.y*.13,ribbon);
}
vec3 evergreenOverture(vec2 p,vec2 extent,float turn,float cycle){
    vec2 panel=p/extent;
    vec3 ink=mix(vec3(.048,.14,.17),vec3(.08,.35,.36),smoothstep(-.42,.32,panel.y));
    float curtain=.085*sin(panel.x*5.0+cycle)+.038*cos(panel.x*14.0-cycle*2.0);
    ink+=vec3(.035,.13,.09)*lightPool(vec2(panel.x*.55,panel.y+.24-curtain),vec2(.75,.12));
    winterStars(ink,p+vec2(2.5,4),cycle,.52,vec3(.85,.97,.83));
    pineWood(ink,p+vec2(.086,0),.148,.27*extent.y,.29,vec3(.07,.25,.28));
    float bank=.236*extent.y+.044*cos(p.x*3.7)-.017*sin(p.x*8.0);
    paper(ink,bank-p.y,vec3(.32,.53,.52));
    pineWood(ink,p+vec2(.35,0),.29,.34*extent.y,.23,vec3(.09,.32,.32));
    float snow=.331*extent.y+.022*cos(p.x*4.0);
    paper(ink,snow-p.y,vec3(.52,.71,.65));
    paper(ink,snow+.018+.011*sin(p.x*10.0)-p.y,vec3(.36,.57,.55));
    float size=.79+.88*dotRadius;
    vec2 q=p/size;
    float seconds=turn*24.0;
    float crown=seasonBeat(seconds,7.0,12.0,18.8,23.5);
    ink+=vec3(.13,.13,.045)*lightPool(q-vec2(0,-.10),vec2(.34,.38))*(.4+.6*crown);
    ink=mix(ink,vec3(.16,.33,.30),lightPool(q-vec2(.024,.345),vec2(.36,.032))*.8);
    cutBox(ink,q-vec2(0,.273),vec2(.042,.069),.012,vec3(.30,.19,.12));
    // From the lowest bough upwards, each tier hides the join of the one below.
    for(int i=0;i<5;++i){
        float n=float(i),bottom=.246-.104*n,top=bottom-(.295-.027*n);
        float w=.360-.056*n;
        float x=q.x+.009*sin(cycle+.70*n)*flow*(n*.25);
        float across=abs(x)/w;
        float branchY=bottom-.052*across*across-.022*(.5+.5*cos(across*15.0+n));
        float outline=max(max((abs(x)-w*clamp((q.y-top)/(bottom-top),0.0,1.0))*.73,top-q.y),q.y-branchY);
        vec3 needles=mix(vec3(.052,.265,.225),vec3(.085,.425,.30),.5+.5*x/w);
        needles+=vec3(.020,.035,.012)*cos(across*16.0+2.0*n);
        paper(ink,outline,needles);
        float snowEdge=branchY-.017-.009*cos(across*13.0+n);
        paper(ink,max(outline,snowEdge-q.y),mix(vec3(.34,.60,.46),vec3(.68,.84,.61),.48+.40*x/w));
    }
    for(int i=0;i<4;++i){
        float n=float(i),w=.275-.056*n,centerY=.154-.104*n;
        float x=clamp(q.x/w,-1.0,1.0);
        float wireY=centerY-.052*x*x+.012*x;
        paper(ink,max(abs(q.y-wireY)-.003,abs(q.x)-w),brass);
        float count=max(3.0,floor(5.0+density*.065-.8*n));
        float bulb=clamp(floor((q.x/w+.94)*(count-1.0)/1.88+.5),0.0,count-1.0);
        float u=mix(-.94,.94,bulb/(count-1.0));
        vec2 hook=vec2(u*w,centerY-.052*u*u+.012*u);
        float sway=sin(2.0*cycle+1.9*bulb+n)*(.002+.005*flow);
        vec2 bead=hook+vec2(sway,.013+.005*mod(bulb,2.0));
        cutLine(ink,q,hook,bead,.0027,brass);
        float bright=seasonBeat(seconds,.45+1.55*n,3.8+1.55*n,18.5+.4*n,22.8+.18*n);
        bright*=.80+.20*cos(cycle*4.0+1.1*bulb+n)*flow;
        float colorIndex=mod(bulb+n,3.0);
        vec3 glass=colorIndex<1.0?vec3(.96,.23,.22):colorIndex<2.0?goldLeaf:vec3(.54,.86,.85);
        float r=.011+.004*mod(bulb+n,2.0);
        ink+=glass*(.16+.49*flow)*bright*lightPool(q-bead,vec2(r*2.9));
        cutDisc(ink,q-bead,r,mix(glass*.60,glass,bright));
        cutDisc(ink,q-bead+vec2(r*.26,r*.25),r*.27,mix(glass,ivory,.72));
    }
    vec2 star=q-vec2(.008*sin(cycle+2.8)*flow,-.373);
    ink+=goldLeaf*lightPool(star,vec2(.080))*(.16+(.45+.26*flow)*crown);
    paper(ink,christmasStar(star,.058),goldLeaf);
    paper(ink,max(christmasStar(star,.054),star.x),ivory);
    if(crown>.0){
        vec2 ray=abs(star);
        float rays=max(min(ray.x,ray.y)*.90+max(ray.x,ray.y)*.025-.0022,max(ray.x,ray.y)-.092);
        ink=mix(ink,ivory,shapeMask(rays)*crown*(.5+.4*flow));
    }
    wrappedPresent(ink,q-vec2(-.164,.292),vec2(.061,.039),cranberry,goldLeaf);
    wrappedPresent(ink,q-vec2(.124,.300),vec2(.063,.047),vec3(.19,.39,.53),ivory);
    wrappedPresent(ink,q-vec2(.208,.319),vec2(.046,.029),brass,cranberry);
    wrappedPresent(ink,q-vec2(-.075,.320),vec2(.040,.027),vec3(.39,.57,.39),ivory);
    return ink;
}

void carolHouse(inout vec3 ink,vec2 p,float variant,float cycle,float lanternX,float worldX){
    if(abs(p.x)>.151||p.y<-.29||p.y>.02)return;
    float height=.123+.050*variant,w=.123-.012*variant;
    vec3 plaster=mix(vec3(.56,.29,.32),vec3(.68,.51,.43),variant);
    cutBox(ink,p-vec2(0,-height*.5),vec2(w,height*.53),.004,plaster);
    cutBox(ink,p-vec2(.067,-height-.046),vec2(.014,.052),.002,vec3(.36,.27,.31));
    cutBox(ink,p-vec2(.067,-height-.096),vec2(.019,.006),.001,ivory);
    float roof=christmasTriangle(p,vec2(-w-.021,-height),vec2(.012,-height-.09),vec2(w+.024,-height));
    paper(ink,roof,vec3(.24,.20,.29));
    paper(ink,max(roof,p.y+height+.013+.007*sin(p.x*46.0)),vec3(.93,.83,.76));
    cutBox(ink,p-vec2(0,-.030),vec2(.018,.035),.013,vec3(.28,.20,.24));
    for(int i=0;i<2;++i){
        float side=i==0?-1.0:1.0;
        vec2 wq=p-vec2(side*.066,-height*.57);
        float seed=fract(variant*2.1+float(i)*.447);
        float litWindow=smoothstep(seed-.08,seed+.08,density/83.0);
        float approach=exp(-abs(worldX-lanternX)*4.0);
        float glow=clamp(.10+litWindow*(.60+.12*sin(cycle+seed*5.0))+.26*approach,0.0,1.0);
        cutBox(ink,wq,vec2(.023,.030),.007,vec3(.27,.21,.26));
        cutBox(ink,wq,vec2(.018,.025),.004,mix(vec3(.32,.31,.40),goldLeaf,glow));
        cutBox(ink,wq,vec2(.002,.024),.001,vec3(.46,.30,.27));
        cutBox(ink,wq,vec2(.017,.002),.001,vec3(.46,.30,.27));
        ink+=vec3(.12,.074,.020)*glow*lightPool(wq,vec2(.043));
    }
}
vec3 clocktowerCarols(vec2 p,vec2 extent,float turn,float cycle){
    vec2 panel=p/extent;
    vec3 ink=mix(vec3(.19,.17,.34),vec3(.83,.51,.43),smoothstep(-.4,.32,panel.y));
    vec2 sunset=p-vec2(-extent.x*.29,-.16*extent.y);
    ink+=vec3(.12,.065,.010)*lightPool(sunset,vec2(.35));
    cutDisc(ink,sunset,.072,vec3(.94,.74,.53));
    winterStars(ink,p+vec2(2,9),cycle,.27*(1.0-smoothstep(-.30,.04,panel.y)),ivory);
    float distantRoof=-.014+.046*abs(mod(p.x+.06,.12)-.06)/.06+.016*cos(p.x*8.0);
    paper(ink,distantRoof-p.y,vec3(.445,.35,.43));
    float walker=mix(-extent.x*.55-.10,extent.x*.55+.10,fract(turn+.15));
    float row=.252,spacing=.267;
    float houseId=floor((p.x+.50*spacing)/spacing);
    float worldX=houseId*spacing;
    carolHouse(ink,p-vec2(worldX,row),cellSeed(vec2(houseId,4.0)),cycle,walker,worldX);
    float road=.246+.018*cos(p.x*4.0);
    paper(ink,road-p.y,vec3(.73,.65,.64));
    paper(ink,road+.024+.007*sin(p.x*12.0)-p.y,vec3(.53,.49,.56));
    vec2 cobble=vec2(mod(p.x+.085*floor(p.y/.043),.17)-.085,mod(p.y,.043)-.0215);
    if(p.y>road+.033)paper(ink,harvestEllipse(cobble,vec2(.056,.0035)),vec3(.63,.57,.61));
    float size=.77+.96*dotRadius;
    vec2 q=(p-vec2(-.082*extent.x,.251))/size;
    cutBox(ink,q-vec2(0,-.203),vec2(.116,.203),.004,vec3(.65,.40,.31));
    cutBox(ink,q-vec2(.083,-.20),vec2(.031,.20),.002,vec3(.48,.29,.29));
    float course=mod(q.y,.052)-.026;
    if(abs(q.x)<.112&&q.y<0.0&&q.y>-.40){
        float joint=mod(q.x+floor(q.y/.052)*.040,.078)-.039;
        paper(ink,min(abs(course)-.0015,abs(joint)-.0013),vec3(.55,.34,.29));
    }
    cutBox(ink,q-vec2(0,-.015),vec2(.138,.015),.003,vec3(.42,.29,.31));
    cutBox(ink,q-vec2(0,-.381),vec2(.130,.013),.004,vec3(.92,.74,.60));
    // Belfry columns, swinging bell and separate clapper under the copper roof.
    cutBox(ink,q-vec2(0,-.435),vec2(.091,.048),.005,vec3(.155,.17,.24));
    for(int i=0;i<2;++i)cutBox(ink,q-vec2(i==0?-.088:.088,-.439),vec2(.012,.054),.003,vec3(.81,.58,.41));
    float swing=(.12+.47*flow)*sin(4.0*cycle);
    vec2 bell=rotatePoint(q-vec2(0,-.485),swing);
    float bellBody=max(abs(bell.x)-(.010+.027*smoothstep(.0,.060,bell.y)),max(-bell.y,bell.y-.063));
    paper(ink,bellBody,brass);
    cutBox(ink,bell-vec2(0,.062),vec2(.043,.007),.006,goldLeaf);
    cutDisc(ink,bell-vec2(.012*sin(4.0*cycle-.65),.075),.009,goldLeaf);
    float roof=christmasTriangle(q,vec2(-.139,-.494),vec2(0,-.657),vec2(.139,-.494));
    paper(ink,roof,vec3(.18,.37,.37));
    paper(ink,max(roof,q.x+.008),vec3(.25,.51,.46));
    paper(ink,max(roof,q.y+.523+.010*sin(q.x*45.0)),vec3(.91,.82,.68));
    cutLine(ink,q,vec2(0,-.65),vec2(0,-.686),.004,brass);
    cutDisc(ink,q-vec2(0,-.690),.010,goldLeaf);
    vec2 dial=q-vec2(0,-.275);
    cutDisc(ink,dial,.081,vec3(.35,.26,.27));
    cutDisc(ink,dial,.073,brass);
    cutDisc(ink,dial,.064,ivory);
    float mark=atan(dial.y,dial.x)*12.0;
    paper(ink,max(.054-length(dial),max(length(dial)-.061,.5-.5*cos(mark)-.05)),vec3(.22,.23,.28));
    // Twelve turns of the minute hand accompany one turn of the hour hand.
    float minute=12.0*cycle+.85,hour=cycle+3.14159265+.85/12.0;
    cutLine(ink,dial,vec2(0),vec2(sin(minute),-cos(minute))*.049,.0035,nightInk);
    cutLine(ink,dial,vec2(0),vec2(sin(hour),-cos(hour))*.031,.005,nightInk);
    cutDisc(ink,dial,.007,brass);
    cutBox(ink,q-vec2(0,-.089),vec2(.038,.067),.026,vec3(.32,.245,.28));
    cutBox(ink,q-vec2(0,-.056),vec2(.037,.036),.002,vec3(.32,.245,.28));
    cutBox(ink,q-vec2(0,-.090),vec2(.003,.06),.001,brass);
    // The lamplighter carries a physical lantern; its warm pool travels with it.
    float stepAngle=cycle*8.0;
    vec2 figure=(p-vec2(walker,.353-.006*cos(stepAngle*2.0)))/size;
    float stride=.024*sin(stepAngle);
    cutLine(ink,figure,vec2(-.007,-.035),vec2(-.012+stride,.0),.006,nightInk);
    cutLine(ink,figure,vec2(.008,-.035),vec2(.013-stride,.0),.006,nightInk);
    paper(ink,christmasTriangle(figure,vec2(0,-.119),vec2(-.034,-.031),vec2(.032,-.031)),vec3(.14,.28,.29));
    cutDisc(ink,figure-vec2(.003,-.123),.020,vec3(.91,.65,.44));
    cutBox(ink,figure-vec2(.0,-.144),vec2(.019,.012),.008,nightInk);
    cutBox(ink,figure-vec2(.004,-.136),vec2(.026,.004),.002,nightInk);
    cutBox(ink,figure-vec2(0,-.102),vec2(.018,.006),.004,cranberry);
    cutLine(ink,figure,vec2(-.01,-.102),vec2(-.050,-.086+.007*sin(stepAngle)),.005,cranberry);
    vec2 hand=vec2(.044,-.085+.007*sin(stepAngle));
    cutLine(ink,figure,vec2(.010,-.094),hand,.006,nightInk);
    vec2 lamp=figure-hand-vec2(.005,.019);
    ink+=vec3(.34,.19,.045)*lightPool(lamp,vec2(.070,.066));
    cutBox(ink,lamp,vec2(.011,.017),.003,brass);
    cutBox(ink,lamp,vec2(.007,.011),.002,ivory);
    return ink;
}

void nutcrackerLeg(inout vec3 ink,vec2 q,float side,float angle){
    vec2 leg=rotatePoint(q-vec2(side*.042,.091),angle);
    cutBox(ink,leg-vec2(0,.057),vec2(.022,.061),.011,vec3(.86,.76,.59));
    cutBox(ink,leg-vec2(-.010,.055),vec2(.003,.053),.002,ivory);
    cutBox(ink,leg-vec2(0,.147),vec2(.025,.054),.009,nightInk);
    cutBox(ink,leg-vec2(0,.106),vec2(.029,.010),.003,brass);
    cutBox(ink,leg-vec2(side*.010,.197),vec2(.038,.013),.010,nightInk);
    cutBox(ink,leg-vec2(side*.010,.208),vec2(.038,.003),.002,brass);
}
void nutcrackerArm(inout vec3 ink,vec2 q,float side,float angle,float cycle){
    vec2 arm=rotatePoint(q-vec2(side*.089,-.079),angle);
    cutBox(ink,arm-vec2(0,.052),vec2(.022,.056),.013,cranberry);
    cutBox(ink,arm-vec2(0,.105),vec2(.024,.016),.004,vec3(.15,.27,.31));
    cutBox(ink,arm-vec2(0,.102),vec2(.025,.004),.002,brass);
    cutBox(ink,arm-vec2(0,.128),vec2(.019,.018),.012,ivory);
    cutBox(ink,arm-vec2(0,.010),vec2(.029,.016),.011,brass);
    for(int i=0;i<4;++i){
        float x=(float(i)-1.5)*.013;
        cutLine(ink,arm,vec2(x,.015),vec2(x,.030),.0024,goldLeaf);
    }
    if(side>0.0){
        vec2 baton=rotatePoint(arm-vec2(.004,.129),-.58-.16*sin(cycle*2.0));
        cutLine(ink,baton,vec2(.003,.009),vec2(.091,-.101),.0042,brass);
        paper(ink,christmasStar(baton-vec2(.091,-.101),.017),ivory);
    }
}
vec3 nutcrackerWaltz(vec2 p,vec2 extent,float turn,float cycle){
    vec2 panel=p/extent;
    vec3 ink=vec3(.118,.066,.14)+vec3(.25,.105,.104)*lightPool(panel-vec2(0,-.17),vec2(.53,.49));
    float beamCentre=.10*sin(cycle);
    ink+=vec3(.15,.11,.066)*lightPool(vec2(panel.x-beamCentre,panel.y+.08),vec2(.27,.50));
    for(int i=0;i<2;++i){
        float side=i==0?-1.0:1.0;
        float spotlightX=side*(.36-(panel.y+.5)*.36)+.026*sin(cycle);
        ink+=vec3(.050,.036,.020)*lightPool(vec2(panel.x-spotlightX,panel.y+.15),vec2(.06+(panel.y+.5)*.09,.58));
    }
    winterStars(ink,p+vec2(3.0,3.4),cycle,.05+density*.0105,vec3(.99,.76,.43));
    float stageLevel=.366;
    paper(ink,stageLevel-p.y,vec3(.37,.19,.22));
    float plank=mod(p.x+.09*p.y,.174)-.087;
    if(p.y>stageLevel)paper(ink,abs(plank)-.0018,vec3(.48,.28,.23));
    float tilt=.035*sin(cycle*2.0);
    float curtainEdge=.38+.022*cos(panel.y*6.0+tilt)-.23*pow(max(.23-panel.y,0.0),1.6);
    float wings=curtainEdge-abs(panel.x);
    if(wings<.012){
        float folds=.5+.5*cos(abs(panel.x)*90.0+3.2*panel.y);
        vec3 velvet=mix(vec3(.25,.065,.115),vec3(.56,.135,.19),folds);
        paper(ink,wings,velvet);
        paper(ink,max(wings,abs(wings)-.005),brass);
        float tie=.051+.28*(abs(panel.x)-.39);
        paper(ink,max(wings,abs(panel.y-tie)-.009),goldLeaf);
    }
    float scallop=-.463+.052*(.5+.5*cos(panel.x*16.0));
    paper(ink,panel.y-scallop,vec3(.36,.075,.14));
    paper(ink,max(panel.y-scallop,scallop-.006-panel.y),brass);
    // A bevelled octagonal music box, winding key and repeating inlaid diamonds.
    float boxWidth=.307,boxY=.343;
    vec2 base=p-vec2(0,boxY);
    float plinth=max(christmasBox(base,vec2(boxWidth,.045),.008),abs(base.x)+abs(base.y)*.46-boxWidth);
    vec2 key=p-vec2(.346,.340);
    cutLine(ink,key,vec2(-.052,0),vec2(.025,0),.007,brass);
    float keyHeight=.010+.016*(.5+.5*cos(4.0*cycle));
    for(int i=0;i<2;++i){
        vec2 loop=key-vec2(.028,i==0?-keyHeight:keyHeight);
        paper(ink,abs(harvestEllipse(loop,vec2(.020,keyHeight)))-.0038,goldLeaf);
    }
    paper(ink,plinth,vec3(.12,.26,.28));
    paper(ink,max(plinth,abs(base.y)-.036),vec3(.17,.35,.34));
    float diamond=abs(mod(base.x+.032,.064)-.032)+abs(base.y)*1.15;
    paper(ink,max(plinth,abs(diamond-.018)-.0028),brass);
    cutBox(ink,base-vec2(0,-.046),vec2(.302,.008),.005,goldLeaf);
    cutBox(ink,base-vec2(0,.046),vec2(.290,.006),.004,brass);
    float size=.76+dotRadius*.91;
    float rhythm=sin(cycle*4.0),bow=seasonBeat(turn*24.0,7.5,10.0,11.6,14.0);
    vec2 pivot=vec2(.078*sin(cycle),.292);
    vec2 q=(p-pivot)/size+vec2(0,.299-.010*rhythm*rhythm);
    q=rotatePoint(q,(.018+.045*flow)*sin(cycle*2.0)+.075*bow*flow);
    nutcrackerLeg(ink,q,-1.0,-.065-(.13+.37*flow)*rhythm);
    nutcrackerLeg(ink,q,1.0,.065+(.13+.37*flow)*rhythm);
    nutcrackerArm(ink,q,-1.0,-.24-(.24+.50*flow)*(.5+.5*cos(cycle*2.0)),cycle);
    nutcrackerArm(ink,q,1.0,.28+(.42+.77*flow)*(.5+.5*sin(cycle*2.0))+.29*bow,cycle);
    float coat=max(abs(q.x)-(.079-.014*clamp((q.y+.089)/.21,0.0,1.0)),max(-.098-q.y,q.y-.106));
    paper(ink,coat,cranberry);
    paper(ink,max(coat,q.x+.045),vec3(.82,.20,.22));
    cutBox(ink,q-vec2(0,.071),vec2(.075,.012),.003,nightInk);
    cutBox(ink,q-vec2(0,.071),vec2(.013,.009),.002,goldLeaf);
    for(int i=0;i<3;++i){
        float y=-.045+.040*float(i);
        cutLine(ink,q,vec2(-.041,y-.004),vec2(0,y+.006),.0036,brass);
        cutLine(ink,q,vec2(0,y+.006),vec2(.041,y-.004),.0036,brass);
        cutDisc(ink,q-vec2(-.042,y-.004),.006,goldLeaf);
        cutDisc(ink,q-vec2(.042,y-.004),.006,goldLeaf);
    }
    // Painted wooden face, hinged jaw, side curls and a cockade on the shako.
    vec2 h=rotatePoint(q-vec2(.004*sin(2.0*cycle),-.167+.008*bow),-.050*rhythm-.085*bow);
    for(int i=0;i<2;++i){
        float side=i==0?-1.0:1.0;
        for(int j=0;j<3;++j){
            float n=float(j);
            cutDisc(ink,h-vec2(side*(.061+.008*n),-.018+.027*n),.022-.002*n,vec3(.82,.78,.67));
            cutDisc(ink,h-vec2(side*(.059+.008*n),-.023+.027*n),.014-.002*n,ivory);
        }
    }
    cutBox(ink,h,vec2(.063,.067),.025,vec3(.88,.60,.42));
    cutDisc(ink,h-vec2(-.043,.012),.011,vec3(.87,.37,.32));
    cutDisc(ink,h-vec2(.043,.012),.011,vec3(.87,.37,.32));
    for(int i=0;i<2;++i){
        float side=i==0?-1.0:1.0;
        cutDisc(ink,h-vec2(side*.026,-.016),.009,ivory);
        cutDisc(ink,h-vec2(side*.024,-.017),.0055,nightInk);
        cutLine(ink,h,vec2(side*.014,-.027),vec2(side*.036,-.033),.0045,nightInk);
        vec2 curl=rotatePoint(h-vec2(side*.021,.016),side*.27);
        paper(ink,harvestEllipse(curl,vec2(.023,.008)),nightInk);
    }
    paper(ink,christmasTriangle(h,vec2(-.007,-.014),vec2(.013,.007),vec2(-.009,.010)),vec3(.75,.43,.29));
    cutBox(ink,h-vec2(0,.045),vec2(.029,.013),.003,nightInk);
    cutBox(ink,h-vec2(0,.040),vec2(.026,.0065),.002,ivory);
    if(abs(h.x)<.025&&abs(h.y-.040)<.006)paper(ink,abs(mod(h.x+.0065,.013)-.0065)-.0012,brass);
    paper(ink,christmasTriangle(h,vec2(-.025,.060),vec2(.024,.060),vec2(.002,.096)),ivory);
    vec2 cap=h-vec2(0,-.112);
    cutBox(ink,cap,vec2(.069,.058),.009,vec3(.11,.27,.30));
    cutBox(ink,cap-vec2(0,-.053),vec2(.072,.007),.004,brass);
    cutBox(ink,cap-vec2(0,.052),vec2(.075,.009),.005,brass);
    paper(ink,christmasStar(cap,.025),goldLeaf);
    vec2 plume=rotatePoint(h-vec2(-.048,-.187),-.29+.08*rhythm);
    paper(ink,harvestEllipse(plume,vec2(.012,.037)),ivory);
    cutDisc(ink,h-vec2(-.040,-.161),.014,cranberry);
    return ink;
}
vec3 christmasScene(vec2 p,vec2 extent,float turn){
    float cycle=turn*christmasTau;
    if(pattern==23)return northPoleExpress(p,extent,turn,cycle);
    if(pattern==24)return evergreenOverture(p,extent,turn,cycle);
    if(pattern==25)return clocktowerCarols(p,extent,turn,cycle);
    return nutcrackerWaltz(p,extent,turn,cycle);
}
