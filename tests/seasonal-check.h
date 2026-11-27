#pragma once
#include "fall-check.h"

// Disposable real OpenGL check. --frames captures one complete 24 s cycle at
// four samples/second for review; passing pixels is not artistic acceptance.
template<class Window> void runSeasonalChecks(Window &window,QApplication &app,QStringList args){
    window.output->resize(1200,675);window.output->show();
    QTimer::singleShot(650,&window,[&window,&app,args]{
        QStringList failures;
        auto require=[&](bool ok,const QString &label){if(!ok)failures<<label;};
        auto option=[&](const QString &name){
            const int i=args.indexOf(name);if(i<0)return QString{};
            if(i+1>=args.size()||args[i+1].startsWith("--")){require(false,name+" requires a path");return QString{};}
            return args[i+1];
        };
        const auto frames=option("--frames"),snapshot=option("--snapshot"),demo=option("--demo");
        auto finish=[&]{qInfo().noquote()<<"SEASONAL CHECK"<<(failures.isEmpty()?"PASS":"FAIL")<<failures.join("; ");app.exit(failures.isEmpty()?0:2);};
        auto *picker=window.template findChild<QComboBox*>("animationPicker");
        auto *pause=window.template findChild<QCheckBox*>("patternPause");
        require(picker&&pause,"seasonal picker and Pause exist");
        require(window.output->graphicsReady&&window.output->graphicsError.isEmpty(),"output shader: "+window.output->graphicsError);
        require(window.canvas->graphicsReady&&window.canvas->graphicsError.isEmpty(),"editor shader: "+window.canvas->graphicsError);
        if(!failures.isEmpty()){finish();return;}
        auto &scene=window.scene;
        scene.newProject();scene.outputSize=QSize(1200,675);
        while(scene.surfaces.size()<4)scene.add();
        const QStringList names{"Hollow Crown","Lantern Keepers","Midnight Workshop","Snowfall"};
        Surface defaults[4];
        for(int i=0;i<4;++i){
            const int id=19+i,entry=picker->findData(id);
            require(entry>=0,names[i]+" stable picker ID "+QString::number(id));
            if(entry<0)continue;
            scene.select(i);picker->setCurrentIndex(entry);pause->setChecked(true);
            auto &s=scene.surfaces[i];
            s.name=names[i];s.patternPhase=0;
            const double left=i%2==0?.035:.535,top=i<2?.04:.54;
            s.corners={QPointF(left,top),QPointF(left+.43,top+.012),QPointF(left+.425,top+.42),QPointF(left+.006,top+.414)};
            s.subdivide(3);s.mesh[5]+=QPointF(.012,-.013);
            require(s.valid(),names[i]+" deformed geometry is valid");
            require(s.pattern==id&&s.patternSize==24&&s.patternSpeed==100&&!s.patternPlaying,names[i]+" native defaults");
            require(s.fx.density==potatoPatternDensity(id)&&s.fx.flow==potatoPatternDefaultFlow(id)&&s.fx.palette==0,names[i]+" density/flow/palette defaults");
            auto json=s.json();json.remove("fxDensity");json.remove("fxFlow");Surface restored;
            require(Surface::fromJson(json,{},restored)&&restored.pattern==id&&restored.fx.density==potatoPatternDensity(id)&&restored.fx.flow==potatoPatternDefaultFlow(id),names[i]+" omitted controls acquire defaults");
            defaults[i]=s;
        }
        if(!failures.isEmpty()){finish();return;}
        scene.touch(true);
        auto capture=[&]{
            auto image=window.output->grabFramebuffer();require(!image.isNull(),"actual seasonal output framebuffer");
            window.output->makeCurrent();auto *context=window.output->context();
            require(context&&context->isValid(),"seasonal OpenGL context");
            if(context&&context->isValid())require(context->functions()->glGetError()==GL_NO_ERROR,"seasonal rendering has no OpenGL error");
            window.output->doneCurrent();return image;
        };
        auto sample=[&](const QImage &image,int i){return PotatoHarvestCheck::samples(image,window.output->canvasRect(),window.output->size(),scene.surfaces[i]);};
        auto compare=[&](const QImage &a,const QImage &b,double tolerance,const QString &label){
            for(int i=0;i<4;++i)require(PotatoHarvestCheck::difference(sample(a,i),sample(b,i))<tolerance,names[i]+" "+label);
        };
        auto setTime=[&](double t){for(auto &s:scene.surfaces)s.patternPhase=t;};
        auto save=[&](const QImage &image,const QString &path){
            require(QDir().mkpath(QFileInfo(path).absolutePath()),"capture folder exists");
            require(image.save(path),"save framebuffer "+path);
        };
        setTime(0);const auto first=capture();setTime(8);const auto moving=capture();
        for(int i=0;i<4;++i){
            require(PotatoFallCheck::visible(sample(moving,i),.0005),names[i]+" visible");
            require(PotatoHarvestCheck::difference(sample(first,i),sample(moving,i))>.04,names[i]+" phase changes visible output");
        }
        for(double t:{0.0,6.5,-3.25}){
            setTime(t);const auto a=capture();setTime(t+24);compare(a,capture(),1.5,"24 second period");
        }
        for(double seam:{-24.0,0.0,24.0}){
            setTime(seam-.001);const auto a=capture();setTime(seam+.001);compare(a,capture(),1.5,"continuous loop seam");
        }
        for(int i=0;i<4;++i){
            auto &s=scene.surfaces[i];s.patternPhase=8;
            s.patternSize=16;const auto small=capture();s.patternSize=32;const auto big=capture();
            require(PotatoHarvestCheck::difference(sample(small,i),sample(big,i))>.015,names[i]+" Size changes content");
            s=defaults[i];s.patternPhase=8;s.fx.density=4;const auto sparse=capture();s.fx.density=80;const auto dense=capture();
            require(PotatoHarvestCheck::difference(sample(sparse,i),sample(dense,i))>.005,names[i]+" Density changes content");
            s=defaults[i];s.fx.flow=0;s.patternPhase=0;const auto still=capture();s.patternPhase=8;const auto basic=capture();
            require(PotatoHarvestCheck::difference(sample(still,i),sample(basic,i))>.015,names[i]+" Flow zero retains basic action");
            s.fx.flow=100;const auto lively=capture();
            require(PotatoHarvestCheck::difference(sample(basic,i),sample(lively,i))>.005,names[i]+" Flow changes expression or wind");
            s=defaults[i];
        }
        // A single snow field and the same field split into adjacent mapped
        // panels must agree. This catches per-surface random seeds/UV resets.
        for(auto &s:scene.surfaces)s.visible=false;
        auto &snow=scene.surfaces[0];snow=defaults[3];snow.visible=true;
        snow.corners={QPointF(.04,.04),QPointF(.96,.04),QPointF(.96,.96),QPointF(.04,.96)};snow.subdivide(1);snow.patternPhase=5.25;
        scene.touch(true);const Surface wholeSnow=snow;const auto whole=capture();
        snow.corners={QPointF(.04,.04),QPointF(.5,.04),QPointF(.5,.96),QPointF(.04,.96)};
        scene.surfaces[1]=wholeSnow;scene.surfaces[1].routeUid=QUuid::createUuid().toString(QUuid::WithoutBraces);
        scene.surfaces[1].corners={QPointF(.5,.04),QPointF(.96,.04),QPointF(.96,.96),QPointF(.5,.96)};
        scene.touch(true);const auto split=capture();
        require(PotatoHarvestCheck::difference(PotatoHarvestCheck::samples(whole,window.output->canvasRect(),window.output->size(),wholeSnow),PotatoHarvestCheck::samples(split,window.output->canvasRect(),window.output->size(),wholeSnow))<1.5,"snow positions remain coherent across adjacent panels");
        for(int i=0;i<4;++i)scene.surfaces[i]=defaults[i];setTime(8);scene.touch(true);
        window.canvas->meshMode=false;scene.select(-1);const auto output=capture(),editor=window.canvas->grabFramebuffer();
        for(int i=0;i<4;++i)require(PotatoHarvestCheck::difference(sample(output,i),PotatoHarvestCheck::samples(editor,window.canvas->canvasRect(),window.canvas->size(),scene.surfaces[i]))<6,names[i]+" editor/output agreement");
        QTemporaryDir temp;require(temp.isValid(),"disposable mapping folder");QString error;const auto saved=scene.json();
        if(scene.save(temp.filePath("seasonal.pmap"),error)){
            scene.newProject();require(scene.load(temp.filePath("seasonal.pmap"),error),"reopen seasonal mapping: "+error);
            require(scene.json()==saved,"new IDs and controls survive save/reopen");
        }else require(false,"save seasonal mapping: "+error);
        if(scene.surfaces.size()!=4){finish();return;}
        for(int i=0;i<4;++i)scene.surfaces[i]=defaults[i];setTime(8);scene.touch(true);
        if(!snapshot.isEmpty())save(capture(),snapshot);
        if(!frames.isEmpty())for(int frame=0;frame<96;++frame){
            setTime(frame*.25);save(capture(),QDir(frames).filePath(QString("%1.png").arg(frame,3,10,QChar('0'))));
        }
        if(!demo.isEmpty()){
            for(auto &s:scene.surfaces){s.patternPlaying=true;s.patternPhase=0;}
            require(QDir().mkpath(QFileInfo(demo).absolutePath()),"demo folder exists");
            require(scene.save(demo,error),"save playing seasonal demo: "+error);
        }
        finish();
    });
}
