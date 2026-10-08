#pragma once
#include <QApplication>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QTimer>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <memory>

template<class Window> void runEntranceChecks(Window &window,QApplication &app,const QStringList &args){
    window.smoke=true;window.output->resize(960,540);window.output->show();
    QTimer::singleShot(650,&window,[&window,&app,args]{
        auto *play=window.template findChild<QPushButton*>("entrancePlay");
        auto *skip=window.template findChild<QPushButton*>("entranceSkip");
        auto *automatic=window.template findChild<QCheckBox*>("entranceAutomatic");
        auto *start=window.template findChild<QPushButton*>("outputStart");
        auto *stop=window.template findChild<QPushButton*>("outputStop");
        auto *displays=window.template findChild<QComboBox*>("outputDisplays");
        if(!play||!skip||!automatic||!start||!stop||!displays){qInfo()<<"ENTRANCE FAIL replay, skip and output-start controls missing";app.exit(2);return;}
        struct State{QStringList errors;QTemporaryDir temp;QJsonObject saved;QImage content;};auto state=std::make_shared<State>();
        auto require=[state](bool good,const QString &label){if(!good)state->errors<<label;};
        auto &scene=window.scene;scene.newProject();
        QImage media(480,320,QImage::Format_RGB32);media.fill(QColor("#c55845"));
        const auto file=state->temp.filePath("media.png");require(media.save(file),"write synthetic content");
        scene.surfaces[0].corners={{.06,.12},{.34,.09},{.36,.50},{.08,.53}};
        scene.surfaces[0].mesh[4].setY(.63);scene.assignMedia(0,file);
        scene.add();scene.surfaces[1].corners={{.53,.06},{.94,.17},{.86,.49},{.45,.38}};scene.assignMedia(1,file);
        scene.add();scene.surfaces[2].corners={{.27,.60},{.73,.56},{.81,.91},{.22,.95}};scene.assignMedia(2,file);scene.touch(true);
        scene.dirty=false;state->saved=scene.json();state->content=window.output->grabFramebuffer();
        require(window.canvas->graphicsReady&&window.output->graphicsReady,"both OpenGL shaders initialize");
        play->click();scene.entrancePlaying=false;scene.setEntranceTime(3.4);
        window.output->profileRendering=true;const auto grid=window.output->grabFramebuffer();const auto uploads=window.output->renderStats.uploads;
        require(scene.entranceActive&&grid!=state->content,"replay renders scan over existing media");
        scene.setEntranceTime(4.8);const auto locked=window.output->grabFramebuffer();
        require(locked!=grid,"stages change real output pixels");
        require(window.output->renderStats.uploads==uploads,"changing scan time reuses uploaded mesh");
        require(scene.json()==state->saved&&!scene.dirty,"entrance does not edit saved settings");
        scene.surfaces[0].mesh[4].setY(.5);scene.touch();require(window.output->grabFramebuffer()!=locked,"scan grid follows interior mesh deformation");
        scene.surfaces[0].mesh[4].setY(.63);scene.touch();scene.dirty=false;
        skip->click();require(!scene.entranceActive&&window.output->grabFramebuffer()==state->content,"skip restores exact underlying projection");window.output->profileRendering=false;
        const int frames=args.indexOf("--frames");if(frames>=0&&frames+1<args.size()){
            const QString folder=args[frames+1];require(QDir().mkpath(folder),"create requested preview folder");
            const double times[]={.6,1.6,2.8,3.8,4.9,5.7,6.5,7.5,8};
            for(int i=0;i<9;++i){scene.setEntranceTime(times[i]);require(window.output->grabFramebuffer().save(QDir(folder).filePath(QString("frame-%1.png").arg(i,2,10,QChar('0')))),"save rendered entrance frame");}
            const int snapshot=args.indexOf("--snapshot");if(snapshot>=0&&snapshot+1<args.size())require(window.grab().save(args[snapshot+1]),"save actual interface preview");
        }
        play->click();scene.entrancePlaying=false;scene.setEntranceTime(4.5);
        const auto revision=scene.entranceGeometry().revision;scene.surfaces[1].visible=false;scene.surfaces[2].opacity=0;scene.touch(true);
        require(scene.entranceGeometry().revision!=revision&&scene.entranceGeometry().ranges.size()==1,"visibility and opacity invalidate cached mapping");
        scene.surfaces[1].visible=true;scene.surfaces[2].opacity=100;scene.touch(true);scene.dirty=false;
        scene.setAppearance(0,100,50);scene.setEntranceTime(5.599);const auto beforeReveal=window.output->grabFramebuffer();
        scene.setEntranceTime(5.601);const auto afterReveal=window.output->grabFramebuffer();int jump=0;
        for(int y=0;y<beforeReveal.height();++y)for(int x=0;x<beforeReveal.width();++x){const auto a=beforeReveal.pixelColor(x,y),b=afterReveal.pixelColor(x,y);jump=std::max({jump,std::abs(a.red()-b.red()),std::abs(a.green()-b.green()),std::abs(a.blue()-b.blue())});}
        require(jump<=4,"partial-opacity content enters reveal continuously");qInfo()<<"ENTRANCE reveal-start largest channel jump"<<jump;
        scene.setAppearance(0,100,100);scene.dirty=false;
        QString error;const auto project=state->temp.filePath("mapping.pmap");require(scene.save(project,error),"save synthetic mapped media");
        QFile invalid(state->temp.filePath("invalid.pmap"));require(invalid.open(QIODevice::WriteOnly),"create invalid project");invalid.write("{broken");invalid.close();
        require(!scene.load(invalid.fileName(),error)&&scene.entranceActive&&scene.json()==state->saved,"failed load preserves current mapping and entrance");
        require(scene.load(project,error)&&!scene.entranceActive&&scene.json()==state->saved,"successful reopen keeps mapped media and ends entrance");
        automatic->setChecked(true);displays->setCurrentIndex(1);start->click();require(window.output->isVisible()&&scene.entranceActive,"Start output plays enabled entrance");
        stop->click();require(!scene.entranceActive&&!window.output->isVisible(),"Stop cancels entrance");
        automatic->setChecked(false);start->click();require(window.output->isVisible()&&!scene.entranceActive,"automatic entrance can be disabled");
        play->click();scene.entranceElapsed=3;scene.blackout=true;emit scene.changed();const auto blackout=window.output->grabFramebuffer();
        bool black=true;for(int y=0;y<blackout.height();y+=13)for(int x=0;x<blackout.width();x+=13){const auto c=blackout.pixelColor(x,y);black=black&&c.red()==0&&c.green()==0&&c.blue()==0;}
        require(black,"blackout hides all scan ink");
        QTimer::singleShot(160,&window,[&window,&app,state,require]{
            auto &scene=window.scene;require(scene.entranceElapsed==3,"blackout pauses entrance clock");scene.blackout=false;scene.entranceElapsed=7.99;scene.dirty=false;emit scene.changed();
            QTimer::singleShot(180,&window,[&window,&app,state,require]{
                auto &scene=window.scene;require(!scene.entranceActive&&!scene.dirty,"timed entrance finishes without dirtying project");
                scene.playEntrance();scene.newProject();require(!scene.entranceActive,"new project ends entrance");
                scene.surfaces.clear();scene.playEntrance();require(!scene.entranceActive,"no entrance without visible mapped surfaces");
                qInfo()<<"ENTRANCE CHECK"<<(state->errors.isEmpty()?"PASS":"FAIL")<<state->errors;app.exit(state->errors.isEmpty()?0:2);
            });
        });
    });
}
