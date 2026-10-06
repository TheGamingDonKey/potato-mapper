#pragma once
#include <QApplication>
#include <QElapsedTimer>
#include <QTimer>
#include <functional>
#include <memory>

// Repaint-rate measurement of actual editor/output windows, not GPU timing.
// All mappings are synthetic. Overlap measures four full-screen shader layers.
template<class Window> void profileFx(Window &window,QApplication &app,const QStringList &args){
    window.output->show();const auto scale=window.output->devicePixelRatioF();
    window.output->resize(qRound(1920/scale),qRound(1080/scale));
    // A second editor Canvas avoids startup-hidden/occluded main-window bias.
    // Both real renderers remain visible on one desktop; output stays 1080p.
    auto preview=std::make_shared<Canvas>(&window.scene,true);
    preview->setWindowTitle("Potato FX performance check — editor");preview->resize(qRound(960/scale),qRound(540/scale));
    window.output->move(0,0);preview->move(12,12);preview->show();preview->raise();
    struct State {int pattern=5,last=int(potatoPatterns().size())-1,layers=1,dynamic=0;};auto state=std::make_shared<State>();
    const int first=args.indexOf("--first-pattern"),last=args.indexOf("--last-pattern");
    if(first>=0&&first+1<args.size())state->pattern=args[first+1].toInt();
    if(last>=0&&last+1<args.size())state->last=args[last+1].toInt();
    const int effect=args.indexOf("--dynamic-effect");if(effect>=0&&effect+1<args.size()){state->dynamic=args[effect+1].toInt();state->pattern=state->last=0;}
    if(state->pattern<0||state->last<state->pattern||state->last>=potatoPatterns().size()||state->dynamic<0||state->dynamic>2){
        QTimer::singleShot(0,&app,[&app]{qInfo()<<"FX PROFILE FAIL invalid pattern range";app.exit(2);});return;
    }
    auto step=std::make_shared<std::function<void()>>();std::weak_ptr<std::function<void()>> weak=step;
    *step=[&window,&app,state,weak,preview]{
        auto keep=weak.lock();if(state->pattern>state->last){app.exit(0);return;}
        auto &scene=window.scene;scene.newProject();
        for(int i=0;i<state->layers;++i){if(i)scene.add();scene.setPattern(i,state->pattern);scene.surfaces[i].corners={QPointF(0,0),QPointF(1,0),QPointF(1,1),QPointF(0,1)};scene.setBlend(i,2);}
        if(state->dynamic){PotatoDynamic::Settings settings;settings.effect=state->dynamic;for(const auto &s:scene.surfaces)settings.members<<s.id;scene.setDynamic(settings);scene.setDynamicTime(30);}
        scene.select(0);scene.dirty=false;
        auto *pulse=new QTimer(&window);if(state->pattern==0){QObject::connect(pulse,&QTimer::timeout,&window,[&window,preview]{window.output->update();preview->update();});pulse->start(16);}
        QTimer::singleShot(400,&window,[&window,&app,state,keep,preview,pulse]{
            const auto out=window.output->paintedFrames,editor=preview->paintedFrames;
            auto elapsed=std::make_shared<QElapsedTimer>();elapsed->start();
            QTimer::singleShot(2000,&window,[&window,&app,state,keep,preview,pulse,out,editor,elapsed]{
                pulse->stop();pulse->deleteLater();
                if(!window.output->graphicsReady||!preview->graphicsReady){qInfo()<<"FX PROFILE FAIL a renderer could not initialize graphics";app.exit(2);return;}
                const auto seconds=elapsed->elapsed()/1000.0;
                qInfo().noquote()<<QString("FX PROFILE %1 layers=%2 output=%3x%4 output-repaints/s=%5 editor-repaints/s=%6")
                    .arg(state->dynamic==1?QString("Hot-Ass Potato"):state->dynamic==2?QString("Potato Focus"):potatoPatterns()[state->pattern]).arg(state->layers)
                    .arg(qRound(window.output->width()*window.output->devicePixelRatioF())).arg(qRound(window.output->height()*window.output->devicePixelRatioF()))
                    .arg((window.output->paintedFrames-out)/seconds,0,'f',1).arg((preview->paintedFrames-editor)/seconds,0,'f',1);
                if(window.output->paintedFrames==out||preview->paintedFrames==editor){qInfo()<<"FX PROFILE FAIL a renderer was not repainting";app.exit(2);return;}
                if(state->layers==1)state->layers=4;else{state->layers=1;++state->pattern;}
                (*keep)();
            });
        });
    };QTimer::singleShot(650,&window,[step]{(*step)();});
}
