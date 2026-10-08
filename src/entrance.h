#pragma once
#include <QString>
#include <algorithm>
#include <cmath>

namespace PotatoEntrance {
inline constexpr double duration=8.0;
inline double reveal(double seconds){
    const double x=std::clamp((seconds-5.6)/(duration-5.6),0.0,1.0);
    return x*x*x*(x*(x*6-15)+10);
}
inline QString stage(double seconds){
    if(seconds<1.1)return "Finding surfaces";
    if(seconds<3.0)return "Tracing edges";
    if(seconds<4.8)return "Building the grid";
    if(seconds<5.6)return "Locking panels";
    return "Revealing projection";
}
}
