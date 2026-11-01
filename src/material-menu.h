#pragma once
#include <QApplication>
#include <QGuiApplication>
#include <QImage>
#include <QLinearGradient>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QPointer>
#include <QRadialGradient>
#include <QScreen>
#include <QtMath>

// These materials are painted by the app. They do not enable or claim a Windows
// DWM backdrop. Only transient menus sample the editor, once on opening.
namespace PotatoMaterial {
inline QColor accent(){
    const auto theme=qApp->property("potatoTheme").toString();
    if(theme=="halloween")return QColor(234,161,93);
    if(theme=="autumn")return QColor(222,184,120);
    if(theme=="christmas")return QColor(214,190,133);
    return QColor(243,179,76);
}

inline QImage noiseImage(const QSize &size,quint32 seed=0x504f5441){
    QImage image(size,QImage::Format_ARGB32_Premultiplied);
    for(int y=0;y<size.height();++y){
        auto *row=reinterpret_cast<QRgb*>(image.scanLine(y));
        for(int x=0;x<size.width();++x){
            seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;
            const int alpha=(seed>>24)&3;
            row[x]=(seed&0x8000)?qRgba(alpha,alpha,alpha,alpha):qRgba(0,0,0,alpha);
        }
    }
    return image;
}

inline QImage blurPass(const QImage &source,bool horizontal){
    // Fixed radius and bounded image dimensions keep opening work predictable.
    constexpr int radius=4;
    QImage result(source.size(),QImage::Format_ARGB32);
    for(int y=0;y<source.height();++y){
        auto *row=reinterpret_cast<QRgb*>(result.scanLine(y));
        for(int x=0;x<source.width();++x){
            int red=0,green=0,blue=0;
            for(int offset=-radius;offset<=radius;++offset){
                const QRgb pixel=source.pixel(qBound(0,x+(horizontal?offset:0),source.width()-1),
                                               qBound(0,y+(horizontal?0:offset),source.height()-1));
                red+=qRed(pixel);green+=qGreen(pixel);blue+=qBlue(pixel);
            }
            row[x]=qRgb(red/9,green/9,blue/9);
        }
    }
    return result;
}

inline void paintFrostedMenu(QMenu *menu,QPaintEvent *event,const QImage &backdrop,const QImage &noise){
    QPainter painter(menu);painter.setClipRegion(event->region());
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(menu->rect(),Qt::transparent);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    QPainterPath panel;panel.addRoundedRect(QRectF(menu->rect()).adjusted(.5,.5,-.5,-.5),10,10);
    painter.setClipPath(panel,Qt::IntersectClip);
    if(!backdrop.isNull())painter.drawImage(QRectF(menu->rect()),backdrop);
    else{
        painter.fillPath(panel,QColor(31,35,42));
        QRadialGradient cloud(menu->width()*.75,0,qMax(menu->width(),menu->height()));
        cloud.setColorAt(0,QColor(114,103,90,32));cloud.setColorAt(1,Qt::transparent);
        painter.fillPath(panel,cloud);
    }
    painter.fillPath(panel,QColor(22,25,30,144));
    QLinearGradient sheen(0,0,0,menu->height());
    sheen.setColorAt(0,QColor(255,255,255,28));sheen.setColorAt(.4,QColor(255,255,255,3));
    sheen.setColorAt(1,QColor(0,0,0,17));painter.fillPath(panel,sheen);
    if(!noise.isNull())painter.drawImage(QRectF(menu->rect()),noise);
    painter.setClipping(false);
    QLinearGradient rim(0,0,0,menu->height());rim.setColorAt(0,QColor(255,255,255,95));
    rim.setColorAt(1,QColor(255,255,255,27));
    painter.setBrush(Qt::NoBrush);painter.setPen(QPen(QBrush(rim),1));painter.drawPath(panel);
    // QMenu paints the real actions over this panel. Its transparent stylesheet
    // preserves Qt's checks, shortcuts, submenu arrows and keyboard semantics.
}

class MenuMaterial final : public QObject {
public:
    explicit MenuMaterial(QMenu *menu,QWidget *editor):QObject(menu),m_menu(menu),m_editor(editor){
        setObjectName("potatoMenuMaterial");
        menu->setAttribute(Qt::WA_TranslucentBackground);menu->setAutoFillBackground(false);
        if(!menu->isVisible())menu->setWindowFlag(Qt::FramelessWindowHint);
        menu->setProperty("potatoMaterialMenu",true);
        menu->setProperty("materialBackdrop","app-owned-sampled-frost");
        menu->installEventFilter(this);
    }
    void prepare(const QPoint &position){
        if(!m_menu||m_sampling)return;
        m_sampling=true;m_position=position;m_size=m_menu->size();
        m_backdrop=QImage();
        const qreal ratio=qMin<qreal>(2,m_menu->devicePixelRatioF());
        const QSize pixels(qBound(1,qRound(m_size.width()*ratio),1536),
                           qBound(1,qRound(m_size.height()*ratio),1536));
        m_noise=noiseImage(pixels);
        // QWidget::grab reads our editor only, never the desktop or output
        // window. Clip the capture to the menu's local neighborhood, not the
        // whole canvas, and discard it when the popup closes.
        if(m_editor&&m_editor->isVisible()){
            constexpr int pad=24;
            const QPoint local=m_editor->mapFromGlobal(position);
            const QRect neighborhood=QRect(local,m_size).adjusted(-pad,-pad,pad,pad);
            const QRect visible=neighborhood.intersected(m_editor->rect());
            if(!visible.isEmpty()){
                QImage crop(pixels+QSize(qRound(2*pad*ratio),qRound(2*pad*ratio)),QImage::Format_ARGB32);
                crop.fill(QColor(27,31,37));
                const QImage frame=m_editor->grab(visible).toImage();
                {QPainter capture(&crop);
                    capture.drawImage(QRectF((visible.x()-neighborhood.x())*ratio,
                                              (visible.y()-neighborhood.y())*ratio,
                                              visible.width()*ratio,visible.height()*ratio),frame);}
                QImage blurred=crop.scaled(qMax(1,crop.width()/4),qMax(1,crop.height()/4),
                                            Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
                for(int pass=0;pass<3;++pass){blurred=blurPass(blurred,true);blurred=blurPass(blurred,false);}
                blurred=blurred.scaled(crop.size(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
                m_backdrop=blurred.copy(qRound(pad*ratio),qRound(pad*ratio),pixels.width(),pixels.height());
            }
        }
        m_sampling=false;m_prepared=true;
    }
protected:
    bool eventFilter(QObject *,QEvent *event)override{
        if(!m_menu)return false;
        if(event->type()==QEvent::Show){
            if(!m_prepared||m_position!=m_menu->pos()||m_size!=m_menu->size())prepare(m_menu->pos());
            m_menu->update();
        }else if(event->type()==QEvent::Hide){
            m_prepared=false;m_backdrop=QImage();m_noise=QImage();
        }else if(event->type()==QEvent::Paint){
            paintFrostedMenu(m_menu,static_cast<QPaintEvent*>(event),m_backdrop,m_noise);
        }
        return false;
    }
private:
    QPointer<QMenu> m_menu;QPointer<QWidget> m_editor;
    QImage m_backdrop,m_noise;QPoint m_position;QSize m_size;
    bool m_sampling=false,m_prepared=false;
};

inline MenuMaterial *installMenuMaterial(QMenu *menu,QWidget *editor){
    for(auto *child:menu->children())if(child->objectName()=="potatoMenuMaterial")
        return static_cast<MenuMaterial*>(child);
    return new MenuMaterial(menu,editor);
}
}

class MaterialSurfaceMenu final : public QMenu {
public:
    explicit MaterialSurfaceMenu(QWidget *parent=nullptr):QMenu(parent){
        setObjectName("materialSurfaceMenu");setMinimumWidth(290);
        // Also works outside the whole-app installer (the component proof).
        setStyleSheet(QStringLiteral(
            "QMenu {background:transparent;color:#f2f4f6;border:0;padding:9px;font-family:'Segoe UI';font-size:13px;}"
            "QMenu::item {padding:9px 28px 9px 17px;border-radius:4px;background:transparent;}"
            "QMenu::item:selected {background:rgba(115,124,137,65);color:white;}"
            "QMenu::item:disabled {color:#929ba6;}"
            "QMenu::separator {height:1px;background:rgba(224,230,240,24);margin:5px 9px;}"));
        m_material=PotatoMaterial::installMenuMaterial(this,parent?parent->window():nullptr);
    }
    explicit MaterialSurfaceMenu(const QString &title,QWidget *parent=nullptr):MaterialSurfaceMenu(parent){setTitle(title);}
    QAction *exec(const QPoint &requested,QAction *atAction=nullptr){
        ensurePolished();resize(sizeHint());QPoint position=requested;
        if(auto *screen=QGuiApplication::screenAt(requested)){
            const auto area=screen->availableGeometry();
            position.setX(qMax(area.left(),qMin(position.x(),area.right()-width()+1)));
            position.setY(qMax(area.top(),qMin(position.y(),area.bottom()-height()+1)));
        }
        m_material->prepare(position);return QMenu::exec(position,atAction);
    }
private:
    PotatoMaterial::MenuMaterial *m_material=nullptr;
};
