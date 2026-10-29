#include "model.h"
#include <QLineF>
#include <QSet>
#include <algorithm>
#include <cmath>
#include <limits>

namespace {
QPointF pixels(QPointF p,QSize size){return {p.x()*size.width(),p.y()*size.height()};}
QVector<int> edgeNodes(const Surface &s,int side){
    QVector<int> nodes;const int n=s.cells,w=n+1;
    for(int i=0;i<=n;++i)nodes<< (side==0?i:side==1?i*w+n:side==2?n*w+n-i:(n-i)*w);
    return nodes;
}
struct Edge {
    QPolygonF points;QVector<double> lengths;double length=0;
    QPointF at(double t) const {
        const double distance=std::clamp(t,0.0,1.0)*length;
        for(int i=1;i<points.size();++i)if(distance<=lengths[i]||i==points.size()-1){
            const double f=(distance-lengths[i-1])/std::max(1e-12,lengths[i]-lengths[i-1]);return points[i-1]*(1-f)+points[i]*f;
        }
        return {};
    }
};
Edge edge(const Surface &s,int side,QSize output){
    Edge e;const auto transform=s.transform();
    for(int node:edgeNodes(s,side)){const auto p=transform.map(s.mesh[node]);if(!e.points.isEmpty())e.length+=QLineF(pixels(e.points.last(),output),pixels(p,output)).length();e.points<<p;e.lengths<<e.length;}
    return e;
}
int indexOf(const Scene &scene,const QString &uid){for(int i=0;i<scene.surfaces.size();++i)if(scene.surfaces[i].routeUid==uid)return i;return -1;}
struct Candidate {int target=-1,sourceEdge=0,targetEdge=0;double begin=0,end=1,gap=0,score=std::numeric_limits<double>::max();};
Candidate nearest(const Scene &scene,int source){
    Candidate best;if(source<0||source>=scene.surfaces.size()||scene.surfaces[source].locked||!scene.surfaces[source].visible)return best;
    for(int j=0;j<scene.surfaces.size();++j)if(j!=source&&!scene.surfaces[j].locked&&scene.surfaces[j].visible){
        for(int a=0;a<4;++a){const auto se=edge(scene.surfaces[source],a,scene.outputSize);if(se.length<1)continue;
            for(int b=0;b<4;++b){const auto te=edge(scene.surfaces[j],b,scene.outputSize);if(te.length+1e-6<se.length)continue;
                const auto sd=pixels(se.at(1)-se.at(0),scene.outputSize),td=pixels(te.at(1)-te.at(0),scene.outputSize);
                const double alignment=QPointF::dotProduct(sd,td)/std::max(1e-12,std::hypot(sd.x(),sd.y())*std::hypot(td.x(),td.y()));
                if(alignment>-.8)continue;
                const auto mid=pixels(se.at(.5),scene.outputSize);double fraction=0,minimum=std::numeric_limits<double>::max();
                for(int k=1;k<te.points.size();++k){const auto start=pixels(te.points[k-1],scene.outputSize),d=pixels(te.points[k]-te.points[k-1],scene.outputSize);
                    const double f=std::clamp(QPointF::dotProduct(mid-start,d)/std::max(1e-12,QPointF::dotProduct(d,d)),0.0,1.0);
                    const double dist=QLineF(mid,start+d*f).length();if(dist<minimum){minimum=dist;fraction=(te.lengths[k-1]+f*(te.lengths[k]-te.lengths[k-1]))/te.length;}}
                const double span=se.length/te.length,begin=std::clamp(fraction-span*.5,0.0,1.0-span),end=begin+span;
                const double gap=std::max(QLineF(pixels(se.at(0),scene.outputSize),pixels(te.at(end),scene.outputSize)).length(),QLineF(pixels(se.at(1),scene.outputSize),pixels(te.at(begin),scene.outputSize)).length());
                if(gap>std::max(scene.outputSize.width(),scene.outputSize.height())*.25)continue;
                bool occupied=false;for(const auto &c:scene.connections){if((c.a==scene.surfaces[source].routeUid&&c.edgeA==a)||(c.b==scene.surfaces[source].routeUid&&c.edgeB==a)||(c.a==scene.surfaces[j].routeUid&&c.edgeA==b)||(c.b==scene.surfaces[j].routeUid&&c.edgeB==b&&begin<c.endB-1e-6&&end>c.beginB+1e-6)){occupied=true;break;}}
                if(!occupied&&gap<best.score)best={j,a,b,begin,end,gap,gap};
            }
        }
    }
    return best;
}
bool sewn(const Scene &scene,const Scene::Connection &c){
    const int a=indexOf(scene,c.a),b=indexOf(scene,c.b);if(a<0||b<0||!scene.surfaces[a].visible||!scene.surfaces[b].visible||scene.surfaces[a].opacity==0||scene.surfaces[b].opacity==0)return false;
    const auto ea=edge(scene.surfaces[a],c.edgeA,scene.outputSize),eb=edge(scene.surfaces[b],c.edgeB,scene.outputSize);
    for(int i=0;i<=16;++i){const double f=i/16.0;if(QLineF(pixels(ea.at(f),scene.outputSize),pixels(eb.at(c.endB+(c.beginB-c.endB)*f),scene.outputSize)).length()>2)return false;}
    return true;
}
}
bool Scene::hasConnections(int i) const {if(i<0||i>=surfaces.size())return false;const auto uid=surfaces[i].routeUid;for(const auto &c:connections)if(c.a==uid||c.b==uid)return true;return false;}
bool Scene::connectionsTied(int i) const {if(i<0||i>=surfaces.size())return false;bool found=false;for(const auto &c:connections)if(c.a==surfaces[i].routeUid||c.b==surfaces[i].routeUid){found=true;if(!c.tied)return false;}return found;}
QString Scene::connectionPreview(int i) const {
    const auto c=nearest(*this,i);if(c.target<0)return {};
    static const QStringList names{"top","right","bottom","left"};
    return QString("%1 %2 edge → %3 %4 edge, %5–%6% of its length.\nGap: %7 output pixels.\n\nFit moves only the source edge's mesh points; target and other source points stay fixed. Without Fit, geometry stays unchanged; contour crossing requires touching edges.")
        .arg(surfaces[i].name,names[c.sourceEdge],surfaces[c.target].name,names[c.targetEdge]).arg(c.begin*100,0,'f',1).arg(c.end*100,0,'f',1).arg(c.gap,0,'f',1);
}
QPolygonF Scene::connectionPreviewGeometry(int i) const {const auto c=nearest(*this,i);if(c.target<0)return {};const auto a=edge(surfaces[i],c.sourceEdge,outputSize),b=edge(surfaces[c.target],c.targetEdge,outputSize);return {a.at(0),a.at(1),b.at(c.end),b.at(c.begin)};}
QString Scene::connectionDescription(int i) const {
    if(i<0||i>=surfaces.size())return "Select a surface.";
    QStringList descriptions;for(const auto &c:connections)if(c.a==surfaces[i].routeUid||c.b==surfaces[i].routeUid){const int other=indexOf(*this,c.a==surfaces[i].routeUid?c.b:c.a);if(other>=0)descriptions<<surfaces[other].name+(sewn(*this,c)?" · continuous boundary":" · linked, gap remains")+(c.tied?" · tied":"");}
    return descriptions.isEmpty()?"No connections. Connect finds a nearby matching edge interval.":descriptions.join("\n");
}
bool Scene::connectNearest(int i,bool fit,QString &error){
    error.clear();const auto c=nearest(*this,i);if(c.target<0){error="No free facing edge found nearby. Unlock and move the panels closer; the target side must be at least as long as the source side.";return false;}
    Surface proposed=surfaces[i];if(fit){const auto source=edge(proposed,c.sourceEdge,outputSize),target=edge(surfaces[c.target],c.targetEdge,outputSize);const auto inverse=proposed.transform().inverted();const auto nodes=edgeNodes(proposed,c.sourceEdge);
        for(int k=0;k<nodes.size();++k){const double f=source.lengths[k]/source.length;proposed.mesh[nodes[k]]=inverse.map(target.at(c.end+(c.begin-c.end)*f));}
        if(!proposed.valid()){error="This fit would fold the source mesh. Nothing changed. Move the panels closer or use a smaller gap.";return false;}
        const auto fitted=edge(proposed,c.sourceEdge,outputSize);
        for(int k=0;k<=32;++k){double f=k/32.0;if(QLineF(pixels(fitted.at(f),outputSize),pixels(target.at(c.end+(c.begin-c.end)*f),outputSize)).length()>2){error="The source mesh is too coarse to follow this bent edge. Increase its Map subdivisions and try again. Nothing changed.";return false;}}}
    checkpoint();surfaces[i]=proposed;connections.append({surfaces[i].routeUid,surfaces[c.target].routeUid,c.sourceEdge,c.targetEdge,c.begin,c.end,false});touch(true);return true;
}
void Scene::disconnectSurface(int i){if(!hasConnections(i))return;checkpoint();const auto uid=surfaces[i].routeUid;connections.erase(std::remove_if(connections.begin(),connections.end(),[&](const auto &c){return c.a==uid||c.b==uid;}),connections.end());touch(true);}
void Scene::setConnectionsTied(int i,bool tied){if(!hasConnections(i))return;const auto uid=surfaces[i].routeUid;bool changed=false;for(const auto &c:connections)if((c.a==uid||c.b==uid)&&c.tied!=tied)changed=true;if(!changed)return;checkpoint();for(auto &c:connections)if(c.a==uid||c.b==uid)c.tied=tied;touch(true);}
void Scene::setBoundaryActors(bool enabled){if(boundaryActors==enabled)return;checkpoint();boundaryActors=enabled;actorElapsed=0;touch(true);}
void Scene::setWizardActors(bool enabled){if(wizardActors==enabled)return;checkpoint();wizardActors=enabled;actorElapsed=0;touch(true);}
void Scene::translateGroup(int i,QPointF delta){
    if(i<0||i>=surfaces.size()||surfaces[i].locked)return;QSet<QString> group{surfaces[i].routeUid};bool more=true;
    while(more){more=false;for(const auto &c:connections)if(c.tied&&(group.contains(c.a)||group.contains(c.b))){if(!group.contains(c.a)){group.insert(c.a);more=true;}if(!group.contains(c.b)){group.insert(c.b);more=true;}}}
    QVector<Surface> proposed=surfaces;for(auto &s:proposed)if(group.contains(s.routeUid)){if(s.locked)return;for(auto &p:s.corners)p+=delta;if(!s.valid())return;}
    surfaces=proposed;touch();
}
QVector<QPolygonF> Scene::exposedContours() const {
    struct Segment{QPointF a,b;QString owner;bool used=false;};QVector<Segment> segments;
    for(const auto &s:surfaces)if(s.visible&&s.opacity>0)for(int side=0;side<4;++side){const auto e=edge(s,side,outputSize);QVector<QPair<double,double>> omitted;QVector<double> knots{0,1};
        for(auto len:e.lengths)knots<<len/std::max(1e-12,e.length);
        for(const auto &c:connections)if(sewn(*this,c)){if(c.a==s.routeUid&&c.edgeA==side)omitted<<qMakePair(0.0,1.0);if(c.b==s.routeUid&&c.edgeB==side){omitted<<qMakePair(c.beginB,c.endB);knots<<c.beginB<<c.endB;}}
        std::sort(knots.begin(),knots.end());for(int k=1;k<knots.size();++k){const double a=knots[k-1],b=knots[k];if(b-a<1e-8)continue;bool skip=false;for(const auto &span:omitted)if((a+b)*.5>span.first-1e-9&&(a+b)*.5<span.second+1e-9)skip=true;if(!skip)segments<<Segment{e.at(a),e.at(b),s.routeUid,false};}}
    QVector<QPolygonF> contours;
    for(int start=0;start<segments.size();++start)if(!segments[start].used){QPolygonF path;int current=start;
        for(int n=0;n<=segments.size();++n){auto &seg=segments[current];if(seg.used)break;seg.used=true;if(path.isEmpty())path<<seg.a;path<<seg.b;
            if(path.size()>2&&QLineF(pixels(path.first(),outputSize),pixels(path.last(),outputSize)).length()<.01)break;
            int next=-1;double best=2.01;for(int j=0;j<segments.size();++j)if(!segments[j].used){
                bool allowed=seg.owner==segments[j].owner;
                if(!allowed)for(const auto &c:connections)if(((c.a==seg.owner&&c.b==segments[j].owner)||(c.b==seg.owner&&c.a==segments[j].owner))&&sewn(*this,c)){
                    const int a=indexOf(*this,c.a);const auto seam=edge(surfaces[a],c.edgeA,outputSize);
                    if(QLineF(pixels(seg.b,outputSize),pixels(seam.at(0),outputSize)).length()<2.01||QLineF(pixels(seg.b,outputSize),pixels(seam.at(1),outputSize)).length()<2.01)allowed=true;}
                if(!allowed)continue;const double d=QLineF(pixels(seg.b,outputSize),pixels(segments[j].a,outputSize)).length();if(d<best){best=d;next=j;}}
            if(next<0)break;current=next;}
        if(path.size()>2)contours<<path;
    }
    return contours;
}
