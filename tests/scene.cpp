#include "model.h"
#include "projects.h"
#include <QApplication>
#include <QTemporaryDir>
#include <QJsonArray>
#include <QFile>
#include <QJsonDocument>
#include <QDebug>
#include <QDir>
#include <cstdio>
int main(int argc,char **argv){
    QApplication app(argc,argv); Scene scene; QString error; int failures=0;
    auto check=[&](bool ok,const char *label){if(!ok){qCritical()<<label; ++failures;}};
    Surface shortPanel,longPanel; shortPanel.name="Short"; longPanel.name="Long";
    shortPanel.corners={{.1,.3},{.3,.3},{.3,.5},{.1,.5}};
    longPanel.corners={{.32,.1},{.52,.1},{.52,.7},{.32,.7}};
    scene.surfaces={shortPanel,longPanel}; scene.selected=0;
    const auto original=scene.json();
    check(scene.connectNearest(0,true,error),"partial-edge fit connects");
    check(scene.connections.size()==1,"one explicit connection");
    check(scene.surfaces[1].json()==longPanel.json(),"target geometry stays fixed");
    check(scene.surfaces[0].valid(),"fit preserves nonfolded mesh");
    const auto connected=scene.json();
    check(scene.exposedContours().size()==1,"fitted panels expose one continuous contour");
    scene.setAppearance(1,100,0,false);auto onlyVisible=scene.exposedContours();check(onlyVisible.size()==1&&!onlyVisible[0].isEmpty()&&QLineF(onlyVisible[0].first(),onlyVisible[0].last()).length()<1e-6,"transparent neighbor restores closed contour");scene.setAppearance(1,100,100,false);
    scene.disconnectSurface(0); check(scene.connections.isEmpty(),"disconnect removes relation");
    check(scene.surfaces[0].json()==connected["surfaces"].toArray()[0].toObject(),"disconnect retains fitted geometry");
    scene.undo();check(scene.connections.size()==1,"undo disconnect restores relation");
    scene.undo(); check(scene.json()==original,"undo connect restores geometry and relation");
    scene.redo();check(scene.connections.size()==1,"redo connect");
    scene.setConnectionsTied(0,true); const auto targetBefore=scene.surfaces[1].corners[0];
    scene.translateGroup(0,{.01,.02});check(QLineF(scene.surfaces[1].corners[0],targetBefore+QPointF(.01,.02)).length()<1e-9,"tie translates target");
    QTemporaryDir temp;check(scene.save(temp.filePath("connected.pmap"),error),"save v2");
    Scene reopened;check(reopened.load(temp.filePath("connected.pmap"),error),"reopen v2");check(reopened.json()==scene.json(),"persistent connection roundtrip");
    auto malformed=scene.json(); malformed["connections"]=QJsonArray{QJsonObject{{"a","missing"},{"b","missing"}}};const auto healthy=reopened.json();
    check(!reopened.restore(malformed,{},error),"reject broken connection references"); check(reopened.json()==healthy,"failed restore transactional");
    check(!scene.connectNearest(-1,true,error),"reject absent selection");
    const auto v2Path=temp.filePath("actor-upgrade.pmap");scene.setBoundaryActors(true);check(scene.save(v2Path,error),"save v2 actor defaults");
    QFile v2File(v2Path);check(v2File.open(QIODevice::ReadOnly),"read v2 fixture");const auto v2Bytes=v2File.readAll();v2File.close();
    scene.setActorSpeed(175);scene.setActorSize(140);scene.setActorPlayback(false);check(scene.json()["version"].toInt()==3,"nondefault actor controls require protected v3");
    check(scene.save(v2Path,error),"save actor-control upgrade");QFile recoveryV2(v2Path+".v2-recovery.pmap");check(recoveryV2.open(QIODevice::ReadOnly)&&recoveryV2.readAll()==v2Bytes,"exact v2 predecessor retained before v3");
    Scene actorReopen;check(actorReopen.load(v2Path,error)&&actorReopen.actorSpeed==175&&actorReopen.actorSize==140&&!actorReopen.actorsPlaying,"v3 actor settings survive reopen");
    Scene concurrent;check(concurrent.load(temp.filePath("connected.pmap"),error),"second edition opens mapping");scene.renameSurface(0,"First writer");check(scene.save(temp.filePath("connected.pmap"),error),"first edition saves");concurrent.renameSurface(0,"Second writer");check(!concurrent.save(temp.filePath("connected.pmap"),error),"second edition cannot silently overwrite external changes");
    Scene touching;Surface first,second;first.corners={{.1,.1},{.3,.1},{.3,.3},{.1,.3}};second.corners={{.3,.3},{.5,.3},{.5,.5},{.3,.5}};touching.surfaces={first,second};check(touching.exposedContours().size()==2,"unconnected touching panels remain separate");
    Scene legacy;legacy.add();const auto oldPath=temp.filePath("legacy.pmap");check(legacy.save(oldPath,error),"save legacy");QFile oldFile(oldPath);oldFile.open(QIODevice::ReadOnly);const auto oldBytes=oldFile.readAll();oldFile.close();QFile collision(oldPath+".v1-recovery.pmap");collision.open(QIODevice::WriteOnly);collision.write("existing recovery must stay");collision.close();legacy.setBoundaryActors(true);check(legacy.save(oldPath,error),"safe v1 conversion despite recovery collision");bool recovered=false;for(const auto &name:QDir(temp.path()).entryList({"legacy.pmap.v1-*.pmap"},QDir::Files)){QFile f(temp.filePath(name));f.open(QIODevice::ReadOnly);recovered=recovered||f.readAll()==oldBytes;}check(recovered,"exact original retained before conversion");
    fprintf(stdout,"SCENE CHECK %s (%d failures)\n",failures?"FAIL":"PASS",failures);return failures?2:0;
}
