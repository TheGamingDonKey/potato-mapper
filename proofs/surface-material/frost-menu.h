#pragma once
#include <QAction>
#include <QGuiApplication>
#include <QLinearGradient>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QScreen>
#include <QStyleFactory>
#include <QStyleHints>
#include <QStyleOptionMenuItem>
#include <QWidgetAction>

// Editor-only, app-rendered frost; NOT Windows DWM Acrylic. A real QMenu owns
// actions, keyboard navigation and dismissal. Sample only this app on opening.
class MaterialSurfaceMenu final : public QMenu {
public:
    explicit MaterialSurfaceMenu(QWidget *parent=nullptr):QMenu(parent){
        setObjectName("materialSurfaceMenu");setMinimumWidth(290);
        setWindowFlags(Qt::Popup|Qt::FramelessWindowHint);
        setAttribute(Qt::WA_TranslucentBackground);setAutoFillBackground(false);
        setStyleSheet("QMenu#materialSurfaceMenu {background:transparent;border:0;padding:9px;font-family:'Segoe UI';font-size:13px;}"
                      "QMenu#materialSurfaceMenu::item {padding:9px 17px;}"
                      "QMenu#materialSurfaceMenu::separator {height:1px;margin:5px 9px;}");
        m_windowsStyle=QStyleFactory::create("windows11");m_fusionStyle=QStyleFactory::create("Fusion");
        if(m_windowsStyle)m_windowsStyle->setParent(this);
        if(m_fusionStyle)m_fusionStyle->setParent(this);
        m_noise=QPixmap::fromImage(grain());
        setProperty("nativeBackdropStatus","app-owned-sampled-frost");
        setProperty("nativeBackdropHresult","DWM-not-used");setProperty("nativeBackdropLayered",true);
    }
    QAction *exec(const QPoint &requested,QAction *atAction=nullptr){
        ensurePolished();resize(sizeHint());QPoint position=requested;
        if(auto *screen=QGuiApplication::screenAt(requested)){
            const auto area=screen->availableGeometry();
            position.setX(qBound(area.left(),position.x(),area.right()-width()+1));
            position.setY(qBound(area.top(),position.y(),area.bottom()-height()+1));
        }
        sampleEditorBackdrop(position);return QMenu::exec(position,atAction);
    }
protected:
    void paintEvent(QPaintEvent *event)override{
        QPainter p(this);p.setClipRegion(event->region());p.setRenderHint(QPainter::Antialiasing);
        p.setCompositionMode(QPainter::CompositionMode_Source);p.fillRect(rect(),Qt::transparent);
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        QPainterPath panel;panel.addRoundedRect(QRectF(rect()).adjusted(.5,.5,-.5,-.5),10,10);
        p.setClipPath(panel,Qt::IntersectClip);
        if(!m_backdrop.isNull())p.drawImage(QRectF(rect()),m_backdrop);else p.fillPath(panel,QColor(35,38,43));
        p.fillPath(panel,QColor(22,25,30,144));
        QLinearGradient sheen(0,0,0,height());sheen.setColorAt(0,QColor(255,255,255,28));
        sheen.setColorAt(.4,QColor(255,255,255,3));sheen.setColorAt(1,QColor(0,0,0,17));
        p.fillPath(panel,sheen);p.drawTiledPixmap(rect(),m_noise);
        QStyle *items=m_windowsStyle;
        if(!items||QGuiApplication::styleHints()->colorScheme()==Qt::ColorScheme::Light)items=m_fusionStyle;
        if(!items)items=style();
        for(auto *action:actions())if(action->isVisible()&&!qobject_cast<QWidgetAction*>(action)){
            QStyleOptionMenuItem option;initStyleOption(&option,action);option.rect=actionGeometry(action);option.palette=menuPalette();
            items->drawControl(QStyle::CE_MenuItem,&option,&p,this);
        }
        p.setClipping(false);QLinearGradient rim(0,0,0,height());
        rim.setColorAt(0,QColor(255,255,255,95));rim.setColorAt(1,QColor(255,255,255,27));
        p.setBrush(Qt::NoBrush);p.setPen(QPen(QBrush(rim),1));p.drawPath(panel);
    }
private:
    void sampleEditorBackdrop(const QPoint &globalPosition){
        auto *editor=parentWidget()?parentWidget()->window():nullptr;if(!editor)return;
        QImage frame=editor->grab().toImage().convertToFormat(QImage::Format_ARGB32);
        const double dpr=frame.devicePixelRatio();frame.setDevicePixelRatio(1);
        const QPoint local=editor->mapFromGlobal(globalPosition);
        const int pad=qRound(24*dpr),w=qRound(width()*dpr),h=qRound(height()*dpr);
        QImage crop(w+2*pad,h+2*pad,QImage::Format_ARGB32);crop.fill(QColor(27,31,37));
        {QPainter capture(&crop);capture.drawImage(QPoint(pad-qRound(local.x()*dpr),pad-qRound(local.y()*dpr)),frame);}
        QImage blurred=crop.scaled(qMax(1,crop.width()/4),qMax(1,crop.height()/4),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
        // Three bounded separable box passes approximate Gaussian blur once
        // per opening. Nothing blurs the live OpenGL output or runs per frame.
        for(int pass=0;pass<3;++pass){blurred=boxPass(blurred,true);blurred=boxPass(blurred,false);}
        blurred=blurred.scaled(crop.size(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
        m_backdrop=blurred.copy(pad,pad,w,h);
    }
    static QImage boxPass(const QImage &source,bool horizontal){
        constexpr int radius=4;QImage result(source.size(),QImage::Format_ARGB32);
        for(int y=0;y<source.height();++y){auto *row=reinterpret_cast<QRgb*>(result.scanLine(y));
            for(int x=0;x<source.width();++x){int red=0,green=0,blue=0;
                for(int offset=-radius;offset<=radius;++offset){
                    const QRgb pixel=source.pixel(qBound(0,x+(horizontal?offset:0),source.width()-1),qBound(0,y+(horizontal?0:offset),source.height()-1));
                    red+=qRed(pixel);green+=qGreen(pixel);blue+=qBlue(pixel);
                }row[x]=qRgb(red/9,green/9,blue/9);
            }
        }return result;
    }
    static QImage grain(){
        QImage image(64,64,QImage::Format_ARGB32_Premultiplied);quint32 state=0x504f5441;
        for(int y=0;y<64;++y)for(int x=0;x<64;++x){state^=state<<13;state^=state>>17;state^=state<<5;const int alpha=(state>>24)&3;
            image.setPixel(x,y,(state&0x8000)?qRgba(alpha,alpha,alpha,alpha):qRgba(0,0,0,alpha));}
        return image;
    }
    static QPalette menuPalette(){
        QPalette palette;for(auto group:{QPalette::Active,QPalette::Inactive,QPalette::Disabled}){
            const QColor text=group==QPalette::Disabled?QColor(170,173,178):QColor(248,249,250);
            palette.setColor(group,QPalette::WindowText,text);palette.setColor(group,QPalette::Text,text);palette.setColor(group,QPalette::ButtonText,text);
            palette.setColor(group,QPalette::Highlight,QColor(94,102,113,120));palette.setColor(group,QPalette::HighlightedText,Qt::white);palette.setColor(group,QPalette::Window,Qt::transparent);
        }return palette;
    }
    QStyle *m_windowsStyle=nullptr,*m_fusionStyle=nullptr;QImage m_backdrop;QPixmap m_noise;
};
