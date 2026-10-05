#pragma once
#include <QStringList>

// Project IDs are stable: keep existing dots at 1 when adding generators.
inline const QStringList &potatoPatterns(){
    static const QStringList names{"Media / white grid","Animated dots","Diagonal stripes","Expanding rings","Stepped waves","SquareWave · flowing cells","Diagonals · flowing strokes","CubicCircles · morphing cells","SquareArray · moving blocks","Waterlight · shimmering web","Contour Flow · drifting lines","Curve Maze · connected paths","Depth Tunnel · perspective grid"};
    return names;
}
inline QString potatoPatternSizeLabel(int pattern){return pattern==1?"Dot size":pattern==5||pattern==7||pattern==8?"Cell size":"Line width";}
inline int potatoPatternDensity(int pattern){return pattern==1?12:pattern==5?48:pattern==6?32:pattern==7?24:pattern==8?18:pattern==9?30:pattern==10?18:pattern==11?24:pattern==12?18:8;}
inline int potatoPatternDefaultSize(int pattern){return pattern>=9?(pattern==9||pattern==11?14:12):pattern==6?12:38;}
struct FxLook {
    int density=8,flow=70,angle=0,palette=0,edge=0;
    bool reverse=false;
    bool operator==(const FxLook &b)const{return density==b.density&&flow==b.flow&&angle==b.angle&&palette==b.palette&&edge==b.edge&&reverse==b.reverse;}
};
