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
    if(scene.outputSize.isEmpty()||rect.isEmpty())return;
    const auto &geometry=scene.worldGeometry();
    p.save();p.setClipRect(rect);p.translate(rect.topLeft());p.scale(rect.width()/scene.outputSize.width(),rect.height()/scene.outputSize.height());p.setClipPath(geometry.outputClip,Qt::IntersectClip);p.setRenderHint(QPainter::Antialiasing);
    int ordinal=0;
    for(const auto &metric:geometry.contours){const auto &contour=metric.outputPoints;const auto &lengths=metric.lengths;const double length=metric.length;
        if(length<50)continue;
        auto at=[&](double distance){distance=std::fmod(std::max(0.0,distance),length);for(int i=1;i<contour.size();++i)if(distance<=lengths[i]){double f=(distance-lengths[i-1])/std::max(1e-12,lengths[i]-lengths[i-1]);return contour[i-1]*(1-f)+contour[i]*f;}return contour.last();};
        const double time=scene.actorElapsed+ordinal++*1.37;
        // A four-second stride phrase has a short look/pause before continuing.
        const double phrase=std::fmod(time,4.0),travel=(std::floor(time/4)*3.6+std::min(phrase,3.6))*55;
        const auto feet=at(travel),forward=at(travel+2)-feet;
        const double angle=std::atan2(forward.y(),forward.x())*180/3.141592653589793;
        const double stride=phrase<3.6?std::sin(time*8):0;
        const auto inward=QPointF(-forward.y(),forward.x())/std::max(1e-9,std::hypot(forward.x(),forward.y()));
        double appearance=0;const auto sample=feet+inward*2;
        for(const auto &panel:geometry.panels)if(panel.outputOutline.containsPoint(sample,Qt::OddEvenFill)){const auto &s=scene.surfaces[panel.surfaceIndex];appearance=s.opacity/100.0*s.brightness/100.0;break;}
        p.save();p.translate(feet);p.rotate(angle);p.scale(scene.actorSize/100.0,scene.actorSize/100.0);p.setOpacity(appearance*(scene.entranceActive?PotatoEntrance::reveal(scene.entranceElapsed):1));
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
