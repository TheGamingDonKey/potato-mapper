// Original cut-paper Santa and flying reindeer. Local coordinates point right and down.
// Pigments accumulate premultiplied; sleighFlight returns straight RGBA to the mapper.
const float sfTau=6.283185307;
const vec3 sfInk=vec3(.145,.079,.112);
const vec3 sfMulberry=vec3(.42,.075,.135);
const vec3 sfRed=vec3(.87,.12,.17);
const vec3 sfPoppy=vec3(1.0,.265,.20);
const vec3 sfGold=vec3(.91,.60,.22);
const vec3 sfGilt=vec3(1.0,.845,.49);
const vec3 sfCream=vec3(1.0,.956,.813);
const vec3 sfFurShade=vec3(.735,.815,.794);
const vec3 sfFir=vec3(.115,.355,.304);

vec2 sfRotate(vec2 p,float a){
    float s=sin(a),c=cos(a);
    return vec2(c*p.x-s*p.y,s*p.x+c*p.y);
}
float sfOval(vec2 p,vec2 radii){return (length(p/radii)-1.0)*min(radii.x,radii.y);}
float sfCapsule(vec2 p,vec2 a,vec2 b,float ra,float rb){
    vec2 ab=b-a;
    float k=clamp(dot(p-a,ab)/max(dot(ab,ab),.000001),0.0,1.0);
    return length(p-a-k*ab)-mix(ra,rb,k);
}
float sfCurve(vec2 p,vec2 a,vec2 b,vec2 c,float ra,float rc){
    float d=10.0;vec2 previous=a;
    for(int i=1;i<=5;++i){
        float t=float(i)*.2;
        vec2 current=mix(mix(a,b,t),mix(b,c,t),t);
        d=min(d,sfCapsule(p,previous,current,mix(ra,rc,t-.2),mix(ra,rc,t)));
        previous=current;
    }
    return d;
}
float sfMerge(float a,float b,float radius){
    float h=max(radius-abs(a-b),0.0)/radius;
    return min(a,b)-.25*h*h*radius;
}
void sfOver(inout vec4 ink,vec3 pigment,float opacity){
    ink=vec4(pigment*opacity,opacity)+ink*(1.0-opacity);
}
void sfPaint(inout vec4 ink,float distance,float aa,vec3 pigment){
    sfOver(ink,pigment,1.0-smoothstep(-aa,aa,distance));
}
void sfLacquer(inout vec4 ink,float d,float aa,vec3 pigment,float rim){
    sfPaint(ink,d-rim,aa,sfInk);
    sfPaint(ink,d,aa,pigment);
}
void sfDisc(inout vec4 ink,vec2 p,float radius,float aa,vec3 pigment){
    sfPaint(ink,length(p)-radius,aa,pigment);
}

// A handful of closed cut-paper contours need concave curves. Each quadratic
// contributes its nearest edge and its crossings to the same winding test.
void sfContour(vec2 p,vec2 a,vec2 b,vec2 c,inout float nearest,inout bool inside){
    for(int i=1;i<=5;++i){
        float t0=float(i-1)*.2,t1=float(i)*.2;
        vec2 m=mix(mix(a,b,t0),mix(b,c,t0),t0);
        vec2 n=mix(mix(a,b,t1),mix(b,c,t1),t1);
        vec2 edge=n-m,rel=p-m;
        vec2 delta=rel-edge*clamp(dot(rel,edge)/max(dot(edge,edge),.000001),0.0,1.0);
        nearest=min(nearest,dot(delta,delta));
        if((m.y>p.y)!=(n.y>p.y)){
            if(p.x<m.x+edge.x*(p.y-m.y)/edge.y)inside=!inside;
        }
    }
}
float sfHull(vec2 p){
    float d=1.0;bool inside=false;
    sfContour(p,vec2(-.49,-.112),vec2(-.438,-.156),vec2(-.401,-.097),d,inside);
    sfContour(p,vec2(-.401,-.097),vec2(-.349,.008),vec2(-.184,.026),d,inside);
    sfContour(p,vec2(-.184,.026),vec2(.075,.071),vec2(.157,-.022),d,inside);
    sfContour(p,vec2(.157,-.022),vec2(.212,-.128),vec2(.275,-.112),d,inside);
    sfContour(p,vec2(.275,-.112),vec2(.227,-.070),vec2(.205,.019),d,inside);
    sfContour(p,vec2(.205,.019),vec2(.163,.166),vec2(-.080,.175),d,inside);
    sfContour(p,vec2(-.080,.175),vec2(-.397,.183),vec2(-.443,.085),d,inside);
    sfContour(p,vec2(-.443,.085),vec2(-.442,-.024),vec2(-.49,-.112),d,inside);
    return sqrt(d)*(inside?-1.0:1.0);
}
void sfRunners(inout vec4 ink,vec2 p,float aa){
    float rail=sfCurve(p,vec2(-.505,.147),vec2(-.482,.218),vec2(-.368,.223),.008,.012);
    rail=min(rail,sfCurve(p,vec2(-.368,.223),vec2(-.105,.257),vec2(.124,.207),.012,.012));
    rail=min(rail,sfCurve(p,vec2(.124,.207),vec2(.278,.184),vec2(.319,.082),.012,.009));
    rail=min(rail,sfCurve(p,vec2(.319,.082),vec2(.344,.012),vec2(.301,.023),.009,.006));
    rail=min(rail,sfCurve(p,vec2(.301,.023),vec2(.278,.026),vec2(.288,.055),.006,.005));
    float brace=min(sfCurve(p,vec2(-.346,.120),vec2(-.309,.189),vec2(-.339,.228),.012,.008),
                    sfCurve(p,vec2(.055,.122),vec2(.056,.179),vec2(.109,.210),.014,.008));
    sfLacquer(ink,brace,aa,sfGold,.003);
    sfLacquer(ink,rail,aa,sfGold,.0035);
    sfPaint(ink,sfCurve(p,vec2(-.366,.219),vec2(-.098,.250),vec2(.122,.202),.0028,.003),aa,sfCream);
}
void sfParcel(inout vec4 ink,vec2 p,vec2 size,float tilt,vec3 wrap,float aa){
    p=sfRotate(p,tilt);
    vec2 edge=abs(p)-size;
    float box=max(edge.x,edge.y);
    sfLacquer(ink,box,aa,wrap,.0025);
    sfPaint(ink,max(box,min(abs(p.x+.005)-.007,abs(p.y+size.y*.43)-.004)),aa,sfGilt);
}
void sfSack(inout vec4 ink,vec2 p,float aa){
    sfParcel(ink,p-vec2(-.372,-.212),vec2(.035,.047),-.27,sfRed,aa);
    sfParcel(ink,p-vec2(-.307,-.230),vec2(.040,.032),.16,sfGold,aa);
    vec2 bag=sfRotate(p-vec2(-.326,-.083),-.14);
    float canvas=sfMerge(sfOval(bag,vec2(.117,.112)),
                         sfCapsule(bag,vec2(-.006,-.045),vec2(-.015,-.123),.069,.031),.032);
    float mouth=sfOval(sfRotate(bag-vec2(-.020,-.145),-.18),vec2(.050,.025));
    canvas=min(canvas,mouth);
    sfLacquer(ink,canvas,aa,mix(sfFir,vec3(.27,.54,.39),smoothstep(-.11,.11,-bag.x)),.005);
    float fold=min(sfCurve(bag,vec2(-.049,-.101),vec2(-.094,-.072),vec2(-.089,-.031),.003,.001),
                   sfCurve(bag,vec2(-.010,-.105),vec2(.034,-.081),vec2(.044,-.040),.004,.001));
    sfPaint(ink,fold,aa,vec3(.53,.69,.46));
    sfPaint(ink,sfCapsule(bag,vec2(-.049,-.120),vec2(.020,-.128),.005,.006),aa,sfGilt);
    sfPaint(ink,sfCurve(bag,vec2(-.045,-.120),vec2(-.085,-.126),vec2(-.083,-.098),.004,.002),aa,sfGold);
}
void sfSleighFront(inout vec4 ink,vec2 p,float aa){
    float hull=sfHull(p);
    vec3 enamel=mix(sfMulberry,sfRed,1.0-smoothstep(-.06,.165,p.y+.055*p.x));
    sfPaint(ink,hull-.006,aa,sfInk);
    sfPaint(ink,hull,aa,sfGold);
    sfPaint(ink,hull+.008,aa,enamel);
    float flute=sfCurve(p,vec2(-.407,.082),vec2(-.395,.128),vec2(-.292,.139),.003,.003);
    flute=min(flute,sfCurve(p,vec2(-.292,.139),vec2(.032,.188),vec2(.143,.073),.003,.002));
    sfPaint(ink,flute,aa,sfGilt);
    vec2 medallion=p-vec2(-.145,.092);
    sfPaint(ink,sfOval(medallion,vec2(.040,.029)),aa,sfGold);
    sfPaint(ink,sfOval(medallion,vec2(.034,.024)),aa,sfMulberry);
    float star=min(abs(medallion.x),abs(medallion.y))*.85+max(abs(medallion.x),abs(medallion.y))*.15-.0031;
    sfPaint(ink,star,aa,sfCream);
    for(int i=0;i<2;++i){
        float side=i==0?-1.0:1.0;
        vec2 leaf=sfRotate(medallion-vec2(side*.055,0.0),side*.46);
        sfPaint(ink,sfOval(leaf,vec2(.015,.004)),aa,sfGilt);
    }
    sfDisc(ink,p-vec2(.274,-.111),.012,aa,sfGilt);
}

float sfCap(vec2 p,vec2 tip){
    float d=1.0;bool inside=false;
    sfContour(p,vec2(.067,-.050),vec2(.047,-.144),vec2(.006,-.162),d,inside);
    sfContour(p,vec2(.006,-.162),vec2(-.064,-.209),vec2(-.117,-.155),d,inside);
    sfContour(p,vec2(-.117,-.155),vec2(tip.x,-.151),tip,d,inside);
    sfContour(p,tip,vec2(-.107,-.143),vec2(-.079,-.119),d,inside);
    sfContour(p,vec2(-.079,-.119),vec2(-.080,-.083),vec2(-.069,-.044),d,inside);
    sfContour(p,vec2(-.069,-.044),vec2(.002,-.070),vec2(.067,-.050),d,inside);
    return sqrt(d)*(inside?-1.0:1.0);
}
float sfBeard(vec2 p){
    float d=1.0;bool inside=false;
    sfContour(p,vec2(-.063,-.018),vec2(-.083,-.009),vec2(-.076,.022),d,inside);
    sfContour(p,vec2(-.076,.022),vec2(-.098,.064),vec2(-.061,.069),d,inside);
    sfContour(p,vec2(-.061,.069),vec2(-.082,.099),vec2(-.046,.102),d,inside);
    sfContour(p,vec2(-.046,.102),vec2(-.048,.134),vec2(.005,.143),d,inside);
    sfContour(p,vec2(.005,.143),vec2(-.001,.128),vec2(.026,.112),d,inside);
    sfContour(p,vec2(.026,.112),vec2(.066,.096),vec2(.059,.070),d,inside);
    sfContour(p,vec2(.059,.070),vec2(.084,.057),vec2(.060,.022),d,inside);
    sfContour(p,vec2(.060,.022),vec2(.048,.004),vec2(.028,.033),d,inside);
    sfContour(p,vec2(.028,.033),vec2(-.030,.057),vec2(-.063,-.018),d,inside);
    return sqrt(d)*(inside?-1.0:1.0);
}
void sfSantaFace(inout vec4 ink,vec2 h,float cycle,float aa){
    if(h.x<-.197||h.x>.101||h.y<-.213||h.y>.164)return;
    vec3 skin=mix(vec3(.94,.595,.406),vec3(1.0,.801,.615),1.0-smoothstep(-.07,.07,h.y));
    sfLacquer(ink,sfOval(h,vec2(.072,.069)),aa,skin,.004);
    sfDisc(ink,h-vec2(-.045,.008),.015,aa,vec3(.98,.475,.385));
    sfDisc(ink,h-vec2(.049,.008),.013,aa,vec3(.98,.475,.385));
    float beard=sfBeard(h);
    sfPaint(ink,beard-.0035,aa,sfInk);
    sfPaint(ink,beard,aa,sfFurShade);
    sfPaint(ink,beard+.004,aa,mix(sfFurShade,sfCream,.57+.43*smoothstep(-.067,.045,h.x-.2*h.y)));
    sfPaint(ink,sfCurve(h,vec2(-.047,.058),vec2(-.059,.075),vec2(-.036,.080),.0025,.001),aa,sfFurShade);
    sfPaint(ink,sfCurve(h,vec2(.025,.075),vec2(.031,.087),vec2(.008,.099),.0025,.001),aa,sfFurShade);
    sfPaint(ink,sfOval(h-vec2(.014,.047),vec2(.016,.009)),aa,sfMulberry);
    for(int eye=0;eye<2;++eye){
        float side=eye==0?-1.0:1.0;
        vec2 center=vec2(.004+side*.028,-.013);
        float blink=pow(max(cos(3.0*cycle+1.3),0.0),64.0);
        sfPaint(ink,sfOval(h-center,vec2(.0068,.002+.007*(1.0-blink))),aa,sfInk);
        sfPaint(ink,sfCurve(h,center+vec2(-.012,-.012),center+vec2(.001,-.023),center+vec2(.012,-.015),.0045,.003),aa,sfCream);
        vec2 moustache=sfRotate(h-vec2(.010+side*.025,.031),-side*.32);
        sfPaint(ink,sfOval(moustache,vec2(.026,.011)),aa,sfCream);
    }
    sfPaint(ink,sfOval(h-vec2(.021,.010),vec2(.020,.014)),aa,vec3(.98,.55,.395));
    sfDisc(ink,h-vec2(.025,.006),.0045,aa,vec3(1.0,.81,.61));
    vec2 tip=vec2(-.151-.009*flow*sin(4.0*cycle),-.099+.013*flow*sin(4.0*cycle+.53));
    float hat=sfCap(h,tip);
    sfLacquer(ink,hat,aa,mix(sfRed,sfPoppy,smoothstep(-.135,.04,h.x)),.004);
    sfPaint(ink,sfCurve(h,vec2(-.088,-.138),vec2(-.067,-.160),vec2(-.032,-.154),.003,.0015),aa,sfMulberry);
    sfDisc(ink,h-tip,.023,aa,sfFurShade);
    sfDisc(ink,h-tip+vec2(.003,.005),.020,aa,sfCream);
    float brim=sfCurve(h,vec2(-.074,-.046),vec2(-.008,-.074),vec2(.066,-.053),.016,.0165);
    sfLacquer(ink,brim,aa,sfFurShade,.0025);
    sfPaint(ink,sfCurve(h,vec2(-.075,-.050),vec2(-.008,-.078),vec2(.065,-.057),.012,.013),aa,sfCream);
}
void sfSanta(inout vec4 ink,vec2 p,float cycle,float turn,float aa){
    vec2 coat=sfRotate(p-vec2(-.129,-.080),-.14);
    sfLacquer(ink,sfOval(coat,vec2(.100,.128)),aa,sfRed,.006);
    sfPaint(ink,sfCurve(coat,vec2(.019,-.097),vec2(.048,-.047),vec2(.053,.051),.011,.013),aa,sfCream);
    sfPaint(ink,sfCurve(coat,vec2(-.086,.031),vec2(-.019,.058),vec2(.075,.035),.011,.012),aa,sfInk);
    vec2 punch=abs(coat-vec2(.022,.046))-vec2(.015,.010);
    sfPaint(ink,max(punch.x,punch.y),aa,sfGilt);
    // One mitten holds the leather reins; the other lifts and waves to the audience.
    vec2 reinHand=vec2(.092,-.070);
    sfLacquer(ink,sfCurve(p,vec2(-.072,-.125),vec2(-.020,-.056),reinHand-vec2(.020,-.003),.029,.020),aa,sfMulberry,.004);
    sfDisc(ink,p-reinHand+vec2(.021,0),.019,aa,sfCream);
    sfPaint(ink,sfOval(p-reinHand,vec2(.021,.015)),aa,sfFir);
    float hello=smoothstep(.14,.33,turn)*(1.0-smoothstep(.69,.87,turn));
    vec2 elbow=vec2(.000,-.146)-vec2(.004,.031)*hello;
    vec2 direction=normalize(mix(vec2(.84,-.40),vec2(.36+(.18+.28*flow)*sin(13.0*cycle),-1.0),hello));
    vec2 wrist=elbow+direction*.087;
    float sleeve=min(sfCapsule(p,vec2(-.082,-.150),elbow,.031,.026),
                     sfCapsule(p,elbow,wrist,.026,.017));
    sfLacquer(ink,sleeve,aa,sfPoppy,.0045);
    sfPaint(ink,sfCapsule(p,wrist-direction*.009,wrist+direction*.009,.019,.018),aa,sfCream);
    vec2 hand=p-(wrist+direction*.030);
    vec2 palm=vec2(dot(hand,vec2(-direction.y,direction.x)),dot(hand,direction));
    float glove=sfMerge(sfOval(palm,vec2(.020,.029)),sfOval(sfRotate(palm-vec2(-.019,-.003),-.50),vec2(.010,.016)),.007);
    sfLacquer(ink,glove,aa,sfFir,.003);
    sfPaint(ink,sfCapsule(palm,vec2(-.006,.011),vec2(-.005,.020),.0026,.0015),aa,vec3(.55,.73,.50));
    vec2 faceCenter=vec2(-.128,-.229-.006*hello+.006*sin(4.0*cycle));
    vec2 h=sfRotate(p-faceCenter,.028*sin(3.0*cycle)-.035*hello);
    sfSantaFace(ink,h,cycle,aa);
}

void sfLeg(inout vec4 ink,vec2 p,float gait,bool front,bool nearSide,float aa){
    float stride=sin(gait)*(.45+.55*flow);
    float lift=(.5+.5*cos(gait))*(.55+.45*flow);
    vec2 hip=vec2(front?.074:-.081,nearSide?.017:.009);
    vec2 joint,ankle,hoof;
    if(front){
        joint=hip+vec2(.012+.052*stride,.064-.027*lift);
        ankle=joint+vec2(.009+.049*stride,.062-.034*lift);
        hoof=ankle+vec2(.015+.015*stride,.027-.010*lift);
    }else{
        joint=hip+vec2(.034+.033*stride,.051-.019*lift);
        ankle=joint+vec2(-.053+.041*stride,.045-.027*lift);
        hoof=ankle+vec2(.012+.032*stride,.052-.020*lift);
    }
    float limb=min(sfCapsule(p,hip,joint,.023,.014),sfCapsule(p,joint,ankle,.014,.008));
    limb=min(limb,sfCapsule(p,ankle,hoof,.009,.007));
    sfLacquer(ink,limb,aa,nearSide?vec3(.80,.48,.265):vec3(.49,.28,.23),.0025);
    vec2 toe=hoof+vec2(.014+.006*stride,.004);
    sfPaint(ink,sfCapsule(p,hoof-vec2(.001,.004),toe,.009,.005),aa,sfInk);
}
float sfAntler(vec2 p){
    vec2 root=vec2(.125,-.169),bend=vec2(.065,-.232),fork=vec2(.049,-.279);
    float horn=sfCurve(p,root,vec2(.114,-.202),bend,.010,.007);
    horn=min(horn,sfCurve(p,bend,vec2(.035,-.251),fork,.007,.005));
    horn=min(horn,sfCurve(p,fork,vec2(.054,-.301),vec2(.071,-.315),.005,.002));
    horn=min(horn,sfCurve(p,bend,vec2(.028,-.225),vec2(.005,-.267),.006,.002));
    horn=min(horn,sfCurve(p,fork,vec2(.024,-.277),vec2(.014,-.306),.005,.002));
    horn=min(horn,sfCurve(p,vec2(.102,-.202),vec2(.119,-.226),vec2(.111,-.251),.006,.002));
    return horn;
}
void sfReindeer(inout vec4 ink,vec2 p,float gait,bool leader,float aa){
    if(p.x<-.223||p.x>.281||p.y<-.350||p.y>.202)return;
    vec3 coat=leader?vec3(.73,.405,.22):vec3(.60,.31,.215);
    vec3 sun=leader?vec3(.93,.64,.37):vec3(.81,.49,.31);
    if(p.y>-.012){
        sfLeg(ink,p,gait+2.62,true,false,aa);
        sfLeg(ink,p,gait+.54,false,false,aa);
    }
    // The far antler has its own silhouette above the crown.
    if(p.y<-.156&&p.x>-.019&&p.x<.180){
        sfLacquer(ink,sfAntler((p-vec2(.034,-.018))/.96)*.96,aa,vec3(.76,.49,.31),.0024);
        sfLacquer(ink,sfAntler(p),aa,sfGilt,.0027);
    }
    vec2 tail=sfRotate(p-vec2(-.147,-.031),.60+.13*sin(gait));
    sfLacquer(ink,sfOval(tail,vec2(.040,.019)),aa,coat,.0025);
    sfPaint(ink,max(sfOval(tail,vec2(.039,.018)),tail.x+.015),aa,sfCream);
    if(p.y>-.012){
        sfLeg(ink,p,gait,true,true,aa);
        sfLeg(ink,p,gait+2.39,false,true,aa);
    }
    float rump=sfOval(sfRotate(p-vec2(-.027,-.007),.035),vec2(.123,.060));
    float neck=sfCapsule(p,vec2(.067,.001),vec2(.140,-.127),.054,.027);
    float body=sfMerge(rump,neck,.047);
    sfLacquer(ink,body,aa,mix(coat,sun,1.0-smoothstep(-.060,.052,p.y)),.0035);
    float bib=sfCurve(p,vec2(.142,-.095),vec2(.132,-.031),vec2(.096,.009),.017,.009);
    sfPaint(ink,max(body,bib),aa,vec3(.97,.798,.543));
    for(int i=0;i<4;++i){
        float n=float(i);
        vec2 spot=p-vec2(-.107+n*.022,-.024-.012*sin(n*1.22));
        sfPaint(ink,sfOval(spot,vec2(.0055,.0034)),aa,vec3(.99,.79,.525));
    }
    float harness=sfCurve(p,vec2(.068,-.075),vec2(.099,-.056),vec2(.129,-.054),.008,.009);
    sfPaint(ink,max(body,harness),aa,sfMulberry);
    sfPaint(ink,max(body,sfCapsule(p,vec2(.066,-.073),vec2(.123,-.053),.004,.004)),aa,sfRed);
    sfDisc(ink,p-vec2(.113,-.041),.008,aa,sfGilt);
    // A long muzzle, upright ears, branching antlers and hooves read in silhouette.
    vec2 ear=sfRotate(p-vec2(.098,-.163),.63);
    sfLacquer(ink,sfOval(ear,vec2(.035,.014)),aa,sun,.0025);
    sfPaint(ink,sfOval(ear-vec2(-.002,0),vec2(.021,.0055)),aa,vec3(.73,.395,.32));
    vec2 rearEar=sfRotate(p-vec2(.155,-.173),-1.05);
    sfLacquer(ink,sfOval(rearEar,vec2(.026,.011)),aa,coat,.002);
    float skull=sfOval(sfRotate(p-vec2(.151,-.129),-.19),vec2(.048,.038));
    float muzzle=sfCapsule(p,vec2(.175,-.133),vec2(.218,-.121),.026,.016);
    float head=sfMerge(skull,muzzle,.020);
    sfLacquer(ink,head,aa,sun,.0035);
    sfPaint(ink,max(head,sfOval(p-vec2(.207,-.117),vec2(.032,.015))),aa,vec3(.98,.785,.566));
    sfDisc(ink,p-vec2(.161,-.141),.0075,aa,sfInk);
    sfDisc(ink,p-vec2(.163,-.144),.0026,aa,sfCream);
    sfPaint(ink,sfCurve(p,vec2(.206,-.111),vec2(.215,-.107),vec2(.219,-.112),.0023,.0015),aa,sfInk);
    sfPaint(ink,max(head,sfCapsule(p,vec2(.178,-.149),vec2(.189,-.111),.0034,.0038)),aa,sfMulberry);
    sfDisc(ink,p-vec2(.184,-.115),.0054,aa,sfGilt);
    vec2 nose=p-vec2(.228,-.129);
    if(leader){
        float halo=1.0-smoothstep(.010,.038,length(nose));
        sfOver(ink,sfPoppy,.27*halo*halo);
    }
    sfLacquer(ink,sfOval(nose,vec2(.013,.0095)),aa,leader?sfRed:sfInk,.0015);
    if(leader)sfDisc(ink,nose+vec2(.002,.003),.004,aa,sfGilt);
}

void sfWake(inout vec4 ink,vec2 p,float turn,float cycle,float aa){
    if(p.x>-.39||p.x< -1.23||abs(p.y)>.31)return;
    float count=clamp(density*.121,0.0,10.0);
    for(int i=0;i<10;++i){
        float n=float(i),presence=clamp(count-n,0.0,1.0);
        if(presence<=.0)continue;
        float age=fract(turn*3.0+n*.381966);
        vec2 center=vec2(-.456-(.54+.13*flow)*age,
            .13+.074*age*sin(cycle*3.0+n*2.39)+.022*cos(cycle*2.0+n));
        vec2 star=abs(p-center);
        float r=.009+.012*(.5+.5*cos(n*2.07+cycle*4.0));
        float d=max(min(star.x,star.y)*.82+max(star.x,star.y)*.18-r*.18,max(star.x,star.y)-r);
        float fade=smoothstep(.01,.16,age)*(1.0-smoothstep(.66,.97,age))*presence;
        sfOver(ink,sfGilt,(1.0-smoothstep(-aa,aa,d))*.90*fade);
    }
}

vec4 sleighFlight(vec2 routeUv,vec2 routeExtent,float turn){
    vec2 physical=(routeUv-.5)*routeExtent;
    vec2 tangent=vec2(cos(flightTravelAngle),-sin(flightTravelAngle));
    vec2 normal=vec2(-tangent.y,tangent.x);
    float size=flightArtSize;
    float cycle=sfTau*turn;
    vec2 origin=flightPosition+vec2(0.0,size*(.055+.035*sin(cycle)+(.012+.025*flow)*sin(3.0*cycle)));
    vec2 delta=physical-(origin.x*tangent+origin.y*normal);
    // Automatic vertical travel keeps the figures upright. The user's Angle
    // still turns the illustration and the team retains its slight flight bank.
    vec2 p=sfRotate(delta/max(size,.001),angle+(.015+.030*flow)*cos(2.0*cycle));
    // Compute the pixel footprint before any spatial branch on the fragment.
    float aa=clamp(.52*max(length(dFdx(p)),length(dFdy(p))),.00055,.045);
    if(p.x< -1.24||p.x>1.23||p.y<-.55||p.y>.31)return vec4(0.0);
    vec4 ink=vec4(0.0);
    sfWake(ink,p,turn,cycle,aa);
    float gait=cycle*14.0;
    float amplitude=.009+.018*flow;
    vec2 first=vec2(.478,-.001+amplitude*sin(gait));
    vec2 lead=vec2(.934,-.088+amplitude*sin(gait+1.9));
    const float firstSize=.81,leadSize=.89;
    vec2 hand=vec2(.092,-.070);
    vec2 collar=first+firstSize*vec2(.106,-.057);
    vec2 leadCollar=lead+leadSize*vec2(.106,-.057);
    if(p.x>.068&&p.x<1.057&&p.y>-.235&&p.y<.12){
        float line=sfCurve(p,hand,vec2(.270,.024),collar,.0038,.0036);
        line=min(line,sfCurve(p,collar,mix(collar,leadCollar,.48)+vec2(0,.051),leadCollar,.0036,.0032));
        sfPaint(ink,line-.0017,aa,sfInk);
        sfPaint(ink,line,aa,sfGilt);
    }
    sfReindeer(ink,(p-first)/firstSize,gait,false,aa/firstSize);
    sfReindeer(ink,(p-lead)/leadSize,gait+1.9,true,aa/leadSize);
    if(p.x>-.53&&p.x<.35){
        if(p.y>-.005&&p.y<.272)sfRunners(ink,p,aa);
        if(p.x<-.174&&p.y>-.303&&p.y<.058)sfSack(ink,p,aa);
        if(p.x>-.342&&p.x<.167&&p.y<.067)sfSanta(ink,p,cycle,turn,aa);
        if(p.x<.299&&p.y>-.165&&p.y<.190)sfSleighFront(ink,p,aa);
    }
    if(ink.a<=0.0)return vec4(0.0);
    return vec4(ink.rgb/ink.a,ink.a);
}
