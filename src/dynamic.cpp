#include "dynamic.h"
#include "dynamic-geometry.h"
#include "dynamic-metric.h"
#include <QJsonArray>
#include <QSet>
#include <QTransform>
#include <algorithm>
#include <cmath>
#include <limits>

namespace PotatoDynamic {
bool Settings::operator==(const Settings &o) const {
    return effect==o.effect && speed==o.speed && density==o.density && palette==o.palette
        && glitch==o.glitch && snakeWidth==o.snakeWidth && cellSize==o.cellSize
        && playing==o.playing && seed==o.seed && members==o.members;
}
QJsonObject Settings::json() const {
    QJsonArray ids; for(const auto &id:members) ids.append(id);
    return {{"effect",effect},{"speed",speed},{"density",density},{"palette",palette},
            {"glitch",glitch},{"snakeWidth",snakeWidth},{"cellSize",cellSize},
            {"playing",playing},{"seed",double(seed)},{"members",ids}};
}
Settings Settings::fromJson(const QJsonObject &o) {
    Settings s;
    s.effect=o.value("effect").toInt(0); if(s.effect<0 || s.effect>2) s.effect=0;
    s.speed=std::clamp(o.value("speed").toInt(100),10,300);
    s.density=std::clamp(o.value("density").toInt(36),8,80);
    s.palette=std::clamp(o.value("palette").toInt(0),0,5);
    s.glitch=std::clamp(o.value("glitch").toInt(35),0,100);
    s.snakeWidth=std::clamp(o.value("snakeWidth").toInt(40),8,96);
    s.cellSize=std::clamp(o.value("cellSize").toInt(56),24,160);
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
double dot(const QPointF &a,const QPointF &b) {return a.x()*b.x()+a.y()*b.y();}
QPointF normalized(const QPointF &v) {const double n=std::sqrt(lengthSquared(v));return n>1e-12?v/n:QPointF();}
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
    void add(const Surface &s,QPolygonF points,QColor color,bool filled=false,double width=.002,bool closed=true,double outputWidth=0) {
        if(out.shapes.size()>=maxShapes || points.size()<2 || color.alpha()==0) return;
        for(auto &p:points) {
            if(!std::isfinite(p.x()) || !std::isfinite(p.y())) return;
            if(outputWidth<=0)p=clipped(p);
            else if(std::abs(p.x())>5||std::abs(p.y())>5)return;
        }
        const bool closePolygon=closed && points.size()>2;
        out.shapes.append({s.id,std::move(points),color,filled,width,closePolygon,outputWidth});
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
struct Stop {
    int index=0;
    QPointF entry{0,.5},exit{1,.5};
    QPointF entryBend{-1,-1},exitBend{-1,-1};
};
struct Portal {QPointF exit,entry,direction;bool found=false;};
Portal widestPortal(const QPolygonF &a,const QPolygonF &b) {
    QPointF centerA,centerB;
    for(const auto &p:a) centerA+=p/4;
    for(const auto &p:b) centerB+=p/4;
    Portal best;double widest=0,shortest=std::numeric_limits<double>::max();
    for(int i=0;i<4;++i) for(int j=0;j<4;++j) {
        const QPointF a0=a[i],a1=a[(i+1)%4],b0=b[j],b1=b[(j+1)%4];
        const QPointF edgeA=normalized(a1-a0),edgeB=normalized(b1-b0);
        if(lengthSquared(edgeA)<.5 || lengthSquared(edgeB)<.5 || std::abs(dot(edgeA,edgeB))<.5) continue;
        QPointF normalA(edgeA.y(),-edgeA.x()),normalB(edgeB.y(),-edgeB.x());
        if(dot(normalA,centerA-(a0+a1)/2)>0) normalA=-normalA;
        if(dot(normalB,centerB-(b0+b1)/2)>0) normalB=-normalB;
        if(dot(normalA,normalB)>-.35 || dot(normalA,centerB-centerA)<=1e-12 || dot(normalB,centerA-centerB)<=1e-12) continue;
        const QPointF tangent=normalized(edgeA+edgeB*(dot(edgeA,edgeB)<0?-1:1));
        const double pa0=dot(a0,tangent),pa1=dot(a1,tangent),pb0=dot(b0,tangent),pb1=dot(b1,tangent);
        const double low=std::max(std::min(pa0,pa1),std::min(pb0,pb1));
        const double high=std::min(std::max(pa0,pa1),std::max(pb0,pb1));
        const double width=high-low;
        if(width<=1e-9) continue; // A corner-only neighbour has no wide opening.
        const double middle=(low+high)/2;
        const QPointF exit=mix(a0,a1,(middle-pa0)/(pa1-pa0));
        const QPointF entry=mix(b0,b1,(middle-pb0)/(pb1-pb0));
        const double distance=lengthSquared(entry-exit);
        if(width>widest+1e-12 || (std::abs(width-widest)<=1e-12 && distance<shortest)) {
            widest=width;shortest=distance;best={exit,entry,normalized(normalA-normalB),true};
        }
    }
    return best;
}
QVector<Stop> tour(const QVector<Surface> &surfaces,quint32 seed) {
    const int count=int(surfaces.size());
    QVector<Stop> result;if(!count) return result;
    QVector<QVector<QPointF>> mapped(count);
    QVector<QTransform> inverses(count);
    QVector<QPolygonF> boundaries(count);
    for(int i=0;i<count;++i) {
        QTransform transform;
        const QPolygonF square{{0,0},{1,0},{1,1},{0,1}};
        const bool projective=surfaces[i].boundary.size()==4 && QTransform::quadToQuad(square,surfaces[i].boundary,transform);
        if(!projective) transform=QTransform();
        inverses[i]=transform.inverted();
        for(const auto &corner:square) boundaries[i]<<transform.map(corner);
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
        QPointF entry=perimeter(double(entryIndex)/boundarySamples),entryBend(-1,-1);
        if(next>=0) {
            const auto portal=widestPortal(boundaries[current],boundaries[next]);
            if(portal.found) {
                QPointF exitMapped=portal.exit,entryMapped=portal.entry;
                const QPointF middle=(exitMapped+entryMapped)/2;
                const QPointF localA=inverses[current].map(middle),localB=inverses[next].map(middle);
                // Overlapping panels share a single point inside their overlap.
                const auto inside=[](const QPointF &p){return p.x()>=0 && p.x()<=1 && p.y()>=0 && p.y()<=1;};
                if(inside(localA)&&inside(localB)) exitMapped=entryMapped=middle;
                active.exit=clipped(inverses[current].map(exitMapped));entry=clipped(inverses[next].map(entryMapped));
                active.exitBend=potatoPortalBend(inverses[current],exitMapped,active.exit,-portal.direction);
                entryBend=potatoPortalBend(inverses[next],entryMapped,entry,portal.direction);
            }
        }
        result.append(active);
        if(next<0) break;
        current=next;active={current,entry,{1,.5},entryBend,{-1,-1}};
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
    // Linked openings use a common mapped heading; unlinked endpoints retain
    // seeded interior bends. The lateral wave has zero endpoint derivative.
    const QPointF a=stop.entryBend.x()>=0?stop.entryBend:QPointF(.28+.18*random(seed,2),.22+.28*random(seed,3));
    const QPointF b=stop.exitBend.x()>=0?stop.exitBend:QPointF(.52+.2*random(seed,4),.5+.26*random(seed,5));
    const double q=1-t;
    QPointF p=entry*(q*q*q)+a*(3*q*q*t)+b*(3*q*t*t)+exit*(t*t*t);
    const double envelope=std::sin(pi*t);
    const double wave=envelope*envelope*std::sin(3*pi*t+random(seed,6)*2*pi)*.075;
    p+=QPointF(wave,-wave*.7);
    return clipped(p);
}
struct Motion {
    QVector<Stop> stops;
    QVector<QVector<double>> lengths;
    QVector<QVector<QPointF>> pixels;
    QVector<double> offsets;
    double total=0;
};
const Motion &cachedMotion(const QVector<Surface> &surfaces,quint32 seed) {
    struct Cache {QVector<Surface> surfaces;quint32 seed=0;Motion motion;};
    static thread_local Cache cache;
    bool same=cache.seed==seed&&cache.surfaces.size()==surfaces.size();
    for(qsizetype i=0;same&&i<surfaces.size();++i){const auto &a=cache.surfaces[i],&b=surfaces[i];
        same=a.id==b.id&&a.boundary==b.boundary&&a.projection==b.projection&&a.cells==b.cells&&a.mesh==b.mesh;}
    if(same)return cache.motion;
    cache.surfaces=surfaces;cache.seed=seed;auto &m=cache.motion;m={};m.stops=cachedTour(surfaces,seed);
    for(const auto &stop:m.stops){
        const auto &s=surfaces[stop.index];const auto routeSeed=idSeed(s.id,seed);
        QVector<double> lengths{0};QVector<QPointF> pixels{mappedPoint(s,path(stop,0,routeSeed))};
        for(int i=1;i<=96;++i){pixels<<mappedPoint(s,path(stop,double(i)/96,routeSeed));lengths<<lengths.back()+std::sqrt(lengthSquared(pixels.back()-pixels[pixels.size()-2]));}
        m.offsets<<m.total;m.total+=std::max(.001,lengths.back());m.lengths<<lengths;m.pixels<<pixels;
    }
    m.offsets<<m.total;return m;
}
// Quintic acceleration only at the beginning/end of the whole tour; no pause
// or change of speed at a panel boundary. Arc length accounts for actual mesh.
double travel(double t){t=unit(t);return t*t*t*(10+t*(-15+6*t));}
double travelTime(double distance,double total,double duration){
    double low=0,high=1;for(int i=0;i<24;++i){const double middle=(low+high)/2;if(travel(middle)*total<distance)low=middle;else high=middle;}return (low+high)*.5*duration;
}
struct Pose {int stop=0;double t=0;QPointF uv,pixel,direction;};
Pose pose(const Motion &m,const QVector<Surface> &surfaces,quint32 seed,double distance,int forced=-1){
    const int index=forced>=0?forced:std::clamp(int(std::upper_bound(m.offsets.begin(),m.offsets.end(),distance)-m.offsets.begin())-1,0,int(m.stops.size())-1);
    const auto &lengths=m.lengths[index];const double d=std::clamp(distance-m.offsets[index],0.0,lengths.back());
    const int sample=std::clamp(int(std::upper_bound(lengths.begin(),lengths.end(),d)-lengths.begin())-1,0,int(lengths.size())-2);
    const double t=(sample+(d-lengths[sample])/std::max(1e-9,lengths[sample+1]-lengths[sample]))/96;
    const auto &stop=m.stops[index];const auto &s=surfaces[stop.index];const auto routeSeed=idSeed(s.id,seed);
    const auto uv=path(stop,t,routeSeed);const auto direction=normalized(mappedPoint(s,path(stop,t+.001,routeSeed))-mappedPoint(s,path(stop,t-.001,routeSeed)));
    return {index,t,uv,mappedPoint(s,uv),direction};
}
QPolygonF pixelPolygon(const Surface &s,QPointF center,double radius,int sides,double angle,double stretch=1){
    QPolygonF p;for(int i=0;i<sides;++i){const double a=2*pi*i/sides;const QPointF v(std::cos(a)*radius*stretch,std::sin(a)*radius/stretch);
        const QPointF rotated(v.x()*std::cos(angle)-v.y()*std::sin(angle),v.x()*std::sin(angle)+v.y()*std::cos(angle));p<<center+pixelOffset(s,center,rotated);}return p;
}
void snake(Builder &builder,const QVector<Surface> &surfaces,const Motion &m,const Settings &settings,double distance,double opacity){
    constexpr int segments=36;const double tailLength=settings.snakeWidth*7.5,step=tailLength/segments;
    auto ribbon=[&](double back,double front,double radiusBack,double radiusFront,int stop){
        if(front<=back||builder.out.shapes.size()>=maxShapes-3)return;const auto a=pose(m,surfaces,settings.seed,back,stop),b=pose(m,surfaces,settings.seed,front,stop);
        const auto &s=surfaces[m.stops[stop].index];
        const QPointF na=pixelOffset(s,a.uv,{-a.direction.y()*radiusBack,a.direction.x()*radiusBack});
        const QPointF nb=pixelOffset(s,b.uv,{-b.direction.y()*radiusFront,b.direction.x()*radiusFront});
        builder.add(s,{a.uv+na,b.uv+nb,b.uv-nb,a.uv-na},ink(settings.palette,0,opacity*.32),true,.0045,true,.8);
    };
    for(int j=segments-1;j>=0;--j){
        const double front=distance-j*step,back=std::max(0.0,front-step);if(front<=0)continue;
        const double rf=settings.snakeWidth*.5*(.09+.91*std::pow(1-double(j)/segments,.85));
        const double rb=settings.snakeWidth*.5*(.09+.91*std::pow(1-double(j+1)/segments,.85));
        const auto a=pose(m,surfaces,settings.seed,back),b=pose(m,surfaces,settings.seed,front);
        if(a.stop==b.stop)ribbon(back,front,rb,rf,a.stop);
        else for(int stop=a.stop;stop<=b.stop;++stop){const double lo=std::max(back,m.offsets[stop]),hi=std::min(front,m.offsets[stop+1]);
            const double f0=(lo-back)/step,f1=(hi-back)/step;ribbon(lo,hi,rb+(rf-rb)*f0,rb+(rf-rb)*f1,stop);}
        if(j%6==2&&builder.out.shapes.size()<maxShapes-3){const auto &s=surfaces[m.stops[b.stop].index];builder.add(s,pixelPolygon(s,b.uv,rf*.38,3,std::atan2(b.direction.y(),b.direction.x())),ink(settings.palette,2,opacity*.48),false,.0018,true,.8);}
    }
    const auto head=pose(m,surfaces,settings.seed,distance);const auto &s=surfaces[m.stops[head.stop].index];
    const double angle=std::atan2(head.direction.y(),head.direction.x());
    const auto polygon=pixelPolygon(s,head.uv,settings.snakeWidth*.54,3,angle);
    // Duplicate only the portion crossing a touching/overlapping opening.
    // UV clipping in the renderer leaves real physical gaps completely dark.
    for(int neighbour:{head.stop-1,head.stop+1})if(neighbour>=0&&neighbour<m.stops.size()){
        const int edge=std::max(neighbour,head.stop);if(std::abs(distance-m.offsets[edge])>settings.snakeWidth*1.4)continue;
        const auto &other=surfaces[m.stops[neighbour].index];
        const auto portal=neighbour<head.stop?m.stops[neighbour].exit:m.stops[neighbour].entry;
        const auto currentPortal=neighbour<head.stop?m.stops[head.stop].entry:m.stops[head.stop].exit;
        if(std::sqrt(lengthSquared(mappedPoint(other,portal)-mappedPoint(s,currentPortal)))>.75)continue;
        QPolygonF copy;for(int i=0;i<polygon.size();++i){const double a=angle+2*pi*i/3;
            const QPointF pixel=head.pixel+QPointF(std::cos(a),std::sin(a))*settings.snakeWidth*.54;
            copy<<portal+pixelOffset(other,portal,pixel-mappedPoint(other,portal));}
        builder.add(other,copy,ink(settings.palette,0,opacity*.92),false,.0055,true,1.6);
    }
    builder.add(s,polygon,ink(settings.palette,0,opacity*.92),false,.0055,true,1.6);
}
struct Cell {QPointF uv;double distance=0,lateral=0;};
const QVector<QVector<Cell>> &cellLayout(const QVector<Surface> &surfaces,const Motion &m,const Settings &settings,int budget){
    struct Cache {QVector<Surface> surfaces;int size=0,density=0,budget=0;quint32 seed=0;QVector<QVector<Cell>> cells;};static thread_local Cache cache;
    bool same=cache.size==settings.cellSize&&cache.density==settings.density&&cache.budget==budget&&cache.seed==settings.seed&&cache.surfaces.size()==surfaces.size();
    for(qsizetype i=0;same&&i<surfaces.size();++i){const auto &a=cache.surfaces[i],&b=surfaces[i];same=a.id==b.id&&a.boundary==b.boundary&&a.projection==b.projection&&a.cells==b.cells&&a.mesh==b.mesh;}
    if(same)return cache.cells;
    cache.surfaces=surfaces;cache.size=settings.cellSize;cache.density=settings.density;cache.budget=budget;cache.seed=settings.seed;cache.cells.clear();
    const double spacing=settings.cellSize*1.8;
    for(int stop=0;stop<m.stops.size();++stop){const auto &s=surfaces[m.stops[stop].index];QVector<Cell> candidates;
        const auto rect=s.boundary.boundingRect();const int columns=std::clamp(int(rect.width()/spacing),1,128),rows=std::clamp(int(rect.height()/spacing),1,128);
        const QPointF origin=rect.center()-QPointF((columns-1)*spacing/2,(rows-1)*spacing/2);
        for(int row=0;row<rows;++row)for(int col=0;col<columns;++col){const QPointF pixel=origin+QPointF(col*spacing,row*spacing);QPointF uv;
            if(!localPoint(s,pixel,uv))continue;
            Cell cell{uv,0,std::numeric_limits<double>::max()};
            for(int k=0;k<m.pixels[stop].size();++k){const double lateral=std::sqrt(lengthSquared(pixel-m.pixels[stop][k]));if(lateral<cell.lateral){cell.lateral=lateral;cell.distance=m.offsets[stop]+m.lengths[stop][k];}}
            candidates<<cell;
        }
        QVector<Cell> cells;const int limit=std::min({int(candidates.size()),settings.density,std::max(0,budget)});
        for(int i=0;i<limit;++i)cells<<candidates[int((i+.5)*candidates.size()/limit)];cache.cells<<cells;
    }
    return cache.cells;
}
void reactiveCells(Builder &builder,const Surface &s,const QVector<Cell> &cells,const Settings &settings,const Motion &m,double phaseTime,double duration,double time,double alpha){
    for(int i=0;i<cells.size();++i){const auto &cell=cells[i];
        const double age=phaseTime-travelTime(cell.distance,m.total,duration)-cell.lateral/220-.08;
        const double response=age>=0&&age<3?smooth(age/.45)*(1-smooth((age-.9)/1.7))*std::exp(-age*.35)*std::exp(-cell.lateral/(settings.cellSize*3.0)):0;
        const double angle=pi/4+response*.68+std::sin(time*.36+cell.uv.x()*4+cell.uv.y()*3)*.025;
        builder.add(s,pixelPolygon(s,cell.uv,settings.cellSize/std::sqrt(2.0),4,angle,1+response*.75),
                    ink(settings.palette,i%11==0?1:0,alpha*(.17+response*.48)),false,.0016,true,1.0+response*.6);
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
        const auto shape=settings.effect==1?pixelPolygon(s,clipped(center),(5+5*random(seed,i+280))*(.88+.17*pulse),sides,rotation):regular(s,center,radius,sides,rotation);
        builder.add(s,shape,ink(settings.palette,i%5==0?2:i%2,(.36+.24*pulse)*fade),i%4==0,.0034,true,settings.effect==1?.85:0);
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
        if(sanitized.mesh.isEmpty())QTransform::quadToQuad({{0,0},{1,0},{1,1},{0,1}},sanitized.boundary,sanitized.projection);
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
    const auto &motion=cachedMotion(surfaces,settings.seed);const auto &stops=motion.stops;
    const auto times=timeline(settings,count);
    const bool explore=time<times.exploreEnd;
    const bool overload=!explore && time<times.overloadEnd;
    const bool reboot=!explore && !overload && time<times.titleEnd;
    out.stage=explore?"Explore":overload?"Overload":reboot?"Reboot":"Ambient";
    const double ambient=time-times.titleEnd;
    const double cycle=std::max(18.0,count*3.1+5.0),visit=ambient>=0?std::fmod(ambient,cycle):0;
    const double duration=explore?times.exploreEnd:count*3.1;
    const double phaseTime=explore?time:visit;
    const double distance=travel(phaseTime/duration)*motion.total;
    const auto active=pose(motion,surfaces,settings.seed,distance);
    const int surfaceBudget=(maxShapes-90)/count;
    const auto &cells=cellLayout(surfaces,motion,settings,std::max(0,surfaceBudget-20));
    for(int i=0;i<count;++i) {
        const auto &stop=stops[i];const auto &s=surfaces[stop.index];
        const double progress=explore?(i<active.stop?1:i==active.stop?active.t:0):1;
        const double age=std::max(0.0,time-travelTime(motion.offsets[i+1],motion.total,times.exploreEnd));
        const double fade=reboot?.32+.25*(1-smooth((time-times.overloadEnd)/3.0)):1;
        int fragmentBudget=std::min(12,surfaceBudget);
        if(explore && surfaceBudget>=4) fragmentBudget-=3;
        // Reserve chromatic duplicate slots during overload.
        if(overload) fragmentBudget=fragmentBudget*4/5;
        const double rebootProgress=reboot?(time-times.overloadEnd)/(times.titleEnd-times.overloadEnd):-1;
        if(explore||(!overload&&!reboot))reactiveCells(builder,s,cells[i],settings,motion,phaseTime,duration,time,explore?1:.8);
        fragments(builder,s,stop,settings,progress,age,time,fragmentBudget,overload,fade,rebootProgress);
        if(explore && progress>0 && surfaceBudget>=4) trace(builder,s,settings,time,.2*progress);
    }
    if(explore) {
        const double progress=unit(time/duration);
        const double fade=smooth(progress/.025)*(1-smooth((progress-.975)/.025));
        snake(builder,surfaces,motion,settings,distance,fade);
    } else if(reboot) {
        const double t=(time-times.overloadEnd)/(times.titleEnd-times.overloadEnd);
        out.titleOpacity=smooth((t-.12)/.3)*(1-smooth((t-.72)/.28));
    } else if(!overload) {
        if(visit<duration) {
            const double progress=unit(visit/duration);
            const double fade=smooth(progress/.025)*(1-smooth((progress-.975)/.025));
            snake(builder,surfaces,motion,settings,distance,.8*fade);
        }
    }
    return out;
}
}
