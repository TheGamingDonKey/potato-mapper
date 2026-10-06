#include "dynamic.h"
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
static bool same(const Frame &a, const Frame &b) {
    if(a.stage != b.stage || a.titleOpacity != b.titleOpacity || a.shapes.size() != b.shapes.size()) return false;
    for(qsizetype i=0; i<a.shapes.size(); ++i) {
        const auto &x=a.shapes[i]; const auto &y=b.shapes[i];
        if(x.surfaceId!=y.surfaceId || x.points!=y.points || x.color!=y.color || x.filled!=y.filled || x.width!=y.width || x.closed!=y.closed) return false;
    }
    return true;
}
static bool bounded(const Frame &f, const QVector<Surface> &surfaces) {
    if(f.shapes.size()>500 || !std::isfinite(f.titleOpacity) || f.titleOpacity<0 || f.titleOpacity>1) return false;
    QSet<QString> ids; for(const auto &s:surfaces) ids.insert(s.id);
    for(const auto &p:f.shapes) {
        if(!ids.contains(p.surfaceId) || p.points.size()<2 || !p.color.isValid() || !std::isfinite(p.width) || p.width<=0 || p.width>.05) return false;
        for(const auto &v:p.points) if(!std::isfinite(v.x()) || !std::isfinite(v.y()) || v.x()<0 || v.x()>1 || v.y()<0 || v.y()>1) return false;
    }
    return true;
}
int main(int argc, char **argv) {
    QCoreApplication app(argc,argv);
    try {
        Settings settings; settings.effect=1; settings.members={"A","B","C"};
        check(Settings::fromJson(settings.json())==settings,"settings round trip including seed and membership");
        QJsonObject malformed{{"effect",99},{"speed",-20},{"density",10000},{"palette",90},{"glitch",-1},
            {"members",QJsonArray{"A","A","",4}},{"seed",4294967295.0}};
        const auto normalized=Settings::fromJson(malformed);
        check(normalized.effect==0 && normalized.speed>=10 && normalized.density<=80 && normalized.palette<=5 && normalized.glitch==0 && normalized.members==QStringList{"A"} && normalized.seed==0xffffffffu,"malformed settings normalized safely");
        auto surfaces=group();
        check(frame(settings,{},4).shapes.isEmpty() && frame(settings,{},4).titleOpacity==0,"empty group produces no geometry or reboot title");
        auto off=settings; off.effect=0;
        check(frame(off,surfaces,10).shapes.isEmpty(),"disabled effect produces no geometry");
        const auto times=timeline(settings,surfaces.size());
        check(times.exploreEnd<times.overloadEnd && times.overloadEnd<times.titleEnd && times.titleEnd>=15 && times.titleEnd<=30,"introduction has bounded ordered stages");
        check(frame(settings,surfaces,2).stage=="Explore" && frame(settings,surfaces,times.exploreEnd+.5).stage=="Overload" && frame(settings,surfaces,times.overloadEnd+1).stage=="Reboot" && frame(settings,surfaces,times.titleEnd+1).stage=="Ambient","snake progresses through all four stages");
        check(frame(settings,surfaces,times.overloadEnd+1.5).titleOpacity>.5 && frame(settings,surfaces,times.titleEnd+1).titleOpacity==0,"reboot title is brief and fades away");
        check(ambientStart(settings,3)==times.titleEnd,"skip target matches ambient stage");
        const auto assembly=frame(settings,surfaces,times.overloadEnd+1.5);
        bool tilesAssembled=!assembly.shapes.isEmpty();
        for(const auto &p:assembly.shapes) {
            QPointF center; for(const auto &point:p.points) center+=point;
            center/=double(p.points.size()); tilesAssembled &= std::abs(center.y()-.5)<.04;
        }
        check(tilesAssembled,"reboot fragments assemble into central title bands");
        bool geometryValid=true;
        for(int i=0;i<160;++i) geometryValid &= bounded(frame(settings,surfaces,i*.47),surfaces);
        auto single=QVector<Surface>{surfaces[1]};
        for(int i=0;i<70;++i) geometryValid &= bounded(frame(settings,single,i*.43),single);
        check(geometryValid,"single and transformed group geometry stays finite and bounded across stages");
        check(same(frame(settings,surfaces,4.2),frame(settings,surfaces,4.2)),"same seed and clock reproduce exact geometry");
        auto changed=settings; changed.seed+=17;
        check(!same(frame(settings,surfaces,4.2),frame(changed,surfaces,4.2)),"seed changes procedural geometry");
        auto reordered=surfaces; std::swap(reordered[0],reordered[2]);
        check(same(frame(settings,surfaces,8.4),frame(settings,reordered,8.4)),"surface list order does not change deterministic tour");
        QVector<Surface> routeSurfaces{{"A",{{.1,.1},{.3,.1},{.3,.3},{.1,.3}},1},
            {"B",{{.85,.8},{.95,.8},{.95,.95},{.85,.95}},1},
            {"C",{{.35,.1},{.55,.1},{.55,.3},{.35,.3}},1}};
        const double routeSlot=times.exploreEnd/3;
        const auto secondStop=frame(settings,routeSurfaces,routeSlot+.7);
        check(!secondStop.shapes.isEmpty() && secondStop.shapes.back().surfaceId=="C","tour visits nearest unvisited surface before farther member");
        const auto exitFrame=frame(settings,routeSurfaces,routeSlot-.3);
        QPointF exitCenter;for(const auto &point:exitFrame.shapes.back().points)exitCenter+=point;
        exitCenter/=double(exitFrame.shapes.back().points.size());
        check(exitCenter.x()>.85 && exitCenter.y()<.2,"tour exits through closest sampled mapped boundary");
        const auto explored=frame(settings,surfaces,times.exploreEnd-.01);
        QSet<QString> visited; for(const auto &p:explored.shapes) visited.insert(p.surfaceId);
        check(visited.size()==3,"exploration leaves fragments on each group member");
        Settings focus=settings; focus.effect=2;
        Settings mint=focus; mint.palette=5;
        check(Settings::fromJson(mint.json())==mint,"Mint palette survives project settings round trip");
        QSet<QRgb> paletteColors;
        for(int palette=0;palette<6;++palette) {
            auto variant=focus; variant.palette=palette;
            const auto colored=frame(variant,surfaces,8.4);
            if(!colored.shapes.isEmpty()) paletteColors.insert(colored.shapes.front().color.rgb());
        }
        check(paletteColors.size()==6,"all six UI palettes render distinct colors");
        const auto mintFrame=frame(mint,surfaces,8.4);
        bool mintColors=!mintFrame.shapes.isEmpty();
        for(const auto &primitive:mintFrame.shapes) mintColors &= primitive.color.green()>primitive.color.red();
        check(mintColors,"Mint palette uses green light throughout its geometry");
        const auto hologram=frame(focus,surfaces,8.4);
        QSet<QString> illuminated; for(const auto &p:hologram.shapes) illuminated.insert(p.surfaceId);
        check(hologram.stage=="Focus" && illuminated.size()==3 && ambientStart(focus,3)==0 && bounded(hologram,surfaces),"Focus animates every transformed surface immediately");
        check(!same(hologram,frame(focus,surfaces,8.9)),"Focus scans and geometry move with group clock");
        auto large=surfaces;
        for(int i=0;i<90;++i) {auto s=surfaces[i%3];s.id=QString::number(i);large.append(s);}
        focus.density=80;
        check(bounded(frame(focus,large,123.4),large) && bounded(frame(settings,large,24.5),large),"large groups retain fixed primitive budget");
        check(bounded(frame(settings,surfaces,std::numeric_limits<double>::infinity()),surfaces),"non-finite elapsed time is sanitized");
    } catch(const std::exception &e) {std::cerr<<"FAIL "<<e.what()<<'\n';return 1;}
    return 0;
}
