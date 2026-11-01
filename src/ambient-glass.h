#pragma once
#include "liquid-glass.h"
#include <array>

// The approved Ambient Glass study, rendered entirely from original light
// fields. One bounded environment is shared by chrome, dialogs and menus;
// neither the desktop nor the scene/projector framebuffer is sampled.
namespace PotatoAmbient {
using Role=PotatoGlass::Role;
inline bool active(){return qApp&&qApp->property("potatoInterfaceTheme").toString()==QStringLiteral("ambientglass1");}
inline bool clearGlass(){return qApp&&qApp->property("potatoAmbientGlass").toString()==QStringLiteral("clear");}
inline QColor accent(){return QColor("#A9D2CA");}
inline bool opaque(){return PotatoGlass::reducedTransparency()||PotatoGlass::increasedContrast();}
inline qreal speed(){
    bool ok=false;const qreal value=qApp?qApp->property("potatoAmbientSpeed").toDouble(&ok):.5;
    return ok&&std::isfinite(value)?qBound(.2,value,1.2):.5;
}
inline qreal intensity(){
    bool ok=false;const qreal value=qApp?qApp->property("potatoAmbientIntensity").toDouble(&ok):.65;
    return ok&&std::isfinite(value)?qBound(.2,value,1.0):.65;
}
inline qreal radius(Role role){
    if(role==Role::Foundation||role==Role::Shell)return 0;
    if(role==Role::Panel||role==Role::Dialog)return 8;
    if(role==Role::Well)return 6;
    return 5;
}

namespace Detail {
constexpr qreal tau=6.28318530717958647692;
struct Store {
    QImage environment;
    std::array<QImage,4> lights;
    QImage caustic;
    QSizeF scene;
    qreal seconds=0,intensity=-1;
    quint64 frames=0;
    bool dirty=true;
};
inline Store &store(){static Store value;return value;}
inline void publish(){
    if(!qApp)return;const auto &s=store();
    qApp->setProperty("potatoAmbientSeconds",s.seconds);
    qApp->setProperty("potatoAmbientFrames",QVariant::fromValue<qulonglong>(s.frames));
    qApp->setProperty("potatoAmbientEnvironmentPixels",QVariant::fromValue<qlonglong>(qint64(s.environment.width())*s.environment.height()));
}
inline QImage glow(const QColor &colour,bool caustic=false){
    QImage image(256,256,QImage::Format_ARGB32_Premultiplied);image.fill(Qt::transparent);
    QRadialGradient gradient(128,128,128);
    const auto stop=[&](qreal position,qreal opacity){QColor color=colour;color.setAlphaF(opacity);gradient.setColorAt(position,color);};
    if(caustic){stop(0,0);stop(.52,0);stop(.64,.045);stop(.72,.17);stop(.79,.055);stop(.92,0);stop(1,0);}
    else{stop(0,.88);stop(.24,.68);stop(.52,.31);stop(.78,.075);stop(1,0);}
    QPainter p(&image);p.fillRect(image.rect(),gradient);return image;
}
inline void ensureTextures(){
    auto &s=store();if(!s.caustic.isNull())return;
    const std::array<QColor,4> colors={QColor(63,184,165),QColor(210,128,88),QColor(148,106,191),QColor(92,143,186)};
    for(size_t i=0;i<colors.size();++i)s.lights[i]=glow(colors[i]);
    s.caustic=glow(QColor(168,211,215),true);
}
inline void ensureEnvironment(const QSizeF &requested){
    auto &s=store();const QSizeF scene(qMax(1.0,requested.width()),qMax(1.0,requested.height()));
    const qreal light=PotatoAmbient::intensity();
    if(!s.dirty&&s.scene==scene&&s.intensity==light&&!s.environment.isNull())return;
    ensureTextures();
    const qreal scale=qMin(1.0,qMin(800.0/scene.width(),500.0/scene.height()));
    const QSize pixels(qMax(1,qFloor(scene.width()*scale)),qMax(1,qFloor(scene.height()*scale)));
    if(s.environment.size()!=pixels)s.environment=QImage(pixels,QImage::Format_RGB32);
    s.scene=scene;s.intensity=light;s.dirty=false;
    QPainter p(&s.environment);p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.fillRect(s.environment.rect(),QColor("#080D17"));
    p.scale(pixels.width(),pixels.height());p.setCompositionMode(QPainter::CompositionMode_Screen);
    const auto field=[&](const QImage &texture,qreal x,qreal y,qreal w,qreal h,qreal angle,qreal opacity){
        p.save();p.translate(x,y);p.rotate(qRadiansToDegrees(angle));p.setOpacity(opacity*light);
        p.drawImage(QRectF(-w*.5,-h*.5,w,h),texture);p.restore();
    };
    const qreal t=s.seconds;
    // Exact normalized paths, colours and constant amplitudes from backdrop.js.
    field(s.lights[0],.13+.16*std::sin(tau*t/61+.2),.35+.21*std::cos(tau*t/73+.5),1.04,1.14,.18*std::sin(tau*t/53),.92);
    field(s.lights[1],.61+.19*std::sin(tau*t/71+1.8),.07+.13*std::sin(tau*t/47+.9),.98,.66,-.35+.2*std::cos(tau*t/67),.76);
    field(s.lights[2],.75+.15*std::cos(tau*t/59+.7),.60+.22*std::sin(tau*t/43+1.4),1.12,1.22,.24*std::sin(tau*t/69+1.2),.64);
    field(s.lights[3],.34+.22*std::sin(tau*t/67+2.7),.79+.14*std::cos(tau*t/53+2.2),1.18,.86,-.3+.17*std::sin(tau*t/41),.49);
    field(s.caustic,.24+.24*std::sin(tau*t/75+.8),.18+.18*std::cos(tau*t/57+1.7),1.32,.91,-.38+.16*std::sin(tau*t/63),.42);
    field(s.caustic,.83+.15*std::sin(tau*t/63+2.4),.71+.19*std::cos(tau*t/71+.4),1.02,1.38,.28+.13*std::sin(tau*t/49),.28);
    p.end();++s.frames;publish();
}
inline void sampleEnvironment(QPainter &p,const QRectF &rect,const QPointF &origin,const QSizeF &scene){
    ensureEnvironment(scene);const auto &image=store().environment;
    // A dialog can extend beyond the editor. Extend its edge pixels rather than
    // introducing blank strips, without allocating a cropped image per control.
    const qreal sx=image.width()/qMax(1.0,scene.width()),sy=image.height()/qMax(1.0,scene.height());
    const QRectF source(origin.x()*sx,origin.y()*sy,rect.width()*sx,rect.height()*sy);
    const qreal x[4]={rect.left(),rect.left()+qBound(0.0,-source.left()/sx,rect.width()),
        rect.left()+qBound(0.0,(image.width()-source.left())/sx,rect.width()),rect.right()};
    const qreal y[4]={rect.top(),rect.top()+qBound(0.0,-source.top()/sy,rect.height()),
        rect.top()+qBound(0.0,(image.height()-source.top())/sy,rect.height()),rect.bottom()};
    for(int row=0;row<3;++row)for(int col=0;col<3;++col){
        const QRectF target(QPointF(x[col],y[row]),QPointF(x[col+1],y[row+1]));
        if(target.width()<=0||target.height()<=0)continue;
        const qreal left=col==0?0:col==2?image.width()-1:qBound(0.0,source.left()+(target.left()-rect.left())*sx,qreal(image.width()-1));
        const qreal top=row==0?0:row==2?image.height()-1:qBound(0.0,source.top()+(target.top()-rect.top())*sy,qreal(image.height()-1));
        p.drawImage(target,image,QRectF(left,top,col==1?target.width()*sx:1,row==1?target.height()*sy:1));
    }
}
}

inline void clearCaches(){
    auto &s=Detail::store();s.environment=QImage();s.scene=QSizeF();s.intensity=-1;s.dirty=true;
    for(auto &light:s.lights)light=QImage();s.caustic=QImage();Detail::publish();
}
inline void advance(double seconds){
    if(!std::isfinite(seconds)||seconds<=0)return;
    auto &s=Detail::store();s.seconds+=seconds;s.dirty=true;
    if(qApp)qApp->setProperty("potatoAmbientSeconds",s.seconds);
}
inline void paintSurface(QPainter &p,const QRectF &rect,Role role,const QPointF &origin,const QSizeF &scene,int state,const QString &family=QString()){
    Q_UNUSED(family);
    if(rect.width()<1||rect.height()<1)return;
    const bool solid=opaque(),clear=clearGlass(),disabled=state&8,pressed=(state&2)&&!disabled;
    const bool primary=(state&32)&&role==Role::Button;
    const qreal r=qMin(PotatoAmbient::radius(role),qMin(rect.width(),rect.height())*.5);
    QPainterPath shape;shape.addRoundedRect(rect,r,r);
    p.save();p.setRenderHint(QPainter::Antialiasing);p.setRenderHint(QPainter::SmoothPixmapTransform);p.setClipPath(shape,Qt::IntersectClip);
    p.fillRect(rect,QColor("#080D17"));
    if(!solid)Detail::sampleEnvironment(p,rect,origin,scene);
    else p.fillRect(rect,PotatoGlass::increasedContrast()?QColor("#101B26"):QColor("#18252D"));
    if(role!=Role::Foundation){
        // Main material tint is the preview's Clear .30/.22 or Smoked .53/.46.
        QLinearGradient tint(rect.topLeft(),rect.bottomRight());
        tint.setColorAt(0,clear?QColor(7,15,21,77):QColor(7,11,17,135));
        tint.setColorAt(1,clear?QColor(6,13,20,56):QColor(6,10,16,117));
        if(!solid)p.fillRect(rect,tint);
        if(role==Role::Input||role==Role::Well)p.fillRect(rect,QColor(3,9,15,role==Role::Input?65:34));
        else if(role==Role::Button||role==Role::Heading){
            const int strength=disabled?6:(pressed?10:((state&1)?30:15));
            p.fillRect(rect,QColor(205,229,238,strength));
        }
        if((state&4)&&!primary)p.fillRect(rect,QColor(142,185,207,32));
        if(primary){
            QLinearGradient button(rect.topLeft(),rect.bottomRight());
            button.setColorAt(0,QColor(64,110,173,disabled?80:230));button.setColorAt(1,QColor(43,77,135,disabled?80:235));
            p.fillRect(rect,button);
        }
        if(pressed)p.fillRect(rect,QColor(0,0,0,20));
        if(!solid&&!disabled){
            QLinearGradient gloss(rect.topLeft(),rect.bottomRight());
            gloss.setColorAt(0,QColor(209,230,239,8));gloss.setColorAt(.43,Qt::transparent);gloss.setColorAt(1,QColor(165,184,198,2));
            p.fillRect(rect,gloss);
        }
    }
    p.restore();
    if(role==Role::Foundation||role==Role::Section)return;
    p.save();p.setRenderHint(QPainter::Antialiasing);p.setBrush(Qt::NoBrush);
    const QRectF edge=rect.adjusted(.5,.5,-.5,-.5);
    const bool control=role==Role::Button||role==Role::Heading||role==Role::Input;
    const int strength=PotatoGlass::increasedContrast()?150:(disabled?18:((state&1)?82:(control?38:(clear?64:46))));
    QLinearGradient rim(edge.topLeft(),edge.bottomRight());rim.setColorAt(0,QColor(220,240,248,strength));rim.setColorAt(1,QColor(190,219,232,qRound(strength*.5)));
    p.setPen(QPen(QBrush(rim),1));
    if(role==Role::Shell)p.drawLine(edge.bottomLeft(),edge.bottomRight());else p.drawRoundedRect(edge,r,r);
    if(!solid&&!disabled){p.setPen(QColor(243,251,255,14));p.drawLine(QPointF(edge.left()+r,edge.top()+1),QPointF(edge.right()-r,edge.top()+1));}
    p.restore();
}
}
