#include "dynamic.h"
#include "dynamic-geometry.h"
#include <QCoreApplication>
#include <QJsonArray>
#include <QSet>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <utility>

using namespace PotatoDynamic;
static void check(bool value, const char *label) {
    if (!value) throw std::runtime_error(label);
    std::cout << "PASS " << label << '\n';
}
static QVector<Surface> group() {
    return {{"A", {{.08,.12},{.31,.09},{.34,.49},{.09,.47}}, .62},
            {"B", {{.48,.1},{.76,.27},{.66,.49},{.39,.33}}, 1.7},
            {"C", {{.68,.61},{.95,.59},{.97,.93},{.7,.96}}, 1.35}};
}
// Legacy route fixtures use normalized positions; the generator's boundary
// contract is output pixels, so convert explicitly at the test boundary.
static Frame render(const Settings &s,QVector<Surface> surfaces,double time) {
    for(auto &surface:surfaces)for(auto &p:surface.boundary)p*=1080;
    return PotatoDynamic::frame(s,surfaces,time);
}
static double crossing(const Settings &s,const QVector<Surface> &surfaces,const QString &next) {
    const double end=timeline(s,int(surfaces.size())).exploreEnd;
    double low=0,high=0;
    for(int i=1;i<1000;++i){const double t=end*i/1000;const auto f=render(s,surfaces,t);
        if(!f.shapes.isEmpty()&&f.shapes.back().surfaceId==next&&std::abs(f.shapes.back().width-.0055)<1e-9){high=t;break;}low=t;}
    for(int i=0;i<25;++i){const double t=(low+high)/2;const auto f=render(s,surfaces,t);
        if(!f.shapes.isEmpty()&&f.shapes.back().surfaceId==next)high=t;else low=t;}
    return (low+high)/2;
}
static bool same(const Frame &a, const Frame &b) {
    if(a.stage != b.stage || a.titleOpacity != b.titleOpacity || a.shapes.size() != b.shapes.size()) return false;
    for(qsizetype i=0; i<a.shapes.size(); ++i) {
        const auto &x=a.shapes[i]; const auto &y=b.shapes[i];
        if(x.surfaceId!=y.surfaceId || x.points!=y.points || x.color!=y.color || x.filled!=y.filled || x.width!=y.width || x.closed!=y.closed || x.outputWidth!=y.outputWidth) return false;
    }
    return true;
}
static bool bounded(const Frame &f, const QVector<Surface> &surfaces) {
    if(f.shapes.size()>500 || !std::isfinite(f.titleOpacity) || f.titleOpacity<0 || f.titleOpacity>1) return false;
    QSet<QString> ids; for(const auto &s:surfaces) ids.insert(s.id);
    for(const auto &p:f.shapes) {
        if(!ids.contains(p.surfaceId) || p.points.size()<2 || !p.color.isValid() || !std::isfinite(p.width) || p.width<=0 || p.width>.05) return false;
        for(const auto &v:p.points) if(!std::isfinite(v.x()) || !std::isfinite(v.y()) || v.x()<(p.outputWidth>0?-5:0) || v.x()>(p.outputWidth>0?5:1) || v.y()<(p.outputWidth>0?-5:0) || v.y()>(p.outputWidth>0?5:1)) return false;
    }
    return true;
}
static QPointF head(const Frame &f) {
    const auto &p=f.shapes.back(); QPointF center;
    for(const auto &point:p.points) center+=point;
    return center/double(p.points.size());
}
static double mappedHeadRadius(const Frame &f,const Surface &surface) {
    QTransform projection;
    QTransform::quadToQuad({{0,0},{1,0},{1,1},{0,1}},surface.boundary,projection);
    QPolygonF points;QPointF center;
    for(const auto &p:f.shapes.back().points){points<<projection.map(p);center+=points.back();}
    center/=double(points.size());double radius=0;
    for(const auto &p:points)radius=std::max(radius,QLineF(center,p).length());
    return radius;
}
int main(int argc, char **argv) {
    QCoreApplication app(argc,argv);
    try {
        Settings settings; settings.effect=1; settings.members={"A","B","C"};
        check(Settings::fromJson(settings.json())==settings,"settings round trip including seed and membership");
        const Surface small{"A",{{0,0},{200,0},{200,200},{0,200}},1};
        const Surface big{"A",{{0,0},{800,0},{800,800},{0,800}},1};
        const double smallRadius=mappedHeadRadius(PotatoDynamic::frame(settings,{small},5.5),small);
        const double bigRadius=mappedHeadRadius(PotatoDynamic::frame(settings,{big},5.5),big);
        check(smallRadius>8 && std::abs(bigRadius/smallRadius-1)<.1,
              "snake head keeps output-pixel size on panels with fourfold different dimensions");
        const auto sizes=Settings::fromJson({{"snakeWidth",44},{"cellSize",72}}).json();
        check(sizes.value("snakeWidth").toInt()==44 && sizes.value("cellSize").toInt()==72,
              "output-pixel controls survive settings round trip");
        const auto countCells=[](const Frame &f){int count=0;for(const auto &p:f.shapes)if(p.points.size()==4&&std::abs(p.width-.0016)<1e-9&&p.outputWidth>0)++count;return count;};
        check(countCells(PotatoDynamic::frame(settings,{big},1))>countCells(PotatoDynamic::frame(settings,{small},1)),
              "smaller panels contain fewer cells instead of squeezing all cells");
        QJsonObject malformed{{"effect",99},{"speed",-20},{"density",10000},{"palette",90},{"glitch",-1},
            {"members",QJsonArray{"A","A","",4}},{"seed",4294967295.0}};
        const auto normalized=Settings::fromJson(malformed);
        check(normalized.effect==0 && normalized.speed>=10 && normalized.density<=80 && normalized.palette<=5 && normalized.glitch==0 && normalized.members==QStringList{"A"} && normalized.seed==0xffffffffu,"malformed settings normalized safely");
        auto surfaces=group();
        check(render(settings,{},4).shapes.isEmpty() && render(settings,{},4).titleOpacity==0,"empty group produces no geometry or reboot title");
        auto off=settings; off.effect=0;
        check(render(off,surfaces,10).shapes.isEmpty(),"disabled effect produces no geometry");
        const auto times=timeline(settings,surfaces.size());
        check(times.exploreEnd<times.overloadEnd && times.overloadEnd<times.titleEnd && times.titleEnd>=15 && times.titleEnd<=30,"introduction has bounded ordered stages");
        check(render(settings,surfaces,2).stage=="Explore" && render(settings,surfaces,times.exploreEnd+.5).stage=="Overload" && render(settings,surfaces,times.overloadEnd+1).stage=="Reboot" && render(settings,surfaces,times.titleEnd+1).stage=="Ambient","snake progresses through all four stages");
        check(render(settings,surfaces,times.overloadEnd+1.5).titleOpacity>.5 && render(settings,surfaces,times.titleEnd+1).titleOpacity==0,"reboot title is brief and fades away");
        check(ambientStart(settings,3)==times.titleEnd,"skip target matches ambient stage");
        const auto assembly=render(settings,surfaces,times.overloadEnd+1.5);
        bool tilesAssembled=!assembly.shapes.isEmpty();
        for(const auto &p:assembly.shapes) {
            QPointF center; for(const auto &point:p.points) center+=point;
            center/=double(p.points.size()); tilesAssembled &= std::abs(center.y()-.5)<.04;
        }
        check(tilesAssembled,"reboot fragments assemble into central title bands");
        bool geometryValid=true;
        for(int i=0;i<160;++i) geometryValid &= bounded(render(settings,surfaces,i*.47),surfaces);
        auto single=QVector<Surface>{surfaces[1]};
        for(int i=0;i<70;++i) geometryValid &= bounded(render(settings,single,i*.43),single);
        check(geometryValid,"single and transformed group geometry stays finite and bounded across stages");
        check(same(render(settings,surfaces,4.2),render(settings,surfaces,4.2)),"same seed and clock reproduce exact geometry");
        auto changed=settings; changed.seed+=17;
        check(!same(render(settings,surfaces,4.2),render(changed,surfaces,4.2)),"seed changes procedural geometry");
        auto reordered=surfaces; std::swap(reordered[0],reordered[2]);
        check(same(render(settings,surfaces,8.4),render(settings,reordered,8.4)),"surface list order does not change deterministic tour");
        QVector<Surface> routeSurfaces{{"A",{{.1,.1},{.3,.1},{.3,.3},{.1,.3}},1},
            {"B",{{.85,.8},{.95,.8},{.95,.95},{.85,.95}},1},
            {"C",{{.35,.1},{.55,.1},{.55,.3},{.35,.3}},1}};
        const double routeSlot=crossing(settings,routeSurfaces,"C");
        const auto secondStop=render(settings,routeSurfaces,routeSlot+.01);
        check(!secondStop.shapes.isEmpty() && secondStop.shapes.back().surfaceId=="C","tour visits nearest unvisited surface before farther member");
        const auto exitFrame=render(settings,routeSurfaces,routeSlot-.03);
        QPointF exitCenter;for(const auto &point:exitFrame.shapes.back().points)exitCenter+=point;
        exitCenter/=double(exitFrame.shapes.back().points.size());
        check(exitCenter.x()>.85 && std::abs(exitCenter.y()-.5)<.08,"equal-distance openings use the middle of the facing edge");
        const QVector<Surface> connected{{"A",{{.05,.1},{.5,.1},{.5,.7},{.05,.7}},.75},
            {"B",{{.5,.3},{.95,.3},{.95,.9},{.5,.9}},.75}};
        const double connectedSlot=crossing(settings,connected,"B");
        const auto leaving=render(settings,connected,connectedSlot*.99);
        const auto entering=render(settings,connected,connectedSlot*1.01);
        const auto outputHead=[](const Frame &f,const QVector<Surface> &surfaces){const auto &shape=f.shapes.back();
            for(const auto &s:surfaces)if(s.id==shape.surfaceId){QTransform t;QTransform::quadToQuad({{0,0},{1,0},{1,1},{0,1}},s.boundary,t);return t.map(head(f));}return QPointF();};
        constexpr double dt=.002;
        const auto h0=outputHead(render(settings,connected,connectedSlot-2*dt),connected);
        const auto h1=outputHead(render(settings,connected,connectedSlot-dt),connected);
        const auto h2=outputHead(render(settings,connected,connectedSlot+dt),connected);
        const auto h3=outputHead(render(settings,connected,connectedSlot+2*dt),connected);
        const QPointF in=h1-h0,out=h3-h2;
        check((in.x()*out.x()+in.y()*out.y())/(QLineF({},in).length()*QLineF({},out).length())>.995&&
              std::abs(QLineF({},in).length()/QLineF({},out).length()-1)<.08,
              "touching portal preserves mapped tangent and speed without a panel pause");
        check(std::abs(head(leaving).y()-2.0/3)<.07 && std::abs(head(entering).y()-1.0/3)<.07,
            "partial facing overlap uses its shared midpoint on both surfaces");
        QSet<QString> bodySurfaces;
        for(const auto &p:entering.shapes) if(p.filled && std::abs(p.width-.0045)<1e-9) bodySurfaces.insert(p.surfaceId);
        check(bodySurfaces==QSet<QString>{"A","B"},"snake body continues across both sides of a surface handoff");
        check(entering.shapes.back().color.alphaF()>.8 && leaving.shapes.back().color.alphaF()>.8,
            "connected handoff keeps the head visible instead of fading each panel");
        auto vertical=connected;
        for(auto &s:vertical) for(auto &p:s.boundary) std::swap(p.rx(),p.ry());
        check(std::abs(head(render(settings,vertical,crossing(settings,vertical,"B")*.99)).y()-2.0/3)<.07,
            "rotated and reversed-winding surfaces keep the centred connection");
        const QVector<Surface> overlap{{"A",{{0,0},{.6,0},{.6,.6},{0,.6}},1},
            {"B",{{.5,.2},{1,.2},{1,.8},{.5,.8}},1}};
        const auto overlapExit=head(render(settings,overlap,crossing(settings,overlap,"B")-.001));
        const auto overlapEntry=head(render(settings,overlap,crossing(settings,overlap,"B")+.001));
        check(std::abs(.6*overlapExit.x()-(.5+.5*overlapEntry.x()))<.015 &&
            std::abs(.6*overlapExit.y()-(.2+.6*overlapEntry.y()))<.015,
            "overlapping surfaces hand off at one mapped point inside their overlap");
        const QPolygonF slanted{{1.4169872981,1.0969872981},{2.2830127019,1.5969872981},
            {1.7830127019,2.4630127019},{.9169872981,1.9630127019}};
        QTransform tilt;check(QTransform::quadToQuad({{0,0},{1,0},{1,1},{0,1}},slanted,tilt),"oblique portal fixture has an invertible mapping");
        const QPointF entryNearCorner(0,.00737205584),opening=tilt.map(entryNearCorner);
        const QPointF heading(std::cos(15*3.141592653589793/180),std::sin(15*3.141592653589793/180));
        const auto bend=potatoPortalBend(tilt.inverted(),opening,entryNearCorner,heading);
        const auto mappedRay=tilt.map(bend)-opening;
        check(bend.x()>=0&&bend.x()<=1&&bend.y()>=0&&bend.y()<=1 &&
            (mappedRay.x()*heading.x()+mappedRay.y()*heading.y())/std::hypot(mappedRay.x(),mappedRay.y())>.99999,
            "near-corner oblique portal preserves the shared mapped heading when bounded");
        const auto explored=render(settings,surfaces,times.exploreEnd-.01);
        QSet<QString> visited; for(const auto &p:explored.shapes) visited.insert(p.surfaceId);
        check(visited.size()==3,"exploration leaves fragments on each group member");
        Settings focus=settings; focus.effect=2;
        Settings mint=focus; mint.palette=5;
        check(Settings::fromJson(mint.json())==mint,"Mint palette survives project settings round trip");
        QSet<QRgb> paletteColors;
        for(int palette=0;palette<6;++palette) {
            auto variant=focus; variant.palette=palette;
            const auto colored=render(variant,surfaces,8.4);
            if(!colored.shapes.isEmpty()) paletteColors.insert(colored.shapes.front().color.rgb());
        }
        check(paletteColors.size()==6,"all six UI palettes render distinct colors");
        const auto mintFrame=render(mint,surfaces,8.4);
        bool mintColors=!mintFrame.shapes.isEmpty();
        for(const auto &primitive:mintFrame.shapes) mintColors &= primitive.color.green()>primitive.color.red();
        check(mintColors,"Mint palette uses green light throughout its geometry");
        const auto hologram=render(focus,surfaces,8.4);
        QSet<QString> illuminated; for(const auto &p:hologram.shapes) illuminated.insert(p.surfaceId);
        check(hologram.stage=="Focus" && illuminated.size()==3 && ambientStart(focus,3)==0 && bounded(hologram,surfaces),"Focus animates every transformed surface immediately");
        check(!same(hologram,render(focus,surfaces,8.9)),"Focus scans and geometry move with group clock");
        auto large=surfaces;
        for(int i=0;i<90;++i) {auto s=surfaces[i%3];s.id=QString::number(i);large.append(s);}
        focus.density=80;
        check(bounded(render(focus,large,123.4),large) && bounded(render(settings,large,24.5),large),"large groups retain fixed primitive budget");
        check(bounded(render(settings,surfaces,std::numeric_limits<double>::infinity()),surfaces),"non-finite elapsed time is sanitized");
    } catch(const std::exception &e) {std::cerr<<"FAIL "<<e.what()<<'\n';return 1;}
    return 0;
}
