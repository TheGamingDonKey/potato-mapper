#pragma once
#include <QTransform>
#include <algorithm>
#include <cmath>

inline QPointF potatoPortalBend(const QTransform &inverse,const QPointF &mapped,const QPointF &local,const QPointF &inward) {
    // Convert a common output direction back into each surface's UV space.
    // Small probes avoid jumping across the horizon of a projective transform.
    const QPointF probe=inverse.map(mapped+inward*.001)-local;
    const double length=std::hypot(probe.x(),probe.y());
    const QPointF direction=length>1e-12?probe/length:QPointF();
    double distance=.28;
    if(direction.x()>1e-12) distance=std::min(distance,(1-local.x())/direction.x());
    else if(direction.x()<-1e-12) distance=std::min(distance,-local.x()/direction.x());
    if(direction.y()>1e-12) distance=std::min(distance,(1-local.y())/direction.y());
    else if(direction.y()<-1e-12) distance=std::min(distance,-local.y()/direction.y());
    // Shorten the whole ray rather than clamping its axes independently.
    // Keep a little room for rounding at the adjacent square boundary.
    return local+direction*(std::max(0.0,distance)*.999);
}
