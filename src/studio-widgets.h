#pragma once

#include <QApplication>
#include <QDrag>
#include <QHash>
#include <QIcon>
#include <QListWidget>
#include <QMimeData>
#include <QPainter>
#include <QPainterPath>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QVariant>
#include <QWidget>

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

inline QIcon studioThumbnail(int pattern){
    return QIcon(QStringLiteral(":/assets/thumbnails/pattern-%1.png").arg(pattern));
}

namespace StudioRoles {
constexpr int Visible=Qt::UserRole+30;
constexpr int Locked=Qt::UserRole+31;
constexpr int Metadata=Qt::UserRole+32;
}

inline QColor studioSelectionAccent(const QWidget *widget){
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
    painter->setPen(QColor("#3B4249"));painter->setBrush(Qt::NoBrush);
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
        painter->fillRect(rect,selected?QColor("#2B2D2D"):hover?QColor("#25292C"):QColor("#1B1E20"));
        painter->setPen(selected?studioSelectionAccent(option.widget):QColor("#33383D"));
        painter->drawRoundedRect(rect.adjusted(0,0,-1,-1),3,3);
        if(selected)painter->fillRect(QRect(rect.left(),rect.top()+2,4,rect.height()-4),studioSelectionAccent(option.widget));
        const QRect thumbnail(rect.left()+9,rect.top()+14,66,48);
        studioPaintThumbnail(painter,thumbnail,qvariant_cast<QIcon>(index.data(Qt::DecorationRole)));
        const auto lines=index.data(Qt::DisplayRole).toString().split('\n');
        const bool hasVisible=index.data(StudioRoles::Visible).isValid();
        const bool hasLocked=index.data(StudioRoles::Locked).isValid();
        const int indicatorWidth=(hasVisible||hasLocked)?28:0;
        const QRect title(thumbnail.right()+10,rect.top()+17,qMax(10,rect.right()-thumbnail.right()-18-indicatorWidth),20);
        auto font=option.font;font.setWeight(QFont::DemiBold);painter->setFont(font);painter->setPen(QColor("#F0F1F2"));
        painter->drawText(title,Qt::AlignVCenter|Qt::AlignLeft,QFontMetrics(font).elidedText(lines.value(0),Qt::ElideRight,title.width()));
        font.setWeight(QFont::Normal);font.setPixelSize(12);painter->setFont(font);painter->setPen(QColor("#A6ADB4"));
        const QRect subtitle(title.left(),title.bottom()+3,title.width(),19);
        painter->drawText(subtitle,Qt::AlignVCenter|Qt::AlignLeft,QFontMetrics(font).elidedText(lines.mid(1).join(" · "),Qt::ElideRight,subtitle.width()));
        if(hasLocked){
            const bool locked=index.data(StudioRoles::Locked).toBool();
            painter->setOpacity(locked?1.0:0.65);
            studioIcon(locked?"lock":"unlock").paint(painter,studioSurfaceIndicatorRect(option.rect,true));
        }
        if(hasVisible){
            const bool visible=index.data(StudioRoles::Visible).toBool();
            painter->setOpacity(visible?1.0:0.55);
            studioIcon(visible?"eye":"eye-off").paint(painter,studioSurfaceIndicatorRect(option.rect,false));
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
        const QRect thumbnail(rect.left(),rect.top(),rect.width(),qMax(1,rect.height()-38));
        studioPaintThumbnail(painter,thumbnail,qvariant_cast<QIcon>(index.data(Qt::DecorationRole)));
        if(option.state&QStyle::State_Selected){
            painter->setBrush(Qt::NoBrush);painter->setPen(QPen(studioSelectionAccent(option.widget),2));
            painter->drawRoundedRect(thumbnail.adjusted(1,1,-1,-1),3,3);
        }else if(option.state&QStyle::State_MouseOver){
            painter->setBrush(Qt::NoBrush);painter->setPen(QColor("#899099"));painter->drawRoundedRect(thumbnail,3,3);
        }
        const auto lines=index.data(Qt::DisplayRole).toString().split('\n');
        auto font=option.font;font.setWeight(QFont::DemiBold);painter->setFont(font);painter->setPen(QColor("#F0F1F2"));
        const QRect title(rect.left()+3,thumbnail.bottom()+4,rect.width()-6,17);
        painter->drawText(title,Qt::AlignLeft|Qt::AlignVCenter,QFontMetrics(font).elidedText(lines.value(0),Qt::ElideRight,title.width()));
        font.setWeight(QFont::Normal);font.setPixelSize(12);painter->setFont(font);painter->setPen(QColor("#A5ACB3"));
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
