#pragma once
#include <QApplication>
#include <QDynamicPropertyChangeEvent>
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
#include <QVariant>
#include <QtMath>
#include "liquid-glass.h"
#include "aurora-glass.h"
#include "ambient-glass.h"
#include "studio-themes.h"

// These materials are painted by the app. They do not enable or claim a Windows
// DWM backdrop. Legacy Material/Liquid menus sample the editor on opening;
// AuroraClear/Transparent share static artwork; Ambient and Studio share their
// original bounded live fields, with no editor or projection capture.
namespace PotatoMaterial {
inline bool enabled(){
    // An unset property also keeps the isolated material component proof useful.
    return !qApp||qApp->property("potatoInterfaceTheme").toString()!=QStringLiteral("classic1");
}

inline QColor accent(){
    if(PotatoThemes::active())return PotatoThemes::current().accent;
    if(PotatoAmbient::active())return PotatoAmbient::accent();
    if(PotatoAurora::active())return PotatoAurora::accent();
    if(PotatoGlass::active())return PotatoGlass::accent();
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
            const int alpha=(seed>>24)&1;
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

inline bool glassMenuUsesSample(){
    return PotatoGlass::active()&&qApp&&!qApp->property("potatoReducedTransparency").toBool()
        &&!qApp->property("potatoIncreasedContrast").toBool();
}

inline QImage lensMenuBackdrop(const QImage &source,qreal pixelRatio){
    // This is a real displacement of the bounded editor sample, computed once
    // on opening. The rounded-rectangle distance field bends only the rim band;
    // it does not distort Qt's action text, checks, arrows or shortcut columns.
    if(source.isNull())return {};
    const QImage input=source.convertToFormat(QImage::Format_ARGB32);
    QImage result=input.copy();
    const qreal halfWidth=input.width()*.5,halfHeight=input.height()*.5;
    const qreal radius=qMin<qreal>(9*pixelRatio,qMin(halfWidth,halfHeight));
    const qreal band=qMax<qreal>(1,14*pixelRatio),strength=7*pixelRatio;
    for(int y=0;y<input.height();++y){
        auto *output=reinterpret_cast<QRgb*>(result.scanLine(y));
        for(int x=0;x<input.width();++x){
            const qreal px=x+.5-halfWidth,py=y+.5-halfHeight;
            const qreal qx=qAbs(px)-(halfWidth-radius),qy=qAbs(py)-(halfHeight-radius);
            const qreal ox=qMax<qreal>(qx,0),oy=qMax<qreal>(qy,0);
            const qreal outside=qSqrt(ox*ox+oy*oy);
            const qreal distance=radius-outside-qMin(qMax(qx,qy),qreal(0));
            if(distance<0||distance>=band)continue;
            qreal nx=0,ny=0;
            if(outside>.001){nx=ox/outside*(px<0?-1:1);ny=oy/outside*(py<0?-1:1);}
            else if(qx>qy)nx=px<0?-1:1;
            else ny=py<0?-1:1;
            const qreal offset=strength*qSin((distance/band)*3.14159265358979323846);
            const qreal sx=qBound<qreal>(0,x-nx*offset,input.width()-1);
            const qreal sy=qBound<qreal>(0,y-ny*offset,input.height()-1);
            const int x0=qFloor(sx),y0=qFloor(sy),x1=qMin(x0+1,input.width()-1),y1=qMin(y0+1,input.height()-1);
            const qreal fx=sx-x0,fy=sy-y0;
            const QRgb a=input.pixel(x0,y0),b=input.pixel(x1,y0),c=input.pixel(x0,y1),d=input.pixel(x1,y1);
            const auto blend=[&](int av,int bv,int cv,int dv){
                return qRound((av*(1-fx)+bv*fx)*(1-fy)+(cv*(1-fx)+dv*fx)*fy);
            };
            output[x]=qRgb(blend(qRed(a),qRed(b),qRed(c),qRed(d)),
                           blend(qGreen(a),qGreen(b),qGreen(c),qGreen(d)),
                           blend(qBlue(a),qBlue(b),qBlue(c),qBlue(d)));
        }
    }
    return result;
}

inline void paintLiquidGlassMenu(QMenu *menu,QPaintEvent *event,const QImage &backdrop,QWidget *editor){
    QPainter painter(menu);painter.setClipRegion(event->region());
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(menu->rect(),Qt::transparent);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    const QRectF rect=QRectF(menu->rect()).adjusted(.5,.5,-.5,-.5);
    QPainterPath panel;panel.addRoundedRect(rect,9,9);
    painter.setClipPath(panel,Qt::IntersectClip);
    const QPointF origin=editor?QPointF(editor->mapFromGlobal(menu->pos())):QPointF();
    const QSizeF sceneSize=editor?QSizeF(editor->size()):QSizeF(menu->size());
    const int state=(menu->isEnabled()?0:8)|((editor&&!editor->isActiveWindow())?16:0);
    PotatoGlass::paintSurface(painter,rect,PotatoGlass::Role::Dialog,origin,sceneSize,state);
    if(!backdrop.isNull()&&glassMenuUsesSample()){
        // Action labels start inside this band. The captured editor remains an
        // optical edge layer rather than a changing background under the text.
        QPainterPath center;center.addRoundedRect(rect.adjusted(14,14,-14,-14),3,3);
        painter.save();painter.setClipPath(panel.subtracted(center),Qt::IntersectClip);
        painter.setOpacity(.66);painter.drawImage(QRectF(menu->rect()),backdrop);painter.setOpacity(1);
        painter.fillPath(panel,QColor(29,34,32,112));
        painter.restore();
    }
    // The center bed is deliberately stable even over a bright captured scene.
    QPainterPath bed;bed.addRoundedRect(rect.adjusted(10,9,-10,-9),4,4);
    painter.fillPath(bed,QColor(29,34,32,qApp->property("potatoIncreasedContrast").toBool()?250:227));
    PotatoGlass::paintRim(painter,rect,PotatoGlass::Role::Dialog,state);
}

inline void paintFrostedMenu(QMenu *menu,QPaintEvent *event,const QImage &backdrop,const QImage &noise){
    QPainter painter(menu);painter.setClipRegion(event->region());
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(menu->rect(),Qt::transparent);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    QPainterPath panel;panel.addRoundedRect(QRectF(menu->rect()).adjusted(.5,.5,-.5,-.5),8,8);
    painter.setClipPath(panel,Qt::IntersectClip);
    if(!backdrop.isNull())painter.drawImage(QRectF(menu->rect()),backdrop);
    else{
        painter.fillPath(panel,QColor(39,43,50));
        QRadialGradient cloud(menu->width()*.75,0,qMax(menu->width(),menu->height()));
        cloud.setColorAt(0,QColor(114,103,90,12));cloud.setColorAt(1,Qt::transparent);
        painter.fillPath(panel,cloud);
    }
    painter.fillPath(panel,QColor(28,31,37,160));
    QLinearGradient sheen(0,0,0,menu->height());
    sheen.setColorAt(0,QColor(255,255,255,16));sheen.setColorAt(.4,QColor(255,255,255,3));
    sheen.setColorAt(1,QColor(0,0,0,9));painter.fillPath(panel,sheen);
    if(!noise.isNull())painter.drawImage(QRectF(menu->rect()),noise);
    painter.setClipping(false);
    QLinearGradient rim(0,0,0,menu->height());rim.setColorAt(0,QColor(255,255,255,64));
    rim.setColorAt(1,QColor(255,255,255,22));
    painter.setBrush(Qt::NoBrush);painter.setPen(QPen(QBrush(rim),1));painter.drawPath(panel);
    // QMenu paints the real actions over this panel. Its transparent stylesheet
    // preserves Qt's checks, shortcuts, submenu arrows and keyboard semantics.
}

inline void paintClearGlassMenu(QMenu *menu,QPaintEvent *event,QWidget *editor){
    QPainter painter(menu);painter.setClipRegion(event->region());
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setCompositionMode(QPainter::CompositionMode_Source);painter.fillRect(menu->rect(),Qt::transparent);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    const QRectF rect(menu->rect());
    const QPointF origin=editor?QPointF(editor->mapFromGlobal(menu->pos())):QPointF();
    const QSizeF sceneSize=editor?QSizeF(editor->size()):QSizeF(menu->size());
    if(PotatoThemes::active())PotatoThemes::paintSurface(painter,rect,PotatoThemes::Role::Dialog,origin,sceneSize,menu->isEnabled()?0:8);
    else if(PotatoAmbient::active())PotatoAmbient::paintSurface(painter,rect,PotatoAmbient::Role::Dialog,origin,sceneSize,menu->isEnabled()?0:8);
    else PotatoAurora::paintSurface(painter,rect,PotatoAurora::Role::Dialog,origin,sceneSize,menu->isEnabled()?0:8);
    // QMenu supplies its own real actions, checks, shortcuts and keyboard focus.
}

class MenuMaterial final : public QObject {
public:
    explicit MenuMaterial(QMenu *menu,QWidget *editor):QObject(menu),m_menu(menu),m_editor(editor),
        m_originalFlags(menu->windowFlags()),m_originalTranslucent(menu->testAttribute(Qt::WA_TranslucentBackground)),
        m_originalNoSystemBackground(menu->testAttribute(Qt::WA_NoSystemBackground)),m_originalAutoFill(menu->autoFillBackground()){
        setObjectName("potatoMenuMaterial");
        setProperty("potatoMaterialPaintCount",QVariant::fromValue<qulonglong>(0));
        setProperty("potatoMaterialCaptureCount",QVariant::fromValue<qulonglong>(0));
        qApp->installEventFilter(this);
        syncTheme();
    }
    ~MenuMaterial()override{if(qApp)qApp->removeEventFilter(this);}
    void syncTheme(){
        if(!m_menu||m_configuring)return;
        const bool material=enabled();
        const bool studio=PotatoThemes::active();
        const bool glass=PotatoGlass::active();const bool clearGlass=PotatoAurora::active()||PotatoAmbient::active()||studio;
        setProperty("potatoMaterialEnabled",material);
        m_menu->setProperty("potatoMaterialMenu",material);
        m_menu->setProperty("materialBackdrop",studio?"app-owned-live-studio-theme":(PotatoAmbient::active()?"app-owned-live-ambient-glass":(clearGlass?"app-owned-continuous-clear-glass":(glass?(glassMenuUsesSample()?"app-owned-editor-lensed-glass":"app-owned-opaque-glass"):
            (material?"app-owned-sampled-frost":"opaque-classic")))));
        if(!material||clearGlass||(m_prepared&&m_preparedGlass!=glass)||(glass&&!glassMenuUsesSample()))clearBackdrop();
        // A live action can change themes before its popup finishes closing.
        // Bypass painting immediately, and alter native popup flags only hidden.
        if(m_menu->isVisible()||(m_configured&&m_windowMaterial==material)){
            m_menu->update();return;
        }
        m_configuring=true;m_configured=true;m_windowMaterial=material;
        m_menu->setAttribute(Qt::WA_TranslucentBackground,material||m_originalTranslucent);
        m_menu->setAttribute(Qt::WA_NoSystemBackground,material||m_originalNoSystemBackground);
        m_menu->setAutoFillBackground(material?false:m_originalAutoFill);
        const auto flags=material?(m_originalFlags|Qt::FramelessWindowHint):m_originalFlags;
        if(m_menu->windowFlags()!=flags)m_menu->setWindowFlags(flags);
        m_configuring=false;m_menu->update();
    }
    void prepare(const QPoint &position){
        syncTheme();
        if(!m_menu||!enabled()||m_sampling)return;
        m_sampling=true;m_position=position;m_size=m_menu->size();
        m_backdrop=QImage();m_noise=QImage();m_preparedGlass=PotatoGlass::active();
        setProperty("potatoGlassMenuLensedPixels",0);
        if(PotatoThemes::active()||PotatoAurora::active()||PotatoAmbient::active()||(m_preparedGlass&&!glassMenuUsesSample())){
            m_sampling=false;m_prepared=true;return;
        }
        constexpr int pad=24;
        const qreal paddedArea=qMax(1.0,double(m_size.width()+2*pad)*(m_size.height()+2*pad));
        const qreal ratio=qMin<qreal>(qMin<qreal>(2,m_menu->devicePixelRatioF()),
            qMin(qSqrt(2000000.0/paddedArea),1536.0/qMax(1,qMax(m_size.width()+2*pad,m_size.height()+2*pad))));
        const QSize pixels(qMax(1,qFloor(m_size.width()*ratio)),qMax(1,qFloor(m_size.height()*ratio)));
        const int pixelPad=qFloor(pad*ratio);
        if(!m_preparedGlass)m_noise=noiseImage(pixels);
        // QWidget::grab reads our editor only, never the desktop or output
        // window. Clip the capture to the menu's local neighborhood, not the
        // whole canvas, and discard it when the popup closes.
        if(m_editor&&m_editor->isVisible()){
            const QPoint local=m_editor->mapFromGlobal(position);
            const QRect neighborhood=QRect(local,m_size).adjusted(-pad,-pad,pad,pad);
            const QRect visible=neighborhood.intersected(m_editor->rect());
            const qreal captureArea=double(visible.width())*visible.height()*qPow(m_editor->devicePixelRatioF(),2);
            if(!visible.isEmpty()&&captureArea<=2000000.0){
                QImage crop(pixels+QSize(2*pixelPad,2*pixelPad),QImage::Format_ARGB32);
                crop.fill(QColor(27,31,37));
                setProperty("potatoMaterialCaptureCount",QVariant::fromValue(++m_captureCount));
                const QImage frame=m_editor->grab(visible).toImage();
                {QPainter capture(&crop);
                    capture.drawImage(QRectF((visible.x()-neighborhood.x())*ratio,
                                              (visible.y()-neighborhood.y())*ratio,
                                              visible.width()*ratio,visible.height()*ratio),frame);}
                if(m_preparedGlass){
                    QImage softened=crop.scaled(qMax(1,crop.width()/2),qMax(1,crop.height()/2),
                                                Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
                    softened=blurPass(blurPass(softened,true),false);
                    softened=softened.scaled(crop.size(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
                    m_backdrop=lensMenuBackdrop(softened.copy(pixelPad,pixelPad,pixels.width(),pixels.height()),ratio);
                    setProperty("potatoGlassMenuLensedPixels",m_backdrop.width()*m_backdrop.height());
                }else{
                    QImage blurred=crop.scaled(qMax(1,crop.width()/4),qMax(1,crop.height()/4),
                                                Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
                    for(int pass=0;pass<3;++pass){blurred=blurPass(blurred,true);blurred=blurPass(blurred,false);}
                    blurred=blurred.scaled(crop.size(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
                    m_backdrop=blurred.copy(pixelPad,pixelPad,pixels.width(),pixels.height());
                }
            }
        }
        m_sampling=false;m_prepared=true;
    }
protected:
    bool eventFilter(QObject *object,QEvent *event)override{
        if(!m_menu)return false;
        if(object==qApp&&event->type()==QEvent::DynamicPropertyChange){
            const auto name=static_cast<QDynamicPropertyChangeEvent*>(event)->propertyName();
            if(name=="potatoInterfaceTheme"||name=="potatoReducedTransparency"||name=="potatoIncreasedContrast")syncTheme();
            return false;
        }
        if(object!=m_menu)return false;
        if(event->type()==QEvent::Show){
            syncTheme();
            if(enabled()&&(!m_prepared||m_position!=m_menu->pos()||m_size!=m_menu->size()))prepare(m_menu->pos());
            m_menu->update();
        }else if(event->type()==QEvent::Hide){
            clearBackdrop();syncTheme();
        }else if(event->type()==QEvent::Paint&&enabled()){
            setProperty("potatoMaterialPaintCount",QVariant::fromValue(++m_paintCount));
            if(PotatoThemes::active()||PotatoAurora::active()||PotatoAmbient::active())paintClearGlassMenu(m_menu,static_cast<QPaintEvent*>(event),m_editor);
            else if(PotatoGlass::active())paintLiquidGlassMenu(m_menu,static_cast<QPaintEvent*>(event),m_backdrop,m_editor);
            else paintFrostedMenu(m_menu,static_cast<QPaintEvent*>(event),m_backdrop,m_noise);
        }
        return false;
    }
private:
    void clearBackdrop(){m_prepared=false;m_backdrop=QImage();m_noise=QImage();setProperty("potatoGlassMenuLensedPixels",0);}
    QPointer<QMenu> m_menu;QPointer<QWidget> m_editor;
    QImage m_backdrop,m_noise;QPoint m_position;QSize m_size;
    Qt::WindowFlags m_originalFlags;
    bool m_originalTranslucent,m_originalNoSystemBackground,m_originalAutoFill;
    bool m_sampling=false,m_prepared=false,m_preparedGlass=false,m_configuring=false,m_configured=false,m_windowMaterial=false;
    qulonglong m_paintCount=0,m_captureCount=0;
};

inline MenuMaterial *installMenuMaterial(QMenu *menu,QWidget *editor){
    for(auto *child:menu->children())if(child->objectName()=="potatoMenuMaterial"){
        auto *material=static_cast<MenuMaterial*>(child);material->syncTheme();return material;
    }
    return new MenuMaterial(menu,editor);
}
}

class MaterialSurfaceMenu final : public QMenu {
public:
    explicit MaterialSurfaceMenu(QWidget *parent=nullptr):QMenu(parent){
        setObjectName("materialSurfaceMenu");setMinimumWidth(290);
        // Inherit the chosen interface stylesheet. A private Material stylesheet
        // would conceal Classic even after the app and painter had switched.
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
