#pragma once
#include <QStringList>

// Project IDs are stable: keep existing dots at 1 when adding generators.
inline const QStringList &potatoPatterns(){
    static const QStringList names{"Media / white grid","Animated dots","Diagonal stripes","Expanding rings","Square waves"};
    return names;
}
inline QString potatoPatternSizeLabel(int pattern){return pattern==1?"Dot size":"Line width";}
