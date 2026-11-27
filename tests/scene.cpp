#include "model.h"
#include "projects.h"
#include <QApplication>
#include <QTemporaryDir>
#include <QJsonArray>
#include <QFile>
#include <QJsonDocument>
#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <cstdio>

// A North Pole Express mapping saved by the packaged 0.9.13 editor after a
// corner drag and a mesh edit. Qt indentation reconstructs the original bytes.
static QByteArray nativeChristmasV2(){
    return QJsonDocument::fromJson(R"({
        "actorSize":100,"actorSpeed":100,"actorsPlaying":true,"boundaryActors":false,"connections":[],
        "dynamic":{"cellSize":56,"density":36,"effect":0,"glitch":35,"members":[],"palette":0,
                   "playing":true,"seed":1347376193,"snakeWidth":40,"speed":100},
        "format":"PotatoMapper","height":1080,"version":2,"width":1920,"wizardActors":false,
        "surfaces":[{
            "blend":0,"brightness":100,"cells":2,
            "corners":[[0.025,0.025],[1.0120065789473682,0.06885964912280701],[0.975,0.975],[0.025,0.975]],
            "fit":0,"fxAngle":0,"fxDensity":44,"fxEdge":0,"fxFlow":65,"fxPalette":0,"fxReverse":false,
            "id":"north-pole-express","locked":false,"media":"",
            "mesh":[[0,0],[0.5,0],[1,0],[0,0.5],[0.56,0.42],[1,0.5],[0,1],[0.5,1],[1,1]],
            "name":"North Pole Express","opacity":100,"pattern":23,"patternPlaying":true,"patternSize":24,
            "patternSpeed":100,"routeUid":"86af440d-7bc5-42f2-8e59-812d910d4deb","visible":true
        }]
    })").toJson();
}

static QByteArray readProjectBytes(const QString &path){
    QFile file(path);
    return file.open(QIODevice::ReadOnly)?file.readAll():QByteArray{};
}

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
    {
        const auto path=temp.filePath("christmas-0.9.13.pmap");
        const auto recovery=path+".v2-recovery.pmap";
        const auto originalBytes=nativeChristmasV2();
        check(QCryptographicHash::hash(originalBytes,QCryptographicHash::Sha256).toHex()
              =="638bf814dd14253ecb4685f18c4267fa185ff08318c42f9e5dcce65c81db8ff2",
              "v2 sample matches the mapping written by 0.9.13");
        QFile fixture(path);
        check(fixture.open(QIODevice::WriteOnly)&&fixture.write(originalBytes)==originalBytes.size(),
              "write the old mapping inside the test directory");
        fixture.close();

        const auto originalProject=QJsonDocument::fromJson(originalBytes).object();
        Scene christmas;
        const bool loaded=christmas.load(path,error)&&christmas.surfaces.size()==1;
        check(loaded,"open the 0.9.13 mapping");
        if(loaded){
            check(!christmas.surfaces[0].subjectOnly&&christmas.json()==originalProject,
                  "old Christmas mapping keeps the full scene and its warped geometry");
            check(christmas.save(path,error)&&readProjectBytes(path)==originalBytes&&!QFile::exists(recovery),
                  "saving the old full scene keeps its original bytes and file version");

            auto expectedCutout=originalProject;
            auto surface=originalProject["surfaces"].toArray().first().toObject();
            surface["subjectOnly"]=true;
            expectedCutout["surfaces"]=QJsonArray{surface};
            expectedCutout["version"]=4;
            christmas.setSubjectOnly(0,true);
            check(christmas.dirty&&christmas.json()==expectedCutout,"Subject only changes just the flag and format");
            christmas.undo();
            check(christmas.json()==originalProject,"Undo restores the full 0.9.13 scene");
            christmas.redo();
            check(christmas.json()==expectedCutout,"Redo restores Subject only");
            check(christmas.save(path,error)&&QJsonDocument::fromJson(readProjectBytes(path)).object()==expectedCutout,
                  "save writes the edited mapping as v4");
            check(readProjectBytes(recovery)==originalBytes,"the permanent v2 recovery copy has the exact old bytes");
            Scene reopenedCutout;
            check(reopenedCutout.load(path,error)&&reopenedCutout.json()==expectedCutout,
                  "reopening v4 restores Subject only and the warped mapping");
            Scene recovered;
            check(recovered.load(recovery,error)&&recovered.json()==originalProject,
                  "the recovered 0.9.13 full scene opens");
        }
    }
    {
        Scene flight;
        flight.add();
        flight.setPattern(0,firstChristmasPattern);
        flight.setSubjectOnly(0,true);
        flight.setAppearance(0,83,64);
        flight.setBlend(0,1);
        auto localLook=flight.surfaces[0].fx;
        localLook.edge=13;
        flight.setFxLook(0,localLook);
        flight.add();
        flight.setPattern(1,sleighFlightPattern);
        auto &pilot=flight.surfaces[1];
        pilot.patternPhase=17.625;
        pilot.patternSpeed=155;
        pilot.patternSize=34;
        pilot.patternPlaying=false;
        auto look=pilot.fx;
        look.density=61;look.flow=78;look.angle=-37;look.palette=4;look.reverse=true;look.edge=6;
        flight.setFxLook(1,look);
        const Surface established=pilot;

        flight.select(0);
        flight.setPattern(0,sleighFlightPattern);
        const auto &joined=flight.surfaces[0];
        check(pilot.patternPhase==established.patternPhase&&pilot.json()==established.json(),
              "joining from an earlier surface leaves the existing sleigh clock and controls intact");
        auto expectedLook=established.fx;
        expectedLook.edge=localLook.edge;
        check(joined.pattern==sleighFlightPattern&&joined.patternPhase==established.patternPhase
              &&joined.patternSpeed==established.patternSpeed&&joined.patternPlaying==established.patternPlaying
              &&joined.patternSize==established.patternSize&&joined.fx==expectedLook,
              "the new flight member inherits phase, playback, size and flight controls");
        check(joined.brightness==83&&joined.opacity==64&&joined.blend==1&&!joined.subjectOnly,
              "the joining surface keeps its own projection settings and clears its old cutout flag");
    }
    fprintf(stdout,"SCENE CHECK %s (%d failures)\n",failures?"FAIL":"PASS",failures);return failures?2:0;
}
