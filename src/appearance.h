#pragma once
#include <QString>

// Editor preferences and catalogue categories never change a saved scene.
namespace PotatoAppearance {
inline QString themeId(const QString &id){
    return id=="halloween"||id=="autumn"||id=="christmas"?id:QStringLiteral("classic1");
}
inline QString category(int pattern){
    if(pattern==16||pattern==17||pattern==19)return QStringLiteral("halloween");
    if(pattern==14||pattern==15||pattern==18||pattern==20)return QStringLiteral("autumn");
    if(pattern==21||pattern==22)return QStringLiteral("christmas");
    return QStringLiteral("patterns");
}
inline bool matches(int pattern,const QString &name,const QString &categoryId,const QString &search){
    return (categoryId=="all"||category(pattern)==categoryId)&&name.contains(search.trimmed(),Qt::CaseInsensitive);
}
}
