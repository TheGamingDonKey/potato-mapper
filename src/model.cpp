#include "model.h"
#include <QImageReader>
#include <QVideoFrame>
#include <QPainter>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>
#include <QFile>
#include <QTimer>
#include <QCoreApplication>
#include <cmath>
#include <algorithm>

MediaSource::MediaSource(const QString &path, QObject *parent) : QObject(parent) {
    if (path.isEmpty()) {
        image = QImage(800,800,QImage::Format_RGBA8888);
        image.fill(QColor(255,255,255,0));
        QPainter p(&image);
        p.setPen(QPen(Qt::white,4));
        for(int i=0;i<=8;i++) {p.drawLine(i*100,0,i*100,800);p.drawLine(0,i*100,800,i*100);}
        p.drawLine(0,400,800,400); p.drawLine(400,0,400,800);
        return;
    }
    QImageReader reader(path);
    reader.setAutoTransform(true);
    image = reader.read();
    if(!image.isNull()) { image=image.convertToFormat(QImage::Format_RGBA8888); return; }
    image=QImage(2,2,QImage::Format_RGBA8888); image.fill(Qt::black);
    player=new QMediaPlayer(this);
    audio=new QAudioOutput(this); audio->setMuted(true);
    auto *sink=new QVideoSink(this);
    player->setAudioOutput(audio); player->setVideoSink(sink);
    player->setLoops(QMediaPlayer::Infinite);
    connect(sink,&QVideoSink::videoFrameChanged,this,&MediaSource::presentFrame);
    connect(player,&QMediaPlayer::errorOccurred,this,[this,path](QMediaPlayer::Error,const QString &detail){
        emit error(QFileInfo(path).fileName()+": "+detail);
    });
    player->setSource(QUrl::fromLocalFile(path)); player->play();
}

MediaSource::~MediaSource(){if(mappedVideo.isMapped())mappedVideo.unmap();}
void MediaSource::presentFrame(const QVideoFrame &frame){
    if(!frame.isValid())return;
    QElapsedTimer conversion;conversion.start();
    if(revision==1&&QCoreApplication::arguments().contains("--profile-media"))qInfo()<<"VIDEO FORMAT"<<frame.size()<<frame.pixelFormat()<<frame.handleType()<<frame.surfaceFormat().colorSpace()<<frame.surfaceFormat().colorRange();
    const auto format=frame.surfaceFormat();
    // Keep one mapped native frame; the shader converts its Y/UV planes to RGB.
    // Qt's image conversion remains the fallback for rotated, HDR or other formats.
    const bool native=frame.pixelFormat()==QVideoFrameFormat::Format_NV12
        &&(format.colorSpace()==QVideoFrameFormat::ColorSpace_BT709
           ||(format.colorSpace()==QVideoFrameFormat::ColorSpace_BT601&&format.colorRange()!=QVideoFrameFormat::ColorRange_Full))
        &&frame.rotation()==QtVideo::Rotation::None&&!frame.mirrored()
        &&format.rotation()==QtVideo::Rotation::None&&!format.isMirrored()
        &&format.scanLineDirection()==QVideoFrameFormat::TopToBottom
        &&format.viewport()==QRect(QPoint(0,0),frame.size())
        &&format.colorTransfer()!=QVideoFrameFormat::ColorTransfer_ST2084
        &&format.colorTransfer()!=QVideoFrameFormat::ColorTransfer_STD_B67;
    QVideoFrame next=frame;
    if(native&&next.map(QVideoFrame::ReadOnly)){
        if(next.planeCount()==2&&next.bytesPerLine(1)%2==0){
            if(mappedVideo.isMapped())mappedVideo.unmap();mappedVideo=next;
            conversionNs+=conversion.nsecsElapsed();++revision;emit frameChanged();return;
        }
        next.unmap();
    }
    auto decoded=frame.toImage();if(decoded.isNull())return;
    QTransform orientation;orientation.rotate(static_cast<int>(frame.rotation()));
    if(frame.mirrored())orientation.scale(-1,1);
    if(!orientation.isIdentity())decoded=decoded.transformed(orientation);
    if(mappedVideo.isMapped())mappedVideo.unmap();mappedVideo={};
    image=decoded.convertToFormat(QImage::Format_RGBA8888);
    conversionNs+=conversion.nsecsElapsed();++revision;emit frameChanged();
}

static double cross(const QPointF &a,const QPointF &b) {return a.x()*b.y()-a.y()*b.x();}
static bool finitePoint(const QPointF &p) {return std::isfinite(p.x())&&std::isfinite(p.y());}
Surface::Surface(){resetMesh();}
void Surface::resetMesh(){
    mesh.clear();
    for(int y=0;y<=cells;y++) for(int x=0;x<=cells;x++) mesh.append(QPointF(double(x)/cells,double(y)/cells));
}
QTransform Surface::transform() const {
    QTransform t;
    QPolygonF unit{QPointF(0,0),QPointF(1,0),QPointF(1,1),QPointF(0,1)};
    QTransform::quadToQuad(unit,corners,t); return t;
}
bool Surface::valid() const {
    if(corners.size()!=4 || cells<1 || cells>16 || mesh.size()!=(cells+1)*(cells+1)) return false;
    for(auto p:corners) if(!finitePoint(p)) return false;
    for(auto p:mesh) if(!finitePoint(p)) return false;
    for(int i=0;i<4;i++) if(cross(corners[(i+1)%4]-corners[i],corners[(i+2)%4]-corners[(i+1)%4])<1e-7) return false;
    const auto t=transform();
    if(!t.isInvertible()) return false;
    for(auto p:mesh) if(t.m13()*p.x()+t.m23()*p.y()+t.m33()<=1e-6) return false;
    for(int y=0;y<cells;y++) for(int x=0;x<cells;x++){
        const int a=y*(cells+1)+x,b=a+1,c=a+cells+1,d=c+1;
        if(cross(mesh[b]-mesh[a],mesh[d]-mesh[a])<1e-7 || cross(mesh[d]-mesh[a],mesh[c]-mesh[a])<1e-7) return false;
    }
    return true;
}
QPointF Surface::sample(double u,double v) const {
    const double xx=std::clamp(u,0.0,1.0)*cells, yy=std::clamp(v,0.0,1.0)*cells;
    const int x=std::min(int(xx),cells-1),y=std::min(int(yy),cells-1),a=y*(cells+1)+x;
    const double fx=xx-x,fy=yy-y;
    const QPointF p=mesh[a], b=mesh[a+1],c=mesh[a+cells+1],d=mesh[a+cells+2];
    return fx>=fy ? p*(1-fx)+b*(fx-fy)+d*fy : p*(1-fy)+d*fx+c*(fy-fx);
}
void Surface::subdivide(int count){
    count=std::clamp(count,1,16);
    QVector<QPointF> replacement;
    for(int y=0;y<=count;y++) for(int x=0;x<=count;x++) replacement.append(sample(double(x)/count,double(y)/count));
    cells=count; mesh=replacement;
}
static QJsonArray pointJson(const QPointF &p){return {p.x(),p.y()};}
QJsonObject Surface::json(const QString &base) const {
    QJsonArray cs,ms; for(auto p:corners)cs.append(pointJson(p));for(auto p:mesh)ms.append(pointJson(p));
    QString path=media;
    if(!path.isEmpty()&&!base.isEmpty())path=QDir(base).relativeFilePath(path);
    return {{"id",id},{"name",name},{"media",path},{"pattern",pattern},{"patternSpeed",patternSpeed},{"patternSize",patternSize},{"patternPlaying",patternPlaying},{"corners",cs},{"cells",cells},{"mesh",ms},{"visible",visible},{"locked",locked},{"fit",fit}};
}
bool Surface::fromJson(const QJsonObject &j,const QString &base,Surface &s){
    s.id=j["id"].toString(s.id);s.name=j["name"].toString("Surface");s.media=j["media"].toString();
    if(!s.media.isEmpty())s.media=QDir::cleanPath(QDir::isAbsolutePath(s.media)?s.media:QDir(base).absoluteFilePath(s.media));
    s.cells=j["cells"].toInt(2);s.visible=j["visible"].toBool(true);s.locked=j["locked"].toBool();s.fit=std::clamp(j["fit"].toInt(),0,2);
    s.pattern=std::clamp(j["pattern"].toInt(),0,1);s.patternSpeed=std::clamp(j["patternSpeed"].toInt(100),0,300);s.patternSize=std::clamp(j["patternSize"].toInt(18),5,45);s.patternPlaying=j["patternPlaying"].toBool(true);
    auto readPoints=[](const QJsonArray &a,QPolygonF &pts){pts.clear();for(auto value:a){const auto p=value.toArray();if(p.size()!=2||!p[0].isDouble()||!p[1].isDouble())return false;pts.append({p[0].toDouble(),p[1].toDouble()});}return true;};
    QPolygonF points;
    if(!readPoints(j["corners"].toArray(),s.corners)||!readPoints(j["mesh"].toArray(),points))return false;
    s.mesh=points;return s.valid();
}

Scene::Scene(QObject *parent):QObject(parent){
    animationClock.start();auto *timer=new QTimer(this);timer->setTimerType(Qt::PreciseTimer);
    connect(timer,&QTimer::timeout,this,[this]{
        const double dt=std::min(animationClock.restart()/1000.0,.1);bool animated=false;
        for(auto &s:surfaces)if(s.pattern&&s.visible&&s.patternPlaying&&s.patternSpeed){s.patternPhase+=dt*s.patternSpeed/100.0;animated=true;}
        if(animated)emit changed();
    });timer->start(16);
}
Surface *Scene::current(){return selected>=0&&selected<surfaces.size()?&surfaces[selected]:nullptr;}
MediaSource *Scene::source(const QString &path){
    if(mediaSources.contains(path))return mediaSources[path];
    auto *s=new MediaSource(path,this);mediaSources.insert(path,s);
    connect(s,&MediaSource::frameChanged,this,&Scene::changed);
    connect(s,&MediaSource::error,this,&Scene::message);return s;
}
void Scene::select(int i){selected=i;emit structureChanged();emit changed();}
void Scene::touch(bool structure){
    dirty=true;
    if(structure)for(auto it=mediaSources.begin();it!=mediaSources.end();++it){
        if(it.value()->player){bool used=false;for(const auto &s:surfaces)if(s.media==it.key()){used=true;break;}if(!used)it.value()->player->pause();}
    }
    emit changed();if(structure)emit structureChanged();
}
void Scene::newProject(){
    for(auto *s:mediaSources)if(s->player)s->player->stop();
    surfaces.clear();projectPath.clear();selected=-1;add();history.clear();future.clear();dirty=false;emit structureChanged();
}
void Scene::add(){
    checkpoint();Surface s;s.name="Surface "+QString::number(surfaces.size()+1);
    const double offset=(surfaces.size()%5)*.04; for(auto &p:s.corners)p+=QPointF(offset,offset);
    surfaces.append(s);selected=int(surfaces.size())-1;source("");touch(true);
}
void Scene::assignMedia(int i,const QString &path){
    if(i<0||i>=surfaces.size()||!QFileInfo::exists(path))return;
    checkpoint();surfaces[i].pattern=0;surfaces[i].media=QFileInfo(path).absoluteFilePath();source(surfaces[i].media);touch(true);
}
QJsonObject Scene::json(const QString &base) const {
    QJsonArray items;for(const auto &s:surfaces)items.append(s.json(base));
    return {{"format","PotatoMapper"},{"version",1},{"width",outputSize.width()},{"height",outputSize.height()},{"surfaces",items}};
}
bool Scene::restore(const QJsonObject &j,const QString &base,QString &error){
    if((j["format"].toString()!="PotatoMapper"&&j["format"].toString()!="HomeMapper")||j["version"].toInt()!=1){error="This is not a supported Potato Mapper project.";return false;}
    QSize size(j["width"].toInt(),j["height"].toInt());
    if(size.width()<1||size.height()<1||size.width()>16384||size.height()>16384){error="Invalid output size.";return false;}
    QVector<Surface> restored;
    for(auto v:j["surfaces"].toArray()){Surface s;if(!Surface::fromJson(v.toObject(),base,s)){error="Invalid surface in project.";return false;}restored.append(s);}
    surfaces=restored;outputSize=size;selected=surfaces.isEmpty()?-1:0;
    for(auto *src:mediaSources)if(src->player)src->player->pause();
    for(const auto &s:surfaces){if(s.media.isEmpty()||QFileInfo::exists(s.media)){auto *src=source(s.media);if(src->player)src->player->play();}else emit message("Missing media: "+s.media+". Select its surface and use Load media to locate it.");}
    emit structureChanged();emit changed();return true;
}
void Scene::checkpoint(){history.append(json());if(history.size()>80)history.removeFirst();future.clear();}
void Scene::undo(){if(history.isEmpty())return;future.append(json());QString e;restore(history.takeLast(),{},e);touch(true);}
void Scene::redo(){if(future.isEmpty())return;history.append(json());QString e;restore(future.takeLast(),{},e);touch(true);}
bool Scene::save(const QString &path,QString &error){
    QSaveFile file(path);if(!file.open(QIODevice::WriteOnly)){error=file.errorString();return false;}
    const auto bytes=QJsonDocument(json(QFileInfo(path).absolutePath())).toJson();
    if(file.write(bytes)!=bytes.size()||!file.commit()){error=file.errorString();return false;}
    projectPath=path;dirty=false;emit structureChanged();return true;
}
bool Scene::load(const QString &path,QString &error){
    QFile file(path);if(!file.open(QIODevice::ReadOnly)){error=file.errorString();return false;}
    QJsonParseError parse;auto doc=QJsonDocument::fromJson(file.readAll(),&parse);
    if(parse.error!=QJsonParseError::NoError){error=parse.errorString();return false;}
    if(!restore(doc.object(),QFileInfo(path).absolutePath(),error))return false;
    projectPath=path;history.clear();future.clear();dirty=false;emit structureChanged();return true;
}
