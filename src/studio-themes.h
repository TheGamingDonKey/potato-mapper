#pragma once
#include "liquid-glass.h"
#include <array>

// Authored native counterparts of the four approved November 1 studies. Qt
// retains every control, label and hit area; this file paints only editor chrome.
namespace PotatoThemes {
using Role=PotatoGlass::Role;
struct Theme {
    QString id,name;
    QColor bg,ink,muted,panel,accent,primary,primaryInk,selection,selectionInk,cap,section,rail,light1,light2;
    qreal panelRadius,buttonRadius;
};
inline const std::array<Theme,4> &themes(){
    static const std::array<Theme,4> values{{
        {"lumenatelier1","Lumen Atelier",QColor("#9FB3C1"),QColor("#152D3F"),QColor("#183044"),QColor("#C3D2DB"),QColor("#456EB5"),QColor("#345C9B"),QColor("#FFFFFF"),QColor("#CCD9E4"),QColor("#183449"),QColor("#DFE5E4"),QColor("#C0CDD4"),QColor("#486B83"),QColor("#ADC1D0"),QColor("#D7B899"),13,7},
        {"nocturnecinema1","Nocturne Cinema",QColor("#0D111A"),QColor("#F0F4FF"),QColor("#AFBBD0"),QColor("#172235"),QColor("#6DA6FF"),QColor("#BBD5FF"),QColor("#0B2141"),QColor("#2A4363"),QColor("#F6F9FF"),QColor("#25354D"),QColor("#24354A"),QColor("#1B2C44"),QColor("#347BDA"),QColor("#DA794F"),12,7},
        {"siennastudio1","Sienna Studio",QColor("#E5DCD0"),QColor("#332927"),QColor("#655954"),QColor("#FFF7E9"),QColor("#A54831"),QColor("#9D4230"),QColor("#FFFFFF"),QColor("#F6D8BF"),QColor("#562F24"),QColor("#FFF8ED"),QColor("#EEE0CE"),QColor("#AA8A70"),QColor("#D88F54"),QColor("#708EA9"),11,6},
        {"tidallens1","Tidal Lens",QColor("#B2CDCC"),QColor("#153B4C"),QColor("#3E6271"),QColor("#DFF8F1"),QColor("#167C92"),QColor("#133F59"),QColor("#F6FFFA"),QColor("#E1ECAF"),QColor("#244538"),QColor("#E2F7F1"),QColor("#B4D7D4"),QColor("#366A7A"),QColor("#438CAC"),QColor("#E6D789"),17,11}
    }};
    return values;
}
inline bool isTheme(const QString &id){for(const auto &theme:themes())if(theme.id==id)return true;return false;}
inline QString currentId(){return qApp?qApp->property("potatoInterfaceTheme").toString():QString();}
inline bool active(){return isTheme(currentId());}
inline const Theme &current(){for(const auto &theme:themes())if(theme.id==currentId())return theme;return themes()[0];}
inline bool opaque(){return PotatoGlass::reducedTransparency()||PotatoGlass::increasedContrast();}

namespace Detail {
constexpr qreal tau=6.28318530717958647692;
inline QColor alpha(QColor colour,qreal opacity){colour.setAlphaF(qBound(0.0,opacity,1.0));return colour;}
inline QColor mix(const QColor &a,const QColor &b,qreal amount){
    return QColor(qRound(a.red()*(1-amount)+b.red()*amount),qRound(a.green()*(1-amount)+b.green()*amount),qRound(a.blue()*(1-amount)+b.blue()*amount));
}
inline qreal intensity(){
    bool ok=false;const qreal value=qApp?qApp->property("potatoAmbientIntensity").toDouble(&ok):.65;
    return ok&&std::isfinite(value)?qBound(.2,value,1.0):.65;
}
struct Store {
    QImage environment; // The only cached image: at most 800 x 500, shared by all surfaces.
    QSizeF scene;
    QString id;
    qreal seconds=0,light=-1;
    quint64 frames=0;
    bool dirty=true;
};
inline Store &store(){static Store value;return value;}
inline void publish(){
    if(!qApp)return;const auto &s=store();
    qApp->setProperty("potatoStudioThemeSeconds",s.seconds);
    qApp->setProperty("potatoStudioThemeFrames",QVariant::fromValue<qulonglong>(s.frames));
    qApp->setProperty("potatoStudioThemeEnvironmentPixels",QVariant::fromValue<qlonglong>(qint64(s.environment.width())*s.environment.height()));
}
inline void ensureEnvironment(const QSizeF &requested){
    auto &s=store();const auto &t=current();const qreal light=intensity();
    const QSizeF scene(qMax(1.0,requested.width()),qMax(1.0,requested.height()));
    if(!s.dirty&&s.scene==scene&&s.id==t.id&&s.light==light&&!s.environment.isNull())return;
    const qreal scale=qMin(1.0,qMin(800.0/scene.width(),500.0/scene.height()));
    const QSize pixels(qMax(1,qFloor(scene.width()*scale)),qMax(1,qFloor(scene.height()*scale)));
    if(s.environment.size()!=pixels)s.environment=QImage(pixels,QImage::Format_RGB32);
    s.scene=scene;s.id=t.id;s.light=light;s.dirty=false;
    QPainter p(&s.environment);p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(s.environment.rect(),t.bg);p.scale(pixels.width(),pixels.height());
    const QRectF area(0,0,1,1);const bool lumen=t.id=="lumenatelier1",night=t.id=="nocturnecinema1",sienna=t.id=="siennastudio1";
    const qreal period=lumen?24:(night?26:(sienna?32:22));
    const qreal phase=tau*s.seconds/period;
    if(lumen){
        // Match the approved blue-grey frost even where Windows composition
        // is unavailable. This bounded fallback is also used by popovers.
        QLinearGradient under(0,0,1,1);under.setColorAt(0,alpha(t.light1,.38));
        under.setColorAt(.48,alpha(t.bg,.08));under.setColorAt(1,alpha(t.light2,.30));p.fillRect(area,under);
    }
    // The studies use broad rotated light planes, not repeated per-key glows.
    const auto plane=[&](const QColor &colour,qreal x,qreal y,qreal width,qreal angle,qreal strength){
        p.save();p.translate(x,y);p.rotate(angle);
        QLinearGradient band(-width*.5,0,width*.5,0);
        band.setColorAt(0,alpha(colour,0));band.setColorAt(.40,alpha(colour,.32*strength*light));
        band.setColorAt(.57,alpha(colour,.72*strength*light));band.setColorAt(.74,alpha(colour,.12*strength*light));band.setColorAt(1,alpha(colour,0));
        p.fillRect(QRectF(-width*.5,-2,width,4),band);p.restore();
    };
    if(lumen){
        plane(t.light1,.08+.14*std::sin(phase),.3,1.30,-22,.25);
        plane(t.light2,.88+.10*std::cos(phase*.8),.6,1.12,24,.23);
    }else{
        plane(t.light1,.12+.12*std::sin(phase),.3,1.02,-25,night?.68:.50);
        plane(t.light2,.39+.11*std::cos(phase/1.3),.5,.80,28,night?.54:(sienna?.33:.45));
    }
    const auto reflection=[&](qreal x,qreal width,qreal angle,qreal strength){
        p.save();p.translate(x,.5);p.rotate(angle);
        QLinearGradient glint(-width*.5,0,width*.5,0);
        glint.setColorAt(0,Qt::transparent);glint.setColorAt(.49,alpha(Qt::white,strength*light));
        glint.setColorAt(.53,alpha(Qt::white,strength*.12*light));glint.setColorAt(1,Qt::transparent);
        p.fillRect(QRectF(-width*.5,-2,width,4),glint);p.restore();
    };
    if(lumen)reflection(.31+.23*std::sin(phase),.70,20,.10);
    else if(night)reflection(.76+.09*std::sin(phase/1.2),.075,30,.10);
    else if(sienna)reflection(.42+.17*std::sin(phase),.48,24,.18);
    else for(int i=0;i<4;++i)reflection(.04+i*.23+.07*std::sin(phase*(i%2?-.84:.84)+i),.085,24,.33);
    p.end();++s.frames;publish();
}
inline void sampleEnvironment(QPainter &p,const QRectF &rect,const QPointF &origin,const QSizeF &scene){
    ensureEnvironment(scene);const auto &im=store().environment;
    const qreal sx=im.width()/qMax(1.0,scene.width()),sy=im.height()/qMax(1.0,scene.height());
    const QRectF source(origin.x()*sx,origin.y()*sy,rect.width()*sx,rect.height()*sy);
    // Floating menus/dialogs may cross the editor edge. Extend edge pixels in
    // place, without a new image allocation or transparent/black fringe.
    const qreal x[4]={rect.left(),rect.left()+qBound(0.0,-source.left()/sx,rect.width()),rect.left()+qBound(0.0,(im.width()-source.left())/sx,rect.width()),rect.right()};
    const qreal y[4]={rect.top(),rect.top()+qBound(0.0,-source.top()/sy,rect.height()),rect.top()+qBound(0.0,(im.height()-source.top())/sy,rect.height()),rect.bottom()};
    for(int row=0;row<3;++row)for(int col=0;col<3;++col){
        const QRectF target(QPointF(x[col],y[row]),QPointF(x[col+1],y[row+1]));
        if(target.width()<=0||target.height()<=0)continue;
        const qreal left=col==0?0:col==2?im.width()-1:qBound(0.0,source.left()+(target.left()-rect.left())*sx,qreal(im.width()-1));
        const qreal top=row==0?0:row==2?im.height()-1:qBound(0.0,source.top()+(target.top()-rect.top())*sy,qreal(im.height()-1));
        p.drawImage(target,im,QRectF(left,top,col==1?target.width()*sx:1,row==1?target.height()*sy:1));
    }
}
inline qreal radius(Role role,const Theme &t){
    if(role==Role::Foundation||role==Role::Shell)return 0;
    if(role==Role::Dialog||role==Role::Panel)return t.panelRadius;
    if(role==Role::Well)return qMin(t.panelRadius,qreal(8));
    if(role==Role::Section)return qMin(t.panelRadius,qreal(6));
    return t.buttonRadius;
}
}
inline void clearCaches(){
    auto &s=Detail::store();s.environment=QImage();s.scene=QSizeF();s.id.clear();s.light=-1;s.dirty=true;Detail::publish();
}
// Pause, speed, reduced motion and minimize are controlled by the existing
// ChromeMaterial clock. There is deliberately no independent timer here.
inline void advance(double seconds){
    if(!std::isfinite(seconds)||seconds<=0)return;
    auto &s=Detail::store();s.seconds+=seconds;s.dirty=true;
    if(qApp)qApp->setProperty("potatoStudioThemeSeconds",s.seconds);
}
inline void paintSurface(QPainter &p,const QRectF &rect,Role role,const QPointF &origin,const QSizeF &scene,int state,const QString &family=QString(),bool desktopGlass=false){
    if(rect.width()<1||rect.height()<1)return;
    const auto &t=current();const bool lumen=t.id=="lumenatelier1",night=t.id=="nocturnecinema1",sienna=t.id=="siennastudio1",tidal=t.id=="tidallens1";
    const bool solid=opaque(),disabled=state&8,pressed=(state&2)&&!disabled,hover=(state&1)&&!disabled,checked=(state&4)&&!disabled;
    if(lumen&&role==Role::Section&&!solid)return;
    const bool control=role==Role::Button||role==Role::Heading||role==Role::Input;
    const bool primary=(state&32)&&role==Role::Button,toolbar=sienna&&family=="toolbar";
    const qreal r=qMin(Detail::radius(role,t),qMin(rect.width(),rect.height())*.5);
    QPainterPath shape;shape.addRoundedRect(rect,r,r);
    p.save();p.setRenderHint(QPainter::Antialiasing);p.setRenderHint(QPainter::SmoothPixmapTransform);p.setClipPath(shape,Qt::IntersectClip);
    // Toolbar keys belong to Sienna's continuous opaque ink deck. Starting
    // them from the pale shared environment would turn a 7% white key overlay
    // into a pale face underneath the intentionally light toolbar text.
    const bool inkDeck=toolbar&&(role==Role::Shell||(role==Role::Button&&!primary));
    const bool seeThrough=desktopGlass&&lumen&&!solid;
    if(seeThrough){
        // The real desktop is composed by Windows, never copied into a bitmap.
        // Paint a blue-grey luminosity veil and broad shared light above the
        // native backdrop. Density keeps dark labels legible on any desktop.
        if(role==Role::Foundation){
            // The reference has cool light at the left and warm light at the
            // right, continuous across the window rather than restarting per
            // toolbar. Keep enough coverage for clear-alpha fallback: copying
            // CSS's 24% veil without desktop diffusion would expose sharp text.
            QLinearGradient veil(QPointF(-origin.x(),-origin.y()),QPointF(scene.width()-origin.x(),scene.height()*.30-origin.y()));
            veil.setColorAt(0,QColor(132,160,181,209));
            veil.setColorAt(.48,QColor(139,161,174,209));
            veil.setColorAt(1,QColor(185,158,134,209));p.fillRect(rect,veil);
            Detail::ensureEnvironment(scene);
            const qreal phase=Detail::tau*Detail::store().seconds/24;
            const auto glow=[&](const QColor &colour,qreal x,qreal y,qreal radius,qreal strength){
                QRadialGradient light(QPointF(-origin.x()+scene.width()*x,-origin.y()+scene.height()*y),scene.width()*radius);
                light.setColorAt(0,Detail::alpha(colour,strength*Detail::intensity()));light.setColorAt(1,Qt::transparent);p.fillRect(rect,light);
            };
            glow(t.light1,.12+.13*std::sin(phase),.22,.72,.17);
            glow(t.light2,.90+.11*std::cos(phase*.8),.75,.64,.12);
            const qreal sweep=.45+.32*std::sin(phase);
            QLinearGradient gloss(-origin.x()+scene.width()*(sweep-.32),-origin.y(),
                                  -origin.x()+scene.width()*(sweep+.32),-origin.y()+scene.height()*.32);
            const int light=qRound(22*Detail::intensity());
            gloss.setColorAt(0,Qt::transparent);gloss.setColorAt(.46,QColor(237,243,245,light));
            gloss.setColorAt(.68,QColor(237,243,245,light/3));gloss.setColorAt(1,Qt::transparent);p.fillRect(rect,gloss);
        }
    }else{
        p.fillRect(rect,inkDeck?QColor("#25323B"):t.bg);
        if(!solid&&!inkDeck)Detail::sampleEnvironment(p,rect,origin,scene);
    }
    if(role!=Role::Foundation){
        QColor face=t.panel; qreal a0=night?.78:(lumen?.72:.66),am=lumen?.30:.46,a1=.70;
        if(role==Role::Dialog){a0=.96;am=.92;a1=.96;}
        if(role==Role::Section){face=t.section;a0=am=a1=night?.18:.16;}
        if(role==Role::Heading){face=t.section;a0=am=a1=lumen?.85:.80;}
        if(role==Role::Input){face=t.cap;a0=am=a1=night?.72:.70;}
        if(role==Role::Well){face=t.panel;a0=am=a1=night?.25:.28;}
        if(role==Role::Button){face=t.cap;a0=lumen?.92:.78;am=lumen?.66:(tidal?.45:.72);a1=lumen?.94:.80;}
        const bool lumenPane=lumen&&(role==Role::Shell||(role==Role::Panel&&family=="output"));
        if(lumen){
            if(role==Role::Panel){a0=.72;am=.60;a1=.48;}
            if(lumenPane){a0=.50;am=.30;a1=.25;}
            if(role==Role::Section)a0=am=a1=0; // Study has headings, no second milky section body.
            if(role==Role::Input){face=QColor("#D3E0E6");a0=am=a1=.50;}
            if(role==Role::Heading){a0=am=a1=.42;}
            if(role==Role::Well){face=QColor("#8FA8BC");a0=am=a1=.28;}
            if(role==Role::Button){a0=.85;am=.76;a1=.70;}
        }
        if(toolbar&&role==Role::Shell){face=QColor("#25323B");a0=am=a1=1;}
        if(night&&role==Role::Button&&!primary){a0=am=a1=family=="toolbar"?.20:.55;}
        if(toolbar&&role==Role::Button&&!primary){face=Qt::white;a0=am=a1=.07;}
        if((hover||checked)&&control&&!primary){
            if(toolbar){face=Qt::white;a0=am=a1=pressed?.16:.14;}
            else{face=t.selection;a0=am=a1=night?.92:.90;}
        }
        if(primary){face=t.primary;a0=am=a1=1;}
        if(disabled){face=Detail::mix(t.panel,t.bg,.55);a0=am=a1=.66;if(toolbar){face=QColor("#25323B");a0=am=a1=1;}}
        if(solid){
            // Fully opaque surfaces use the same high-contrast palette and no
            // light layer, while retaining the four material identities.
            if(toolbar&&!primary)face=role==Role::Button?QColor("#35414A"):QColor("#25323B");
            p.fillRect(rect,face);
        }else{
            QLinearGradient tint(rect.topLeft(),lumen&&(role==Role::Panel||role==Role::Shell)?rect.bottomRight():rect.bottomLeft());
            QColor top=face,middle=face,bottom=face;
            if(lumen&&!disabled){
                if(role==Role::Panel){top=QColor("#C3D2DB");middle=QColor("#B0C1CB");bottom=QColor("#D7C7B5");}
                if(lumenPane){top=QColor("#DAE2E6");middle=QColor("#B3C5CE");bottom=QColor("#D2C7B9");}
                if(role==Role::Button&&!primary){
                    top=QColor(hover?"#EDF1ED":"#E0E6E4");bottom=QColor(hover?"#D3E0E5":"#C5D1D5");middle=Detail::mix(top,bottom,.6);
                    if(hover)a0=am=a1=.96;
                    if(checked||pressed){top=middle=bottom=QColor("#B6C9D6");a0=am=a1=.94;}
                }
            }
            tint.setColorAt(0,Detail::alpha(top,a0));tint.setColorAt(lumen?.6:.65,Detail::alpha(middle,am));tint.setColorAt(1,Detail::alpha(bottom,a1));p.fillRect(rect,tint);
        }
        if(primary&&!disabled&&!solid){
            // Lumen's white label needs a darker cobalt even when hover lifts the face.
            const QColor top=night?QColor("#BBD5FF"):(sienna?QColor("#BA6244"):(tidal?QColor("#216583"):QColor("#436BAA")));
            QLinearGradient action(rect.topLeft(),rect.bottomLeft());action.setColorAt(0,hover?top.lighter(108):top);action.setColorAt(1,t.primary);p.fillRect(rect,action);
        }
        if(pressed)p.fillRect(rect,Detail::alpha(night?Qt::white:Qt::black,night?.06:.065));
        if(lumen&&control&&role!=Role::Heading&&!disabled&&!solid){
            // Silver keys catch a quiet top light; fields turn that contour
            // inward. All strokes remain inside the existing control bounds.
            const bool inset=role==Role::Input||pressed;
            QLinearGradient depth(rect.topLeft(),QPointF(rect.left(),rect.top()+qMin(5.0,rect.height()*.2)));
            depth.setColorAt(0,inset?QColor(40,71,94,23):QColor(255,255,255,28));depth.setColorAt(1,Qt::transparent);p.fillRect(rect,depth);
        }
        if(tidal&&control&&!primary&&!disabled&&!solid){
            QLinearGradient lens(rect.topLeft(),QPointF(rect.left(),rect.top()+qMin(10.0,rect.height()*.4)));
            lens.setColorAt(0,QColor(255,255,255,54));lens.setColorAt(1,Qt::transparent);p.fillRect(rect,lens);
        }
    }
    p.restore();
    if(role==Role::Foundation||role==Role::Section||(lumen&&role==Role::Heading&&!solid&&!hover&&!(state&16)))return;
    p.save();p.setRenderHint(QPainter::Antialiasing);p.setBrush(Qt::NoBrush);
    const QRectF edge=rect.adjusted(.5,.5,-.5,-.5);
    // Nocturne instrument keys are flat. Ceramic keys have a fine copper seam;
    // optical/seaglass panes have a clear rim, never inflated button bevels.
    const bool flatKey=(night||toolbar)&&role==Role::Button;
    if(!flatKey||solid||(state&16)){
        QColor rim=PotatoGlass::increasedContrast()?t.muted:Detail::alpha(night?QColor("#FFFFFF"):t.rail,disabled?.20:(control?.22:.28));
        if(hover)rim=Detail::alpha(t.accent,.62);
        if((state&16)&&!disabled)rim=t.accent;
        p.setPen(QPen(rim,(state&16)&&!disabled?2:1));
        if(lumen&&!solid&&!hover&&!(state&16)){
            // Frost panes and silver caps have a soft reflected upper edge.
            // Inputs reverse the light to read as shallow inset fields.
            QLinearGradient contour(edge.topLeft(),edge.bottomLeft());
            const bool inset=role==Role::Input||pressed;
            contour.setColorAt(0,inset?QColor(72,107,131,43):QColor(255,255,255,disabled?30:100));
            contour.setColorAt(.40,inset?QColor(72,107,131,30):QColor(243,244,235,disabled?20:70));
            contour.setColorAt(1,inset?QColor(255,255,255,54):QColor(36,69,92,disabled?14:31));p.setPen(QPen(QBrush(contour),1));
        }
        if(role==Role::Shell)p.drawLine(edge.bottomLeft(),edge.bottomRight());else p.drawRoundedRect(edge,r,r);
    }
    if(!solid&&!disabled&&!flatKey&&!lumen){
        const qreal white=lumen?.24:(night?.15:(sienna?.30:.80));
        p.setPen(Detail::alpha(Qt::white,white));
        const qreal inset=qMin(r+1,edge.width()*.5);
        p.drawLine(QPointF(edge.left()+inset,edge.top()+1),QPointF(edge.right()-inset,edge.top()+1));
        if(sienna){p.setPen(QColor(179,141,108,87));p.drawLine(QPointF(edge.left()+inset,edge.bottom()),QPointF(edge.right()-inset,edge.bottom()));}
    }
    p.restore();
}

// Same compact 0.8.8 metrics as style.h. This is a complete native control
// sheet, with foregrounds written explicitly for light and dark profiles.
inline QString styleSheet(){
    const auto &t=current();const bool night=t.id=="nocturnecinema1",sienna=t.id=="siennastudio1";
    QString style=QStringLiteral(R"QSS(
QWidget { background:transparent; color:@INK@; font-family:'Segoe UI Variable','Segoe UI'; font-size:12px; }
QLabel { background:transparent; border:0; }
QLabel:disabled { color:@DISABLED@; }
QMainWindow,QDialog { background:transparent; }
QToolBar { background:transparent; spacing:7px; padding:9px; border:0; }
QToolBar::separator { width:1px; background:@LINE@; margin:5px 7px; }
QToolBar#projectToolbar { spacing:5px; padding:8px; }
QToolBar#projectToolbar::separator { margin:5px; }
QToolBar#projectToolbar QToolButton { padding:7px 6px; }
QMenuBar { background:transparent; color:@INK@; border:0; padding:2px 0; }
QMenuBar::item { background:transparent; padding:4px 9px; border-radius:4px; }
QMenuBar::item:selected,QMenuBar::item:pressed { background:@SELECTION@; color:@SELINK@; }
QMenuBar::item:disabled { color:@DISABLED@; }
QMenu { background:transparent; color:@INK@; border:0; padding:9px; font-size:13px; }
QMenu::item { background:transparent; padding:9px 28px 9px 17px; border-radius:4px; }
QMenu::item:selected { background:@SELECTION@; color:@SELINK@; }
QMenu::item:disabled { color:@DISABLED@; }
QMenu::separator { height:1px; background:@LINE@; margin:5px 9px; }
QMenu::indicator { width:14px; height:14px; }
QMenu::indicator:checked { image:url(@CHECK@); background:@ACCENT@; border-radius:3px; }
QToolButton,QPushButton { background:transparent; color:@INK@; border:1px solid transparent; border-radius:@BUTTON@px; padding:7px; }
QToolButton:hover,QPushButton:hover,QToolButton:pressed,QPushButton:pressed,QToolButton:checked,QPushButton:checked { background:transparent; color:@SELINK@; }
QToolButton:disabled,QPushButton:disabled { background:transparent; color:@DISABLED@; border-color:transparent; }
QToolButton:focus,QPushButton:focus { border-color:@ACCENT@; }
QPushButton:default { border-color:@ACCENT@; }
QPushButton[compactSurface="true"] { padding:5px 3px; font-size:12px; }
QPushButton#outputStart,QToolButton#outputStart { font-weight:600; color:@PRIMARYINK@; }
QPushButton#outputStart:hover,QPushButton#outputStart:pressed,QPushButton#outputStart:checked,QToolButton#outputStart:hover,QToolButton#outputStart:pressed,QToolButton#outputStart:checked { color:@PRIMARYINK@; }
QPushButton#outputStart:disabled,QToolButton#outputStart:disabled,QPushButton#blackoutButton:disabled { color:@DISABLED@; }
QToolButton#sectionHeading { text-align:left; background:transparent; border:0; border-radius:@BUTTON@px; font-weight:600; color:@INK@; padding:8px; }
QToolButton#sectionHeading:hover,QToolButton#sectionHeading:checked { background:transparent; color:@INK@; }
QToolButton#sectionHeading:disabled { color:@DISABLED@; }
QLabel#brandName { font-size:17px; font-weight:700; color:@INK@; }
QLabel#projectName,QLabel#projectTitle { color:@MUTED@; }
QLabel#surfaceHeading,QLabel#inspectorHeading,QLabel#libraryHeading { color:@INK@; font-size:12px; font-weight:600; }
QLabel#taskSurface { font-size:17px; font-weight:600; color:@INK@; }
QLabel#canvasHint,QLabel#scopeLabel,QLabel#browserStatus,QLabel#outputStatus,QLabel#entranceInfo { color:@MUTED@; font-size:12px; }
QLabel#dynamicStage { color:@MUTED@; font-size:11px; }
QLabel#updateDialogTitle { color:@INK@; font-size:20px; font-weight:700; }
QLabel#surfaceMenuTargetLabel { color:@INK@; font:600 13px 'Segoe UI'; }
QLabel#connectionDescription { background:transparent; border:1px solid transparent; border-radius:6px; padding:9px; color:@MUTED@; }
QWidget#surfaceSidebar,QWidget#inspectorPanel,QWidget#outputToolbar,QWidget#projectHeader { background:transparent; border:0; }
QListWidget,QListView,QTreeView,QTableView { background:transparent; alternate-background-color:@ALTERNATE@; color:@INK@; border:1px solid transparent; border-radius:6px; outline:0; }
QAbstractItemView::item { color:@INK@; }
QListWidget::item { padding:8px; border:1px solid transparent; border-radius:3px; }
QListWidget::item:selected,QAbstractItemView::item:selected { background:@SELECTION@; color:@SELINK@; border:1px solid transparent; border-left:3px solid @ACCENT@; }
QListWidget::item:selected:!active,QAbstractItemView::item:selected:!active { background:@INACTIVE@; color:@INK@; border-left-color:@MUTED@; }
QListWidget::item:hover:!selected,QAbstractItemView::item:hover:!selected { background:@HOVER@; }
QListWidget::item:disabled,QAbstractItemView::item:disabled { color:@DISABLED@; }
QListWidget:focus,QListView:focus,QTreeView:focus,QTableView:focus { border-color:@ACCENT@; }
QListWidget#animationLibrary::item { padding:4px; }
QHeaderView::section { background:@SECTION@; color:@INK@; padding:5px; border:1px solid @LINE@; }
QTableCornerButton::section { background:@SECTION@; border:0; }
QComboBox,QSpinBox,QDoubleSpinBox,QLineEdit,QTextEdit,QPlainTextEdit { background:transparent; color:@INK@; border:1px solid transparent; border-radius:@INPUT@px; padding:5px; selection-background-color:@SELECTION@; selection-color:@SELINK@; }
QComboBox:disabled,QSpinBox:disabled,QDoubleSpinBox:disabled,QLineEdit:disabled,QTextEdit:disabled,QPlainTextEdit:disabled { color:@DISABLED@; }
QLineEdit:read-only,QTextEdit:read-only,QPlainTextEdit:read-only { color:@MUTED@; }
QLineEdit,QTextEdit,QPlainTextEdit { placeholder-text-color:@MUTED@; }
QComboBox:focus,QSpinBox:focus,QDoubleSpinBox:focus,QLineEdit:focus,QTextEdit:focus,QPlainTextEdit:focus { border-color:@ACCENT@; }
QComboBox::drop-down { width:21px; border:0; background:transparent; }
QComboBox::down-arrow { image:none; width:0; height:0; }
QComboBox QAbstractItemView { background:@PANEL@; color:@INK@; selection-background-color:@SELECTION@; selection-color:@SELINK@; border:1px solid @MUTED@; outline:0; padding:3px; }
QComboBox QAbstractItemView::item { padding:6px 9px; min-height:18px; }
QSpinBox::up-button,QDoubleSpinBox::up-button,QSpinBox::down-button,QDoubleSpinBox::down-button { width:17px; border:0; background:transparent; }
QSpinBox::up-button:hover,QDoubleSpinBox::up-button:hover,QSpinBox::down-button:hover,QDoubleSpinBox::down-button:hover { background:@HOVER@; border-radius:3px; }
QSpinBox::up-button:pressed,QDoubleSpinBox::up-button:pressed,QSpinBox::down-button:pressed,QDoubleSpinBox::down-button:pressed { background:@SELECTION@; }
QSpinBox::up-button:disabled,QDoubleSpinBox::up-button:disabled,QSpinBox::down-button:disabled,QDoubleSpinBox::down-button:disabled { background:transparent; }
QSpinBox::up-arrow,QDoubleSpinBox::up-arrow,QSpinBox::down-arrow,QDoubleSpinBox::down-arrow { image:none; width:0; height:0; }
QSlider { background:transparent; }
QSlider::groove:horizontal { height:5px; background:@TRACK@; border:1px solid @LINE@; border-radius:3px; }
QSlider::sub-page:horizontal { background:@ACCENT@; border-radius:2px; }
QSlider::handle:horizontal { background:@HANDLE@; width:14px; height:14px; margin:-5px 0; border:1px solid @ACCENT@; border-radius:7px; }
QSlider::handle:horizontal:hover { background:@PANEL@; border-color:@ACCENT@; }
QSlider::handle:horizontal:pressed { background:@SELECTION@; border-color:@ACCENT@; }
QSlider::handle:horizontal:disabled { background:@TRACK@; border-color:@DISABLED@; }
QSlider::sub-page:horizontal:disabled { background:@DISABLED@; }
QSlider::handle:horizontal:focus { border-color:@ACCENT@; }
QCheckBox,QRadioButton { background:transparent; spacing:5px; color:@INK@; }
QRadioButton { spacing:6px; }
QCheckBox:disabled,QRadioButton:disabled { color:@DISABLED@; }
QCheckBox:focus,QRadioButton:focus { text-decoration:underline; }
QCheckBox::indicator { width:14px; height:14px; border:1px solid @MUTED@; border-radius:3px; background:@TRACK@; }
QCheckBox::indicator:checked { background:@ACCENT@; border-color:@ACCENT@; image:url(@CHECK@); }
QCheckBox::indicator:indeterminate { background:@ACCENT@; border:3px solid @PANEL@; }
QCheckBox::indicator:hover,QRadioButton::indicator:hover { border-color:@ACCENT@; }
QCheckBox::indicator:pressed,QRadioButton::indicator:pressed { background:@SELECTION@; }
QCheckBox::indicator:checked:pressed { background:@ACCENT@; }
QCheckBox::indicator:disabled { border-color:@DISABLED@; background:@TRACK@; }
QCheckBox::indicator:checked:disabled { background:@DISABLED@; image:url(@CHECK@); }
QRadioButton::indicator { width:14px; height:14px; border:1px solid @MUTED@; border-radius:7px; background:@CAP@; }
QRadioButton::indicator:checked { border:4px solid @ACCENT@; background:@CAP@; }
QRadioButton::indicator:disabled { border-color:@DISABLED@; background:@TRACK@; }
QRadioButton::indicator:checked:disabled { border:4px solid @DISABLED@; background:@CAP@; }
QScrollArea { background:transparent; border:0; }
QScrollArea > QWidget > QWidget { background:transparent; }
QScrollBar:vertical { background:@ALTERNATE@; width:12px; margin:2px; border:0; }
QScrollBar::handle:vertical { background:@SCROLL@; min-height:30px; border:1px solid @LINE@; border-radius:4px; }
QScrollBar:horizontal { background:@ALTERNATE@; height:12px; margin:2px; border:0; }
QScrollBar::handle:horizontal { background:@SCROLL@; min-width:30px; border:1px solid @LINE@; border-radius:4px; }
QScrollBar::handle:vertical:hover,QScrollBar::handle:horizontal:hover { background:@MUTED@; }
QScrollBar::handle:vertical:pressed,QScrollBar::handle:horizontal:pressed { background:@ACCENT@; }
QScrollBar::handle:vertical:disabled,QScrollBar::handle:horizontal:disabled { background:@DISABLED@; }
QScrollBar::add-line:vertical,QScrollBar::sub-line:vertical { height:0; }
QScrollBar::add-line:horizontal,QScrollBar::sub-line:horizontal { width:0; }
QScrollBar::add-page:vertical,QScrollBar::sub-page:vertical,QScrollBar::add-page:horizontal,QScrollBar::sub-page:horizontal { background:none; }
QTabWidget::pane { background:transparent; border:0; }
QTabBar { background:transparent; }
QTabBar::tab { background:@ALTERNATE@; color:@MUTED@; border:0; border-bottom:2px solid transparent; padding:8px 10px; }
QTabBar::tab:selected { background:@SELECTION@; color:@SELINK@; border-bottom-color:@ACCENT@; }
QTabBar::tab:hover:!selected { background:@HOVER@; color:@INK@; }
QTabBar::tab:disabled { color:@DISABLED@; }
QStatusBar { background:transparent; color:@MUTED@; border:0; font-size:12px; }
QStatusBar::item { border:0; }
QSplitter::handle { background:@LINE@; }
QSplitter::handle:hover { background:@ACCENT@; }
QFrame[frameShape="4"],QFrame[frameShape="5"] { color:@LINE@; }
QToolTip { background:@PANEL@; color:@INK@; border:1px solid @MUTED@; border-radius:4px; padding:7px; }
QProgressBar { border:1px solid @MUTED@; border-radius:4px; background:@TRACK@; text-align:center; color:@INK@; }
QProgressBar::chunk { background:@ACCENT@; border-radius:3px; }
QGroupBox { background:transparent; color:@INK@; border:1px solid @LINE@; border-radius:6px; margin-top:9px; padding-top:5px; }
QGroupBox::title { subcontrol-origin:margin; left:8px; padding:0 3px; color:@INK@; }
QDialogButtonBox { background:transparent; }
)QSS");
    // The cobalt focus accent is too dark beneath a dark progress percentage.
    // Keep both the filled and empty portions readable with the same ink.
    if(t.id=="lumenatelier1")style+=QStringLiteral("QProgressBar::chunk { background:#94AEC9; } QListWidget::item:selected:!active,QAbstractItemView::item:selected:!active { border-left-color:@ACCENT@; }");
    if(sienna)style+=QStringLiteral(R"QSS(
QToolBar#projectToolbar,QToolBar#projectToolbar QLabel,QToolBar#projectToolbar QLabel#brandName { color:#FAF3EA; }
QToolBar#projectToolbar QToolButton,QToolBar#projectToolbar QPushButton { color:#F7F1E8; }
QToolBar#projectToolbar QToolButton:hover,QToolBar#projectToolbar QToolButton:pressed,QToolBar#projectToolbar QToolButton:checked { color:#FFFFFF; }
QToolBar#projectToolbar QToolButton:disabled,QToolBar#projectToolbar QPushButton:disabled { color:#ACB4B8; }
QToolBar#projectToolbar::separator { background:rgba(248,231,208,56); }
)QSS");
    if(PotatoGlass::increasedContrast())style+=QStringLiteral(R"QSS(
QComboBox,QSpinBox,QDoubleSpinBox,QLineEdit,QTextEdit,QPlainTextEdit,QListWidget,QListView,QTreeView,QTableView,QToolButton,QPushButton { border-color:@MUTED@; }
QToolButton:focus,QPushButton:focus,QComboBox:focus,QSpinBox:focus,QDoubleSpinBox:focus,QLineEdit:focus,QTextEdit:focus,QPlainTextEdit:focus,QListWidget:focus,QListView:focus,QTreeView:focus,QTableView:focus { border-color:@ACCENT@; }
QLabel#canvasHint,QLabel#scopeLabel,QLabel#browserStatus,QLabel#outputStatus,QLabel#entranceInfo,QLabel#dynamicStage,QStatusBar { color:@INK@; }
)QSS");
    const auto rgba=[](const QColor &c,qreal a){return QStringLiteral("rgba(%1,%2,%3,%4)").arg(c.red()).arg(c.green()).arg(c.blue()).arg(qRound(a*255));};
    const QColor disabled=Detail::mix(t.muted,t.bg,night?.20:.23);
    const std::array<QPair<QString,QString>,23> tokens{{
        {"@INK@",t.ink.name()},{"@MUTED@",t.muted.name()},{"@PANEL@",t.panel.name()},
        {"@ACCENT@",t.accent.name()},{"@PRIMARYINK@",t.primaryInk.name()},{"@SELECTION@",t.selection.name()},
        {"@SELINK@",t.selectionInk.name()},{"@CAP@",t.cap.name()},{"@SECTION@",t.section.name()},
        {"@DISABLED@",disabled.name()},{"@BUTTON@",QString::number(t.buttonRadius)},
        {"@INPUT@",QString::number(qMin(t.buttonRadius,qreal(8)))},{"@LINE@",rgba(t.rail,PotatoGlass::increasedContrast()?.9:.32)},
        {"@ALTERNATE@",rgba(t.rail,.07)},{"@HOVER@",rgba(t.accent,night?.16:.09)},
        {"@INACTIVE@",Detail::mix(t.selection,t.panel,.35).name()},{"@TRACK@",Detail::mix(t.rail,t.panel,.68).name()},
        {"@SCROLL@",rgba(t.muted,.40)},{"@HANDLE@",night?QStringLiteral("#EDF7FF"):QStringLiteral("#FFFFFF")},
        {"@CHECK@",night?QStringLiteral(":/assets/icons/check.svg"):QStringLiteral(":/assets/icons/check-light.svg")},
        {"@BG@",t.bg.name()},{"@PRIMARY@",t.primary.name()},{"@RAIL@",t.rail.name()}
    }};
    for(const auto &token:tokens)style.replace(token.first,token.second);
    return style;
}
}
