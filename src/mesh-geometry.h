#pragma once
#include <QPolygonF>
#include <algorithm>
#include <array>
#include <cmath>

// Split a UV triangle at the same cell diagonals used by Surface::sample.
// Each resulting triangle lies in one affine part of the deformation mesh.
template<class Emit> inline void potatoEachMeshVertex(QPointF a,QPointF b,QPointF c,int cells,Emit appendVertex){
    cells=std::clamp(cells,1,16);
    const int minX=std::clamp(int(std::floor(std::min({a.x(),b.x(),c.x()})*cells)),0,cells-1);
    const int maxX=std::clamp(int(std::floor(std::max({a.x(),b.x(),c.x()})*cells)),0,cells-1);
    const int minY=std::clamp(int(std::floor(std::min({a.y(),b.y(),c.y()})*cells)),0,cells-1);
    const int maxY=std::clamp(int(std::floor(std::max({a.y(),b.y(),c.y()})*cells)),0,cells-1);
    if(minX==maxX&&minY==maxY&&std::min({a.x(),b.x(),c.x(),a.y(),b.y(),c.y()})>=0&&std::max({a.x(),b.x(),c.x(),a.y(),b.y(),c.y()})<=1){
        auto side=[&](QPointF p){return p.x()*cells-minX-(p.y()*cells-minY);};
        const double sa=side(a),sb=side(b),sc=side(c);
        if(std::min({sa,sb,sc})>=-1e-12||std::max({sa,sb,sc})<=1e-12){appendVertex(a);appendVertex(b);appendVertex(c);return;}
    }
    auto cross=[](QPointF x,QPointF y){return x.x()*y.y()-x.y()*y.x();};
    for(int y=minY;y<=maxY;++y)for(int x=minX;x<=maxX;++x){
        const QPointF p(double(x)/cells,double(y)/cells),q(double(x+1)/cells,double(y)/cells),r(double(x+1)/cells,double(y+1)/cells),s(double(x)/cells,double(y+1)/cells);
        for(int half=0;half<2;++half){
            const std::array<QPointF,3> clip=half==0?std::array<QPointF,3>{p,q,r}:std::array<QPointF,3>{p,r,s};
            // Two intersecting triangles have at most six vertices. Spare slots
            // also retain points coincident with a clipping edge.
            std::array<QPointF,8> polygon{a,b,c};int count=3;
            for(int edge=0;edge<3&&count;++edge){
                std::array<QPointF,8> next;int nextCount=0;const QPointF start=clip[edge],direction=clip[(edge+1)%3]-start;
                auto append=[&](QPointF point){Q_ASSERT(nextCount<int(next.size()));next[nextCount++]=point;};
                auto previous=polygon[count-1];double previousDistance=cross(direction,previous-start);
                for(int point=0;point<count;++point){
                    const auto current=polygon[point];
                    const double distance=cross(direction,current-start);
                    const bool in=distance>=-1e-12,previousIn=previousDistance>=-1e-12;
                    if(in!=previousIn){const double denominator=previousDistance-distance;if(std::abs(denominator)>1e-15)append(previous+(current-previous)*(previousDistance/denominator));}
                    if(in)append(current);previous=current;previousDistance=distance;
                }
                polygon=next;count=nextCount;
            }
            for(int i=1;i+1<count;++i)if(std::abs(cross(polygon[i]-polygon[0],polygon[i+1]-polygon[0]))>1e-14){appendVertex(polygon[0]);appendVertex(polygon[i]);appendVertex(polygon[i+1]);}
        }
    }
}
inline QVector<QPointF> potatoMeshTriangles(QPointF a,QPointF b,QPointF c,int cells){
    QVector<QPointF> result;potatoEachMeshVertex(a,b,c,cells,[&](QPointF point){result<<point;});
    return result;
}
