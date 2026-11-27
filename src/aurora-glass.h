#pragma once
#include "liquid-glass.h"

// Independent clear-glass profiles. Only app-owned, stationary artwork is
// transmitted through chrome; neither profile reads the desktop or GL canvas.
namespace PotatoAurora {
using Role=PotatoGlass::Role;
inline bool auroraClear(){return qApp&&qApp->property("potatoInterfaceTheme").toString()==QStringLiteral("auroraclear1");}
inline bool transparentGlass(){return qApp&&qApp->property("potatoInterfaceTheme").toString()==QStringLiteral("transparentglass1");}
inline bool active(){return auroraClear()||transparentGlass();}
inline QColor accent(){return auroraClear()?QColor("#A1F2F2"):QColor("#80E2D7");}
inline bool reducedTransparency(){return PotatoGlass::reducedTransparency();}
inline bool increasedContrast(){return PotatoGlass::increasedContrast();}
inline bool reducedMotion(){return PotatoGlass::reducedMotion();}
inline qreal radius(Role role){
    if(role==Role::Foundation||role==Role::Shell)return 0;
    if(role==Role::Panel||role==Role::Dialog)return 16;
    if(role==Role::Well)return 10;
    return 8;
}

namespace Detail {
struct Store {
    QImage environment;
    QSizeF sceneSize;
    QString profile;
    QCache<QString,QImage> plates{16384}; // KiB; one shared, bounded LRU.
    quint64 environmentBuilds=0,plateBuilds=0;
};
inline Store &store(){static Store value;return value;}
inline void publishBudget(){
    if(!qApp)return;
    const auto &s=store();
    qApp->setProperty("potatoAuroraCacheKiB",s.plates.totalCost());
    qApp->setProperty("potatoAuroraCacheLimitKiB",16384);
    qApp->setProperty("potatoAuroraCacheBytes",QVariant::fromValue<qlonglong>(qint64(s.plates.totalCost())*1024));
    qApp->setProperty("potatoAuroraEnvironmentBytes",QVariant::fromValue<qlonglong>(s.environment.sizeInBytes()));
    qApp->setProperty("potatoAuroraEnvironmentPixels",QVariant::fromValue<qlonglong>(qint64(s.environment.width())*s.environment.height()));
    qApp->setProperty("potatoAuroraEnvironmentBuilds",QVariant::fromValue<qulonglong>(s.environmentBuilds));
    qApp->setProperty("potatoAuroraPlateBuilds",QVariant::fromValue<qulonglong>(s.plateBuilds));
}
inline void ensureEnvironment(const QSizeF &requested){
    auto &s=store();
    const QSizeF logical(qMax(1.0,requested.width()),qMax(1.0,requested.height()));
    const QString profile=auroraClear()?QStringLiteral("auroraclear1"):QStringLiteral("transparentglass1");
    if(s.sceneSize==logical&&s.profile==profile&&!s.environment.isNull())return;
    // Sampling density is independent of child-monitor DPR. A dialog moving
    // between monitors must not rebuild the editor backdrop on each repaint.
    const qreal scale=qMin<qreal>(1,qSqrt(1000000.0/(logical.width()*logical.height())));
    const QSize pixels(qMax(1,qFloor(logical.width()*scale)),qMax(1,qFloor(logical.height()*scale)));
    s.sceneSize=logical;s.profile=profile;s.plates.clear();
    s.environment=QImage(pixels,QImage::Format_RGB32);
    QPainter p(&s.environment);p.setRenderHint(QPainter::Antialiasing);
    p.scale(pixels.width()/logical.width(),pixels.height()/logical.height());
    const qreal w=logical.width(),h=logical.height();const QRectF area(QPointF(),logical);
    QLinearGradient base(0,0,w,h);
    if(auroraClear()){
        // Exact AuroraClear Figma base, ribbon stops and normalized paths.
        base.setColorAt(0,QColor("#285A9C"));base.setColorAt(.43,QColor("#484A97"));base.setColorAt(1,QColor("#176D7E"));
        p.fillRect(area,base);
        QLinearGradient ribbon(0,0,w,h);
        ribbon.setColorAt(0,QColor("#AD5276"));ribbon.setColorAt(.35,QColor("#6B4BBD"));
        ribbon.setColorAt(.68,QColor("#3A64B3"));ribbon.setColorAt(1,QColor("#15778C"));
        QPainterPath flow;flow.moveTo(-w*.2,h*1.12);
        flow.cubicTo(w*.68,h*.82,-w*.23,h*.34,w*.48,h*.06);
        flow.cubicTo(w*1.19,-h*.22,w*.97,h*.1,w*1.25,-h*.26);
        p.setOpacity(.85);p.setPen(QPen(QBrush(ribbon),qMin(w,h)*.30));p.drawPath(flow);
        QPainterPath cyan;cyan.moveTo(-w*.14,h*1.05);
        cyan.cubicTo(w*.60,h*.84,-w*.13,h*.39,w*.56,h*.14);
        cyan.cubicTo(w*1.25,-h*.11,w*.97,h*.21,w*1.23,-h*.10);
        p.setOpacity(.52);p.setPen(QPen(QColor("#23849B"),qMin(w,h)*.12));p.drawPath(cyan);
        p.setOpacity(.24);p.setPen(QPen(QColor("#9FDDF3"),1.5));p.drawPath(cyan);
        p.setOpacity(.12);p.setPen(Qt::NoPen);p.setBrush(QColor("#BE5B85"));
        p.drawEllipse(QPointF(w*.1,h*.1),w*.27,h*.20);
    }else{
        // The earlier transparent study has its own navy environment, broader
        // rose flow and soft cyan fold. It is not a desaturated Aurora preset.
        base.setColorAt(0,QColor("#2C223C"));base.setColorAt(.55,QColor("#113B46"));base.setColorAt(1,QColor("#102335"));
        p.fillRect(area,base);
        QLinearGradient ribbon(0,0,w*.8,h*.65);
        ribbon.setColorAt(0,QColor("#A77776"));ribbon.setColorAt(.5,QColor("#766394"));ribbon.setColorAt(1,QColor("#347F88"));
        QPainterPath flow;flow.moveTo(-w*.19,h*1.04);
        flow.cubicTo(w*.52,h*.80,-w*.12,h*.39,w*.33,h*.12);
        flow.cubicTo(w*.63,-h*.06,w*.89,h*.18,w*1.14,-h*.16);
        p.setOpacity(.76);p.setPen(QPen(QBrush(ribbon),qMin(w,h)*.38));p.drawPath(flow);
        QPainterPath cyan;cyan.moveTo(-w*.12,h*1.04);
        cyan.cubicTo(w*.49,h*.78,-w*.01,h*.47,w*.39,h*.18);
        cyan.cubicTo(w*.67,-h*.02,w*.94,h*.16,w*1.13,-h*.1);
        p.setOpacity(.25);p.setPen(QPen(QColor("#70B3B7"),qMin(w,h)*.11));p.drawPath(cyan);
        p.setOpacity(.25);p.setPen(QPen(QColor("#B7E6EB"),1.5));p.drawPath(cyan);
        QRadialGradient rose(w*.05,0,qMax(w,h)*.76);
        rose.setColorAt(0,QColor(188,146,162,70));rose.setColorAt(1,Qt::transparent);
        p.setOpacity(1);p.fillRect(area,rose);
    }
    p.end();++s.environmentBuilds;publishBudget();
}
inline void familyColors(const QString &family,QColor &first,QColor &last){
    first=QColor("#597EBB");last=QColor("#3D5F9F");
    if(family=="peach"){first=QColor("#DC7445");last=QColor("#B43F76");}
    else if(family=="iris"){first=QColor("#956BE6");last=QColor("#6540BD");}
    else if(family=="lagoon"){first=QColor("#1C9F9B");last=QColor("#2665BC");}
    else if(family=="coral"){first=QColor("#DC5279");last=QColor("#A43C83");}
}
inline QColor solidBed(Role role){
    if(increasedContrast())return QColor("#14243D");
    if(PotatoGlass::recessed(role))return auroraClear()?QColor("#263F6D"):QColor("#1D3545");
    return auroraClear()?QColor("#354B81"):QColor("#30485B");
}
inline QImage plate(const QSizeF &logical,Role role,const QPointF &origin,const QSizeF &scene,qreal dpr,int state,const QString &family){
    auto &s=store();const bool solid=reducedTransparency()||increasedContrast();
    if(!solid)ensureEnvironment(scene);
    // Low-frequency substrates stay <=512k pixels each; rims, ink and icons
    // remain native-DPR vectors. The entire plate working set is capped at 16MiB.
    const qreal scale=qMin<qreal>(qMin<qreal>(1,dpr),qSqrt(512000.0/qMax(1.0,logical.width()*logical.height())));
    const QSize pixels(qMax(1,qFloor(logical.width()*scale)),qMax(1,qFloor(logical.height()*scale)));
    const QString key=QStringLiteral("%1/%2/%3x%4/%5,%6/%7x%8/%9/%10/%11/%12/%13x%14")
        .arg(auroraClear()?1:2).arg(int(role)).arg(pixels.width()).arg(pixels.height())
        .arg(QString::number(origin.x(),'g',16)).arg(QString::number(origin.y(),'g',16))
        .arg(QString::number(scene.width(),'g',16)).arg(QString::number(scene.height(),'g',16))
        .arg(qRound(scale*1000)).arg(state&46).arg(solid?(increasedContrast()?2:1):0).arg(family)
        .arg(QString::number(logical.width(),'g',16)).arg(QString::number(logical.height(),'g',16));
    if(auto *cached=s.plates.object(key))return *cached;
    QImage image(pixels,QImage::Format_ARGB32_Premultiplied);image.fill(Qt::transparent);image.setDevicePixelRatio(scale);
    const qreal w=logical.width(),h=logical.height(),r=qMin(PotatoAurora::radius(role),qMin(w,h)*.5);
    const bool primary=(state&32)&&role==Role::Button,disabled=state&8;
    const bool aurora=auroraClear();const QColor bed=solidBed(role);
    QColor first(Qt::black),middle(Qt::black),last(Qt::black);
    qreal firstAlpha=0,middleAlpha=0,lastAlpha=0,middleStop=0;
    if(aurora){
        if(role==Role::Dialog){
            first=QColor("#1E4678");middle=QColor("#2B4383");last=QColor("#17586F");
            firstAlpha=.16;middleAlpha=.13;lastAlpha=.12;middleStop=.5;
        }else if(role==Role::Input){first=last=QColor("#223F72");firstAlpha=lastAlpha=.28;}
        else if(role==Role::Well){first=last=QColor("#24476F");firstAlpha=lastAlpha=.14;}
        else if(role==Role::Heading){first=last=QColor("#2E4D8E");firstAlpha=lastAlpha=.18;}
        else if(role==Role::Button){familyColors(family,first,last);firstAlpha=(state&4)?.34:.22;lastAlpha=(state&4)?.24:.12;}
        else if(role!=Role::Foundation){
            first=QColor("#5CA5D2");middle=QColor("#1F3B6B");last=QColor("#447EA4");
            firstAlpha=.12;middleAlpha=.10;lastAlpha=.08;middleStop=.46;
        }
        if(primary){first=QColor("#147B81");last=QColor("#225FA5");firstAlpha=lastAlpha=disabled?.28:1;middleStop=0;}
    }else{
        if(role==Role::Input||role==Role::Well){first=last=QColor("#0B1B2D");firstAlpha=lastAlpha=role==Role::Input?.34:.27;}
        else if(role!=Role::Foundation){first=last=QColor(Qt::white);firstAlpha=lastAlpha=role==Role::Button?.10:(role==Role::Heading?.07:.095);}
        if((state&4)&&role==Role::Button){first=last=QColor("#80E2D7");firstAlpha=lastAlpha=.18;}
        if(primary){first=last=QColor("#80E2D7");firstAlpha=lastAlpha=disabled?.28:1;}
    }
    const qreal tintFactor=(solid&&!primary)?0:((disabled&&!primary)?.36:1);
    const qreal pressed=(state&2)&&!disabled?.92:1;
    for(int y=0;y<pixels.height();++y){
        auto *row=reinterpret_cast<QRgb*>(image.scanLine(y));const qreal py=(y+.5)/scale;
        for(int x=0;x<pixels.width();++x){
            const qreal px=(x+.5)/scale;
            const qreal dx=qAbs(px-w*.5)-(w*.5-r),dy=qAbs(py-h*.5)-(h*.5-r);
            const qreal distance=-(std::hypot(qMax(dx,0.0),qMax(dy,0.0))+qMin(qMax(dx,dy),0.0)-r);
            const qreal coverage=qBound(0.0,.5+distance*scale,1.0);if(coverage<=0)continue;
            const qreal band=qMin(7.0,qMin(w,h)*.22),edge=qBound(0.0,1-distance/qMax(1.0,band),1.0);
            qreal nx=px-qBound(r,px,w-r),ny=py-qBound(r,py,h-r);const qreal length=std::hypot(nx,ny);
            if(length>.001){nx/=length;ny/=length;}
            else if(qMin(px,w-px)<qMin(py,h-py)){nx=px<w*.5?-1:1;ny=0;}
            else{nx=0;ny=py<h*.5?-1:1;}
            const qreal displacement=(solid||role==Role::Foundation)?0:edge*edge*(PotatoGlass::recessed(role)?-1.6:2.6);
            const qreal sx=(origin.x()+px+nx*displacement)/qMax(1.0,scene.width())*qMax(1,s.environment.width()-1);
            const qreal sy=(origin.y()+py+ny*displacement)/qMax(1.0,scene.height())*qMax(1,s.environment.height()-1);
            const QRgb source=solid?bed.rgb():PotatoGlass::Detail::sample(s.environment,sx,sy);
            const qreal t=qBound(0.0,(px/w+py/h)*.5,1.0);
            const bool firstSegment=middleStop>0&&t<middleStop;
            const QColor &from=firstSegment||middleStop==0?first:middle;
            const QColor &to=firstSegment?middle:last;
            const qreal mix=middleStop==0?t:(firstSegment?t/middleStop:(t-middleStop)/(1-middleStop));
            const qreal alphaFrom=firstSegment||middleStop==0?firstAlpha:middleAlpha;
            const qreal alphaTo=firstSegment?middleAlpha:lastAlpha;
            const qreal amount=(alphaFrom*(1-mix)+alphaTo*mix)*tintFactor;
            const auto channel=[&](int sourceChannel,int fromChannel,int toChannel){
                return qBound(0,qRound((sourceChannel*(1-amount)+(fromChannel*(1-mix)+toChannel*mix)*amount)*pressed),255);
            };
            const int red=channel(qRed(source),from.red(),to.red());
            const int green=channel(qGreen(source),from.green(),to.green());
            const int blue=channel(qBlue(source),from.blue(),to.blue());
            row[x]=qPremultiply(qRgba(qBound(0,red,255),qBound(0,green,255),qBound(0,blue,255),qRound(255*coverage)));
        }
    }
    ++s.plateBuilds;s.plates.insert(key,new QImage(image),int(qMax<qsizetype>(1,(image.sizeInBytes()+1023)/1024)));publishBudget();
    return image;
}
}

inline void clearCaches(){auto &s=Detail::store();s.plates.clear();s.environment=QImage();s.sceneSize=QSizeF();s.profile.clear();Detail::publishBudget();}
inline void paintRim(QPainter &p,const QRectF &bounds,Role role,int state){
    if(bounds.width()<2||bounds.height()<2||role==Role::Foundation||role==Role::Section)return;
    p.save();p.setRenderHint(QPainter::Antialiasing);p.setBrush(Qt::NoBrush);
    const QRectF rect=bounds.adjusted(.5,.5,-.5,-.5);const qreal r=PotatoAurora::radius(role);
    const int strength=increasedContrast()?170:((state&8)?20:((state&1)?88:56));
    QLinearGradient rim(rect.topLeft(),rect.bottomRight());
    rim.setColorAt(0,QColor(215,241,255,strength));rim.setColorAt(.48,QColor(215,241,255,qRound(strength*.62)));
    rim.setColorAt(1,QColor(183,226,246,qRound(strength*.48)));
    p.setPen(QPen(QBrush(rim),1));
    if(role==Role::Shell){p.drawLine(rect.bottomLeft(),rect.bottomRight());p.restore();return;}
    p.drawRoundedRect(rect,r,r);
    if(!(state&8)&&!reducedTransparency()&&!increasedContrast()){
        // One faint optical crest; no broad white veil or stacked bevels.
        QLinearGradient light(rect.left(),0,rect.right(),0);
        light.setColorAt(0,Qt::transparent);light.setColorAt(.22,QColor(223,246,255,60));
        light.setColorAt(.72,QColor(223,246,255,28));light.setColorAt(1,Qt::transparent);
        p.setPen(QPen(QBrush(light),1));
        p.drawLine(QPointF(rect.left()+r,rect.top()+1),QPointF(rect.right()-r,rect.top()+1));
    }
    p.restore();
}
inline void paintSurface(QPainter &p,const QRectF &rect,Role role,const QPointF &sceneOrigin,const QSizeF &sceneSize,int state,const QString &family=QString()){
    if(rect.width()<1||rect.height()<1)return;
    const qreal dpr=p.device()?p.device()->devicePixelRatioF():1;
    const QImage image=Detail::plate(rect.size(),role,sceneOrigin,sceneSize,dpr,state,family);
    p.save();p.setRenderHint(QPainter::SmoothPixmapTransform);p.drawImage(rect,image);p.restore();
    PotatoAurora::paintRim(p,rect,role,state);
}
}
