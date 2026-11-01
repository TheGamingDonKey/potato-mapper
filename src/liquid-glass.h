#pragma once
#include <QApplication>
#include <QCache>
#include <QColor>
#include <QDebug>
#include <QImage>
#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <QRadialGradient>
#include <QVariant>
#include <QtMath>
#include <cmath>

// App-rendered glass. The continuous studio environment belongs to editor
// chrome, never to the scene, screen or projector framebuffer. Qt continues to
// paint all foreground text and own the real controls and their hit regions.
namespace PotatoGlass {
enum class Role { Foundation,Shell,Panel,Section,Heading,Button,Input,Well,Dialog };
inline bool active(){return qApp&&qApp->property("potatoInterfaceTheme").toString()==QStringLiteral("liquidglass1");}
inline QColor accent(){return QColor(143,208,178);}
inline bool reducedTransparency(){return qApp&&qApp->property("potatoReducedTransparency").toBool();}
inline bool increasedContrast(){return qApp&&qApp->property("potatoIncreasedContrast").toBool();}
inline bool reducedMotion(){return qApp&&qApp->property("potatoReducedMotion").toBool();}
inline bool recessed(Role role){return role==Role::Input||role==Role::Well;}
inline bool control(Role role){return role==Role::Button||role==Role::Heading;}
inline qreal radius(Role role){
    if(role==Role::Foundation||role==Role::Shell)return 0;
    if(role==Role::Dialog)return 9;
    if(role==Role::Panel||role==Role::Section||role==Role::Well)return 6;
    return 5;
}

namespace Detail {
struct Store {
    QImage environment;
    QSizeF sceneSize;
    QSize pixelSize;
    QCache<QString,QImage> plates{16384}; // KiB; shared by all Glass widgets.
    quint64 environmentBuilds=0,plateBuilds=0;
};
inline Store &store(){static Store value;return value;}
inline void publishBudget(){
    if(!qApp)return;
    const auto &s=store();
    qApp->setProperty("potatoGlassCacheKiB",QVariant::fromValue<qlonglong>(s.plates.totalCost()));
    qApp->setProperty("potatoGlassCacheLimitKiB",16384);
    qApp->setProperty("potatoGlassCacheBytes",QVariant::fromValue<qlonglong>(s.plates.totalCost()*1024));
    qApp->setProperty("potatoGlassEnvironmentBytes",QVariant::fromValue<qlonglong>(s.environment.sizeInBytes()));
    qApp->setProperty("potatoGlassEnvironmentPixels",QVariant::fromValue<qlonglong>(qint64(s.environment.width())*s.environment.height()));
    qApp->setProperty("potatoGlassEnvironmentBuilds",QVariant::fromValue<qulonglong>(s.environmentBuilds));
    qApp->setProperty("potatoGlassPlateBuilds",QVariant::fromValue<qulonglong>(s.plateBuilds));
}
inline void ensureEnvironment(const QSizeF &requested){
    auto &s=store();const QSizeF logical(qMax(1.0,requested.width()),qMax(1.0,requested.height()));
    // One resolution for the editor space, independent of the painting child's
    // monitor DPR. Floating dialogs on another monitor must not alternate the
    // environment size and evict every other widget's cached plate.
    const qreal scale=qMin<qreal>(1,qSqrt(1000000.0/(logical.width()*logical.height())));
    const QSize pixels(qMax(1,qFloor(logical.width()*scale)),qMax(1,qFloor(logical.height()*scale)));
    if(s.sceneSize==logical&&s.pixelSize==pixels&&!s.environment.isNull())return;
    s.sceneSize=logical;s.pixelSize=pixels;s.plates.clear();
    s.environment=QImage(pixels,QImage::Format_RGB32);s.environment.fill(QColor(23,26,25));
    QPainter p(&s.environment);p.scale(pixels.width()/logical.width(),pixels.height()/logical.height());
    const QRectF area(QPointF(0,0),logical);const qreal w=logical.width(),h=logical.height();
    // Broad studio softboxes and their reflection edges, in one shared space.
    // These are light sources, not moving artwork or repeated button textures.
    QLinearGradient studio(0,0,w*.72,h*.4);
    studio.setColorAt(0,QColor(95,99,97));studio.setColorAt(.17,QColor(54,57,55));
    studio.setColorAt(.24,QColor(42,45,43));studio.setColorAt(.30,QColor(60,63,61));
    studio.setColorAt(.36,QColor(97,101,98));studio.setColorAt(.43,QColor(74,78,75));
    studio.setColorAt(.52,QColor(44,48,45));studio.setColorAt(.70,QColor(28,31,29));
    studio.setColorAt(1,QColor(52,57,54));p.fillRect(area,studio);
    QRadialGradient pearl(w*.18,-h*.16,qMax(w*.55,h*.55));
    pearl.setColorAt(0,QColor(232,238,232,166));pearl.setColorAt(.45,QColor(190,198,192,34));
    pearl.setColorAt(1,Qt::transparent);p.fillRect(area,pearl);
    QRadialGradient cool(w*.92,h*.67,qMax(w,h)*.52);
    cool.setColorAt(0,QColor(121,134,129,34));cool.setColorAt(1,Qt::transparent);p.fillRect(area,cool);
    QLinearGradient reflectedEdge(w*.06,0,w*.38,h);
    reflectedEdge.setColorAt(0,Qt::transparent);reflectedEdge.setColorAt(.36,Qt::transparent);
    reflectedEdge.setColorAt(.43,QColor(222,225,222,12));reflectedEdge.setColorAt(.5,QColor(232,236,231,48));
    reflectedEdge.setColorAt(.57,QColor(195,204,197,12));reflectedEdge.setColorAt(.66,Qt::transparent);
    reflectedEdge.setColorAt(1,Qt::transparent);p.fillRect(area,reflectedEdge);p.end();
    ++s.environmentBuilds;publishBudget();
}
inline QColor bed(Role role){
    if(role==Role::Foundation)return QColor(23,26,25);
    if(role==Role::Shell)return QColor(36,39,37);
    if(recessed(role))return QColor(29,34,32);
    if(control(role))return QColor(45,49,47);
    if(role==Role::Dialog)return QColor(39,42,41);
    return QColor(39,42,41);
}
inline QRgb sample(const QImage &source,qreal x,qreal y){
    x=qBound(0.0,x,double(source.width()-1));y=qBound(0.0,y,double(source.height()-1));
    const int x0=int(x),y0=int(y),x1=qMin(x0+1,source.width()-1),y1=qMin(y0+1,source.height()-1);
    const qreal fx=x-x0,fy=y-y0;
    const auto *a=reinterpret_cast<const QRgb*>(source.constScanLine(y0));
    const auto *b=reinterpret_cast<const QRgb*>(source.constScanLine(y1));
    auto channel=[&](int shift){
        const qreal top=((a[x0]>>shift)&255)*(1-fx)+((a[x1]>>shift)&255)*fx;
        const qreal bottom=((b[x0]>>shift)&255)*(1-fx)+((b[x1]>>shift)&255)*fx;
        return qRound(top*(1-fy)+bottom*fy);
    };
    return qRgb(channel(16),channel(8),channel(0));
}
inline QImage plate(const QSizeF &logical,Role role,const QPointF &origin,const QSizeF &scene,qreal dpr,int state){
    auto &s=store();const bool solid=reducedTransparency()||increasedContrast();
    if(!solid)ensureEnvironment(scene);
    // Optical substrates are soft low-frequency images. Rasterizing every
    // overlapping container at monitor DPR exhausted the shared 16-MiB LRU:
    // an 8-MB foundation plus dialog/sidebar/sections evicted one another on
    // every otherwise-identical repaint. Keep substrate density at logical 1x
    // and <=512k pixels; foregrounds and paintRim remain native-DPR vectors.
    const qreal scale=qMin<qreal>(qMin<qreal>(1,dpr),qSqrt(512000.0/qMax(1.0,logical.width()*logical.height())));
    const QSize pixels(qMax(1,qFloor(logical.width()*scale)),qMax(1,qFloor(logical.height()*scale)));
    const QString key=QStringLiteral("glass1/%1/%2x%3/%4,%5/%6x%7/%8/%9/%10%11/%12x%13")
        .arg(int(role)).arg(pixels.width()).arg(pixels.height()).arg(qRound(origin.x()*scale)).arg(qRound(origin.y()*scale))
        .arg(qRound(scene.width()*scale)).arg(qRound(scene.height()*scale)).arg(qRound(scale*1000))
        .arg(state&~1).arg(reducedTransparency()?1:0).arg(increasedContrast()?1:0)
        .arg(QString::number(logical.width(),'g',16)).arg(QString::number(logical.height(),'g',16));
    if(auto *cached=s.plates.object(key))return *cached;
    if(qEnvironmentVariableIsSet("POTATO_GLASS_TRACE_CACHE"))
        qInfo()<<"GLASS CACHE MISS role"<<int(role)<<"logical"<<logical<<"pixels"<<pixels<<"origin"<<origin<<"state"<<(state&~1)<<"retained KiB"<<s.plates.totalCost()<<"entries"<<s.plates.count();
    QImage image(pixels,QImage::Format_ARGB32_Premultiplied);image.fill(Qt::transparent);
    image.setDevicePixelRatio(scale);
    const qreal w=logical.width(),h=logical.height(),r=qMin(radius(role),qMin(w,h)*.5);
    QColor base=bed(role);
    if(state&4)base=QColor((base.red()*4+143)/5,(base.green()*4+208)/5,(base.blue()*4+178)/5);
    if(state&2)base=base.darker(113);
    if(state&8)base=QColor(36,39,37);
    const qreal transmission=solid?0:((state&8)? .12:(recessed(role)? .12:(role==Role::Foundation?.64:.30)));
    for(int y=0;y<pixels.height();++y){
        auto *row=reinterpret_cast<QRgb*>(image.scanLine(y));const qreal py=(y+.5)/scale;
        for(int x=0;x<pixels.width();++x){
            const qreal px=(x+.5)/scale;
            const qreal dx=qAbs(px-w*.5)-(w*.5-r),dy=qAbs(py-h*.5)-(h*.5-r);
            const qreal outsideX=qMax(dx,0.0),outsideY=qMax(dy,0.0);
            const qreal distance=-(std::hypot(outsideX,outsideY)+qMin(qMax(dx,dy),0.0)-r);
            const qreal coverage=qBound(0.0,.5+distance*scale,1.0);
            if(coverage<=0)continue;
            const qreal band=qMin(qMin(w,h)*.26,recessed(role)?5.5:10.0);
            const qreal edge=qBound(0.0,1-distance/qMax(1.0,band),1.0);
            qreal nx=px-qBound(r,px,w-r),ny=py-qBound(r,py,h-r);
            const qreal length=std::hypot(nx,ny);
            if(length>.001){nx/=length;ny/=length;}
            else if(qMin(px,w-px)<qMin(py,h-py)){nx=px<w*.5?-1:1;ny=0;}
            else{nx=0;ny=py<h*.5?-1:1;}
            // Inverse sampling bends the actual shared source at the perimeter.
            // Smoked absorption dominates the center so foreground ink stays calm.
            const qreal displacement=solid?0:edge*edge*(recessed(role)?-3.2:7.5);
            const qreal sx=(origin.x()+px+nx*displacement)/qMax(1.0,scene.width())*qMax(1,s.environment.width()-1);
            const qreal sy=(origin.y()+py+ny*displacement)/qMax(1.0,scene.height())*qMax(1,s.environment.height()-1);
            const QRgb source=solid?qRgb(base.red(),base.green(),base.blue()):sample(s.environment,sx,sy);
            const qreal amount=qMin(.82,transmission+((state&8)?0:edge*edge*.45));
            const qreal inactive=(state&16)?.82:1.0;
            const int red=qBound(0,qRound((base.red()*(1-amount)+qRed(source)*amount)*inactive),255);
            const int green=qBound(0,qRound((base.green()*(1-amount)+qGreen(source)*amount)*inactive),255);
            const int blue=qBound(0,qRound((base.blue()*(1-amount)+qBlue(source)*amount)*inactive),255);
            const int alpha=qRound(255*coverage);row[x]=qPremultiply(qRgba(red,green,blue,alpha));
        }
    }
    {
        QPainter p(&image);p.setRenderHint(QPainter::Antialiasing);
        const QRectF rect(QPointF(0,0),logical);QPainterPath shape;shape.addRoundedRect(rect,r,r);p.setClipPath(shape);
        if(!solid&&!(state&8)){
            const qreal depth=recessed(role)?5:qMin<qreal>(control(role)?h*.58:23,h*.68);
            QLinearGradient reflection(0,0,0,qMax(1.0,depth));
            const int intensity=(state&16)?20:(control(role)?64:45);
            reflection.setColorAt(0,QColor(238,242,237,intensity));
            reflection.setColorAt(.18,QColor(229,235,229,intensity/2));
            reflection.setColorAt(.78,QColor(213,220,214,5));reflection.setColorAt(1,Qt::transparent);
            p.fillRect(QRectF(0,0,w,depth),reflection);
            if(control(role)){
                QPainterPath crest;crest.moveTo(1,h*.30);crest.cubicTo(w*.24,h*.04,w*.65,h*.53,w-1,h*.19);
                crest.lineTo(w-1,0);crest.lineTo(1,0);crest.closeSubpath();
                QLinearGradient curved(0,0,0,h*.48);curved.setColorAt(0,QColor(242,246,241,25));curved.setColorAt(1,Qt::transparent);
                p.fillPath(crest,curved);
            }
        }
        if(recessed(role)){
            QLinearGradient inset(0,0,0,qMin(h,12.0));inset.setColorAt(0,QColor(0,0,0,68));inset.setColorAt(1,Qt::transparent);
            p.fillRect(rect,inset);
        }
    }
    ++s.plateBuilds;
    s.plates.insert(key,new QImage(image),qMax<qsizetype>(1,(image.sizeInBytes()+1023)/1024));publishBudget();
    return image;
}
}

inline void clearCaches(){auto &s=Detail::store();s.plates.clear();s.environment=QImage();s.sceneSize=QSizeF();s.pixelSize=QSize();Detail::publishBudget();}

inline void paintRim(QPainter &p,const QRectF &bounds,Role role,int state){
    if(bounds.width()<2||bounds.height()<2||role==Role::Foundation)return;
    p.save();p.setRenderHint(QPainter::Antialiasing);
    const QRectF rect=bounds.adjusted(.5,.5,-.5,-.5);const qreal r=radius(role);
    const bool disabled=state&8,inactive=state&16,quiet=reducedTransparency()||increasedContrast();
    const bool compact=control(role)||recessed(role)||role==Role::Section;
    const qreal rimWeight=compact&&!increasedContrast()?.5:1.0;
    const qreal intensity=(disabled?.30:(inactive?.58:1.0))*rimWeight;
    const int strength=increasedContrast()?220:(recessed(role)?108:170);
    QLinearGradient pearl(rect.topLeft(),rect.bottomRight());
    pearl.setColorAt(0,QColor(232,238,232,qRound(strength*intensity)));
    pearl.setColorAt(.24,QColor(232,238,232,qRound((strength*.62)*intensity)));
    pearl.setColorAt(.5,QColor(129,139,132,qRound(36*intensity)));
    pearl.setColorAt(.82,QColor(18,22,19,qRound(170*intensity)));
    pearl.setColorAt(1,QColor(213,223,214,qRound(82*intensity)));
    p.setBrush(Qt::NoBrush);p.setPen(QPen(QBrush(pearl),1));p.drawRoundedRect(rect,r,r);
    if(role==Role::Shell){
        p.setPen(QColor(7,13,9,175));p.drawLine(rect.bottomLeft(),rect.bottomRight());
        p.setPen(QColor(221,230,221,quiet?55:95));p.drawLine(rect.topLeft(),rect.topRight());p.restore();return;
    }
    if(!disabled){
        QLinearGradient inner(rect.topLeft(),rect.bottomLeft());
        inner.setColorAt(0,QColor(7,11,9,qRound((quiet?80:140)*intensity)));inner.setColorAt(.46,QColor(10,15,12,qRound(42*intensity)));
        inner.setColorAt(1,QColor(219,231,220,qRound((quiet?28:78)*intensity)));
        p.setPen(QPen(QBrush(inner),1));p.drawRoundedRect(rect.adjusted(1,1,-1,-1),qMax(1.0,r-1),qMax(1.0,r-1));
        if(!quiet&&!recessed(role)){
            // The narrow lower caustic sits inside the allocation; no shadows
            // expand hit regions or require a transparent sibling overlay.
            QLinearGradient caustic(rect.left(),0,rect.right(),0);
            caustic.setColorAt(0,Qt::transparent);caustic.setColorAt(.34,QColor(211,225,215,qRound(95*intensity)));
            caustic.setColorAt(.7,QColor(232,240,232,qRound(48*intensity)));caustic.setColorAt(1,Qt::transparent);
            p.setPen(QPen(QBrush(caustic),(state&2)?1:1.5));
            p.drawLine(QPointF(rect.left()+r+1,rect.bottom()-2),QPointF(rect.right()-r-1,rect.bottom()-2));
        }
    }
    if((state&1)&&!disabled&&control(role)){
        p.setPen(QPen(QColor(236,242,235,qRound((quiet?110:180)*rimWeight)),1));
        p.drawRoundedRect(rect.adjusted(.5,.5,-.5,-.5),qMax(1.0,r-.5),qMax(1.0,r-.5));
    }
    p.restore();
}

inline void paintSurface(QPainter &p,const QRectF &rect,Role role,const QPointF &sceneOrigin,const QSizeF &sceneSize,int state){
    if(rect.width()<1||rect.height()<1)return;
    const qreal dpr=p.device()?p.device()->devicePixelRatioF():1;
    const QImage image=Detail::plate(rect.size(),role,sceneOrigin,sceneSize,dpr,state);
    p.save();p.setRenderHint(QPainter::SmoothPixmapTransform);p.drawImage(rect,image);p.restore();
    paintRim(p,rect,role,state);
}
}
