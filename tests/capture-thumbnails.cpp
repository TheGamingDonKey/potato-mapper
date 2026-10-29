#include "model.h"
#include "canvas.h"
#include <QApplication>
#include <QSurfaceFormat>
#include <QTimer>
#include <QDir>
#include <cstdio>
int main(int argc,char **argv){
    QSurfaceFormat format;format.setVersion(3,3);format.setProfile(QSurfaceFormat::CoreProfile);QSurfaceFormat::setDefaultFormat(format);
    QApplication app(argc,argv);if(app.arguments().size()!=2)return 2;const QString destination=app.arguments()[1];if(!QDir().mkpath(destination))return 2;
    Scene scene;scene.outputSize={1920,1080};scene.add();scene.surfaces[0].corners={{0,0},{1,0},{1,1},{0,1}};
    Canvas output(&scene,false);output.resize(640,360);output.show();
    QTimer::singleShot(600,&output,[&]{
        if(!output.graphicsReady){fprintf(stderr,"%s\n",output.graphicsError.toUtf8().constData());app.exit(2);return;}
        for(int id=0;id<potatoPatterns().size();++id){scene.setPattern(0,id);auto &s=scene.surfaces[0];s.patternPlaying=false;s.patternPhase=8;scene.touch(true);
            const auto frame=output.grabFramebuffer().scaled(320,180,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
            if(frame.isNull()||!frame.save(QDir(destination).filePath(QString("pattern-%1.png").arg(id)))){app.exit(2);return;}}
        fprintf(stdout,"Native thumbnails: %lld\n",static_cast<long long>(potatoPatterns().size()));app.exit(0);
    });return app.exec();
}
