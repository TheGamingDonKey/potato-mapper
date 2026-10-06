#pragma once
#include <QPolygonF>
#include <algorithm>
#include <cmath>

// Split a UV triangle at the same cell diagonals used by Surface::sample.
// Each resulting triangle lies in one affine part of the deformation mesh.
inline QVector<QPointF> potatoMeshTriangles(QPointF a,QPointF b,QPointF c,int cells){
    QVector<QPointF> result;cells=std::clamp(cells,1,16);
    const int minX=std::clamp(int(std::floor(std::min({a.x(),b.x(),c.x()})*cells)),0,cells-1);
    const int maxX=std::clamp(int(std::floor(std::max({a.x(),b.x(),c.x()})*cells)),0,cells-1);
    const int minY=std::clamp(int(std::floor(std::min({a.y(),b.y(),c.y()})*cells)),0,cells-1);
    const int maxY=std::clamp(int(std::floor(std::max({a.y(),b.y(),c.y()})*cells)),0,cells-1);
    if(minX==maxX&&minY==maxY&&std::min({a.x(),b.x(),c.x(),a.y(),b.y(),c.y()})>=0&&std::max({a.x(),b.x(),c.x(),a.y(),b.y(),c.y()})<=1){
        auto side=[&](QPointF p){return p.x()*cells-minX-(p.y()*cells-minY);};
        const double sa=side(a),sb=side(b),sc=side(c);
        if(std::min({sa,sb,sc})>=-1e-12||std::max({sa,sb,sc})<=1e-12)return {a,b,c};
    }
    auto cross=[](QPointF x,QPointF y){return x.x()*y.y()-x.y()*y.x();};
    for(int y=minY;y<=maxY;++y)for(int x=minX;x<=maxX;++x){
        const QPointF p(double(x)/cells,double(y)/cells),q(double(x+1)/cells,double(y)/cells),r(double(x+1)/cells,double(y+1)/cells),s(double(x)/cells,double(y+1)/cells);
        for(const auto &clip:QVector<QPolygonF>{QPolygonF{p,q,r},QPolygonF{p,r,s}}){
            QPolygonF polygon{a,b,c};
            for(int edge=0;edge<3&&!polygon.isEmpty();++edge){
                QPolygonF next;const QPointF start=clip[edge],direction=clip[(edge+1)%3]-start;
                auto previous=polygon.last();double previousDistance=cross(direction,previous-start);
                for(const auto &current:polygon){
                    const double distance=cross(direction,current-start);
                    const bool in=distance>=-1e-12,previousIn=previousDistance>=-1e-12;
                    if(in!=previousIn){const double denominator=previousDistance-distance;if(std::abs(denominator)>1e-15)next<<previous+(current-previous)*(previousDistance/denominator);}
                    if(in)next<<current;previous=current;previousDistance=distance;
                }
                polygon=next;
            }
            for(int i=1;i+1<polygon.size();++i)if(std::abs(cross(polygon[i]-polygon[0],polygon[i+1]-polygon[0]))>1e-14)result<<polygon[0]<<polygon[i]<<polygon[i+1];
        }
    }
    return result;
}
