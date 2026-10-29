#pragma once
#include <QStringList>

// Project IDs are stable: keep existing dots at 1 when adding generators.
inline const QStringList &potatoPatterns(){
    static const QStringList names{"Media / white grid","Animated dots","Diagonal stripes","Expanding rings","Stepped waves","SquareWave · flowing cells","Diagonals · flowing strokes","CubicCircles · morphing cells","SquareArray · moving blocks","Waterlight · shimmering web","Contour Flow · drifting lines","Curve Maze · connected paths","Depth Tunnel · perspective grid","Hex Tide · breathing hexagons","Maple Supper · Thanksgiving","Harvest Lanterns · autumn glow","Pumpkin Mischief · jack-o'-lanterns","Ghost Parade · lantern procession","Rainlit Window · autumn rain","Hollow Crown · woodland revenant","Lantern Keepers · autumn journey","Midnight Workshop · winter visitors","Snowfall · shared winter sky"};
    return names;
}
inline QString potatoPatternSizeLabel(int pattern){return pattern==22?"Snowflake size":pattern>=14?"Scene size":pattern==1?"Dot size":pattern==5||pattern==7||pattern==8||pattern==13?"Cell size":"Line width";}
inline QString potatoPatternDensityLabel(int pattern){return pattern==19?"Forest lights":pattern==20?"Travelers / leaves":pattern==21||pattern==22?"Snowfall":"Density";}
inline QString potatoPatternFlowLabel(int pattern){return pattern==19?"Awakening":pattern==20?"Expression":pattern==21?"Workshop bustle":pattern==22?"Wind":"Flow";}
inline int potatoPatternDensity(int pattern){return pattern==19?28:pattern==20?36:pattern==21?28:pattern==22?36:pattern==16?24:pattern==17?32:pattern==18?24:pattern==14?28:pattern==15?16:pattern==1?12:pattern==5?48:pattern==6?32:pattern==7?24:pattern==8?18:pattern==9?30:pattern==10?18:pattern==11?24:pattern==12?18:pattern==13?36:8;}
inline int potatoPatternDefaultSize(int pattern){return pattern>=14?24:pattern==13?38:pattern>=9?(pattern==9||pattern==11?14:12):pattern==6?12:38;}
inline int potatoPatternPalette(int pattern){return pattern>=5&&pattern<14?1:0;}
inline int potatoPatternDefaultFlow(int pattern){return pattern==19?65:pattern==20?60:pattern==21?55:pattern==22?40:pattern==18?50:pattern==16||pattern==17?60:70;}
inline double potatoPatternPeriod(int pattern){return pattern>=19&&pattern<=22?24.0:pattern>=14&&pattern<=18?12.0:10000.0;}
struct FxLook {
    int density=8,flow=70,angle=0,palette=0,edge=0;
    bool reverse=false;
    bool operator==(const FxLook &b)const{return density==b.density&&flow==b.flow&&angle==b.angle&&palette==b.palette&&edge==b.edge&&reverse==b.reverse;}
};
