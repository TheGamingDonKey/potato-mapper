#pragma once
#include "fall-check.h"
#include <QAction>
#include <QEventLoop>

// Opens the shipped examples and captures real mapped OpenGL frames in both windows.
template<class Window> void runChristmasChecks(Window &window,QApplication &app,const QStringList &args){
    window.output->resize(960,540);
    window.output->show();
    QTimer::singleShot(650,&window,[&window,&app,args]{
        QStringList failures;
        int checks=0;
        auto require=[&](bool ok,const QString &label){++checks;if(!ok)failures<<label;};
        auto finish=[&]{
            qInfo().noquote()<<"CHRISTMAS CHECK"<<(failures.isEmpty()?"PASS":"FAIL")<<checks<<"checks"<<failures.join("; ");
            app.exit(failures.isEmpty()?0:2);
        };
        auto option=[&](const QString &key,QString fallback=QString()){
            int pos=args.indexOf(key);
            if(pos<0)return fallback;
            if(pos+1>=args.size()||args[pos+1].startsWith("--")){require(false,key+" requires an argument");return QString();}
            return args[pos+1];
        };
        const QDir examples(option("--christmas-examples",QDir(QCoreApplication::applicationDirPath()).filePath("examples")));
        const QString stills=option("--christmas-stills"),frames=option("--christmas-motion");
        const QString single=option("--christmas-pattern");
        bool parsed=false;
        const int filmPattern=single.toInt(&parsed);
        if(!single.isEmpty())require(parsed&&filmPattern>=23&&filmPattern<=26&&!frames.isEmpty(),"single-scene motion needs an ID from 23 to 26 and a capture directory");
        const int framesPerSecond=option("--christmas-fps","3").toInt(&parsed);
        require(parsed&&framesPerSecond>=1&&framesPerSecond<=24,"frame sampling rate is between 1 and 24");
        auto *picker=window.template findChild<QComboBox*>("animationPicker");
        auto *pause=window.template findChild<QCheckBox*>("patternPause");
        require(picker&&pause,"editor offers Animation and Pause");
        require(window.canvas->graphicsReady&&window.canvas->graphicsError.isEmpty(),"editor OpenGL: "+window.canvas->graphicsError);
        require(window.output->graphicsReady&&window.output->graphicsError.isEmpty(),"output OpenGL: "+window.output->graphicsError);
        for(const auto &dir:{stills,frames})if(!dir.isEmpty())require(QDir().mkpath(dir),"capture directory "+dir);
        if(!failures.isEmpty()){finish();return;}

        const QStringList names{"North Pole Express","Evergreen Overture","Clocktower Carols","Nutcracker Waltz"};
        const double coverTime[]{8.0,13.25,8.0,3.0};
        const QPolygonF full{{0,0},{1,0},{1,1},{0,1}};
        auto &scene=window.scene;
        QVector<Surface> surfaces;
        auto output=[&]{return window.output->grabFramebuffer();};
        auto capturePng=[&](const QImage &image,const QString &folder,const QString &name,QSize size=QSize(960,540)){
            const auto path=QDir(folder).filePath(name+".png");
            require(!image.isNull()&&image.scaled(size,Qt::IgnoreAspectRatio,Qt::SmoothTransformation).save(path),"save "+path);
        };
        QString error;
        const auto menuActions=window.template findChildren<QAction*>();
        for(int i=0;i<names.size();++i){
            const int id=23+i;const QString &name=names[i];
            require(std::any_of(menuActions.cbegin(),menuActions.cend(),[&](QAction *a){return a->text()==name;}),name+" Examples menu entry");
            require(QFile::exists(QString(":/assets/thumbnails/pattern-%1.png").arg(id)),name+" embedded library image");
            const int entry=picker->findData(id);
            require(entry>=0&&picker->itemText(entry).startsWith(name)&&PotatoAppearance::category(id)=="christmas",name+" stable Christmas library ID");
            if(!scene.load(examples.filePath(name+".pmap"),error)){require(false,name+" example load: "+error);continue;}
            require(scene.surfaces.size()==1&&scene.current(),name+" opens with its editable surface");
            if(!scene.current())continue;
            auto &s=*scene.current();
            require(s.pattern==id&&s.patternPlaying&&s.media.isEmpty()&&!s.locked&&s.valid(),name+" plays without media");
            require(s.patternSize==potatoPatternDefaultSize(id)&&s.fx.density==potatoPatternDensity(id)&&s.fx.flow==potatoPatternDefaultFlow(id),name+" example matches picker defaults");
            const auto corners=s.corners;
            scene.setPattern(0,0);picker->setCurrentIndex(entry);pause->setChecked(true);
            require(s.pattern==id&&!s.patternPlaying&&s.corners==corners,name+" re-applies and pauses through the inspector");
            surfaces<<s;
            if(!stills.isEmpty()){
                s.corners=full;s.subdivide(1);s.patternPhase=coverTime[i];scene.touch(true);
                capturePng(output(),stills,name.toLower().replace(' ','-'));
            }
        }
        if(!failures.isEmpty()||surfaces.size()!=4){finish();return;}

        scene.newProject();
        scene.outputSize={960,540};
        scene.surfaces=surfaces;
        for(int i=0;i<4;++i){
            auto &s=scene.surfaces[i];
            double x=(i%2)*.5+.018,y=(i/2)*.5+.026;
            s.corners={{x,y+.007},{x+.463,y},{x+.456,y+.441},{x+.006,y+.451}};
            s.subdivide(3);s.mesh[5]+=QPointF(.013,-.010);
            require(s.valid(),names[i]+" valid deformed mesh");
        }
        scene.select(-1);scene.touch(true);
        auto time=[&](double t){for(auto &s:scene.surfaces)s.patternPhase=t;};
        auto pixels=[&](const QImage &image,int i,Canvas *view){return PotatoHarvestCheck::samples(image,view->canvasRect(),view->size(),scene.surfaces[i]);};
        auto difference=[&](const QImage &a,const QImage &b,int i){return PotatoHarvestCheck::difference(pixels(a,i,window.output),pixels(b,i,window.output));};
        time(8.0);const auto mapped=output(),editor=window.canvas->grabFramebuffer();
        require(!mapped.isNull()&&!editor.isNull(),"actual editor and projector framebuffers");
        require(PotatoHarvestCheck::darkGaps(mapped,window.output->canvasRect(),window.output->size()),"space between the projected panels is black");
        require(output()==mapped,"frozen mapped output is repeatable");
        for(int i=0;i<4;++i){
            require(PotatoFallCheck::shaded(pixels(mapped,i,window.output)),names[i]+" visible illustration");
            require(PotatoHarvestCheck::difference(pixels(mapped,i,window.output),pixels(editor,i,window.canvas))<7.0,names[i]+" editor/output agree on the deformed interior");
        }
        time(2.1);const auto other=output();
        for(int i=0;i<4;++i)require(difference(mapped,other,i)>.38,names[i]+" time changes the projected image");
        for(double start:{-3.25,5.125}){
            time(start);const auto before=output();time(start+24.0);const auto after=output();
            for(int i=0;i<4;++i)require(difference(before,after,i)<.35,names[i]+" exact 24-second period including reverse time");
        }
        time(-.0001);const auto preSeam=output();time(.0001);const auto postSeam=output();
        for(int i=0;i<4;++i)require(difference(preSeam,postSeam,i)<.75,names[i]+" continuous loop seam");
        for(int i=0;i<4;++i){
            auto &s=scene.surfaces[i];s.patternPhase=8.1;
            const auto prior=s;
            s.fx.density=4;const auto sparse=output();s.fx.density=80;
            require(difference(sparse,output(),i)>.01,names[i]+" density changes the scene");
            s=prior;s.fx.flow=0;const auto quiet=output();s.fx.flow=100;
            require(difference(quiet,output(),i)>.01,names[i]+" expressive motion control changes the scene");
            s=prior;s.patternSize=15;const auto small=output();s.patternSize=34;
            require(difference(small,output(),i)>.25,names[i]+" scene size changes the subject");
            s=prior;
        }
        // Exercise the ordinary Scene clock with all four surfaces in reverse.
        for(auto &s:scene.surfaces){s.patternPhase=8;s.fx.reverse=true;s.patternPlaying=true;}
        scene.dirty=false;const auto playing=scene.json();
        QEventLoop beat;QTimer::singleShot(180,&beat,&QEventLoop::quit);beat.exec();
        for(int i=0;i<4;++i)require(scene.surfaces[i].patternPhase<8.0,names[i]+" runs backwards");
        require(!scene.dirty&&scene.json()==playing,"animation ticks preserve the saved project");
        for(auto &s:scene.surfaces)s.patternPlaying=false;
        const auto frozen=output();QTimer::singleShot(90,&beat,&QEventLoop::quit);beat.exec();
        require(output()==frozen,"paused surfaces hold their pixels across clock ticks");
        QTemporaryDir temp;
        const auto project=scene.json();
        require(temp.isValid()&&scene.save(temp.filePath("christmas.pmap"),error)&&scene.load(temp.filePath("christmas.pmap"),error)&&scene.json()==project,"save/reopen retains geometry, four IDs and controls: "+error);
        if(scene.surfaces.size()!=4){finish();return;}
        for(int i=0;i<4;++i)scene.surfaces[i].patternPhase=coverTime[i];
        auto contact=output();
        if(!stills.isEmpty())capturePng(contact,stills,"christmas-mapped");
        if(!frames.isEmpty()){
            if(!single.isEmpty()){
                auto subject=surfaces[filmPattern-23];subject.corners=full;subject.subdivide(1);
                scene.surfaces={subject};scene.select(-1);scene.touch(true);
            }
            for(int frame=0;frame<24*framesPerSecond;++frame){
                time(double(frame)/framesPerSecond);
                capturePng(output(),frames,QString("%1").arg(frame,3,10,QChar('0')),QSize(640,360));
            }
        }
        finish();
    });
}
