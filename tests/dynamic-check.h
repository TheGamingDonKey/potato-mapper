#pragma once
#include <QApplication>
#include <QComboBox>
#include <QListWidget>
#include <QCheckBox>
#include <QTemporaryDir>
#include <QJsonDocument>
#include <QFile>
#include <memory>
#include <QTimer>
#include <QSpinBox>

template<class Window> void runDynamicChecks(Window &window,QApplication &app,const QStringList &args){
    window.output->resize(960,540);window.output->show();
    QTimer::singleShot(650,&window,[&window,&app,args]{
        auto *picker=window.template findChild<QComboBox*>("dynamicEffect");
        auto *members=window.template findChild<QListWidget*>("dynamicMembers");
        auto *pause=window.template findChild<QCheckBox*>("dynamicPause");
        auto *snakeWidth=window.template findChild<QSpinBox*>("dynamicSnakeWidth");
        auto *cellSize=window.template findChild<QSpinBox*>("dynamicCellSize");
        if(!picker||!members||!pause||picker->findText("Hot-Ass Potato")<0||picker->findText("Potato Focus")<0){qInfo()<<"DYNAMIC FAIL controls missing";app.exit(2);return;}
        auto &scene=window.scene;scene.newProject();scene.surfaces[0].id="A";
        scene.surfaces[0].corners={{.06,.12},{.34,.09},{.36,.50},{.08,.53}};
        scene.add();scene.surfaces[1].id="B";scene.surfaces[1].corners={{.53,.06},{.94,.17},{.86,.49},{.45,.38}};
        scene.add();scene.surfaces[2].id="C";scene.surfaces[2].corners={{.27,.60},{.73,.56},{.81,.91},{.22,.95}};scene.touch(true);scene.setPattern(0,13);
        picker->setCurrentIndex(1);pause->setChecked(true);scene.dirty=false;scene.setDynamicTime(6);
        struct State{QStringList errors;QTemporaryDir temp;QImage paused;QJsonObject saved;quint64 pausedBuilds=0,pausedPaints=0,pausedUploads=0;};auto state=std::make_shared<State>();
        auto require=[state](bool good,const QString &label){if(!good)state->errors<<label;};
        require(scene.dynamic.effect==1&&scene.dynamic.members.size()==3,"effect selector includes all visible surfaces");
        require(!scene.dynamic.playing,"pause control freezes shared clock");
        require(snakeWidth&&cellSize,"output-pixel size controls exist");
        if(snakeWidth&&cellSize){snakeWidth->setValue(44);cellSize->setValue(64);
            require(scene.dynamic.snakeWidth==44&&scene.dynamic.cellSize==64,"size controls update scene settings");
            scene.undo();require(scene.dynamic.cellSize==56,"cell size has Undo");scene.redo();require(scene.dynamic.cellSize==64,"cell size has Redo");scene.dirty=false;}
        require(window.canvas->graphicsReady&&window.output->graphicsReady,"both OpenGL renderers initialized");
        window.output->profileRendering=true;state->paused=window.output->grabFramebuffer();state->pausedBuilds=window.output->renderStats.meshBuilds;state->pausedPaints=window.output->paintedFrames;state->pausedUploads=window.output->renderStats.uploads;state->saved=scene.json();
        QTimer::singleShot(160,&window,[&window,&app,args,picker,pause,state,require]{
            auto &scene=window.scene;
            require(window.output->grabFramebuffer()==state->paused,"paused output remains identical");
            require(window.output->renderStats.meshBuilds==state->pausedBuilds,"paused redraw reuses polygon geometry");window.output->profileRendering=false;
            require(window.output->renderStats.uploads==state->pausedUploads,"paused redraw retains uploaded vertices");
            require(window.output->paintedFrames<=state->pausedPaints+2,"covered individual FX do not repaint a paused group");
            require(scene.json()==state->saved&&!scene.dirty,"animation seeking preserves saved settings and clean state");
            QString error;const auto path=state->temp.filePath("dynamic.pmap");require(scene.save(path,error)&&scene.load(path,error)&&scene.json()==state->saved,"dynamic settings save and reopen");
            scene.setDynamicTime(7);require(window.output->grabFramebuffer()!=state->paused,"snake motion changes real output pixels");
            scene.setDynamicTime(30);scene.setAppearance(0,90,100);scene.undo();require(scene.dynamicElapsed==30,"Undo appearance preserves ambient clock");
            scene.surfaces.removeLast();scene.touch(true);require(scene.dynamicSurfaces.size()==2,"removed member excluded from geometry");scene.undo();
            picker->setCurrentIndex(2);scene.setDynamicTime(3);const auto focus=window.output->grabFramebuffer();require(focus!=state->paused&&!scene.dynamicFrame.shapes.isEmpty(),"Focus produces distinct rendered geometry");
            const int snapshot=args.indexOf("--snapshot");if(snapshot>=0&&snapshot+1<args.size()){
                window.showDynamicControls();QApplication::processEvents();
                auto *scroll=window.template findChild<QScrollArea*>("surfaceInspector");
                require(scroll&&scroll->viewport()->rect().contains(picker->mapTo(scroll->viewport(),picker->rect().center())),"Dynamic FX shortcut reveals effect controls");
                require(window.grab().save(args[snapshot+1]),"save actual Dynamic FX interface preview");
            }
            // An invalid file must never replace the current scene.
            const auto before=scene.json();QFile invalid(state->temp.filePath("invalid.pmap"));require(invalid.open(QIODevice::WriteOnly),"write invalid load fixture");invalid.write("{\"format\":\"PotatoMapper\",\"version\":1,\"width\":1920,\"height\":1080,\"surfaces\":[{}]}");invalid.close();
            require(!scene.load(invalid.fileName(),error)&&scene.json()==before&&error.contains("Surface 1"),"failed load retains scene and identifies failing surface");
            require(invalid.open(QIODevice::WriteOnly|QIODevice::Truncate),"write malformed list fixture");invalid.write("{\"format\":\"PotatoMapper\",\"version\":1,\"width\":1920,\"height\":1080,\"surfaces\":\"bad\"}");invalid.close();
            require(!scene.load(invalid.fileName(),error)&&scene.json()==before,"malformed surfaces list does not replace scene");
            scene.newProject();scene.surfaces[0].corners={{0,0},{1,0},{1,1},{0,1}};scene.surfaces[0].mesh[4].setY(.65);
            PotatoDynamic::Settings deformation;deformation.effect=2;deformation.playing=false;deformation.members={scene.surfaces[0].id};scene.setDynamic(deformation);
            PotatoDynamic::Primitive line;line.surfaceId=scene.surfaces[0].id;line.points={{.08,.5},{.92,.5}};line.color=Qt::white;line.width=.012;line.closed=false;scene.dynamicFrame.shapes={line};
            const auto warped=window.output->grabFramebuffer();require(warped.pixelColor(qRound(warped.width()*.5),qRound(warped.height()*.65)).red()>60,"dynamic scan follows deformed mesh interior");
            auto second=scene.surfaces[0];second.corners={{.6,0},{1,0},{1,1},{.6,1}};second.mesh[4].setY(.35);
            scene.surfaces[0].corners={{0,0},{.4,0},{.4,1},{0,1}};scene.surfaces.append(second);scene.touch(true);scene.dynamicFrame.shapes={line};
            const auto duplicate=window.output->grabFramebuffer();
            require(duplicate.pixelColor(qRound(duplicate.width()*.2),qRound(duplicate.height()*.65)).red()>60&&duplicate.pixelColor(qRound(duplicate.width()*.8),qRound(duplicate.height()*.35)).red()>60,"accepted duplicate surface IDs retain distinct mesh geometry");
            scene.newProject();scene.surfaces[0].corners={{0,0},{1,0},{1,1},{0,1}};deformation.members={scene.surfaces[0].id};scene.setDynamic(deformation);
            auto edgeLine=line;edgeLine.surfaceId=scene.surfaces[0].id;edgeLine.points={{-.3,.3},{.1,.7}};edgeLine.outputWidth=3;scene.dynamicFrame.shapes={edgeLine};
            const auto clippedStroke=window.output->grabFramebuffer();
            require(clippedStroke.pixelColor(qRound(clippedStroke.width()*.05),qRound(clippedStroke.height()*.65)).red()>60,
                    "output-pixel stroke remains visible when its midpoint is beyond a clipped edge");
            scene.newProject();scene.surfaces[0].id="A";scene.surfaces[0].corners={{.05,.1},{.5,.1},{.5,.7},{.05,.7}};
            scene.add();scene.surfaces[1].id="B";scene.surfaces[1].corners={{.5,.3},{.95,.3},{.95,.9},{.5,.9}};scene.touch(true);
            PotatoDynamic::Settings connected;connected.effect=1;connected.playing=false;connected.palette=1;connected.members={"A","B"};scene.setDynamic(connected);
            const double handoff=PotatoDynamic::timeline(connected,2).exploreEnd/2;
            const auto portalLit=[](const QImage &image){int bright=0;for(int y=qRound(image.height()*.485);y<=qRound(image.height()*.515);++y)for(int x=qRound(image.width()*.49);x<=qRound(image.width()*.51);++x)if(image.pixelColor(x,y).red()>100)++bright;return bright>12;};
            scene.setDynamicTime(handoff-.001);const auto beforePortal=window.output->grabFramebuffer();
            scene.setDynamicTime(handoff+.001);const auto afterPortal=window.output->grabFramebuffer();
            require(portalLit(beforePortal)&&portalLit(afterPortal),"connected snake stays bright at the shared opening before and after handoff");
            qInfo()<<"DYNAMIC CHECK"<<(state->errors.isEmpty()?"PASS":"FAIL")<<state->errors;
            if(!state->errors.isEmpty()){app.exit(2);return;}
            if(!args.contains("--test-dynamic-motion")){pause->setChecked(false);scene.dirty=false;QTimer::singleShot(160,&window,[&window,&app]{qInfo()<<"DYNAMIC timed clean"<<!window.scene.dirty;app.exit(window.scene.dirty?2:0);});return;}
            const int argument=args.indexOf("--frames");const QString folder=argument>=0&&argument+1<args.size()?args[argument+1]:QString();
            if(folder.isEmpty()||!QDir().mkpath(folder)){app.exit(2);return;}
            const bool portals=args.contains("--portal-frames");
            const bool fluid=args.contains("--fluid-frames");
            scene.newProject();scene.surfaces.clear();for(int i=0;i<(portals?2:3);++i){Surface surface;surface.id=QString(QChar('A'+i));surface.name="Surface "+QString::number(i+1);surface.corners=portals?(i==0?QPolygonF{{.05,.1},{.5,.1},{.5,.7},{.05,.7}}:QPolygonF{{.5,.3},{.95,.3},{.95,.9},{.5,.9}}):i==0?QPolygonF{{.06,.12},{.34,.09},{.36,.50},{.08,.53}}:i==1?QPolygonF{{.53,.06},{.94,.17},{.86,.49},{.45,.38}}:QPolygonF{{.27,.60},{.73,.56},{.81,.91},{.22,.95}};scene.surfaces<<surface;}
            if(fluid){scene.surfaces[0].corners={{.06,.12},{.46,.12},{.46,.84},{.06,.84}};
                scene.surfaces[1].corners={{.46,.28},{.70,.30},{.70,.68},{.46,.66}};
                scene.surfaces[2].corners={{.71,.39},{.93,.24},{.99,.53},{.77,.70}};
                for(auto &s:scene.surfaces){s.subdivide(4);for(auto &p:s.mesh)p.setY(p.y()+.06*std::sin(p.x()*3.141592653589793)*std::sin(p.y()*3.141592653589793));}}
            PotatoDynamic::Settings settings;settings.effect=1;settings.members={"A","B","C"};settings.playing=false;scene.setDynamic(settings);
            if(fluid){auto demo=settings;demo.playing=true;scene.setDynamic(demo);QString demoError;
                if(!scene.save(QDir(folder).filePath("../fluid-snake-demo.pmap"),demoError)){qInfo()<<"DYNAMIC demo save failed"<<demoError;app.exit(2);return;}
                scene.setDynamic(settings);scene.setDynamicTime(6);window.showDynamicControls();QApplication::processEvents();
                window.grab().save(QDir(folder).filePath("../fluid-controls.png"));}
            auto index=std::make_shared<int>(0);auto capture=std::make_shared<std::function<void()>>();std::weak_ptr<std::function<void()>> weak=capture;
            const bool reference=args.contains("--reference-frames");
            *capture=[&window,&app,folder,index,weak,reference,portals,fluid]{auto keep=weak.lock();if(*index==(fluid?240:portals?24:reference?8:60)){app.exit(0);return;}auto settings=window.scene.dynamic;settings.effect=(fluid||portals)?1:*index<(reference?6:48)?1:2;window.scene.setDynamic(settings);const double phases[]={5*.58,14*.58,30*.58,34*.58,38*.58,46*.58,0,8*.35};window.scene.setDynamicTime(fluid?*index*.07+.3:portals?6.2+*index*.08:reference?phases[*index]:*index<48?*index*.58:(*index-48)*.35);
                const auto file=QDir(folder).filePath(QString("frame-%1.png").arg(*index,3,10,QChar('0')));if(!window.output->grabFramebuffer().save(file)){app.exit(2);return;}++*index;QTimer::singleShot(30,&window,[keep]{(*keep)();});};(*capture)();
        });
    });
}
