#include "model.h"
#include "dynamic-metric.h"
#include "projects.h"
#include "mesh-geometry.h"
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
#include "entrance.h"
#include <QCoreApplication>
#include <QSet>
#include <QCryptographicHash>
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
    const double cellX=xx-x,fy=yy-y;
    const QPointF p=mesh[a], b=mesh[a+1],c=mesh[a+cells+1],d=mesh[a+cells+2];
    return cellX>=fy ? p*(1-cellX)+b*(cellX-fy)+d*fy : p*(1-fy)+d*cellX+c*(fy-cellX);
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
    return {{"id",id},{"routeUid",routeUid},{"name",name},{"media",path},{"pattern",pattern},{"patternSpeed",patternSpeed},{"patternSize",patternSize},{"patternPlaying",patternPlaying},{"fxDensity",fx.density},{"fxFlow",fx.flow},{"fxAngle",fx.angle},{"fxPalette",fx.palette},{"fxEdge",fx.edge},{"fxReverse",fx.reverse},{"brightness",brightness},{"opacity",opacity},{"blend",blend},{"corners",cs},{"cells",cells},{"mesh",ms},{"visible",visible},{"locked",locked},{"fit",fit}};
}
bool Surface::fromJson(const QJsonObject &j,const QString &base,Surface &s){
    s.routeUid=j["routeUid"].toString(s.routeUid);s.id=j["id"].toString(s.id);s.name=j["name"].toString("Surface");s.media=j["media"].toString();
    if(!s.media.isEmpty())s.media=QDir::cleanPath(QDir::isAbsolutePath(s.media)?s.media:QDir(base).absoluteFilePath(s.media));
    s.cells=j["cells"].toInt(2);s.visible=j["visible"].toBool(true);s.locked=j["locked"].toBool();s.fit=std::clamp(j["fit"].toInt(),0,2);
    s.pattern=std::clamp(j["pattern"].toInt(),0,int(potatoPatterns().size())-1);s.patternSpeed=std::clamp(j["patternSpeed"].toInt(100),0,300);s.patternSize=std::clamp(j["patternSize"].toInt(18),5,45);s.patternPlaying=j["patternPlaying"].toBool(true);
    s.blend=std::clamp(j["blend"].toInt(),0,2);
    s.fx.density=std::clamp(j["fxDensity"].toInt(potatoPatternDensity(s.pattern)),4,80);s.fx.flow=std::clamp(j["fxFlow"].toInt(potatoPatternDefaultFlow(s.pattern)),0,100);s.fx.angle=std::clamp(j["fxAngle"].toInt(),-180,180);
    s.fx.palette=std::clamp(j["fxPalette"].toInt(potatoPatternPalette(s.pattern)),0,5);s.fx.edge=std::clamp(j["fxEdge"].toInt(),0,25);s.fx.reverse=j["fxReverse"].toBool();
    s.brightness=std::clamp(j["brightness"].toInt(100),0,100);s.opacity=std::clamp(j["opacity"].toInt(100),0,100);
    auto readPoints=[](const QJsonArray &a,QPolygonF &pts){pts.clear();for(auto value:a){const auto p=value.toArray();if(p.size()!=2||!p[0].isDouble()||!p[1].isDouble())return false;pts.append({p[0].toDouble(),p[1].toDouble()});}return true;};
    QPolygonF points;
    if(!readPoints(j["corners"].toArray(),s.corners)||!readPoints(j["mesh"].toArray(),points))return false;
    s.mesh=points;return s.valid();
}

Scene::Scene(QObject *parent):QObject(parent){
    animationClock.start();auto *timer=new QTimer(this);timer->setTimerType(Qt::PreciseTimer);
    connect(timer,&QTimer::timeout,this,[this]{
        const double dt=std::min(animationClock.restart()/1000.0,.1);bool animated=false;
        if((boundaryActors||wizardActors)&&!blackout){actorElapsed+=dt;animated=true;}
        if(entranceActive&&entrancePlaying&&!blackout){
            entranceElapsed=std::min(PotatoEntrance::duration,entranceElapsed+dt);
            if(entranceElapsed>=PotatoEntrance::duration)entranceActive=false;
            animated=true;
        }
        for(auto &s:surfaces)if(s.pattern&&s.visible&&s.patternPlaying&&s.patternSpeed){s.patternPhase+=dt*s.patternSpeed/100.0*(s.fx.reverse?-1.0:1.0);if(!dynamicSurface(s.id))animated=true;}
        if(dynamic.effect&&dynamic.playing&&!dynamicSurfaces.isEmpty()){
            dynamicElapsed+=dt*dynamic.speed/100.0;
            updateDynamicFrame();animated=true;
        }
        if(animated)emit changed();
    });timer->start(16);
}
Surface *Scene::current(){return selected>=0&&selected<surfaces.size()?&surfaces[selected]:nullptr;}
void Scene::playEntrance(){
    entranceActive=std::any_of(surfaces.begin(),surfaces.end(),[](const Surface &s){return s.visible&&s.opacity>0;});
    if(entranceActive)blackout=false;
    entranceElapsed=0;entrancePlaying=true;emit changed();
}
void Scene::skipEntrance(){
    if(!entranceActive)return;
    entranceActive=false;entranceElapsed=PotatoEntrance::duration;emit changed();
}
void Scene::setEntranceTime(double seconds){
    entranceElapsed=std::clamp(std::isfinite(seconds)?seconds:0.0,0.0,PotatoEntrance::duration);
    entranceActive=entranceElapsed<PotatoEntrance::duration&&std::any_of(surfaces.begin(),surfaces.end(),[](const Surface &s){return s.visible&&s.opacity>0;});emit changed();
}
const Scene::DynamicGeometry &Scene::entranceGeometry(){
    auto &geometry=cachedEntranceGeometry;
    if(geometry.revision==mappingRevision)return geometry;
    geometry.vertices.clear();geometry.ranges.clear();
    for(int i=0;i<surfaces.size();++i){
        const auto &s=surfaces[i];if(!s.visible||s.opacity==0)continue;
        const int first=int(geometry.vertices.size()/4);
        auto vertex=[&](int index){const auto p=s.mesh[index];geometry.vertices<<float(p.x())<<float(p.y())<<float(index%(s.cells+1))/s.cells<<float(index/(s.cells+1))/s.cells;};
        for(int y=0;y<s.cells;++y)for(int x=0;x<s.cells;++x){const int a=y*(s.cells+1)+x,b=a+1,c=a+s.cells+1,d=c+1;vertex(a);vertex(b);vertex(d);vertex(a);vertex(d);vertex(c);}
        geometry.ranges.insert(i,{first,int(geometry.vertices.size()/4)-first});
    }
    geometry.revision=mappingRevision;return geometry;
}
bool Scene::dynamicSurface(const QString &id) const {return dynamic.effect&&dynamic.members.contains(id);}
void Scene::updateDynamicFrame(){
    dynamicFrame=PotatoDynamic::frame(dynamic,dynamicSurfaces,dynamicElapsed);++dynamicRevision;
}
const Scene::DynamicGeometry &Scene::dynamicGeometry(){
    if(cachedDynamicGeometry.revision==dynamicRevision)return cachedDynamicGeometry;
    auto &geometry=cachedDynamicGeometry;geometry.vertices.clear();geometry.ranges.clear();geometry.vertices.reserve(20000);
    for(int surfaceIndex=0;surfaceIndex<surfaces.size();++surfaceIndex){
        const auto &s=surfaces[surfaceIndex];
        if(!s.visible||!dynamicSurface(s.id))continue;
        bool regular=s.mesh.size()==(s.cells+1)*(s.cells+1);
        for(int i=0;regular&&i<s.mesh.size();++i){
            const QPointF expected(double(i%(s.cells+1))/s.cells,double(i/(s.cells+1))/s.cells);
            regular=QLineF(s.mesh[i],expected).length()<1e-12;
        }
        const int first=int(geometry.vertices.size()/6);
        auto vertex=[&](QPointF uv,QColor colour,double alpha){
            const auto p=regular?uv:s.sample(uv.x(),uv.y());
            geometry.vertices<<float(p.x())<<float(p.y())<<float(colour.redF())<<float(colour.greenF())<<float(colour.blueF())<<float(std::min(1.0,colour.alphaF()*alpha*1.5));
        };
        auto triangle=[&](QPointF a,QPointF b,QPointF c,QColor colour,double alpha){
            if(regular&&std::min({a.x(),b.x(),c.x(),a.y(),b.y(),c.y()})>=0&&std::max({a.x(),b.x(),c.x(),a.y(),b.y(),c.y()})<=1){vertex(a,colour,alpha);vertex(b,colour,alpha);vertex(c,colour,alpha);return;}
            potatoEachMeshVertex(a,b,c,regular?1:s.cells,[&](QPointF p){vertex(p,colour,alpha);});
        };
        double aspect=1;for(const auto &surface:dynamicSurfaces)if(surface.instanceIndex==surfaceIndex||(surface.instanceIndex<0&&surface.id==s.id)){aspect=surface.aspect;break;}
        PotatoDynamic::Surface physical;physical.projection=s.transform()*QTransform::fromScale(outputSize.width(),outputSize.height());physical.cells=s.cells;physical.mesh=s.mesh;
        for(const auto &shape:dynamicFrame.shapes)if(shape.surfaceId==s.id&&(shape.surfaceInstance<0||shape.surfaceInstance==surfaceIndex)&&shape.points.size()>=2){
            const auto &points=shape.points;
            if(shape.filled)for(int i=1;i+1<points.size();++i)triangle(points[0],points[i],points[i+1],shape.color,1);
            for(int layer=0;layer<2;++layer){
                const double width=shape.width*(layer?1:3.2),alpha=layer?1:.13;
                const int count=shape.closed&&points.size()>2?int(points.size()):int(points.size())-1;
                for(int i=0;i<count;++i){
                    const auto a=points[i],b=points[(i+1)%points.size()];const auto delta=b-a;
                    const double dx=delta.x()*aspect,dy=delta.y(),length=std::hypot(dx,dy);if(length<1e-8)continue;
                    QPointF normal(-dy/length*width/aspect/2,dx/length*width/2);
                    if(shape.outputWidth>0){const auto centre=(a+b)/2;const auto mappedDelta=PotatoDynamic::mappedPoint(physical,b)-PotatoDynamic::mappedPoint(physical,a);const double pixels=std::hypot(mappedDelta.x(),mappedDelta.y());
                        if(pixels<1e-8)continue;const double half=shape.outputWidth*(layer?1:3.2)/2;
                        normal=PotatoDynamic::pixelOffset(physical,centre,{-mappedDelta.y()/pixels*half,mappedDelta.x()/pixels*half});}
                    triangle(a+normal,b+normal,b-normal,shape.color,alpha);triangle(a+normal,b-normal,a-normal,shape.color,alpha);
                }
            }
        }
        geometry.ranges.insert(surfaceIndex,{first,int(geometry.vertices.size()/6)-first});
    }
    geometry.revision=dynamicRevision;++dynamicGeometryBuilds;return geometry;
}
void Scene::rebuildDynamic(){
    ++mappingRevision;
    QStringList previous;for(const auto &s:dynamicSurfaces)previous<<s.id;
    dynamicSurfaces.clear();
    for(int surfaceIndex=0;surfaceIndex<surfaces.size();++surfaceIndex){const auto &s=surfaces[surfaceIndex];if(!s.visible||!dynamic.members.contains(s.id))continue;
        const auto pixel=[this](QPointF p){return QPointF(p.x()*outputSize.width(),p.y()*outputSize.height());};
        const auto t=s.transform();QPolygonF boundary;
        for(const auto uv:QPolygonF{{0,0},{1,0},{1,1},{0,1}})boundary<<pixel(t.map(s.sample(uv.x(),uv.y())));
        const double w=(QLineF(boundary[0],boundary[1]).length()+QLineF(boundary[3],boundary[2]).length())/2;
        const double h=(QLineF(boundary[0],boundary[3]).length()+QLineF(boundary[1],boundary[2]).length())/2;
        dynamicSurfaces.append({s.id,boundary,std::clamp(w/std::max(h,.001),.05,20.0),t*QTransform::fromScale(outputSize.width(),outputSize.height()),s.cells,s.mesh,surfaceIndex});
    }
    QStringList next;for(const auto &s:dynamicSurfaces)next<<s.id;
    previous.sort();next.sort();
    if(previous!=next)dynamicElapsed=0;
    updateDynamicFrame();
}
void Scene::setDynamic(PotatoDynamic::Settings settings){
    settings=PotatoDynamic::Settings::fromJson(settings.json());
    if(settings==dynamic)return;
    checkpoint();if(settings.effect!=dynamic.effect||settings.members!=dynamic.members)dynamicElapsed=0;
    dynamic=settings;touch(true);
}
void Scene::setDynamicTime(double seconds){
    dynamicElapsed=std::isfinite(seconds)?std::max(0.0,seconds):0;
    updateDynamicFrame();emit changed();
}
MediaSource *Scene::source(const QString &path){
    if(mediaSources.contains(path))return mediaSources[path];
    auto *s=new MediaSource(path,this);mediaSources.insert(path,s);
    connect(s,&MediaSource::frameChanged,this,&Scene::changed);
    connect(s,&MediaSource::error,this,&Scene::message);return s;
}
void Scene::select(int i){selected=i;emit structureChanged();emit changed();}
void Scene::touch(bool structure){
    QSet<QString> routes;
    for(auto &surface:surfaces){if(surface.routeUid.isEmpty()||routes.contains(surface.routeUid))surface.routeUid=QUuid::createUuid().toString(QUuid::WithoutBraces);routes.insert(surface.routeUid);}
    connections.erase(std::remove_if(connections.begin(),connections.end(),[&](const auto &c){return !routes.contains(c.a)||!routes.contains(c.b);}),connections.end());
    if(selected>=0&&selected<surfaces.size()&&surfaces[selected].pattern==22){
        const auto master=surfaces[selected];QSet<QString> group{master.routeUid};bool more=true;
        while(more){more=false;for(const auto &c:connections)if(group.contains(c.a)||group.contains(c.b))for(auto &s:surfaces)if(s.pattern==22&&(s.routeUid==c.a||s.routeUid==c.b)&&!group.contains(s.routeUid)){group.insert(s.routeUid);more=true;}}
        for(auto &s:surfaces)if(group.contains(s.routeUid)&&s.routeUid!=master.routeUid){s.patternPhase=master.patternPhase;s.patternPlaying=master.patternPlaying;s.patternSpeed=master.patternSpeed;s.patternSize=master.patternSize;s.fx=master.fx;}
    }
    dirty=true;
    rebuildDynamic();
    if(structure)for(auto it=mediaSources.begin();it!=mediaSources.end();++it){
        if(it.value()->player){bool used=false;for(const auto &s:surfaces)if(s.media==it.key()){used=true;break;}if(!used)it.value()->player->pause();}
    }
    emit changed();if(structure)emit structureChanged();
}
void Scene::newProject(){
    skipEntrance();
    for(auto *s:mediaSources)if(s->player)s->player->stop();
    surfaces.clear();connections.clear();boundaryActors=false;wizardActors=false;actorElapsed=0;projectPath.clear();dynamic={};dynamicElapsed=0;selected=-1;add();history.clear();future.clear();dirty=false;emit structureChanged();
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
void Scene::clearMedia(int i){
    if(i<0||i>=surfaces.size())return;
    auto &s=surfaces[i];if(s.media.isEmpty()&&!s.pattern)return;
    checkpoint();s.media.clear();s.pattern=0;s.patternPhase=0;touch(true);
}
void Scene::renameSurface(int i,const QString &name){
    const auto clean=name.trimmed().left(80);
    if(i<0||i>=surfaces.size()||clean.isEmpty()||surfaces[i].name==clean)return;
    checkpoint();surfaces[i].name=clean;touch(true);
}
void Scene::setAppearance(int i,int brightness,int opacity,bool recordUndo){
    if(i<0||i>=surfaces.size())return;
    brightness=std::clamp(brightness,0,100);opacity=std::clamp(opacity,0,100);
    auto &s=surfaces[i];if(s.brightness==brightness&&s.opacity==opacity)return;
    if(recordUndo)checkpoint();s.brightness=brightness;s.opacity=opacity;touch(true);
}
void Scene::setBlend(int i,int blend){
    if(i<0||i>=surfaces.size())return;
    blend=std::clamp(blend,0,2);if(surfaces[i].blend==blend)return;
    checkpoint();surfaces[i].blend=blend;touch(true);
}
void Scene::resetAppearance(int i){
    if(i<0||i>=surfaces.size())return;
    auto &s=surfaces[i];if(s.brightness==100&&s.opacity==100&&!s.blend)return;
    checkpoint();s.brightness=100;s.opacity=100;s.blend=0;touch(true);
}
void Scene::setPattern(int i,int pattern){
    if(i<0||i>=surfaces.size())return;
    pattern=std::clamp(pattern,0,int(potatoPatterns().size())-1);auto &s=surfaces[i];if(s.pattern==pattern)return;
    if(!pattern){clearMedia(i);return;}
    checkpoint();s.pattern=pattern;s.media.clear();s.patternPhase=0;s.patternPlaying=true;
    s.fx.density=potatoPatternDensity(pattern);s.fx.palette=potatoPatternPalette(pattern);
    if(pattern>=16)s.fx.flow=potatoPatternDefaultFlow(pattern);
    if(pattern>=5)s.patternSize=potatoPatternDefaultSize(pattern);
    touch(true);
}
void Scene::setFxLook(int i,FxLook look){
    if(i<0||i>=surfaces.size()||!surfaces[i].pattern)return;
    look.density=std::clamp(look.density,4,80);look.flow=std::clamp(look.flow,0,100);look.angle=std::clamp(look.angle,-180,180);look.palette=std::clamp(look.palette,0,5);look.edge=std::clamp(look.edge,0,25);
    if(surfaces[i].fx==look)return;checkpoint();surfaces[i].fx=look;touch(true);
}
QJsonObject Scene::json(const QString &base) const {
    QJsonArray items;for(const auto &s:surfaces)items.append(s.json(base));
    QJsonArray links;for(const auto &c:connections)links.append(QJsonObject{{"a",c.a},{"b",c.b},{"edgeA",c.edgeA},{"edgeB",c.edgeB},{"beginB",c.beginB},{"endB",c.endB},{"tied",c.tied}});
    const bool modern=wizardActors||boundaryActors||!connections.isEmpty()||std::any_of(surfaces.begin(),surfaces.end(),[](const Surface &s){return s.pattern>=19;});
    QJsonObject result{{"format","PotatoMapper"},{"version",modern?2:1},{"width",outputSize.width()},{"height",outputSize.height()},{"surfaces",items},{"dynamic",dynamic.json()}};
    if(modern){result["connections"]=links;result["boundaryActors"]=boundaryActors;result["wizardActors"]=wizardActors;}
    return result;
}
bool Scene::restore(const QJsonObject &j,const QString &base,QString &error,bool restartDynamic){
    if(!PotatoProjects::validateDocument(j,error))return false;
    if((j["format"].toString()!="PotatoMapper"&&j["format"].toString()!="HomeMapper")||(j["version"].toInt()!=1&&j["version"].toInt()!=2)){error="This is not a supported Potato Mapper project.";return false;}
    QSize size(j["width"].toInt(),j["height"].toInt());
    if(size.width()<1||size.height()<1||size.width()>16384||size.height()>16384){error="Invalid output size.";return false;}
    QVector<Surface> restored;
    int surfaceIndex=0;
    for(auto v:j["surfaces"].toArray()){Surface s;if(!Surface::fromJson(v.toObject(),base,s)){error=QString("Surface %1 (%2) has invalid corners or mesh points. The current project was kept.").arg(++surfaceIndex).arg(v.toObject()["name"].toString("unnamed"));return false;}restored.append(s);++surfaceIndex;}
    for(const auto &v:j["surfaces"].toArray()){const double id=v.toObject()["pattern"].toDouble();if(!std::isfinite(id)||id!=std::floor(id)||id<0||id>=potatoPatterns().size()){error="This project uses an unsupported animation ID. Current project kept.";return false;}}
    QSet<QString> routeIds;
    for(auto &s:restored){if(s.routeUid.isEmpty()||routeIds.contains(s.routeUid)){if(j["version"].toInt()==2){error="Duplicate or empty surface route identity.";return false;}s.routeUid=QUuid::createUuid().toString(QUuid::WithoutBraces);}routeIds.insert(s.routeUid);}
    QVector<Connection> restoredConnections;
    if(j.contains("connections")&&!j["connections"].isArray()){error="Invalid connections.";return false;}
    for(const auto &value:j["connections"].toArray()){
        const auto o=value.toObject();Connection c{o["a"].toString(),o["b"].toString(),o["edgeA"].toInt(-1),o["edgeB"].toInt(-1),o["beginB"].toDouble(-1),o["endB"].toDouble(-1),o["tied"].toBool()};
        if(!routeIds.contains(c.a)||!routeIds.contains(c.b)||c.a==c.b||c.edgeA<0||c.edgeA>3||c.edgeB<0||c.edgeB>3||o["edgeA"].toDouble()!=c.edgeA||o["edgeB"].toDouble()!=c.edgeB||!std::isfinite(c.beginB)||!std::isfinite(c.endB)||c.beginB<0||c.endB>1||c.endB-c.beginB<1e-6){error="Invalid connection: missing surface or invalid edge interval. Current project kept.";return false;}
        auto overlap=[](const Connection &x,const Connection &y){
            const QString xu[2]{x.a,x.b},yu[2]{y.a,y.b};const int xe[2]{x.edgeA,x.edgeB},ye[2]{y.edgeA,y.edgeB};
            const double loX[2]{0,x.beginB},hiX[2]{1,x.endB},loY[2]{0,y.beginB},hiY[2]{1,y.endB};
            for(int a=0;a<2;++a)for(int b=0;b<2;++b)if(xu[a]==yu[b]&&xe[a]==ye[b]&&loX[a]<hiY[b]-1e-9&&hiX[a]>loY[b]+1e-9)return true;return false;};
        for(const auto &other:restoredConnections)if(overlap(c,other)){error="Overlapping connections.";return false;}
        restoredConnections<<c;
    }
    surfaces=restored;connections=restoredConnections;boundaryActors=j["boundaryActors"].toBool();wizardActors=j["wizardActors"].toBool();actorElapsed=0;outputSize=size;selected=surfaces.isEmpty()?-1:0;
    const auto restoredDynamic=PotatoDynamic::Settings::fromJson(j["dynamic"].toObject());
    if(restartDynamic||restoredDynamic.effect!=dynamic.effect||restoredDynamic.members!=dynamic.members)dynamicElapsed=0;
    dynamic=restoredDynamic;rebuildDynamic();
    for(auto *src:mediaSources)if(src->player)src->player->pause();
    for(const auto &s:surfaces){if(s.media.isEmpty()||QFileInfo::exists(s.media)){auto *src=source(s.media);if(src->player)src->player->play();}else emit message("Missing media: "+s.media+". Select its surface and use Load media to locate it.");}
    emit structureChanged();emit changed();return true;
}
void Scene::checkpoint(){history.append(json());if(history.size()>80)history.removeFirst();future.clear();}
void Scene::undo(){if(history.isEmpty())return;future.append(json());QString e;restore(history.takeLast(),{},e);touch(true);}
void Scene::redo(){if(future.isEmpty())return;history.append(json());QString e;restore(future.takeLast(),{},e);touch(true);}
bool Scene::save(const QString &path,QString &error){
    if(!savedDigest.isEmpty()&&QFileInfo(path).absoluteFilePath()==QFileInfo(projectPath).absoluteFilePath()){
        QFile current(path);if(!current.open(QIODevice::ReadOnly)||QCryptographicHash::hash(current.readAll(),QCryptographicHash::Sha256)!=savedDigest){error="This mapping changed on disk in another app, or was moved. Your open work is intact. Use Save as to keep a separate copy, then reopen the other version.";return false;}
    }
    if(json()["version"].toInt()==2&&QFileInfo::exists(path)){
        QFile old(path);if(!old.open(QIODevice::ReadOnly)){error=old.errorString();return false;}
        const auto oldBytes=old.readAll();old.close();
        if(QJsonDocument::fromJson(oldBytes).object()["version"].toInt()==1){
            QString recovery=path+".v1-recovery.pmap";
            auto matches=[&](const QString &p){QFile f(p);return f.open(QIODevice::ReadOnly)&&f.readAll()==oldBytes;};
            if(QFileInfo::exists(recovery)&&!matches(recovery))recovery=path+".v1-"+QString::fromLatin1(QCryptographicHash::hash(oldBytes,QCryptographicHash::Sha256).toHex())+".pmap";
            if(QFileInfo::exists(recovery)){if(!matches(recovery)){error="Recovery path collision. Original project kept.";return false;}}
            else {QSaveFile backup(recovery);if(!backup.open(QIODevice::WriteOnly)||backup.write(oldBytes)!=oldBytes.size()||!backup.commit()){error="Cannot retain permanent pre-conversion recovery copy. Original unchanged.";return false;}}
        }
    }
    if(!PotatoProjects::backup(path,error))return false;
    QSaveFile file(path);if(!file.open(QIODevice::WriteOnly)){error=file.errorString();return false;}
    const auto bytes=QJsonDocument(json(QFileInfo(path).absolutePath())).toJson();
    if(file.write(bytes)!=bytes.size()||!file.commit()){error=file.errorString();return false;}
    projectPath=path;savedDigest=QCryptographicHash::hash(bytes,QCryptographicHash::Sha256);dirty=false;emit structureChanged();return true;
}
bool Scene::load(const QString &path,QString &error){
    QFile file(path);if(!file.open(QIODevice::ReadOnly)){error=file.errorString();return false;}
    const auto savedBytes=file.readAll();QJsonParseError parse;auto doc=QJsonDocument::fromJson(savedBytes,&parse);
    if(parse.error!=QJsonParseError::NoError){error=parse.errorString();return false;}
    if(!restore(doc.object(),QFileInfo(path).absolutePath(),error,true))return false;
    skipEntrance();
    projectPath=path;savedDigest=QCryptographicHash::hash(savedBytes,QCryptographicHash::Sha256);history.clear();future.clear();dirty=false;emit structureChanged();return true;
}
