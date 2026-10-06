#include "dynamic.h"
#include <QJsonArray>
#include <QSet>
#include <QTransform>
#include <algorithm>
#include <cmath>
#include <limits>

namespace PotatoDynamic {
bool Settings::operator==(const Settings &o) const {
    return effect==o.effect && speed==o.speed && density==o.density && palette==o.palette
        && glitch==o.glitch && playing==o.playing && seed==o.seed && members==o.members;
}
QJsonObject Settings::json() const {
    QJsonArray ids; for(const auto &id:members) ids.append(id);
    return {{"effect",effect},{"speed",speed},{"density",density},{"palette",palette},
            {"glitch",glitch},{"playing",playing},{"seed",double(seed)},{"members",ids}};
}
Settings Settings::fromJson(const QJsonObject &o) {
    Settings s;
    s.effect=o.value("effect").toInt(0); if(s.effect<0 || s.effect>2) s.effect=0;
    s.speed=std::clamp(o.value("speed").toInt(100),10,300);
    s.density=std::clamp(o.value("density").toInt(36),8,80);
    s.palette=std::clamp(o.value("palette").toInt(0),0,5);
    s.glitch=std::clamp(o.value("glitch").toInt(35),0,100);
    s.playing=o.value("playing").toBool(true);
    const auto value=o.value("seed");
    const double seedNumber=value.toDouble(double(s.seed));
    if(std::isfinite(seedNumber) && seedNumber>=0 && seedNumber<=double(std::numeric_limits<quint32>::max())) s.seed=quint32(seedNumber);
    QSet<QString> seen;
    for(const auto &v:o.value("members").toArray()) {
        if(!v.isString()) continue;
        const auto id=v.toString();
        if(id.isEmpty() || seen.contains(id)) continue;
        seen.insert(id); s.members.append(id);
    }
    return s;
}
Timeline timeline(const Settings &s, int count) {
    if(s.effect!=1 || count<=0) return {};
    const double explore=std::clamp(8.0+3.0*std::min(count,4),11.0,20.0);
    return {explore,explore+3.5,explore+6.5};
}
double ambientStart(const Settings &s, int count) {return timeline(s,count).titleEnd;}

namespace {
constexpr double pi=3.14159265358979323846;
constexpr int maxShapes=500;
constexpr int boundarySamples=32;
double unit(double v) {return std::clamp(v,0.0,1.0);}
double smooth(double v) {v=unit(v);return v*v*(3-2*v);}
double fract(double v) {return v-std::floor(v);}
double lengthSquared(const QPointF &v) {return v.x()*v.x()+v.y()*v.y();}
QPointF mix(const QPointF &a,const QPointF &b,double v) {return a+(b-a)*v;}
QPointF clipped(QPointF p) {return {unit(p.x()),unit(p.y())};}
quint32 scramble(quint32 v) {
    v^=v>>16;v*=0x7feb352du;v^=v>>15;v*=0x846ca68bu;v^=v>>16;return v;
}
quint32 idSeed(const QString &id,quint32 seed) {
    for(const auto c:id) seed=scramble(seed^quint32(c.unicode()));
    return seed;
}
double random(quint32 seed,int index) {return double(scramble(seed+quint32(index)*0x9e3779b9u))/4294967296.0;}
QColor ink(int palette,int index,double alpha) {
    static const QColor colors[6][3]={
        {QColor(65,237,255),QColor(155,100,255),QColor(255,191,94)},
        {QColor(235,235,235),QColor(215,215,215),QColor(255,255,255)},
        {QColor(65,237,255),QColor(44,190,225),QColor(169,248,255)},
        {QColor(160,108,255),QColor(125,78,225),QColor(216,175,255)},
        {QColor(255,188,86),QColor(255,133,59),QColor(255,224,151)},
        {QColor(112,255,196),QColor(60,226,183),QColor(182,255,215)}};
    QColor c=colors[std::clamp(palette,0,5)][((index%3)+3)%3];c.setAlphaF(unit(alpha));return c;
}
struct Builder {
    Frame &out;
    void add(const Surface &s,QPolygonF points,QColor color,bool filled=false,double width=.002,bool closed=true) {
        if(out.shapes.size()>=maxShapes || points.size()<2 || color.alpha()==0) return;
        for(auto &p:points) {
            if(!std::isfinite(p.x()) || !std::isfinite(p.y())) return;
            p=clipped(p);
        }
        const bool closePolygon=closed && points.size()>2;
        out.shapes.append({s.id,std::move(points),color,filled,width,closePolygon});
    }
};
// Physical-length offsets expressed in UV. This preserves polygon proportions
// on tall/wide mapped surfaces without coupling the generator to Canvas.
QPointF metric(const Surface &s,QPointF p) {
    const double aspect=std::isfinite(s.aspect)?std::clamp(s.aspect,.1,10.0):1.0;
    if(aspect>=1) p.setX(p.x()/aspect);else p.setY(p.y()*aspect);
    return p;
}
QPolygonF regular(const Surface &s,QPointF center,double radius,int sides,double angle) {
    QPolygonF p;
    for(int i=0;i<sides;++i) {
        const double a=angle+2*pi*i/sides;
        p.append(center+metric(s,{std::cos(a)*radius,std::sin(a)*radius}));
    }
    return p;
}
QPointF perimeter(double position,double inset=0) {
    const double t=fract(position)*4;
    const double span=1-2*inset;
    if(t<1) return {inset+t*span,inset};
    if(t<2) return {1-inset,inset+(t-1)*span};
    if(t<3) return {1-inset-(t-2)*span,1-inset};
    return {inset,1-inset-(t-3)*span};
}
struct Stop {int index=0;QPointF entry{0,.5},exit{1,.5};};
QVector<Stop> tour(const QVector<Surface> &surfaces,quint32 seed) {
    const int count=int(surfaces.size());
    QVector<Stop> result;if(!count) return result;
    QVector<QVector<QPointF>> mapped(count);
    for(int i=0;i<count;++i) {
        QTransform transform;
        const QPolygonF square{{0,0},{1,0},{1,1},{0,1}};
        const bool projective=surfaces[i].boundary.size()==4 && QTransform::quadToQuad(square,surfaces[i].boundary,transform);
        for(int k=0;k<boundarySamples;++k) {
            const QPointF local=perimeter(double(k)/boundarySamples);
            QPointF point=projective?transform.map(local):local;
            if(!std::isfinite(point.x()) || !std::isfinite(point.y())) point=local;
            mapped[i].append(point);
        }
    }
    QVector<bool> visited(count,false);
    int current=0;
    Stop active{current,perimeter(random(seed,0)),{1,.5}};
    for(int step=0;step<count;++step) {
        visited[current]=true;
        int next=-1,exitIndex=0,entryIndex=0;
        double closest=std::numeric_limits<double>::max();
        // A greedy visual tour: nearest remaining surface, then closest sampled
        // boundary pair. IDs have already been sorted for stable tie breaking.
        for(int candidate=0;candidate<count;++candidate) {
            if(visited[candidate]) continue;
            for(int a=0;a<boundarySamples;++a) for(int b=0;b<boundarySamples;++b) {
                const double distance=lengthSquared(mapped[current][a]-mapped[candidate][b]);
                if(distance<closest-1e-12) {closest=distance;next=candidate;exitIndex=a;entryIndex=b;}
            }
        }
        active.exit=next<0?perimeter(random(seed,step+17)):perimeter(double(exitIndex)/boundarySamples);
        result.append(active);
        if(next<0) break;
        current=next;active={current,perimeter(double(entryIndex)/boundarySamples),{1,.5}};
    }
    return result;
}
const QVector<Stop> &cachedTour(const QVector<Surface> &surfaces,quint32 seed) {
    // Geometry edits change the key, while advancing phase preserves the route.
    // The GUI scene computes one frame on its thread; editor/output share it.
    struct Cache {QVector<Surface> surfaces;quint32 seed=0;QVector<Stop> stops;};
    static thread_local Cache cache;
    bool same=cache.seed==seed && cache.surfaces.size()==surfaces.size();
    if(same) for(qsizetype i=0;i<surfaces.size();++i) {
        if(cache.surfaces[i].id!=surfaces[i].id || cache.surfaces[i].boundary!=surfaces[i].boundary) {same=false;break;}
    }
    if(!same) {cache.surfaces=surfaces;cache.seed=seed;cache.stops=tour(surfaces,seed);}
    return cache.stops;
}
QPointF path(const Stop &stop,double t,quint32 seed) {
    t=unit(t);
    const auto entry=stop.entry,exit=stop.exit;
    // Cubic turns begin/end at the mapped boundary; two seeded interior bends
    // and a soft lateral wave articulate the body without sharp reversals.
    const QPointF a(.28+.18*random(seed,2),.22+.28*random(seed,3));
    const QPointF b(.52+.2*random(seed,4),.5+.26*random(seed,5));
    const double q=1-t;
    QPointF p=entry*(q*q*q)+a*(3*q*q*t)+b*(3*q*t*t)+exit*(t*t*t);
    const double wave=std::sin(pi*t)*std::sin(3*pi*t+random(seed,6)*2*pi)*.075;
    p+=QPointF(wave,-wave*.7);
    return clipped(p);
}
void snake(Builder &builder,const Surface &s,const Stop &stop,const Settings &settings,double progress,double opacity) {
    const quint32 seed=idSeed(s.id,settings.seed);
    constexpr int segments=25;
    for(int j=segments-1;j>=0;--j) {
        const double t=progress-j*.012;
        if(t<0 || t>1) continue;
        const QPointF center=path(stop,t,seed);
        QPointF direction=path(stop,t+.004,seed)-path(stop,t-.004,seed);
        const double angle=std::atan2(direction.y(),direction.x()*std::clamp(s.aspect,.1,10.0));
        const double taper=.22+.78*(1-double(j)/segments);
        const double radius=.038*taper*(1+.13*std::sin(t*34-j*.45));
        QPolygonF body;
        const QPointF forward=metric(s,{std::cos(angle)*radius,std::sin(angle)*radius});
        const QPointF side=metric(s,{-std::sin(angle)*radius*.78,std::cos(angle)*radius*.78});
        body<<center+forward<<center+side<<center-forward*.85<<center-side;
        builder.add(s,body,ink(settings.palette,j%7==0?1:0,opacity*(.34+.25*taper)),true,.0045);
        if(j%6==0) builder.add(s,regular(s,center,radius*.38,3,angle),ink(settings.palette,2,opacity*.68),false,.0018);
    }
    if(progress>=0 && progress<=1) {
        const auto head=path(stop,progress,seed);
        const auto v=path(stop,progress+.006,seed)-path(stop,progress-.006,seed);
        const double angle=std::atan2(v.y(),v.x()*std::clamp(s.aspect,.1,10.0));
        builder.add(s,regular(s,head,.043,3,angle),ink(settings.palette,0,opacity*.92),false,.0055);
    }
}
void trace(Builder &builder,const Surface &s,const Settings &settings,double time,double alpha) {
    const quint32 seed=idSeed(s.id,settings.seed);
    const double start=fract(time*.027+random(seed,20));
    for(int i=0;i<3;++i) {
        QPolygonF points;
        for(int k=0;k<=5;++k) points<<perimeter(start+i/3.0+k*.016,.07);
        builder.add(s,points,ink(settings.palette,i,alpha),false,.0021,false);
    }
}
void fragments(Builder &builder,const Surface &s,const Stop &stop,const Settings &settings,
               double visitedProgress,double age,double time,int budget,bool overload,double fade,double rebootProgress=-1) {
    if(visitedProgress<=0 || budget<=0) return;
    const quint32 seed=idSeed(s.id,settings.seed);
    const int count=std::min(budget,std::clamp(settings.density/2,8,40));
    for(int i=0;i<count;++i) {
        const double birth=.06+.8*random(seed,i+100);
        if(visitedProgress<birth) continue;
        const double localAge=age+(visitedProgress-birth)*2.8;
        const double settled=smooth(localAge/2.2);
        QPointF origin=path(stop,birth,seed);
        QPointF destination=perimeter(random(seed,i+160),.1+.035*random(seed,i+200));
        const QPointF control=mix(origin,destination,.5)+QPointF(.07*std::sin(i*2.4),.06*std::cos(i*1.7));
        const double q=1-settled;
        QPointF center=origin*(q*q)+control*(2*q*settled)+destination*(settled*settled);
        const double pulse=std::sin(time*1.4+i*.83)*.5+.5;
        center+=metric(s,{std::sin(time*.7+i)*.0035*settled,std::cos(time*.65+i)*.0035*settled});
        double rotation=random(seed,i+240)*2*pi+time*.12*(1-settled);
        double radius=(.011+.012*random(seed,i+280))*(.88+.17*pulse);
        if(overload) {
            const double strength=std::clamp(settings.glitch,0,100)/100.0;
            const double burst=.5+.5*std::sin(time*23+i*1.31);
            center+=metric(s,{std::sin(time*17+i)*.065*strength*burst,std::cos(time*19+i)*.038*strength*burst});
            rotation+=strength*std::sin(time*13+i)*.7;
            radius*=1+.35*strength*burst;
        }
        if(rebootProgress>=0) {
            const double assemble=smooth(rebootProgress/.42)*(1-smooth((rebootProgress-.7)/.3));
            const int columns=(count+1)/2;
            const QPointF tile(.18+.64*(i%columns+.5)/columns,
                               .5+metric(s,{0,(i/columns-.5)*.055}).y());
            center=mix(center,tile,assemble);
            rotation=rotation*(1-assemble)+(pi/4)*assemble;
            radius=radius*(1-assemble)+.014*assemble;
        }
        const int sides=random(seed,i+320)<.5?3:4;
        const auto shape=regular(s,center,radius,sides,rotation);
        builder.add(s,shape,ink(settings.palette,i%5==0?2:i%2,(.36+.24*pulse)*fade),i%4==0,.0034);
        if(overload && settings.glitch>0 && i%4==0 && builder.out.shapes.size()<maxShapes) {
            QPolygonF echo=shape; const auto offset=metric(s,{.008*std::sin(time*27+i),.004});
            for(auto &point:echo) point+=offset;
            builder.add(s,echo,ink(settings.palette,1,.28*fade*settings.glitch/100.0),false,.0017);
        }
    }
}
void field(Builder &builder,const Surface &s,const Settings &settings,double time,int budget,double alpha,bool focus) {
    if(budget<=0) return;
    const quint32 seed=idSeed(s.id,settings.seed);
    if(budget<7) {
        builder.add(s,regular(s,{.5,.5},.09*(1+.1*std::sin(time*.8)),6,pi/6),ink(settings.palette,0,alpha*.45),false,.002);
        if(budget>=4) trace(builder,s,settings,time,alpha*.64);
        return;
    }
    // A sparse breathing hex/triangle field with a shared travelling phase.
    const int cells=std::min(std::max(0,budget-7),std::clamp(settings.density,8,80));
    const int columns=std::max(2,int(std::ceil(std::sqrt(std::max(1,cells)*std::clamp(s.aspect,.35,3.0)))));
    const int rows=std::max(1,(cells+columns-1)/columns);
    for(int i=0;i<cells;++i) {
        const int row=i/columns,column=i%columns;
        const double x=(column+.55+(row%2)*.23)/(columns+.4);
        const double y=(row+.6)/(rows+.2);
        const QPointF center(unit(x),unit(y));
        const double wave=.5+.5*std::sin(time*.83-center.x()*5-center.y()*4+random(seed,i+400)*.6);
        const double radius=std::min(.052,.28/std::max(columns,rows))*(.7+.26*wave);
        const int sides=i%5==0?3:6;
        builder.add(s,regular(s,center,radius,sides,pi/6+std::sin(time*.22+i)*.08),
                    ink(settings.palette,i%9==0?2:i%2,alpha*(.15+.45*wave)),false,.0016+.0005*wave);
    }
    if(budget>=4) trace(builder,s,settings,time,alpha*.64);
    if(focus && budget>=7) {
        const double scan=fract(time*.075);
        const double band=.018;
        builder.add(s,{{.07,scan-band},{.93,scan-band},{.93,scan+band},{.07,scan+band}},ink(settings.palette,0,.11*alpha),true,.0014);
        builder.add(s,{{.08,scan},{.92,scan}},ink(settings.palette,0,.65*alpha),false,.002);
        const double sweep=fract(time*.045+.38);
        builder.add(s,{{sweep,.08},{sweep,.92}},ink(settings.palette,1,.23*alpha),false,.0014);
    }
}
}

Frame frame(const Settings &settings,const QVector<Surface> &input,double elapsed) {
    Frame out;
    if(settings.effect<1 || settings.effect>2 || input.isEmpty()) return out;
    const double time=std::isfinite(elapsed)?std::max(0.0,elapsed):0.0;
    QVector<Surface> surfaces;
    for(const auto &s:input) if(!s.id.isEmpty()) {
        auto sanitized=s;
        sanitized.aspect=std::isfinite(s.aspect)?std::clamp(s.aspect,.1,10.0):1.0;
        surfaces.append(std::move(sanitized));
    }
    std::sort(surfaces.begin(),surfaces.end(),[](const Surface &a,const Surface &b){return a.id<b.id;});
    surfaces.erase(std::unique(surfaces.begin(),surfaces.end(),[](const Surface &a,const Surface &b){return a.id==b.id;}),surfaces.end());
    // The practical mapper has small groups. Limit exceptional/malformed input
    // so route search and the minimum per-surface geometry are also bounded.
    if(surfaces.size()>128) surfaces.resize(128);
    if(surfaces.isEmpty()) return out;
    const int count=int(surfaces.size());
    Builder builder{out};
    if(settings.effect==2) {
        out.stage="Focus";
        const int budget=maxShapes/count;
        for(const auto &s:surfaces) field(builder,s,settings,time,budget,1,true);
        return out;
    }
    const auto &stops=cachedTour(surfaces,settings.seed);
    const auto times=timeline(settings,count);
    const bool explore=time<times.exploreEnd;
    const bool overload=!explore && time<times.overloadEnd;
    const bool reboot=!explore && !overload && time<times.titleEnd;
    out.stage=explore?"Explore":overload?"Overload":reboot?"Reboot":"Ambient";
    const double slot=times.exploreEnd/count;
    const int active=std::min(count-1,int(time/slot));
    const int surfaceBudget=(maxShapes-40)/count;
    for(int i=0;i<count;++i) {
        const auto &stop=stops[i];const auto &s=surfaces[stop.index];
        const double progress=explore?unit((time-i*slot)/slot):1;
        const double age=std::max(0.0,time-(i+1)*slot);
        const double fade=reboot?.32+.25*(1-smooth((time-times.overloadEnd)/3.0)):1;
        int fragmentBudget=surfaceBudget;
        if(explore && surfaceBudget>=4) fragmentBudget-=3;
        if(!explore && !overload && !reboot) fragmentBudget=std::max(2,surfaceBudget/3);
        // Reserve chromatic duplicate slots during overload.
        if(overload) fragmentBudget=fragmentBudget*4/5;
        const double rebootProgress=reboot?(time-times.overloadEnd)/(times.titleEnd-times.overloadEnd):-1;
        fragments(builder,s,stop,settings,progress,age,time,fragmentBudget,overload,fade,rebootProgress);
        if(explore && progress>0 && surfaceBudget>=4) trace(builder,s,settings,time,.2*progress);
        if(!explore && !overload && !reboot) field(builder,s,settings,time,surfaceBudget-fragmentBudget,.55,false);
    }
    if(explore) {
        const double progress=unit((time-active*slot)/slot);
        const auto &stop=stops[active];
        const double fade=smooth(progress/.07)*(1-smooth((progress-.94)/.06));
        snake(builder,surfaces[stop.index],stop,settings,progress,fade);
    } else if(reboot) {
        const double t=(time-times.overloadEnd)/(times.titleEnd-times.overloadEnd);
        out.titleOpacity=smooth((t-.12)/.3)*(1-smooth((t-.72)/.28));
    } else if(!overload) {
        const double ambient=time-times.titleEnd;
        const double cycle=std::max(18.0,count*3.1+5.0);
        const double visit=std::fmod(ambient,cycle);
        const int current=int(visit/3.1);
        if(current<count) {
            const double progress=fract(visit/3.1);
            const auto &stop=stops[current];
            const double fade=smooth(progress/.09)*(1-smooth((progress-.9)/.1));
            snake(builder,surfaces[stop.index],stop,settings,progress,.72*fade);
        }
    }
    return out;
}
}
