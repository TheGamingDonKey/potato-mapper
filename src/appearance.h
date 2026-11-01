#pragma once
#include <QString>
#include <QSettings>

// Editor preferences and catalogue categories never change a saved scene.
namespace PotatoAppearance {
inline QString accentId(const QString &id){
    return id=="halloween"||id=="autumn"||id=="christmas"?id:QStringLiteral("classic1");
}
// Retain the original palette IDs for older runtimes and catalogue checks.
inline QString themeId(const QString &id){return accentId(id);}
inline QString interfaceThemeId(const QString &id){
    if(id=="classic1"||id=="liquidglass1"||id=="auroraclear1"||id=="transparentglass1"||id=="ambientglass1")return id;
    return QStringLiteral("material");
}
inline bool clearGlass(const QString &id){return id=="auroraclear1"||id=="transparentglass1"||id=="ambientglass1";}
struct AmbientPreferences {
    QString glass=QStringLiteral("smoked");
    bool paused=false;
    int speed=50;
    int intensity=65;
};
inline AmbientPreferences normalizedAmbientPreferences(AmbientPreferences value){
    value.glass=value.glass=="clear"?QStringLiteral("clear"):QStringLiteral("smoked");
    value.speed=qBound(20,value.speed,120);value.intensity=qBound(20,value.intensity,100);
    return value;
}
inline AmbientPreferences readAmbientPreferences(QSettings &settings){
    bool speedOk=false,intensityOk=false;
    const int speed=settings.value("ui/ambientSpeed",50).toInt(&speedOk);
    const int intensity=settings.value("ui/ambientIntensity",65).toInt(&intensityOk);
    return normalizedAmbientPreferences({settings.value("ui/ambientGlass","smoked").toString(),
        settings.value("ui/ambientPaused",false).toBool(),speedOk?speed:50,intensityOk?intensity:65});
}
inline void writeAmbientPreferences(QSettings &settings,const AmbientPreferences &requested){
    const auto value=normalizedAmbientPreferences(requested);
    settings.setValue("ui/ambientGlass",value.glass);settings.setValue("ui/ambientPaused",value.paused);
    settings.setValue("ui/ambientSpeed",value.speed);settings.setValue("ui/ambientIntensity",value.intensity);
}
struct Preferences {
    QString interfaceTheme=QStringLiteral("material");
    QString accent=QStringLiteral("classic1");
};
inline Preferences readPreferences(QSettings &settings){
    // ui/theme previously selected a Material palette, including "classic1".
    // Upgrading that preference must therefore keep the Material interface.
    return {interfaceThemeId(settings.value("ui/interfaceTheme","material").toString()),
            accentId(settings.value("ui/accent",settings.value("ui/theme","classic1")).toString())};
}
inline void writePreferences(QSettings &settings,const Preferences &preferences){
    settings.setValue("ui/interfaceTheme",interfaceThemeId(preferences.interfaceTheme));
    const auto accent=accentId(preferences.accent);
    settings.setValue("ui/accent",accent);
    settings.setValue("ui/theme",accent); // preserve the palette when using the prior runtime
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
