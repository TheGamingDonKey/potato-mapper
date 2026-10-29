#pragma once
#include "fall-check.h"
#include <QFile>
template<class Window> void runConnectionActorChecks(Window &window,QApplication &app,QStringList args){
    window.output->resize(1200,675);window.output->show();
    QTimer::singleShot(650,&window,[&window,&app,args]{
        QStringList failures;auto require=[&](bool ok,const QString &label){if(!ok)failures<<label;};QTemporaryDir temp;
        QImage black(16,16,QImage::Format_RGBA8888);black.fill(Qt::black);black.save(temp.filePath("black.png"));
        auto &scene=window.scene;scene.newProject();scene.add();
        scene.surfaces[0].corners={{.1,.3},{.3,.3},{.3,.5},{.1,.5}};
        scene.surfaces[1].corners={{.32,.1},{.62,.1},{.62,.7},{.32,.7}};
        for(int i=0;i<2;++i)scene.assignMedia(i,temp.filePath("black.png"));scene.select(0);QString error;
        require(scene.connectNearest(0,true,error),"fit connected panels: "+error);require(scene.exposedContours().size()==1,"compiled shared exterior");
        auto capture=[&]{return window.output->grabFramebuffer();};const auto background=capture();scene.setBoundaryActors(true);scene.actorElapsed=1;auto skeleton=capture();
        require(PotatoFallCheck::changedPixels(background,skeleton)>50,"skeleton appears on mapped output");scene.actorElapsed=3;require(PotatoFallCheck::changedPixels(skeleton,capture())>50,"skeleton travels on contour");
        scene.setBoundaryActors(false);scene.setWizardActors(true);scene.actorElapsed=5.8;auto casting=capture();qInfo()<<"WIZARD STATE"<<scene.wizardActors<<scene.surfaces[0].valid()<<scene.surfaces[1].valid()<<"pixels"<<PotatoFallCheck::changedPixels(background,casting);require(PotatoFallCheck::changedPixels(background,casting)>50,"wizard actors visible");scene.actorElapsed=9;require(PotatoFallCheck::changedPixels(casting,capture())>50,"duel changes through action and recovery");
        scene.blackout=true;auto dark=capture();require(PotatoFallCheck::dark(PotatoHarvestCheck::samples(dark,window.output->canvasRect(),window.output->size(),scene.surfaces[1])),"blackout suppresses actors");scene.blackout=false;
        scene.setBoundaryActors(true);scene.actorElapsed=5.8;const int shot=args.indexOf("--snapshot");if(shot>=0&&shot+1<args.size())require(capture().save(args[shot+1]),"save actor capture");
        const int frames=args.indexOf("--frames");if(frames>=0&&frames+1<args.size()){QDir().mkpath(args[frames+1]);for(int i=0;i<64;++i){scene.actorElapsed=i*.5;require(capture().save(QDir(args[frames+1]).filePath(QString("%1.png").arg(i,3,10,QChar('0')))),"save actor frame");}}
        const auto document=scene.json();require(scene.save(temp.filePath("actors.pmap"),error)&&scene.load(temp.filePath("actors.pmap"),error)&&scene.json()==document,"actor/connection persistence");
        qInfo().noquote()<<"CONNECTION ACTORS"<<(failures.isEmpty()?"PASS":"FAIL")<<failures.join("; ");app.exit(failures.isEmpty()?0:2);
    });
}
