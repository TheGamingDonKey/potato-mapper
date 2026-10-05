#pragma once
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QJsonArray>
#include <QPainter>
#include <QPushButton>
#include <QScrollArea>
#include <QToolButton>
#include <QSpinBox>
#include <QTemporaryDir>
#include <QTimer>
#include <algorithm>
#include <cmath>
#include <functional>
#include <memory>

// Actual UI, blend pixels and timed rendering. All files/scenes are disposable.
template<class Window> void runFxChecks(Window &window,QApplication &app,const QStringList &args){
    window.output->resize(640,360);window.output->show();
    QTimer::singleShot(650,&window,[&window,&app,args]{
        struct State {QStringList failures;QTemporaryDir temp;QImage sheet{1280,768,QImage::Format_RGB32};int pattern=1;};
        auto state=std::make_shared<State>();state->sheet.fill(QColor("#101820"));
        auto require=[state](bool ok,const QString &label){if(!ok)state->failures<<label;};
        auto &scene=window.scene;
        auto *blend=window.template findChild<QComboBox*>("blendPicker");
        auto *picker=window.template findChild<QComboBox*>("animationPicker");
        auto *speed=window.template findChild<QSpinBox*>("patternSpeed");
        auto *size=window.template findChild<QSpinBox*>("patternSize");
        auto *pause=window.template findChild<QCheckBox*>("patternPause");
        auto *density=window.template findChild<QSpinBox*>("patternDensity");
        auto *flow=window.template findChild<QSpinBox*>("patternFlow");
        auto *angle=window.template findChild<QSpinBox*>("patternAngle");
        auto *palette=window.template findChild<QComboBox*>("patternPalette");
        auto *edge=window.template findChild<QSpinBox*>("patternEdge");
        auto *reverse=window.template findChild<QCheckBox*>("patternReverse");
        if(!blend||!picker||!speed||!size||!pause||!density||!flow||!angle||!palette||!edge||!reverse){qInfo()<<"FX FAIL controls missing";app.exit(2);return;}
        auto sample=[](Canvas *canvas){auto image=canvas->grabFramebuffer();return image.pixelColor(image.width()/2,image.height()/2);};
        auto near=[](QColor a,QColor b){return std::max({std::abs(a.red()-b.red()),std::abs(a.green()-b.green()),std::abs(a.blue()-b.blue())})<=4;};
        auto fixture=[&](const QString &name,QColor colour){QImage img(32,32,QImage::Format_RGBA8888);img.fill(colour);const auto path=state->temp.filePath(name);require(img.save(path),"write "+name);return path;};
        const auto lower=fixture("lower.png",QColor(60,90,120));const auto upper=fixture("upper.png",QColor(180,100,40));
        scene.assignMedia(0,lower);scene.surfaces[0].corners={QPointF(0,0),QPointF(1,0),QPointF(1,1),QPointF(0,1)};
        scene.add();scene.assignMedia(1,upper);scene.surfaces[1].corners=scene.surfaces[0].corners;scene.touch(true);
        const auto other=scene.surfaces[0].json();blend->setCurrentIndex(1);require(scene.surfaces[1].blend==1&&scene.surfaces[0].json()==other,"blend UI scoped to selected surface");
        scene.undo();require(scene.surfaces[1].blend==0,"blend Undo");scene.redo();require(scene.surfaces[1].blend==1,"blend Redo");scene.select(1);
        for(int mode=0;mode<3;++mode)for(int opacity:{0,50,100}){
            blend->setCurrentIndex(mode);scene.setAppearance(1,100,opacity);QColor expected;
            for(int channel=0;channel<3;++channel){const double s=channel==0?180:channel==1?100:40,d=channel==0?60:channel==1?90:120,a=opacity/100.0;
                const int value=qRound(mode==0?s*a+d*(1-a):mode==1?s*a+d*(1-s*a/255):std::min(255.0,s*a+d));
                if(channel==0)expected.setRed(value);else if(channel==1)expected.setGreen(value);else expected.setBlue(value);
            }
            require(near(sample(window.output),expected)&&near(sample(window.canvas),expected),QString("blend %1 opacity %2 pixels in both canvases").arg(mode).arg(opacity));
        }
        // Screen must respect image alpha as well as the surface opacity.
        scene.assignMedia(1,fixture("alpha.png",QColor(180,100,40,128)));scene.setAppearance(1,100,50);blend->setCurrentIndex(1);
        require(near(sample(window.output),QColor(95,106,125)),"Screen image alpha and opacity");
        scene.assignMedia(1,fixture("black.png",Qt::black));scene.setAppearance(1,100,100);
        for(int mode:{1,2}){blend->setCurrentIndex(mode);require(near(sample(window.output),QColor(60,90,120)),"black overlay retains lower image");}
        scene.assignMedia(1,upper);blend->setCurrentIndex(0);require(near(sample(window.output),QColor(180,100,40)),"Normal resets blend state after Add");
        scene.setBlend(1,2);auto *reset=window.template findChild<QPushButton*>("resetAppearanceButton");if(reset)reset->click();require(scene.surfaces[1].blend==0,"reset includes blend");
        scene.undo();require(scene.surfaces[1].blend==2,"single Undo restores reset");scene.select(1);
        // Native NV12 video uses the same Screen path as RGB images.
        scene.clearMedia(1);QVideoFrameFormat format(QSize(66,34),QVideoFrameFormat::Format_NV12);format.setColorSpace(QVideoFrameFormat::ColorSpace_BT709);format.setColorRange(QVideoFrameFormat::ColorRange_Video);
        QVideoFrame frame(format);if(frame.map(QVideoFrame::WriteOnly)){
            for(int y=0;y<34;++y)std::fill_n(frame.bits(0)+y*frame.bytesPerLine(0),66,uchar(150));
            for(int y=0;y<17;++y)std::fill_n(frame.bits(1)+y*frame.bytesPerLine(1),66,uchar(128));frame.unmap();scene.source("")->presentFrame(frame);
            scene.setBlend(1,0);const auto video=sample(window.output);scene.setBlend(1,1);scene.setAppearance(1,100,50);
            require(near(sample(window.output),QColor(qRound(video.red()*.5+60*(1-video.red()*.5/255)),qRound(video.green()*.5+90*(1-video.green()*.5/255)),qRound(video.blue()*.5+120*(1-video.blue()*.5/255)))),"NV12 Screen pixels");
        }else require(false,"NV12 frame map");
        auto legacy=scene.json();auto items=legacy["surfaces"].toArray();for(int i=0;i<items.size();++i){auto item=items[i].toObject();item.remove("blend");items[i]=item;}legacy["surfaces"]=items;legacy["format"]="HomeMapper";QString error;
        require(scene.restore(legacy,{},error)&&scene.surfaces[1].blend==0,"old project defaults Normal");
        scene.select(1);scene.setAppearance(1,100,100);scene.surfaces[0].visible=false;
        picker->setCurrentIndex(5);pause->setChecked(true);
        const auto originalLook=scene.surfaces[1].fx;const auto otherSurface=scene.surfaces[0].json();
        density->setValue(22);require(scene.surfaces[1].fx.density==22&&scene.surfaces[0].json()==otherSurface,"density scoped to selected surface");
        scene.undo();require(scene.surfaces[1].fx==originalLook,"density Undo");scene.redo();require(scene.surfaces[1].fx.density==22,"density Redo");scene.select(1);
        const auto originalFrame=window.output->grabFramebuffer();
        flow->setValue(15);angle->setValue(37);palette->setCurrentIndex(2);edge->setValue(12);reverse->setChecked(true);
        require(scene.surfaces[1].fx.flow==15&&scene.surfaces[1].fx.angle==37&&scene.surfaces[1].fx.palette==2&&scene.surfaces[1].fx.edge==12&&scene.surfaces[1].fx.reverse,"FX controls update model");
        require(window.output->grabFramebuffer()!=originalFrame,"FX controls affect output");
        const auto controlled=scene.json();require(scene.save(state->temp.filePath("look.pmap"),error)&&scene.load(state->temp.filePath("look.pmap"),error)&&scene.json()==controlled,"all FX controls save/reopen");scene.select(1);
        auto oldSurface=scene.surfaces[1].json();oldSurface["pattern"]=1;
        for(const auto *key:{"fxDensity","fxFlow","fxAngle","fxPalette","fxEdge","fxReverse"})oldSurface.remove(key);
        Surface restored;require(Surface::fromJson(oldSurface,{},restored)&&restored.fx.density==12&&restored.fx.palette==0&&restored.fx.angle==0&&restored.fx.edge==0&&!restored.fx.reverse,"legacy dots retain appearance defaults");
        for(int pattern=2;pattern<=4;++pattern){oldSurface["pattern"]=pattern;require(Surface::fromJson(oldSurface,{},restored)&&restored.fx.density==8&&restored.fx.palette==0,"legacy pattern defaults");}
        scene.setFxLook(1,FxLook{});
        // Iterate each generator with real elapsed time, then pause and round-trip.
        auto step=std::make_shared<std::function<void()>>();
        std::weak_ptr<std::function<void()>> weakStep=step;
        *step=[&window,&app,args,state,require,picker,speed,size,pause,weakStep]{
            auto step=weakStep.lock();
            auto &scene=window.scene;const int pattern=state->pattern;
            if(pattern>=potatoPatterns().size()){
                picker->setCurrentIndex(0);require(!scene.current()->pattern&&scene.current()->media.isEmpty(),"return to alignment grid");scene.undo();require(scene.surfaces[1].pattern==potatoPatterns().size()-1,"clear FX Undo");scene.select(1);
                const int preview=args.indexOf("--preview");if(preview>=0&&preview+1<args.size())require(state->sheet.save(args[preview+1]),"pattern preview");
                const int snapshot=args.indexOf("--snapshot");if(snapshot>=0&&snapshot+1<args.size()){window.resize(1024,700);for(auto *h:window.template findChildren<QToolButton*>("sectionHeading"))h->setChecked(h->text()=="Potato FX");app.processEvents();if(auto *scroll=window.template findChild<QScrollArea*>("surfaceInspector"))scroll->ensureWidgetVisible(picker);app.processEvents();require(window.grab().save(args[snapshot+1]),"UI snapshot");}
                qInfo().noquote()<<"FX"<<(state->failures.isEmpty()?"PASS":"FAIL")<<state->failures.join(", ");app.exit(state->failures.isEmpty()?0:2);return;
            }
            picker->setCurrentIndex(pattern);speed->setValue(130);size->setValue(18);pause->setChecked(true);scene.setBlend(1,0);scene.current()->patternPhase=.37;
            auto energy=[](const QImage &image){quint64 result=0;for(int y=0;y<image.height();y+=2)for(int x=0;x<image.width();x+=2){const auto c=image.pixelColor(x,y);result+=c.red()+c.green()+c.blue();}return result;};
            const auto still=window.output->grabFramebuffer();require(energy(still)>100000,"visible "+potatoPatterns()[pattern]);
            scene.setAppearance(1,50,50);const auto faded=window.output->grabFramebuffer();scene.surfaces[0].visible=true;scene.setBlend(1,1);const auto screened=window.output->grabFramebuffer();bool correct=true;
            for(int y=20;y<screened.height();y+=30)for(int x=20;x<screened.width();x+=30){const auto c=faded.pixelColor(x,y),actual=screened.pixelColor(x,y);
                const QColor expected(qRound(c.red()+60*(1-c.red()/255.0)),qRound(c.green()+90*(1-c.green()/255.0)),qRound(c.blue()+120*(1-c.blue()/255.0)));
                correct=correct&&std::max({std::abs(expected.red()-actual.red()),std::abs(expected.green()-actual.green()),std::abs(expected.blue()-actual.blue())})<=4;
            }
            require(correct,"Screen respects pattern alpha "+potatoPatterns()[pattern]);scene.surfaces[0].visible=false;scene.setBlend(1,0);scene.setAppearance(1,100,100);
            size->setValue(32);const auto wide=window.output->grabFramebuffer();require(wide!=still&&energy(wide)>energy(still),"width control "+potatoPatterns()[pattern]);size->setValue(18);
            if(pattern>=5){size->setValue(pattern==6?12:38);const auto preview=window.output->grabFramebuffer();size->setValue(18);QPainter painter(&state->sheet);const int x=((pattern-5)%2)*640,y=((pattern-5)/2)*384;painter.setPen(Qt::white);painter.drawText(x+12,y+18,potatoPatterns()[pattern]);painter.drawImage(QRect(x,y+24,640,360),preview);}
            pause->setChecked(false);scene.dirty=false;const auto saved=scene.json();const double phase=scene.current()->patternPhase;const auto editor=window.canvas->grabFramebuffer();
            QTimer::singleShot(260,&window,[&window,&app,state,require,pause,step,still,editor,phase,saved,pattern]{
                auto &scene=window.scene;require(scene.current()->patternPhase>phase&&window.output->grabFramebuffer()!=still&&window.canvas->grabFramebuffer()!=editor,"motion in both canvases "+potatoPatterns()[pattern]);
                require(!scene.dirty&&scene.json()==saved,"animation ticks preserve project "+potatoPatterns()[pattern]);pause->setChecked(true);const auto frozen=window.output->grabFramebuffer();const double stopped=scene.current()->patternPhase;
                QTimer::singleShot(90,&window,[&window,&app,state,require,step,frozen,stopped,pattern]{
                    auto &scene=window.scene;require(scene.current()->patternPhase==stopped&&window.output->grabFramebuffer()==frozen,"pause "+potatoPatterns()[pattern]);
                    scene.setBlend(1,pattern%3);const auto saved=scene.json();QString error;require(scene.save(state->temp.filePath("fx.pmap"),error)&&scene.load(state->temp.filePath("fx.pmap"),error)&&scene.json()==saved,"save/reopen "+potatoPatterns()[pattern]);scene.select(1);
                    ++state->pattern;QTimer::singleShot(0,&window,[step]{(*step)();});
                });
            });
        };
        (*step)();
    });
}

// An opt-in preview rendered by the same four mapped surfaces as live output.
// Deterministic phase steps make the exported frames easy to compare visually.
template<class Window> void renderFxMotion(Window &window,QApplication &app,const QString &directory){
    if(directory.isEmpty()||!QDir().mkpath(directory)){app.exit(2);return;}
    auto &scene=window.scene;scene.newProject();
    for(int i=0;i<4;++i){
        if(i)scene.add();scene.setPattern(i,i+5);
        auto &s=scene.surfaces[i];const double x=(i%2)*.5,y=(i/2)*.5;
        s.corners={QPointF(x,y),QPointF(x+.5,y),QPointF(x+.5,y+.5),QPointF(x,y+.5)};s.patternPlaying=false;
    }
    scene.select(0);window.output->resize(960,540);window.output->show();
    QTimer::singleShot(650,&window,[&window,&app,directory]{
        bool ok=true;
        for(int frame=0;frame<48;++frame){
            for(auto &s:window.scene.surfaces)s.patternPhase=frame*.12;
            const auto image=window.output->grabFramebuffer();
            ok=ok&&!image.isNull()&&image.save(QDir(directory).filePath(QString("%1.png").arg(frame,3,10,QChar('0'))));
        }
        qInfo()<<"FX MOTION"<<ok;app.exit(ok?0:2);
    });
}
