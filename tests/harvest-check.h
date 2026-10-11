#pragma once
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QJsonArray>
#include <QJsonObject>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QSpinBox>
#include <QTemporaryDir>
#include <QTimer>
#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <memory>

namespace PotatoHarvestCheck {
// Sample inside the actual deformed mapping, so another panel's motion cannot
// make a static preset pass. No exact images or silhouette guesses are used.
inline QVector<QColor> samples(const QImage &image,const QRectF &rect,
                              const QSize &logicalSize,const Surface &surface){
    QVector<QColor> result;
    if(image.isNull()||logicalSize.isEmpty())return result;
    const auto transform=surface.transform();
    for(int y=0;y<40;++y)for(int x=0;x<64;++x){
        const auto point=transform.map(surface.sample(.06+.88*(x+.5)/64,.06+.88*(y+.5)/40));
        const int px=std::clamp(qRound((rect.left()+point.x()*rect.width())*image.width()/logicalSize.width()),0,image.width()-1);
        const int py=std::clamp(qRound((rect.top()+point.y()*rect.height())*image.height()/logicalSize.height()),0,image.height()-1);
        result.append(image.pixelColor(px,py));
    }
    return result;
}
inline double difference(const QVector<QColor> &a,const QVector<QColor> &b){
    if(a.isEmpty()||a.size()!=b.size())return std::numeric_limits<double>::infinity();
    double total=0;
    for(int i=0;i<a.size();++i)total+=std::abs(a[i].red()-b[i].red())+std::abs(a[i].green()-b[i].green())+std::abs(a[i].blue()-b[i].blue());
    return total/(3*a.size());
}
inline bool warmVisible(const QVector<QColor> &pixels){
    int lit=0,warm=0;
    for(const auto &c:pixels)if(std::max({c.red(),c.green(),c.blue()})>40){
        ++lit;if(c.red()>c.green()+12&&c.green()>c.blue()+8)++warm;
    }
    return lit>pixels.size()*.02&&warm>lit*.30;
}
inline bool darkGaps(const QImage &image,const QRectF &rect,const QSize &logicalSize){
    if(image.isNull()||logicalSize.isEmpty())return false;
    // Both mapped outlines stay away from x=.01 and from the central x=.50 gap.
    for(double nx:{.01,.50})for(int i=0;i<18;++i){
        const double ny=.06+i*.05;
        const int x=std::clamp(qRound((rect.left()+nx*rect.width())*image.width()/logicalSize.width()),0,image.width()-1);
        const int y=std::clamp(qRound((rect.top()+ny*rect.height())*image.height()/logicalSize.height()),0,image.height()-1);
        const auto c=image.pixelColor(x,y);if(std::max({c.red(),c.green(),c.blue()})>8)return false;
    }
    return true;
}
}

// Disposable scenes only. Root's CLI owns startup and dispatch to this fixture.
template<class Window> void runHarvestChecks(Window &window,QApplication &app,QStringList args){
    window.output->resize(1200,675);window.output->show();
    QTimer::singleShot(650,&window,[&window,&app,args]{
        struct State {
            QStringList failures;
            QTemporaryDir temp;
            QVector<QColor> start[2];
            QVector<QColor> moving[2];
            double phases[2]{};
            FxLook defaultLook[2];
            int defaultSize[2]{};
            QJsonObject saved;
        };
        auto state=std::make_shared<State>();
        auto require=[state](bool ok,const QString &label){if(!ok)state->failures<<label;};
        auto finish=[state,&app]{
            qInfo().noquote()<<"HARVEST CHECK"<<(state->failures.isEmpty()?"PASS":"FAIL")<<state->failures.join("; ");
            app.exit(state->failures.isEmpty()?0:2);
        };
        auto option=[args,require](const QString &name){
            const int index=args.indexOf(name);if(index<0)return QString{};
            if(index+1>=args.size()||args[index+1].startsWith("--")){require(false,name+" needs a path");return QString{};}
            return args[index+1];
        };
        const auto frames=option("--frames"),snapshot=option("--snapshot"),demo=option("--demo");
        auto *picker=window.template findChild<QComboBox*>("animationPicker");
        auto *pause=window.template findChild<QCheckBox*>("patternPause");
        auto *speed=window.template findChild<QSpinBox*>("patternSpeed");
        auto *size=window.template findChild<QSpinBox*>("patternSize");
        require(picker&&pause&&speed&&size,"native animation controls are available");
        require(state->temp.isValid(),"disposable project folder");
        require(window.output->graphicsReady&&window.output->graphicsError.isEmpty(),"output shader: "+window.output->graphicsError);
        require(window.canvas->graphicsReady&&window.canvas->graphicsError.isEmpty(),"editor shader: "+window.canvas->graphicsError);
        if(!picker||!pause||!speed||!size){finish();return;}
        require(picker->findText("Maple Supper",Qt::MatchStartsWith)==14,"Maple Supper is selectable with stable ID 14");
        require(picker->findText("Harvest Lanterns",Qt::MatchStartsWith)==15,"Harvest Lanterns is selectable with stable ID 15");
        if(!state->failures.isEmpty()){finish();return;}

        auto &scene=window.scene;scene.newProject();scene.outputSize=QSize(1200,675);
        scene.add();
        for(int i=0;i<2;++i){
            scene.select(i);picker->setCurrentIndex(14+i);pause->setChecked(true);
            auto &s=scene.surfaces[i];
            s.name=i==0?"Maple Supper - harvest table":"Harvest Lanterns - drifting light";
            s.corners=i==0?QPolygonF{QPointF(.03,.07),QPointF(.485,.03),QPointF(.465,.93),QPointF(.06,.97)}:
                           QPolygonF{QPointF(.525,.05),QPointF(.96,.12),QPointF(.975,.94),QPointF(.54,.90)};
            s.subdivide(3);s.mesh[5]+=QPointF(.035,-.025);s.mesh[10]+=QPointF(-.025,.035);
            s.patternPhase=0;
            require(s.valid(),QString("irregular deformed panel %1").arg(i+1));
            require(s.pattern==14+i&&!s.patternPlaying,QString("picker and pause select preset %1").arg(14+i));
            require(s.fx.palette==0,QString("preset %1 starts with its warm original palette").arg(14+i));
            state->defaultLook[i]=s.fx;state->defaultSize[i]=s.patternSize;
        }
        scene.select(0);scene.touch(true);
        auto capture=[&window,require]{
            auto image=window.output->grabFramebuffer();
            require(!image.isNull(),"actual output framebuffer is readable");
            require(window.output->graphicsReady&&window.output->graphicsError.isEmpty(),"rendered output has no shader initialization error");
            window.output->makeCurrent();
            auto *context=window.output->context();
            require(context&&context->isValid(),"output OpenGL context is valid");
            if(context&&context->isValid()){
                const auto error=context->functions()->glGetError();
                require(error==GL_NO_ERROR,QString("framebuffer OpenGL error 0x%1").arg(error,0,16));
            }
            window.output->doneCurrent();return image;
        };
        auto sample=[&window](const QImage &image,int i){
            return PotatoHarvestCheck::samples(image,window.output->canvasRect(),window.output->size(),window.scene.surfaces[i]);
        };
        const auto start=capture();
        for(int i=0;i<2;++i){
            state->start[i]=sample(start,i);
            require(PotatoHarvestCheck::warmVisible(state->start[i]),QString("preset %1 renders visible warm amber by default").arg(14+i));
            scene.surfaces[i].patternPhase=3;
        }
        const auto later=capture();
        for(int i=0;i<2;++i){
            require(PotatoHarvestCheck::difference(state->start[i],sample(later,i))>.75,QString("preset %1 moves between phases 0 and 3").arg(14+i));
            scene.surfaces[i].patternPhase=12;
        }
        const auto seam=capture();
        for(int i=0;i<2;++i)require(PotatoHarvestCheck::difference(state->start[i],sample(seam,i))<1.5,QString("preset %1 joins its 12-second loop seam").arg(14+i));
        // The legacy renderer wraps its uniform at 10000 seconds, which is
        // not a whole number of seasonal loops. Check both clock directions.
        for(double direction:{1.0,-1.0}){
            for(auto &s:scene.surfaces)s.patternPhase=direction*4;
            const auto reference=capture();
            for(auto &s:scene.surfaces)s.patternPhase=direction*10000;
            const auto longRunning=capture();
            for(int i=0;i<2;++i)require(PotatoHarvestCheck::difference(sample(reference,i),sample(longRunning,i))<1.5,
                    QString("preset %1 keeps its seasonal loop after long %2 playback").arg(14+i).arg(direction>0?"forward":"reverse"));
        }
        require(PotatoHarvestCheck::darkGaps(later,window.output->canvasRect(),window.output->size()),"mapped panels leave their outside edges and central gap dark");
        for(int i=0;i<2;++i){scene.select(i);pause->setChecked(false);scene.surfaces[i].patternPhase=3;state->phases[i]=3;}
        scene.dirty=false;state->saved=scene.json();
        const auto playing=capture();for(int i=0;i<2;++i)state->moving[i]=sample(playing,i);
        QTimer::singleShot(260,&window,[&window,&app,state,require,finish,picker,pause,speed,size,capture,sample,frames,snapshot,demo]{
            auto &scene=window.scene;const auto advanced=capture();
            require(!scene.dirty&&scene.json()==state->saved,"timed ticks keep the project clean and settings unchanged");
            for(int i=0;i<2;++i){
                require(scene.surfaces[i].patternPhase>state->phases[i]&&PotatoHarvestCheck::difference(state->moving[i],sample(advanced,i))>.05,
                        QString("preset %1 advances in the actual timed output").arg(14+i));
                scene.select(i);pause->setChecked(true);state->phases[i]=scene.surfaces[i].patternPhase;
            }
            const auto frozen=capture();
                QTimer::singleShot(90,&window,[&window,state,require,finish,picker,pause,speed,size,capture,sample,frames,snapshot,demo,frozen]{
                    auto &scene=window.scene;const auto paused=capture();
                    for(int i=0;i<2;++i)require(scene.surfaces[i].patternPhase==state->phases[i]&&PotatoHarvestCheck::difference(sample(frozen,i),sample(paused,i))<.01,
                                             QString("preset %1 remains frozen while paused").arg(14+i));
                    for(int i=0;i<2;++i){
                        scene.select(i);speed->setValue(i==0?121:83);size->setValue(i==0?31:29);
                        auto look=scene.surfaces[i].fx;look.flow=i==0?64:52;look.angle=i==0?13:-7;look.edge=i==0?8:4;scene.setFxLook(i,look);
                    }
                    const auto saved=scene.json();QString error;const auto path=state->temp.filePath("harvest.pmap");
                    bool reopened=scene.save(path,error);require(reopened,"save seasonal mapping: "+error);
                    if(reopened){scene.newProject();reopened=scene.load(path,error);require(reopened,"reopen seasonal mapping: "+error);}
                    if(!reopened){finish();return;}
                    require(scene.json()==saved,"new IDs, paused settings and deformed geometry survive save/reopen");
                    for(int i=0;i<2;++i){scene.select(i);require(scene.surfaces[i].pattern==14+i&&picker->currentIndex()==14+i,QString("reopened preset %1 stays selectable").arg(14+i));}

                    // Restore native defaults for the visual evidence, while retaining
                    // the same real mapped outlines and interior mesh deformation.
                    for(int i=0;i<2;++i){
                        scene.select(i);picker->setCurrentIndex(0);picker->setCurrentIndex(14+i);
                        scene.setFxLook(i,state->defaultLook[i]);speed->setValue(100);size->setValue(state->defaultSize[i]);
                        pause->setChecked(true);scene.surfaces[i].patternPhase=3;
                    }
                    auto saveFrame=[require](const QImage &image,const QString &path){
                        require(QDir().mkpath(QFileInfo(path).absolutePath()),"capture destination exists");
                        require(!image.isNull()&&image.scaled(QSize(1200,675),Qt::IgnoreAspectRatio,Qt::SmoothTransformation).save(path),"save framebuffer "+path);
                    };
                    if(!snapshot.isEmpty())saveFrame(capture(),snapshot);
                    if(!frames.isEmpty()){
                        require(QDir().mkpath(frames),"frame directory exists");
                        // 96 uniformly sampled actual framebuffer frames, 8 fps/12 s.
                        // No event processing or timed delay between deterministic frames.
                        for(int frame=0;frame<96;++frame){
                            for(auto &s:scene.surfaces)s.patternPhase=frame*.125;
                            saveFrame(capture(),QDir(frames).filePath(QString("%1.png").arg(frame,3,10,QChar('0'))));
                        }
                    }
                    if(!demo.isEmpty()){
                        for(auto &s:scene.surfaces){s.patternPlaying=true;s.patternPhase=0;}
                        require(QDir().mkpath(QFileInfo(demo).absolutePath()),"demo project directory exists");
                        error.clear();require(scene.save(demo,error),"save playing disposable demo: "+error);
                    }
                    finish();
                });
        });
    });
}
