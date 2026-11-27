#include "model.h"
#include "actors.h"
#include "wizard-duel.h"
#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <cmath>
#include <cstdio>
#include <limits>

int main(int argc,char **argv){
    QApplication app(argc,argv);int failures=0;
    auto check=[&](bool ok,const char *label){if(!ok){qCritical()<<label;++failures;}};
    auto near=[](double a,double b){return std::abs(a-b)<1e-6;};
    Scene scene;Surface panel;panel.corners={{.1,.1},{.3,.1},{.3,.3},{.1,.3}};
    scene.outputSize={1000,500};scene.surfaces={panel};scene.touch();
    const auto &initial=scene.worldGeometry();
    check(initial.panels.size()==1,"cache contains mapped visible panel");
    check(initial.outputClip.contains({200,100})&&!initial.outputClip.contains({400,100}),"pixel clip follows normalized mapping");
    check(initial.contours.size()==1&&near(initial.contours[0].length,600),"contour metric uses projector pixels");
    const auto builds=scene.worldGeometryBuilds;
    scene.actorElapsed=7;scene.surfaces[0].patternPhase=8;scene.setDynamicTime(9);scene.setEntranceTime(3);
    QImage first(400,200,QImage::Format_ARGB32_Premultiplied),second(800,400,QImage::Format_ARGB32_Premultiplied);first.fill(Qt::black);second.fill(Qt::black);
    {QPainter p(&first);potatoDrawBoundaryActors(p,scene,first.rect());potatoDrawWizardDuel(p,scene,first.rect());}
    {QPainter p(&second);potatoDrawBoundaryActors(p,scene,second.rect());potatoDrawWizardDuel(p,scene,second.rect());}
    check(scene.worldGeometryBuilds==builds,"different animation times and both view sizes reuse geometry");
    scene.surfaces[0].corners.translate(.1,0);scene.touch();
    check(!scene.worldGeometry().outputClip.contains({150,100})&&scene.worldGeometry().outputClip.contains({350,100}),"moving panel invalidates cached clip");
    scene.surfaces[0].subdivide(4);scene.surfaces[0].mesh[2].ry()-=.1;scene.touch();
    check(scene.worldGeometry().panels[0].normalizedOutline.size()==16&&scene.worldGeometry().outputClip.contains({300,46}),"subdivided bent edge updates actual mesh outline");
    scene.setAppearance(0,75,0);
    check(scene.worldGeometry().panels.isEmpty()&&scene.exposedContours().isEmpty(),"transparent panel removed from clips and routes");
    scene.setAppearance(0,75,50);scene.surfaces[0].visible=false;scene.touch();
    check(scene.worldGeometry().panels.isEmpty(),"hidden panel removed from cache");
    scene.surfaces[0].visible=true;scene.touch();const auto beforeResize=scene.worldGeometryBuilds;
    scene.worldGeometry();scene.outputSize={2000,1000};
    check(scene.worldGeometry().outputClip.contains({600,200})&&!scene.worldGeometry().outputClip.contains({300,200}),"direct output resize refreshes pixel clip");
    check(scene.worldGeometryBuilds==beforeResize+2,"output resize builds once independent of mapping touch");
    Scene other;other.surfaces={panel};other.touch();
    check(other.worldGeometry().normalizedClip.contains({.15,.15})&&!scene.worldGeometry().normalizedClip.contains({.15,.15}),"two scenes retain independent geometry");
    Scene joined;Surface a,b;a.corners={{.1,.3},{.3,.3},{.3,.5},{.1,.5}};b.corners={{.32,.1},{.52,.1},{.52,.7},{.32,.7}};
    joined.surfaces={a,b};QString error;check(joined.connectNearest(0,true,error),"partial connection fixture fits");
    check(joined.exposedContours().size()==1,"connected panels share exterior contour");
    joined.setAppearance(1,100,0);const auto closed=joined.exposedContours();
    check(closed.size()==1&&QLineF(closed[0].first(),closed[0].last()).length()<1e-8,"transparent neighbor restores closed route after cache invalidation");
    Scene touching;b.corners={{.3,.5},{.5,.5},{.5,.7},{.3,.7}};touching.surfaces={a,b};touching.touch();
    check(touching.exposedContours().size()==2,"unconnected touching panels retain independent loops");
    Scene invalid;invalid.surfaces={panel};invalid.surfaces[0].mesh[0].rx()=std::numeric_limits<double>::quiet_NaN();invalid.touch();
    check(invalid.worldGeometry().panels.isEmpty(),"nonfinite mapping is excluded without unsafe metric arithmetic");
    fprintf(stdout,"GEOMETRY CACHE CHECK %s (%d failures)\n",failures?"FAIL":"PASS",failures);return failures?2:0;
}
