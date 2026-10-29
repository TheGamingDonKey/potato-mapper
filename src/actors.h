#pragma once
#include "model.h"
#include "entrance.h"
#include <QPainter>
#include <QPainterPath>
#include <QLineF>
#include <cmath>

// One Scene clock drives both preview and output. Contour distances are output
// pixels, so walking speed and body size do not change between differently sized panels.
inline void potatoDrawBoundaryActors(QPainter &p,const Scene &scene,const QRectF &rect){
    auto pixel=[&](QPointF q){return QPointF(q.x()*scene.outputSize.width(),q.y()*scene.outputSize.height());};
    QPainterPath clip;
    for(const auto &s:scene.surfaces)if(s.visible&&s.opacity>0){QPolygonF polygon;for(int side=0;side<4;++side)for(int k=0;k<s.cells;++k){double f=double(k)/s.cells;const QPointF uv=side==0?QPointF(f,0):side==1?QPointF(1,f):side==2?QPointF(1-f,1):QPointF(0,1-f);polygon<<pixel(s.transform().map(s.sample(uv.x(),uv.y())));}QPainterPath panel;panel.addPolygon(polygon);clip=clip.united(panel);}
    p.save();p.setClipRect(rect);p.translate(rect.topLeft());p.scale(rect.width()/scene.outputSize.width(),rect.height()/scene.outputSize.height());p.setClipPath(clip,Qt::IntersectClip);p.setRenderHint(QPainter::Antialiasing);
    const auto contours=scene.exposedContours();int ordinal=0;
    for(const auto &contour:contours){QVector<double> lengths{0};double length=0;
        for(int i=1;i<contour.size();++i){length+=QLineF(pixel(contour[i-1]),pixel(contour[i])).length();lengths<<length;}
        if(length<50)continue;
        auto at=[&](double distance){distance=std::fmod(std::max(0.0,distance),length);for(int i=1;i<contour.size();++i)if(distance<=lengths[i]){double f=(distance-lengths[i-1])/std::max(1e-12,lengths[i]-lengths[i-1]);return pixel(contour[i-1])*(1-f)+pixel(contour[i])*f;}return pixel(contour.last());};
        const double time=scene.actorElapsed+ordinal++*1.37;
        // A four-second stride phrase has a short look/pause before continuing.
        const double phrase=std::fmod(time,4.0),travel=(std::floor(time/4)*3.6+std::min(phrase,3.6))*55;
        const auto feet=at(travel),forward=at(travel+2)-feet;
        const double angle=std::atan2(forward.y(),forward.x())*180/3.141592653589793;
        const double stride=phrase<3.6?std::sin(time*8):0;
        const auto inward=QPointF(-forward.y(),forward.x())/std::max(1e-9,std::hypot(forward.x(),forward.y()));
        double appearance=0;const auto sample=feet+inward*2;
        for(const auto &s:scene.surfaces)if(s.visible&&s.opacity>0){QPolygonF polygon;for(int side=0;side<4;++side)for(int k=0;k<s.cells;++k){double f=double(k)/s.cells;const QPointF uv=side==0?QPointF(f,0):side==1?QPointF(1,f):side==2?QPointF(1-f,1):QPointF(0,1-f);polygon<<pixel(s.transform().map(s.sample(uv.x(),uv.y())));}if(polygon.containsPoint(sample,Qt::OddEvenFill)){appearance=s.opacity/100.0*s.brightness/100.0;break;}}
        p.save();p.translate(feet);p.rotate(angle);p.setOpacity(appearance*(scene.entranceActive?PotatoEntrance::reveal(scene.entranceElapsed):1));
        p.setPen(QPen(QColor("#d9f4df"),2.8,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));p.setBrush(QColor("#d9f4df"));
        // Local +Y points into a clockwise panel boundary; feet stay on the contour.
        const double bob=std::abs(stride)*1.2;p.drawLine(QPointF(0,12+bob),QPointF(0,30+bob));
        p.drawLine(QPointF(0,13+bob),QPointF(-6+stride*3,6));p.drawLine(QPointF(-6+stride*3,6),QPointF(-8+stride*5,0));
        p.drawLine(QPointF(0,13+bob),QPointF(6-stride*3,6));p.drawLine(QPointF(6-stride*3,6),QPointF(8-stride*5,0));
        for(int rib=0;rib<3;++rib)p.drawLine(QPointF(-5,20+rib*3+bob),QPointF(5,20+rib*3+bob));
        p.drawLine(QPointF(0,28+bob),QPointF(-8,22-stride*4+bob));p.drawLine(QPointF(0,28+bob),QPointF(8,22+stride*4+bob));
        p.drawEllipse(QRectF(-7,31+bob,14,12));p.setPen(QPen(QColor("#152c32"),2.3,Qt::SolidLine,Qt::RoundCap));
        const bool blink=std::fmod(time+ordinal*.7,4.7)<.13;
        if(blink){p.drawLine(QPointF(-4,37+bob),QPointF(-2,37+bob));p.drawLine(QPointF(2,37+bob),QPointF(4,37+bob));}
        else{p.drawPoint(QPointF(-3,37+bob));p.drawPoint(QPointF(3,37+bob));}
        p.drawLine(QPointF(-2,41+bob),QPointF(2,41+bob));p.restore();
    }
    p.restore();
}
