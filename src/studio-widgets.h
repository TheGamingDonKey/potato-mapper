#pragma once

#include <QApplication>
#include <QAbstractButton>
#include <QAction>
#include <QDrag>
#include <QEvent>
#include <QHash>
#include <QIcon>
#include <QListWidget>
#include <QMimeData>
#include <QPainter>
#include <QPainterPath>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QToolButton>
#include <QToolBar>
#include <QTimer>
#include <QVariant>
#include <QWidget>
#include "appearance.h"
#include "studio-themes.h"

// Geometry is from Microsoft's MIT-licensed Fluent System Icons (20px Regular).
// QIcon's SVG engine keeps the actual vector assets sharp at Windows DPI scales.
inline QIcon studioIcon(const QString &name){
    static QHash<QString,QIcon> cache;
    if(cache.contains(name))return cache.value(name);
    QIcon result(QStringLiteral(":/assets/icons/")+name+QStringLiteral(".svg"));
    for(int size:{20,32,40,64}){
        QPixmap selected=result.pixmap(size,size);
        if(selected.isNull())continue;
        QPainter painter(&selected);
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(selected.rect(),QColor("#191B1D"));
        painter.end();
        result.addPixmap(selected,QIcon::Normal,QIcon::On);
        result.addPixmap(selected,QIcon::Active,QIcon::On);
    }
    cache.insert(name,result);
    return result;
}

inline bool studioClearGlass(){return qApp&&PotatoAppearance::clearGlass(qApp->property("potatoInterfaceTheme").toString());}
inline bool studioAuroraClear(){const auto profile=qApp?qApp->property("potatoInterfaceTheme").toString():QString();return profile=="auroraclear1"||profile=="ambientglass1";}
inline bool studioRichIcons(){return studioAuroraClear()||PotatoThemes::active();}

// Small cached tiles reuse the shipped Fluent glyphs. No replacement action,
// font glyph dependency, SVG module or scene artwork is introduced.
inline QIcon studioAuroraIcon(const QString &name,const QString &family){
    static QHash<QString,QIcon> cache;
    const auto key=family+"/"+name;if(cache.contains(key))return cache.value(key);
    QColor start("#597EBB"),end("#3D5F9F");
    if(family=="peach"){start=QColor("#DC7445");end=QColor("#B43F76");}
    else if(family=="iris"){start=QColor("#956BE6");end=QColor("#6540BD");}
    else if(family=="lagoon"){start=QColor("#1C9F9B");end=QColor("#2665BC");}
    else if(family=="coral"){start=QColor("#DC5279");end=QColor("#A43C83");}
    const QIcon source(QStringLiteral(":/assets/icons/")+name+QStringLiteral(".svg"));
    QIcon icon;
    for(int size:{20,24,32,40,48,64}){
        QPixmap tile(size,size);tile.fill(Qt::transparent);QPainter painter(&tile);painter.setRenderHint(QPainter::Antialiasing);
        QLinearGradient gradient(0,0,size,size);gradient.setColorAt(0,start);gradient.setColorAt(1,end);
        painter.setBrush(gradient);painter.setPen(QColor(255,255,255,46));const qreal inset=size/44.0;
        painter.drawRoundedRect(QRectF(inset,inset,size-2*inset,size-2*inset),size*7.0/22,size*7.0/22);
        const int glyphSize=qRound(size*16.0/22);auto glyph=source.pixmap(glyphSize,glyphSize);
        if(!glyph.isNull()){
            QPainter ink(&glyph);ink.setCompositionMode(QPainter::CompositionMode_SourceIn);ink.fillRect(glyph.rect(),QColor("#FAFCFF"));ink.end();
            painter.drawPixmap((size-glyphSize)/2,(size-glyphSize)/2,glyphSize,glyphSize,glyph);
        }
        painter.end();
        for(auto state:{QIcon::Off,QIcon::On}){icon.addPixmap(tile,QIcon::Normal,state);icon.addPixmap(tile,QIcon::Active,state);icon.addPixmap(tile,QIcon::Selected,state);}
        QPixmap disabled(size,size);disabled.fill(Qt::transparent);QPainter dim(&disabled);dim.setOpacity(.38);dim.drawPixmap(0,0,tile);dim.end();
        icon.addPixmap(disabled,QIcon::Disabled,QIcon::Off);icon.addPixmap(disabled,QIcon::Disabled,QIcon::On);
    }
    cache.insert(key,icon);return icon;
}

inline QColor studioIconColour(const QString &name,const QString &family,bool toolbar=false){
    const auto &theme=PotatoThemes::current();
    const bool sienna=theme.id=="siennastudio1",nocturne=theme.id=="nocturnecinema1",tidal=theme.id=="tidallens1";
    // These semantic colours follow the approved studies. The same Fluent
    // asset remains identifiable in menus, the command deck and inspector.
    const bool warm=family=="peach"||family=="coral";
    const bool violet=family=="iris";
    const bool green=family=="lagoon";
    const bool blue=(name=="add"&&family=="peach")||name=="save"||name=="fit";
    if(theme.id=="lumenatelier1")return QColor(blue?"#A6C6E2":warm?"#E7BD9F":violet?"#C6B9DE":green?"#A8CDC9":"#CDD6E1");
    if(sienna&&toolbar)return QColor(blue?"#9EBBFF":warm?"#EEAA79":violet?"#D2BDF0":green?"#B9D5BA":"#C3CCCE");
    if(nocturne)return QColor(blue?"#80ADFF":warm?"#FFAD7D":violet?"#C7B6FF":green?"#7EDCC6":"#B4C1D4");
    if(sienna)return QColor(blue?"#315AA1":warm?"#A54A2F":violet?"#77629D":green?"#567D60":"#576470");
    if(tidal)return QColor(blue?"#2564C5":warm?"#BA5737":violet?"#6E5C9B":green?"#2A8170":"#436B7C");
    return QColor(blue?"#2853D9":warm?"#D96830":violet?"#7858BD":green?"#23866F":"#4D647B");
}

inline QIcon studioTintedIcon(const QString &name,const QColor &colour){
    static QHash<QString,QIcon> cache;
    const auto key=name+colour.name(QColor::HexArgb);if(cache.contains(key))return cache.value(key);
    const QIcon source(QStringLiteral(":/assets/icons/")+name+QStringLiteral(".svg"));QIcon result;
    for(int size:{20,24,32,40,48,64}){
        auto glyph=source.pixmap(size,size);if(glyph.isNull())continue;
        QPainter painter(&glyph);painter.setCompositionMode(QPainter::CompositionMode_SourceIn);painter.fillRect(glyph.rect(),colour);painter.end();
        result.addPixmap(glyph);
    }
    cache.insert(key,result);return result;
}

inline QIcon studioThemeIcon(const QString &name,const QString &family,bool toolbar=false,bool primary=false){
    const auto &theme=PotatoThemes::current();
    static QHash<QString,QIcon> cache;
    const auto key=theme.id+"/"+family+"/"+name+(toolbar?"/toolbar":"/control")+(primary?"/primary":"");
    if(cache.contains(key))return cache.value(key);
    const QColor colour=studioIconColour(name,family,toolbar);
    const bool nocturne=theme.id=="nocturnecinema1",sienna=theme.id=="siennastudio1",tidal=theme.id=="tidallens1",lumen=theme.id=="lumenatelier1";
    QIcon result;
    for(int size:{20,24,32,40,48,64})for(auto state:{QIcon::Off,QIcon::On}){
        QPixmap tile(size,size);tile.fill(Qt::transparent);QPainter painter(&tile);painter.setRenderHint(QPainter::Antialiasing);
        const qreal unit=size/22.0;
        QColor wash=colour;wash.setAlpha(primary?45:tidal?33:sienna?30:lumen?20:46);
        if(!nocturne){
            painter.setPen(Qt::NoPen);painter.setBrush(wash);
            const QRectF lens(1.5*unit,1.5*unit,size-3*unit,size-3*unit);
            if(tidal)painter.drawEllipse(lens);
            else painter.drawRoundedRect(lens,(sienna?5:5.5)*unit,(sienna?5:5.5)*unit);
            if(!sienna){
                painter.setBrush(Qt::NoBrush);painter.setPen(QPen(QColor(255,255,255,tidal?185:lumen?50:195),.8*unit));
                if(tidal)painter.drawArc(lens.adjusted(1.5*unit,1.5*unit,-1.5*unit,-1.5*unit),42*16,96*16);
                else painter.drawLine(QPointF(5*unit,2.5*unit),QPointF(17*unit,2.5*unit));
            }
        }
        const QPixmap baseTile=tile;
        const QColor ink=primary?theme.primaryInk:state==QIcon::On&&!(sienna&&toolbar)?theme.selectionInk:colour;
        const int glyphSize=qRound(size*(nocturne?18.0:16.0)/22.0);
        const auto glyph=studioTintedIcon(name,ink).pixmap(glyphSize,glyphSize);
        painter.drawPixmap((size-glyphSize)/2,(size-glyphSize)/2,glyphSize,glyphSize,glyph);painter.end();
        for(auto mode:{QIcon::Normal,QIcon::Active})result.addPixmap(tile,mode,state);
        // Menu selection uses the same legible foreground as its selected row.
        QPixmap selected=tile;
        if(!primary&&!(sienna&&toolbar)){
            selected=baseTile;QPainter highlight(&selected);
            highlight.drawPixmap((size-glyphSize)/2,(size-glyphSize)/2,glyphSize,glyphSize,studioTintedIcon(name,theme.selectionInk).pixmap(glyphSize,glyphSize));
        }
        result.addPixmap(selected,QIcon::Selected,state);
        QPixmap disabled(size,size);disabled.fill(Qt::transparent);QPainter dim(&disabled);dim.setOpacity(.38);dim.drawPixmap(0,0,tile);dim.end();result.addPixmap(disabled,QIcon::Disabled,state);
    }
    cache.insert(key,result);return result;
}

inline QPair<QString,QString> studioCommandAppearance(QString text){
    text.remove('&');text.remove(QChar(0x2026));text=text.section('\t',0,0).trimmed();
    if(text=="New"||text=="Open"||text=="Save"||text=="Save as")return {text=="New"?"add":text=="Open"?"open":"save","peach"};
    if(text.startsWith("Load media"))return {"open","peach"};
    if(text=="Undo"||text=="Redo")return {text.toLower(),"neutral"};
    if(text=="+ Surface")return {"add","lagoon"};
    if(text=="Potato FX"||text.startsWith("Browse animations")||text.startsWith("Choose animation"))return {"library","iris"};
    if(text=="Dynamic FX")return {"play","iris"};
    if(text=="Start output"||text=="Play entrance"||text=="Replay intro"||text=="Play / pause"||text=="Restart")return {"play","lagoon"};
    if(text=="Stop"||text=="Skip"||text=="Skip to ambient")return {"stop","lagoon"};
    if(text.startsWith("Blackout"))return {"eye-off","neutral"};
    if(text=="Duplicate")return {"duplicate","peach"};
    if(text.startsWith("Delete"))return {"trash","coral"};
    if(text.startsWith("Clear media"))return {"stop","neutral"};
    if(text=="Connect"||text=="Disconnect")return {"connect","lagoon"};
    if(text=="Fit view")return {"fit","lagoon"};
    if(text=="Reset mesh")return {"mesh","neutral"};
    if(text=="Refresh"||text=="Display settings")return {"settings","lagoon"};
    if(text=="Download update")return {"save","lagoon"};
    return {};
}

// A command can share its QAction between a light menu and Sienna's dark
// toolbar. Qt resynchronizes tool buttons whenever that action changes state.
// Reapply only the visual icon afterwards; the action and hit area stay intact.
class StudioToolbarIconFilter final : public QObject {
public:
    explicit StudioToolbarIconFilter(QToolButton *button):QObject(button){button->installEventFilter(this);}
protected:
    bool eventFilter(QObject *object,QEvent *event)override{
        if(event->type()==QEvent::ActionChanged)if(auto *button=qobject_cast<QToolButton*>(object)){
            QTimer::singleShot(0,button,[button]{
                auto *action=button->defaultAction();if(!action||!PotatoThemes::active())return;
                const auto glyph=action->property("potatoIconGlyph").toString();if(glyph.isEmpty())return;
                button->setIcon(studioThemeIcon(glyph,action->property("potatoIconFamily").toString(),true));
            });
        }
        return false;
    }
};

inline void studioApplyAppearanceIcons(QWidget *root){
    if(!root)return;const bool rich=studioRichIcons(),studio=PotatoThemes::active();
    for(auto *action:root->findChildren<QAction*>()){
        const auto role=studioCommandAppearance(action->text());if(role.first.isEmpty())continue;
        if(!action->property("potatoOriginalIcon").isValid())action->setProperty("potatoOriginalIcon",QVariant::fromValue(action->icon()));
        action->setProperty("potatoIconGlyph",role.first);action->setProperty("potatoIconFamily",role.second);
        action->setIcon(studio?studioThemeIcon(role.first,role.second):rich?studioAuroraIcon(role.first,role.second):qvariant_cast<QIcon>(action->property("potatoOriginalIcon")));
    }
    for(auto *button:root->findChildren<QAbstractButton*>()){
        const auto role=studioCommandAppearance(button->text());if(role.first.isEmpty())continue;
        button->setProperty("potatoIconFamily",role.second);
        if(auto *tool=qobject_cast<QToolButton*>(button);tool&&tool->defaultAction()){
            bool toolbar=false;for(auto *parent=tool->parentWidget();parent;parent=parent->parentWidget())if(qobject_cast<QToolBar*>(parent)){toolbar=true;break;}
            if(toolbar&&!tool->property("potatoToolbarIconFilter").toBool()){
                new StudioToolbarIconFilter(tool);tool->setProperty("potatoToolbarIconFilter",true);
            }
            tool->setIcon(studio&&toolbar?studioThemeIcon(role.first,role.second,true):tool->defaultAction()->icon());
            continue;
        }
        // Preserve the compact four-button surface row and two-button Dynamic
        // rows. Colour supports recognition without pushing labels offscreen.
        const auto id=button->objectName();
        const bool showTile=id=="outputStart"||id=="outputStop"||id=="blackoutButton"||id=="browseAnimations"||id=="downloadUpdateButton"||button->text().startsWith("Load media");
        if(!showTile)continue;
        if(!button->property("potatoOriginalIcon").isValid()){
            button->setProperty("potatoOriginalIcon",QVariant::fromValue(button->icon()));button->setProperty("potatoOriginalIconSize",button->iconSize());
        }
        button->setProperty("potatoIconGlyph",role.first);
        button->setIcon(studio?studioThemeIcon(role.first,role.second,false,id=="outputStart"):rich?studioAuroraIcon(role.first,role.second):qvariant_cast<QIcon>(button->property("potatoOriginalIcon")));
        button->setIconSize(rich?QSize(20,20):button->property("potatoOriginalIconSize").toSize());
    }
}

inline QIcon studioThumbnail(int pattern){
    return QIcon(QStringLiteral(":/assets/thumbnails/pattern-%1.png").arg(pattern));
}

namespace StudioRoles {
constexpr int Visible=Qt::UserRole+30;
constexpr int Locked=Qt::UserRole+31;
constexpr int Metadata=Qt::UserRole+32;
}

inline QColor studioSelectionAccent(const QWidget *widget){
    if(PotatoThemes::active())return PotatoThemes::current().accent;
    if(studioClearGlass())return QColor("#8CE8E6");
    const auto colour=widget?widget->property("studioAccent"):QVariant();
    return colour.isValid()?QColor(colour.toString()):QColor("#F3B34C");
}

// Shared with the surface list's click handler; use the list's visualItemRect.
inline QRect studioSurfaceIndicatorRect(const QRect &row,bool locked){
    const auto rect=row.adjusted(1,1,-1,-1);
    return QRect(rect.right()-24,rect.top()+(locked?43:16),20,20);
}

inline void studioPaintThumbnail(QPainter *painter,const QRect &rect,const QIcon &icon){
    painter->save();
    QPainterPath clip;clip.addRoundedRect(QRectF(rect),3,3);
    painter->setClipPath(clip);
    painter->fillRect(rect,QColor("#090C0F"));
    if(!icon.isNull()){
        const auto image=icon.pixmap(rect.size()*2);
        if(!image.isNull()){
            const auto scaled=image.scaled(rect.size()*2,Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation);
            const QRect source((scaled.width()-rect.width()*2)/2,(scaled.height()-rect.height()*2)/2,rect.width()*2,rect.height()*2);
            painter->drawPixmap(rect,scaled,source);
        }
    }
    painter->restore();
    painter->setPen(PotatoThemes::active()?PotatoThemes::current().rail:QColor("#3B4249"));painter->setBrush(Qt::NoBrush);
    painter->drawRoundedRect(rect.adjusted(0,0,-1,-1),3,3);
}

class StudioSurfaceDelegate final : public QStyledItemDelegate {
public:
    explicit StudioSurfaceDelegate(QObject *parent=nullptr):QStyledItemDelegate(parent){}
    QSize sizeHint(const QStyleOptionViewItem &,const QModelIndex &)const override{return QSize(220,78);}
    void paint(QPainter *painter,const QStyleOptionViewItem &option,const QModelIndex &index)const override{
        painter->save();painter->setRenderHint(QPainter::Antialiasing);
        const auto rect=option.rect.adjusted(1,1,-1,-1);
        const bool selected=option.state&QStyle::State_Selected;
        const bool hover=option.state&QStyle::State_MouseOver;
        const bool studio=PotatoThemes::active();
        const auto &theme=PotatoThemes::current();
        painter->fillRect(rect,studio?(selected?theme.selection:hover?theme.section:theme.panel):studioClearGlass()?(selected?QColor(86,160,204,65):hover?QColor(110,181,226,36):QColor(34,63,114,25)):(selected?QColor("#2B2D2D"):hover?QColor("#25292C"):QColor("#1B1E20")));
        painter->setPen(selected?studioSelectionAccent(option.widget):studio?theme.rail:studioClearGlass()?QColor(215,241,255,40):QColor("#33383D"));
        painter->drawRoundedRect(rect.adjusted(0,0,-1,-1),3,3);
        if(selected)painter->fillRect(QRect(rect.left(),rect.top()+2,4,rect.height()-4),studioSelectionAccent(option.widget));
        const QRect thumbnail(rect.left()+9,rect.top()+14,66,48);
        studioPaintThumbnail(painter,thumbnail,qvariant_cast<QIcon>(index.data(Qt::DecorationRole)));
        const auto lines=index.data(Qt::DisplayRole).toString().split('\n');
        const bool hasVisible=index.data(StudioRoles::Visible).isValid();
        const bool hasLocked=index.data(StudioRoles::Locked).isValid();
        const int indicatorWidth=(hasVisible||hasLocked)?28:0;
        const QRect title(thumbnail.right()+10,rect.top()+17,qMax(10,rect.right()-thumbnail.right()-18-indicatorWidth),20);
        auto font=option.font;font.setWeight(QFont::DemiBold);painter->setFont(font);painter->setPen(studio?(selected?theme.selectionInk:theme.ink):studioClearGlass()?QColor("#F7FAFF"):QColor("#F0F1F2"));
        painter->drawText(title,Qt::AlignVCenter|Qt::AlignLeft,QFontMetrics(font).elidedText(lines.value(0),Qt::ElideRight,title.width()));
        font.setWeight(QFont::Normal);font.setPixelSize(12);painter->setFont(font);painter->setPen(studio?(selected?theme.selectionInk:theme.muted):studioClearGlass()?QColor("#E5EEFF"):QColor("#A6ADB4"));
        const QRect subtitle(title.left(),title.bottom()+3,title.width(),19);
        painter->drawText(subtitle,Qt::AlignVCenter|Qt::AlignLeft,QFontMetrics(font).elidedText(lines.mid(1).join(" · "),Qt::ElideRight,subtitle.width()));
        if(hasLocked){
            const bool locked=index.data(StudioRoles::Locked).toBool();
            painter->setOpacity(locked?1.0:0.65);
            (studio?studioTintedIcon(locked?"lock":"unlock",selected?theme.selectionInk:theme.ink):studioIcon(locked?"lock":"unlock")).paint(painter,studioSurfaceIndicatorRect(option.rect,true));
        }
        if(hasVisible){
            const bool visible=index.data(StudioRoles::Visible).toBool();
            painter->setOpacity(visible?1.0:0.55);
            (studio?studioTintedIcon(visible?"eye":"eye-off",selected?theme.selectionInk:theme.ink):studioIcon(visible?"eye":"eye-off")).paint(painter,studioSurfaceIndicatorRect(option.rect,false));
        }
        painter->restore();
    }
};

class StudioLibraryDelegate final : public QStyledItemDelegate {
public:
    explicit StudioLibraryDelegate(QObject *parent=nullptr):QStyledItemDelegate(parent){}
    QSize sizeHint(const QStyleOptionViewItem &,const QModelIndex &)const override{return QSize(178,124);}
    void paint(QPainter *painter,const QStyleOptionViewItem &option,const QModelIndex &index)const override{
        painter->save();painter->setRenderHint(QPainter::Antialiasing);
        const QRect rect=option.rect.adjusted(5,4,-5,-2);
        const bool studio=PotatoThemes::active(),selected=option.state&QStyle::State_Selected;
        const auto &theme=PotatoThemes::current();
        if(studio)painter->fillRect(rect,selected?theme.selection:theme.panel);
        const QRect thumbnail(rect.left(),rect.top(),rect.width(),qMax(1,rect.height()-38));
        studioPaintThumbnail(painter,thumbnail,qvariant_cast<QIcon>(index.data(Qt::DecorationRole)));
        if(option.state&QStyle::State_Selected){
            painter->setBrush(Qt::NoBrush);painter->setPen(QPen(studioSelectionAccent(option.widget),2));
            painter->drawRoundedRect(thumbnail.adjusted(1,1,-1,-1),3,3);
        }else if(option.state&QStyle::State_MouseOver){
            painter->setBrush(Qt::NoBrush);painter->setPen(studio?theme.accent:QColor("#899099"));painter->drawRoundedRect(thumbnail,3,3);
        }
        const auto lines=index.data(Qt::DisplayRole).toString().split('\n');
        auto font=option.font;font.setWeight(QFont::DemiBold);painter->setFont(font);painter->setPen(studio?(selected?theme.selectionInk:theme.ink):studioClearGlass()?QColor("#F7FAFF"):QColor("#F0F1F2"));
        const QRect title(rect.left()+3,thumbnail.bottom()+4,rect.width()-6,17);
        painter->drawText(title,Qt::AlignLeft|Qt::AlignVCenter,QFontMetrics(font).elidedText(lines.value(0),Qt::ElideRight,title.width()));
        font.setWeight(QFont::Normal);font.setPixelSize(12);painter->setFont(font);painter->setPen(studio?(selected?theme.selectionInk:theme.muted):studioClearGlass()?QColor("#E5EEFF"):QColor("#A5ACB3"));
        const auto metadata=index.data(StudioRoles::Metadata).isValid()?index.data(StudioRoles::Metadata).toString():lines.mid(1).join(" · ");
        painter->drawText(QRect(title.left(),title.bottom()+1,title.width(),16),Qt::AlignLeft|Qt::AlignVCenter,QFontMetrics(font).elidedText(metadata,Qt::ElideRight,title.width()));
        painter->restore();
    }
};

// Browsing stays read-only. Only the canvas applies the stable pattern ID on drop.
class StudioLibraryWidget final : public QListWidget {
public:
    explicit StudioLibraryWidget(QWidget *parent=nullptr):QListWidget(parent){
        setDragEnabled(true);setDragDropMode(QAbstractItemView::DragOnly);
        setDefaultDropAction(Qt::CopyAction);setDropIndicatorShown(false);
    }
protected:
    void startDrag(Qt::DropActions)override{
        const auto *item=currentItem();if(!item)return;
        bool valid=false;const auto id=item->data(Qt::UserRole).toInt(&valid);if(!valid||id<0)return;
        auto *mime=new QMimeData;mime->setData("application/x-potato-pattern",QByteArray::number(id));
        auto *drag=new QDrag(this);drag->setMimeData(mime);
        const auto thumbnail=item->icon().pixmap(160,90);
        if(!thumbnail.isNull()){drag->setPixmap(thumbnail);drag->setHotSpot(QPoint(80,45));}
        drag->exec(Qt::CopyAction,Qt::CopyAction);
    }
};
