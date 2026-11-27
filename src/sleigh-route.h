#pragma once
#include <QPointF>
#include <QRectF>
#include <QVector>
#include <algorithm>
#include <cmath>

// The path is measured in projector space. Panel meshes clip the artwork
// afterwards, so a real gap interrupts the light while the flight continues.
struct SleighRoute {
    QRectF bounds;
    QVector<QPointF> stops; // travel distance, displacement perpendicular to travel
    double angle=0, artSize=0, travel=0;

    void prepare(const QVector<QPointF> &centres,double outputAspect,int angleDegrees,int sizePercent){
        if(centres.isEmpty() || bounds.isEmpty())return;
        const double width=bounds.width()*outputAspect, height=bounds.height();
        const double shortSide=std::min(width,height);
        const QPointF extent(width/shortSide,height/shortSide);
        double minX=centres.first().x(),maxX=minX,minY=centres.first().y(),maxY=minY;
        for(const auto &c:centres){
            minX=std::min(minX,c.x());maxX=std::max(maxX,c.x());
            minY=std::min(minY,c.y());maxY=std::max(maxY,c.y());
        }
        const bool stacked=(maxY-minY)>(maxX-minX)*outputAspect*1.25;
        angle=(angleDegrees-(stacked?90:0))*3.141592653589793/180.0;
        const QPointF tangent(std::cos(angle),-std::sin(angle)),normal(-tangent.y(),tangent.x());
        const double span=std::abs(tangent.x())*extent.x()+std::abs(tangent.y())*extent.y();
        artSize=(.43+.92*sizePercent/100.0)*std::min(1.0,span/1.52);
        travel=span+2.8*artSize; // lead reindeer, runners and trailing sparkles clear the loop seam
        QVector<QPointF> stations;
        for(const auto &c:centres){
            const QPointF offset=c-bounds.center();
            const QPointF p(offset.x()*outputAspect/shortSide,offset.y()/shortSide);
            stations.append({QPointF::dotProduct(p,tangent),QPointF::dotProduct(p,normal)});
        }
        std::sort(stations.begin(),stations.end(),[](QPointF a,QPointF b){return a.x()<b.x();});
        // Panels side by side at the same point along the flight share a waypoint.
        for(qsizetype i=0;i<stations.size();){
            const auto first=i++;
            QPointF sum=stations[first];
            while(i<stations.size() && stations[i].x()-stations[first].x()<.01)sum+=stations[i++];
            stops.append(sum/double(i-first));
        }
    }
    QPointF position(double turn) const{
        const double along=(turn-.5)*travel;
        if(stops.isEmpty())return {along,0};
        const auto end=std::upper_bound(stops.cbegin(),stops.cend(),along,
            [](double distance,QPointF point){return distance<point.x();});
        if(end==stops.cbegin())return {along,stops.first().y()};
        if(end==stops.cend())return {along,stops.last().y()};
        const auto &a=*(end-1), &b=*end;
        double t=(along-a.x())/(b.x()-a.x());
        t=t*t*(3-2*t);
        return {along,a.y()+(b.y()-a.y())*t};
    }
};
