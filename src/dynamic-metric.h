#pragma once
#include "dynamic.h"
#include <algorithm>
#include <cmath>

namespace PotatoDynamic {
inline QPointF mappedPoint(const Surface &s,QPointF uv) {
    if(s.mesh.size()==(s.cells+1)*(s.cells+1) && s.cells>=1 && s.cells<=16) {
        // Outlines can straddle a panel edge. Extrapolate the nearest boundary
        // triangle for their metric; final UV geometry clipping stays in Scene.
        const double xx=uv.x()*s.cells,yy=uv.y()*s.cells;
        const int x=std::clamp(int(std::floor(xx)),0,s.cells-1),y=std::clamp(int(std::floor(yy)),0,s.cells-1),a=y*(s.cells+1)+x;
        const double u=xx-x,v=yy-y;
        uv=u>=v?s.mesh[a]*(1-u)+s.mesh[a+1]*(u-v)+s.mesh[a+s.cells+2]*v
                 :s.mesh[a]*(1-v)+s.mesh[a+s.cells+2]*u+s.mesh[a+s.cells+1]*(v-u);
    }
    return s.projection.map(uv);
}
// Invert the local output Jacobian. This compensates perspective and mesh
// deformation at the centre; a shape spanning several triangles is approximate.
inline QPointF pixelOffset(const Surface &s,QPointF center,QPointF delta) {
    constexpr double step=1e-5;
    const QPointF base=mappedPoint(s,center);
    const double sx=center.x()>.9999?-step:step,sy=center.y()>.9999?-step:step;
    const QPointF x=(mappedPoint(s,center+QPointF(sx,0))-base)/sx;
    const QPointF y=(mappedPoint(s,center+QPointF(0,sy))-base)/sy;
    const double det=x.x()*y.y()-x.y()*y.x();
    if(!std::isfinite(det)||std::abs(det)<1e-8)return {};
    QPointF offset((delta.x()*y.y()-delta.y()*y.x())/det,(x.x()*delta.y()-x.y()*delta.x())/det);
    if(!std::isfinite(offset.x())||!std::isfinite(offset.y())||std::abs(offset.x())>4||std::abs(offset.y())>4)return {};
    return offset;
}
inline bool localPoint(const Surface &s,QPointF pixel,QPointF &uv) {
    bool invertible=false;const auto inverse=s.projection.inverted(&invertible);
    if(!invertible)return false;
    const QPointF point=inverse.map(pixel);
    if(s.mesh.isEmpty()){uv=point;return uv.x()>=0&&uv.x()<=1&&uv.y()>=0&&uv.y()<=1;}
    auto cross=[](QPointF a,QPointF b){return a.x()*b.y()-a.y()*b.x();};
    for(int y=0;y<s.cells;++y)for(int x=0;x<s.cells;++x)for(int half=0;half<2;++half){
        const int a=y*(s.cells+1)+x,b=half?a+s.cells+2:a+1,c=half?a+s.cells+1:a+s.cells+2;
        const auto first=s.mesh[b]-s.mesh[a],second=s.mesh[c]-s.mesh[a],offset=point-s.mesh[a];
        const double det=cross(first,second);if(std::abs(det)<1e-12)continue;
        const double u=cross(offset,second)/det,v=cross(first,offset)/det;
        if(u>=-1e-9&&v>=-1e-9&&u+v<=1+1e-9){
            const QPointF origin(double(x)/s.cells,double(y)/s.cells);
            uv=origin+(half?QPointF(u,u+v):QPointF(u+v,v))/s.cells;return true;
        }
    }
    return false;
}
}
