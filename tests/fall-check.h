#pragma once
#include "harvest-check.h"
#include <QDebug>

namespace PotatoFallCheck {
inline bool visible(const QVector<QColor> &pixels,double fraction=.006){
    int lit=0;
    for(const auto &c:pixels)if(std::max({c.red(),c.green(),c.blue()})>30)++lit;
    return !pixels.isEmpty()&&lit>pixels.size()*fraction;
}
inline bool dark(const QVector<QColor> &pixels){
    return !pixels.isEmpty()&&std::all_of(pixels.begin(),pixels.end(),[](QColor c){return std::max({c.red(),c.green(),c.blue()})<=8;});
}
inline bool shaded(const QVector<QColor> &pixels){
    int low=255,high=0;
    for(const auto &c:pixels){
        int v=std::max({c.red(),c.green(),c.blue()});low=std::min(low,v);high=std::max(high,v);
    }
    return !pixels.isEmpty()&&high-low>35;
}
inline int changedPixels(const QImage &a,const QImage &b){
    if(a.isNull()||a.size()!=b.size())return 0;
    int changed=0;
    for(int y=0;y<a.height();++y)for(int x=0;x<a.width();++x){
        const auto ca=a.pixelColor(x,y),cb=b.pixelColor(x,y);
        if(std::max({std::abs(ca.red()-cb.red()),std::abs(ca.green()-cb.green()),std::abs(ca.blue()-cb.blue())})>3)++changed;
    }
    return changed;
}
inline bool gaps(const QImage &image,const QRectF &rect,const QSize &size){
    if(image.isNull()||size.isEmpty())return false;
    auto black=[&](double nx,double ny){
        int x=std::clamp(qRound((rect.left()+nx*rect.width())*image.width()/size.width()),0,image.width()-1);
        int y=std::clamp(qRound((rect.top()+ny*rect.height())*image.height()/size.height()),0,image.height()-1);
        const auto c=image.pixelColor(x,y);return std::max({c.red(),c.green(),c.blue()})<=8;
    };
    for(double x:{.01,.625,.99})for(int y=0;y<18;++y)if(!black(x,.04+y*.05))return false;
    for(int x=0;x<10;++x)if(!black(.06+x*.05,.50))return false;
    return true;
}
}

// Real UI and OpenGL output; all scenes and saved files are disposable.
template<class Window> void runFallChecks(Window &window,QApplication &app,QStringList args){
    window.output->resize(1200,675);window.output->show();
    QTimer::singleShot(650,&window,[&window,&app,args]{
        struct State {
            QStringList failures;QTemporaryDir temp;
            Surface defaults[3];QVector<QColor> moving[3];double phases[3]{};
            QJsonObject saved;QString originalName;
        };
        auto state=std::make_shared<State>();
        auto require=[state](bool ok,const QString &label){if(!ok)state->failures<<label;};
        auto finish=[state,&app]{
            qInfo().noquote()<<"FALL CHECK"<<(state->failures.isEmpty()?"PASS":"FAIL")<<state->failures.join("; ");
            app.exit(state->failures.isEmpty()?0:2);
        };
        auto option=[args,require](const QString &name){
            const int i=args.indexOf(name);if(i<0)return QString{};
            if(i+1>=args.size()||args[i+1].startsWith("--")){require(false,name+" needs a path");return QString{};}
            return args[i+1];
        };
        const auto frames=option("--frames"),snapshot=option("--snapshot"),demo=option("--demo");
        auto *picker=window.template findChild<QComboBox*>("animationPicker");
        auto *pause=window.template findChild<QCheckBox*>("patternPause");
        auto *reverse=window.template findChild<QCheckBox*>("patternReverse");
        auto *speed=window.template findChild<QSpinBox*>("patternSpeed");
        auto *size=window.template findChild<QSpinBox*>("patternSize");
        auto *density=window.template findChild<QSpinBox*>("patternDensity");
        auto *flow=window.template findChild<QSpinBox*>("patternFlow");
        require(picker&&pause&&reverse&&speed&&size&&density&&flow,"native fall animation controls are available");
        if(!picker||!pause||!reverse||!speed||!size||!density||!flow){finish();return;}
        const QStringList names{"Pumpkin Mischief","Ghost Parade","Rainlit Window"};
        for(int i=0;i<3;++i)require(picker->findText(names[i],Qt::MatchStartsWith)==16+i,
                                  names[i]+QString(" is missing or not selectable with stable ID %1").arg(16+i));
        // Fail before selection, rather than silently selecting an older ID.
        if(!state->failures.isEmpty()){finish();return;}
        require(state->temp.isValid(),"disposable fall project folder");
        require(window.output->graphicsReady&&window.output->graphicsError.isEmpty(),"output shader: "+window.output->graphicsError);
        require(window.canvas->graphicsReady&&window.canvas->graphicsError.isEmpty(),"editor shader: "+window.canvas->graphicsError);
        if(!state->failures.isEmpty()){finish();return;}
        auto &scene=window.scene;
        scene.newProject();scene.outputSize=QSize(1200,675);scene.add();scene.add();
        const int densities[3]{24,32,24},flows[3]{60,60,50};
        const QPolygonF outlines[3]{
            {QPointF(.04,.06),QPointF(.585,.08),QPointF(.575,.46),QPointF(.055,.43)},
            {QPointF(.65,.07),QPointF(.96,.04),QPointF(.955,.935),QPointF(.66,.95)},
            {QPointF(.035,.56),QPointF(.58,.535),QPointF(.60,.93),QPointF(.07,.965)}};
        for(int i=0;i<3;++i){
            scene.select(i);picker->setCurrentIndex(16+i);
            require(scene.surfaces[i].patternPlaying,names[i]+" starts playing when selected");
            pause->setChecked(true);auto &s=scene.surfaces[i];
            s.name=names[i];s.corners=outlines[i];s.subdivide(3);
            s.mesh[5]+=QPointF(.018,-.020);s.mesh[10]+=QPointF(-.018,.022);s.patternPhase=0;
            require(s.valid(),names[i]+" has valid irregular deformed geometry");
            require(s.pattern==16+i&&!s.patternPlaying&&s.patternSize==24&&s.patternSpeed==100,
                    names[i]+" picker retains speed/size defaults while paused");
            require(s.fx.density==densities[i]&&s.fx.flow==flows[i]&&s.fx.palette==0&&!s.fx.reverse,
                    names[i]+" supplies its specified Density/Flow/Original defaults");state->defaults[i]=s;
            auto old=s.json();Surface restored;
            old.remove("fxDensity");old.remove("fxFlow");old.remove("fxPalette");
            require(Surface::fromJson(old,{},restored)&&restored.pattern==16+i&&restored.fx.density==densities[i]&&restored.fx.flow==flows[i]&&restored.fx.palette==0,
                    names[i]+" missing JSON controls acquire seasonal defaults");
            old["fxFlow"]=17;
            require(Surface::fromJson(old,{},restored)&&restored.fx.flow==17,
                    names[i]+" preserves an explicitly saved Flow 17");
        }
        for(int pattern=0;pattern<=15;++pattern){
            auto legacy=state->defaults[0].json();legacy["pattern"]=pattern;legacy.remove("fxFlow");Surface restored;
            require(Surface::fromJson(legacy,{},restored)&&restored.pattern==pattern&&restored.fx.flow==70,
                    QString("legacy preset %1 retains missing Flow default 70").arg(pattern));
            legacy["fxFlow"]=17;
            require(Surface::fromJson(legacy,{},restored)&&restored.fx.flow==17,
                    QString("legacy preset %1 preserves explicit Flow 17").arg(pattern));
        }
        scene.touch(true);
        auto capture=[&window,require]{
            auto image=window.output->grabFramebuffer();require(!image.isNull(),"read actual fall framebuffer");
            window.output->makeCurrent();auto *context=window.output->context();
            require(context&&context->isValid(),"fall output OpenGL context is valid");
            if(context&&context->isValid()){auto error=context->functions()->glGetError();require(error==GL_NO_ERROR,QString("fall framebuffer GL error 0x%1").arg(error,0,16));}
            window.output->doneCurrent();return image;
        };
        auto sample=[&window](const QImage &image,int i){
            return PotatoHarvestCheck::samples(image,window.output->canvasRect(),window.output->size(),window.scene.surfaces[i]);
        };
        auto compare=[sample,require](const QImage &a,const QImage &b,double limit,const QString &label){
            for(int i=0;i<3;++i)require(PotatoHarvestCheck::difference(sample(a,i),sample(b,i))<limit,label+QString(" preset %1").arg(16+i));
        };
        auto saveFrame=[require](const QImage &image,const QString &path){
            require(QDir().mkpath(QFileInfo(path).absolutePath()),"capture destination exists");
            require(!image.isNull()&&image.scaled(QSize(1200,675),Qt::IgnoreAspectRatio,Qt::SmoothTransformation).save(path),"save framebuffer "+path);
        };
        const auto first=capture();
        for(int i=0;i<3;++i)require(PotatoFallCheck::visible(sample(first,i)),names[i]+" is visible in its own panel");
        for(auto &s:scene.surfaces)s.patternPhase=3;
        const auto later=capture();
        for(int i=0;i<3;++i)require(PotatoHarvestCheck::difference(sample(first,i),sample(later,i))>.20,names[i]+" independently moves between phases 0 and 3");
        require(PotatoFallCheck::gaps(later,window.output->canvasRect(),window.output->size()),"outside edges and both mapped gaps stay black");
        for(double t:{0.0,2.375,-2.375}){
            for(auto &s:scene.surfaces)s.patternPhase=t;
            const auto a=capture();
            for(auto &s:scene.surfaces)s.patternPhase=t+12.0;
            compare(a,capture(),1.5,"t and t+12 agree");
        }
        for(double sign:{1.0,-1.0}){
            for(auto &s:scene.surfaces)s.patternPhase=sign*4;
            const auto a=capture();
            for(auto &s:scene.surfaces)s.patternPhase=sign*10000;
            compare(a,capture(),1.5,"long-running seasonal phase agrees");
        }
        for(double seam:{-12.0,0.0,12.0}){
            for(auto &s:scene.surfaces)s.patternPhase=seam-.001;
            const auto a=capture();
            for(auto &s:scene.surfaces)s.patternPhase=seam+.001;
            compare(a,capture(),1.5,"nearby loop-seam frames remain continuous");
        }
        // Corner-only editing leaves labels and handles outside these interior samples.
        window.canvas->meshMode=false;scene.select(-1);for(auto &s:scene.surfaces)s.patternPhase=2.7;
        const auto output=capture(),editor=window.canvas->grabFramebuffer();
        for(int i=0;i<3;++i)require(PotatoHarvestCheck::difference(sample(output,i),PotatoHarvestCheck::samples(editor,window.canvas->canvasRect(),window.canvas->size(),scene.surfaces[i]))<6,
                                  names[i]+" editor/output interiors agree without handles");
        for(int i=0;i<3;++i){
            scene.select(i);auto &s=scene.surfaces[i];s.patternPhase=2.7;
            size->setValue(18);const auto small=capture();size->setValue(32);const auto large=capture();
            require(PotatoHarvestCheck::difference(sample(small,i),sample(large,i))>.03,names[i]+" Size changes illustrated content");size->setValue(24);
            // Pumpkin Density changes emitted embers; sample during its hiccup,
            // while the parade and rain have active density content throughout.
            s.patternPhase=i==0?4.5:2.7;
            bool visibility[3];for(int k=0;k<3;++k)visibility[k]=scene.surfaces[k].visible;
            if(i==0){for(int k=1;k<3;++k)scene.surfaces[k].visible=false;scene.touch(true);}
            density->setValue(4);const auto sparse=capture();density->setValue(80);const auto dense=capture();
            // Tiny embers can lie between the regular interior sample points.
            // Compare the isolated paused panel's actual pixels, retaining a
            // visible channel-change threshold rather than diluting its art.
            require(i==0?PotatoFallCheck::changedPixels(sparse,dense)>=8:
                         PotatoHarvestCheck::difference(sample(sparse,i),sample(dense,i))>.03,
                    names[i]+" Density changes content spacing/count");
            if(i==0){for(int k=0;k<3;++k)scene.surfaces[k].visible=visibility[k];scene.touch(true);}
            density->setValue(densities[i]);
            flow->setValue(0);s.patternPhase=0;const auto basicStart=capture();s.patternPhase=3;const auto basicLater=capture();
            require(PotatoHarvestCheck::difference(sample(basicStart,i),sample(basicLater,i))>.03,names[i]+" Flow 0 retains basic phase motion");
            s.patternPhase=2.7;const auto calm=capture();flow->setValue(100);const auto lively=capture();
            require(PotatoHarvestCheck::difference(sample(calm,i),sample(lively,i))>.03,names[i]+" Flow changes expressive content");flow->setValue(flows[i]);
            const auto before=scene.json();density->setValue(47);const auto changed=scene.json();
            scene.undo();require(scene.json()==before,names[i]+" density UI supports one Undo");
            scene.redo();require(scene.json()==changed,names[i]+" density UI supports Redo");
            scene.select(i);density->setValue(densities[i]);
            // Undo restores the surface vector; acquire its new instance here.
            auto &controlled=scene.surfaces[i];
            struct Corner {int size,density,flow,angle,palette,edge;};
            const Corner corners[]{{5,4,0,-180,0,0},{45,80,100,180,5,25},{24,24,60,37,1,12},{18,44,15,-37,2,0},{32,60,85,0,3,12},{24,32,50,0,4,0}};
            for(const auto &c:corners){
                size->setValue(c.size);auto look=controlled.fx;
                look.density=c.density;look.flow=c.flow;look.angle=c.angle;look.palette=c.palette;look.edge=c.edge;
                scene.setFxLook(i,look);const auto pixels=sample(capture(),i);
                require(PotatoFallCheck::visible(pixels,0.0),names[i]+QString(" renders representative control corner palette %1").arg(c.palette));
                if(c.palette!=0)require(PotatoFallCheck::shaded(pixels),names[i]+QString(" palette %1 retains shaded detail").arg(c.palette));
            }
            controlled=state->defaults[i];controlled.patternPhase=2.7;scene.touch(true);scene.select(i);const auto full=sample(capture(),i);
            scene.setAppearance(i,50,100);const auto dim=sample(capture(),i);QVector<QColor> expected;
            for(const auto &c:full)expected.append(QColor(qRound(c.red()*.5),qRound(c.green()*.5),qRound(c.blue()*.5)));
            require(PotatoHarvestCheck::difference(expected,dim)<2,names[i]+" 50% brightness dims native artwork");
            scene.setAppearance(i,100,50);require(PotatoHarvestCheck::difference(expected,sample(capture(),i))<2,names[i]+" 50% opacity fades native artwork over black");
            scene.setAppearance(i,0,100);require(PotatoFallCheck::dark(sample(capture(),i)),names[i]+" zero brightness is black");
            scene.setAppearance(i,100,0);require(PotatoFallCheck::dark(sample(capture(),i)),names[i]+" zero opacity is black");scene.setAppearance(i,100,100);
        }
        // Exercise geometry and save optional evidence separately from the 96 loop frames.
        const QPolygonF shapes[]{
            {QPointF(.265,.08),QPointF(.735,.08),QPointF(.735,.92),QPointF(.265,.92)},
            {QPointF(.08,.25),QPointF(.92,.25),QPointF(.92,.75),QPointF(.08,.75)},
            {QPointF(.37,.04),QPointF(.63,.04),QPointF(.63,.96),QPointF(.37,.96)}};
        const QStringList shapeNames{"square","wide","tall"};
        for(int i=0;i<3;++i)for(int j=0;j<3;++j){
            for(int k=0;k<3;++k)scene.surfaces[k].visible=k==i;
            auto &s=scene.surfaces[i];s=state->defaults[i];s.corners=shapes[j];s.subdivide(1);s.patternPhase=3;scene.touch(true);
            require(s.valid(),names[i]+" "+shapeNames[j]+" geometry is valid");const auto image=capture();
            require(PotatoFallCheck::visible(sample(image,i)),names[i]+" remains visible on "+shapeNames[j]+" panel");
            if(!frames.isEmpty())saveFrame(image,QDir(frames+"-shapes").filePath(QString("%1-%2.png").arg(16+i).arg(shapeNames[j])));
        }
        for(int i=0;i<3;++i)scene.surfaces[i]=state->defaults[i];scene.touch(true);
        // Use distinctive real UI settings, then verify version-1 persistence.
        for(int i=0;i<3;++i){
            scene.select(i);size->setValue(29+i);speed->setValue(83+i*17);auto look=scene.surfaces[i].fx;
            look.flow=41+i*13;look.angle=13-i*7;look.edge=4+i*2;look.reverse=i==1;scene.setFxLook(i,look);
        }
        QString error;const auto saved=scene.json();const auto path=state->temp.filePath("fall.pmap");
        bool reopened=scene.save(path,error);require(reopened,"save native fall mapping: "+error);
        if(reopened){scene.newProject();reopened=scene.load(path,error);require(reopened,"reopen native fall mapping: "+error);}
        if(!reopened){finish();return;}require(scene.json()==saved,"new IDs, controls and deformed mapping survive save/reopen");
        for(int i=0;i<3;++i){
            scene.select(i);require(picker->currentIndex()==16+i,names[i]+" reopened picker selection");
            scene.surfaces[i]=state->defaults[i];scene.select(i);pause->setChecked(false);
            scene.surfaces[i].patternPhase=1.8;state->phases[i]=1.8;
        }
        scene.touch(true);state->originalName=scene.surfaces[0].name;
        scene.renameSurface(0,"Fall tick history sentinel");scene.dirty=false;state->saved=scene.json();
        const auto playing=capture();
        for(int i=0;i<3;++i)state->moving[i]=sample(playing,i);
        QTimer::singleShot(240,&window,[&window,state,require,finish,pause,reverse,speed,capture,sample,saveFrame,frames,snapshot,demo]{
            auto &scene=window.scene;const auto moving=capture();require(!scene.dirty&&scene.json()==state->saved,"animation ticks preserve clean settings");
            for(int i=0;i<3;++i)require(scene.surfaces[i].patternPhase>state->phases[i]&&PotatoHarvestCheck::difference(state->moving[i],sample(moving,i))>.025,QString("preset %1 advances in timed output").arg(16+i));
            scene.undo();require(scene.surfaces[0].name==state->originalName,"ticks do not insert Undo records before last user edit");
            scene.redo();require(scene.json()==state->saved,"Redo restores the last user edit after ticks");
            for(int i=0;i<3;++i){scene.select(i);pause->setChecked(true);state->phases[i]=scene.surfaces[i].patternPhase;}
            const auto frozen=capture();
            QTimer::singleShot(90,&window,[&window,state,require,finish,pause,reverse,speed,capture,sample,saveFrame,frames,snapshot,demo,frozen]{
                auto &scene=window.scene;const auto paused=capture();
                for(int i=0;i<3;++i){
                    require(scene.surfaces[i].patternPhase==state->phases[i]&&PotatoHarvestCheck::difference(sample(frozen,i),sample(paused,i))<.01,QString("preset %1 freezes with Pause").arg(16+i));
                    scene.select(i);reverse->setChecked(true);
                }
                const auto reversed=capture();
                for(int i=0;i<3;++i)require(PotatoHarvestCheck::difference(sample(paused,i),sample(reversed,i))<.01,QString("preset %1 paused Reverse preserves pixels").arg(16+i));
                for(int i=0;i<3;++i){scene.select(i);pause->setChecked(false);state->phases[i]=scene.surfaces[i].patternPhase;}
                QTimer::singleShot(120,&window,[&window,state,require,finish,pause,speed,capture,sample,saveFrame,frames,snapshot,demo,reversed]{
                    auto &scene=window.scene;const auto reverseMoving=capture();
                    for(int i=0;i<3;++i){
                        require(scene.surfaces[i].patternPhase<state->phases[i],QString("preset %1 resumes backwards after Reverse").arg(16+i));
                        require(PotatoHarvestCheck::difference(sample(reversed,i),sample(reverseMoving,i))>.025,
                                QString("preset %1 visibly moves after Reverse resumes").arg(16+i));
                        scene.select(i);speed->setValue(0);state->phases[i]=scene.surfaces[i].patternPhase;
                    }
                    const auto zero=capture();scene.dirty=false;state->saved=scene.json();
                    QTimer::singleShot(90,&window,[&window,state,require,finish,capture,sample,saveFrame,frames,snapshot,demo,zero]{
                        auto &scene=window.scene;const auto stopped=capture();require(!scene.dirty&&scene.json()==state->saved,"Speed 0 keeps settings clean");
                        for(int i=0;i<3;++i){
                            require(scene.surfaces[i].patternPlaying&&scene.surfaces[i].patternPhase==state->phases[i]&&PotatoHarvestCheck::difference(sample(zero,i),sample(stopped,i))<.01,QString("preset %1 Speed 0 freezes while playing").arg(16+i));
                            scene.surfaces[i]=state->defaults[i];scene.surfaces[i].patternPhase=3;
                        }
                        scene.touch(true);if(!snapshot.isEmpty())saveFrame(capture(),snapshot);
                        if(!frames.isEmpty())for(int frame=0;frame<96;++frame){
                            for(auto &s:scene.surfaces)s.patternPhase=frame*.125;
                            saveFrame(capture(),QDir(frames).filePath(QString("%1.png").arg(frame,3,10,QChar('0'))));
                        }
                        if(!demo.isEmpty()){
                            for(auto &s:scene.surfaces){s.patternPlaying=true;s.patternPhase=0;}
                            require(QDir().mkpath(QFileInfo(demo).absolutePath()),"demo folder exists");
                            QString error;require(scene.save(demo,error),"save native playing demo: "+error);
                        }
                        finish();
                    });
                });
            });
        });
    });
}
