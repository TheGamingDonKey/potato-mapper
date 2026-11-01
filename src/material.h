#pragma once
#include <QAbstractButton>
#include <QAbstractItemView>
#include <QAbstractSpinBox>
#include <QApplication>
#include <QCache>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenuBar>
#include <QPainter>
#include <QPainterPath>
#include <QPointer>
#include <QPixmap>
#include <QSlider>
#include <QSpinBox>
#include <QStatusBar>
#include <QStyleOptionButton>
#include <QStyleOptionComboBox>
#include <QStyleOptionSpinBox>
#include <QToolBar>
#include "material-menu.h"

namespace PotatoMaterial {
enum class Surface { None,Foundation,Shell,Panel,Section,Heading,Button,Input,Well,Dialog,FocusOnly };

// One bounded cache for the editor and its owned dialogs. Static material never
// samples the animated canvas and has no timer or graphics effect. Random grain
// is generated for the whole cached surface, rather than tiling a visible motif.
class ChromeMaterial final : public QObject {
public:
    explicit ChromeMaterial(QWidget *editor):QObject(editor),m_editor(editor),m_cache(16384){
        setObjectName("potatoChromeMaterial");
        qApp->installEventFilter(this);
        prepare(editor);
        for(auto *widget:editor->findChildren<QWidget*>())prepare(widget);
    }
    ~ChromeMaterial()override{if(qApp)qApp->removeEventFilter(this);}
protected:
    bool eventFilter(QObject *object,QEvent *event)override{
        auto *widget=qobject_cast<QWidget*>(object);
        if(!widget||!owned(widget)||excluded(widget))return false;
        if(event->type()==QEvent::Polish||event->type()==QEvent::Show){prepare(widget);}
        if(qobject_cast<QMenu*>(widget))return false;
        if(event->type()==QEvent::Paint){
            const auto surface=role(widget);
            if(surface!=Surface::None)paint(widget,static_cast<QPaintEvent*>(event),surface);
        }else if(event->type()==QEvent::Enter||event->type()==QEvent::Leave||
                 event->type()==QEvent::HoverEnter||event->type()==QEvent::HoverLeave||
                 event->type()==QEvent::FocusIn||event->type()==QEvent::FocusOut||
                 event->type()==QEvent::EnabledChange||event->type()==QEvent::MouseButtonPress||
                 event->type()==QEvent::MouseButtonRelease){
            if(role(widget)!=Surface::None)widget->update();
        }
        return false; // QWidget/Qt still paint content and own all interactions.
    }
private:
    bool owned(QWidget *widget)const{
        for(auto *parent=widget;parent;parent=parent->parentWidget())if(parent==m_editor)return true;
        return false;
    }
    static bool excluded(QWidget *widget){
        for(auto *parent=widget;parent;parent=parent->parentWidget())
            if(parent->inherits("QOpenGLWidget")||parent->property("potatoProjectionOutput").toBool())return true;
        return false;
    }
    Surface role(QWidget *widget)const{
        if(widget==m_editor)return Surface::Foundation;
        if(qobject_cast<QMenu*>(widget))return Surface::None;
        if(qobject_cast<QDialog*>(widget))return Surface::Dialog;
        if(qobject_cast<QMenuBar*>(widget)||qobject_cast<QToolBar*>(widget)||qobject_cast<QStatusBar*>(widget))return Surface::Shell;
        const auto name=widget->objectName();
        if(name=="projectHeader"||name=="projectToolbar"||name=="outputToolbar"||name=="canvasTools"||
           name=="stageTransport"||name=="sceneTransport"||name=="libraryHeader")return Surface::Shell;
        if(name=="surfaceSidebar"||name=="inspectorPanel"||name=="libraryPanel"||name=="libraryShelf"||
           name=="outputDetails"||name=="connectionDescription")return Surface::Panel;
        if(widget->property("potatoMaterialSection").toBool())return Surface::Section;
        if(name=="sectionHeading")return Surface::Heading;
        if(qobject_cast<QCheckBox*>(widget)||widget->inherits("QRadioButton")||qobject_cast<QSlider*>(widget))return Surface::FocusOnly;
        if(qobject_cast<QAbstractButton*>(widget)&&!qobject_cast<QCheckBox*>(widget)&&!widget->inherits("QRadioButton"))return Surface::Button;
        if(qobject_cast<QComboBox*>(widget)||qobject_cast<QAbstractSpinBox*>(widget))return Surface::Input;
        if(qobject_cast<QLineEdit*>(widget)){
            if(qobject_cast<QAbstractSpinBox*>(widget->parentWidget())||qobject_cast<QComboBox*>(widget->parentWidget()))return Surface::None;
            return Surface::Input;
        }
        if(widget->inherits("QComboBoxPrivateContainer"))return Surface::Dialog;
        if(widget->inherits("QTextEdit")||widget->inherits("QPlainTextEdit"))return Surface::Input;
        if(qobject_cast<QAbstractItemView*>(widget))return Surface::Well;
        return Surface::None;
    }
    void prepare(QWidget *widget){
        if(!owned(widget)||excluded(widget))return;
        if(auto *menu=qobject_cast<QMenu*>(widget)){installMenuMaterial(menu,m_editor);return;}
        if(widget->objectName()=="sectionHeading"&&widget->parentWidget()){
            widget->parentWidget()->setProperty("potatoMaterialSection",true);
            widget->parentWidget()->setAutoFillBackground(false);
        }
        if(role(widget)!=Surface::None){
            widget->setAutoFillBackground(false);widget->setAttribute(Qt::WA_Hover);
        }
        if(auto *view=qobject_cast<QAbstractItemView*>(widget))view->viewport()->setAutoFillBackground(false);
    }
    static qreal radius(Surface surface){
        if(surface==Surface::Foundation||surface==Surface::Shell)return 0;
        if(surface==Surface::Dialog)return 9;
        if(surface==Surface::Panel||surface==Surface::Section||surface==Surface::Well)return 6;
        return 5;
    }
    QPixmap plate(QWidget *widget,Surface surface,int state){
        const QSize logical=widget->size();
        const qreal dpr=qMin<qreal>(2,qMin<qreal>(widget->devicePixelRatioF(),
            qSqrt(2000000.0/qMax(1.0,double(logical.width())*logical.height()))));
        const QString key=QString::number(int(surface))+QLatin1Char('/')+QString::number(logical.width())+
            QLatin1Char('x')+QString::number(logical.height())+QLatin1Char('/')+QString::number(qRound(dpr*100))+
            QLatin1Char('/')+QString::number(state)+QLatin1Char('/')+qApp->property("potatoTheme").toString();
        if(auto *cached=m_cache.object(key))return *cached;
        QImage image(QSize(qMax(1,qRound(logical.width()*dpr)),qMax(1,qRound(logical.height()*dpr))),QImage::Format_ARGB32_Premultiplied);
        image.setDevicePixelRatio(dpr);image.fill(Qt::transparent);
        {
            QPainter painter(&image);painter.setRenderHint(QPainter::Antialiasing);
            const QRectF rect=QRectF(QPointF(0,0),QSizeF(logical)).adjusted(.5,.5,-.5,-.5);
            QPainterPath path;path.addRoundedRect(rect,radius(surface),radius(surface));painter.setClipPath(path);
            QColor base(30,35,42);
            if(surface==Surface::Foundation)base=QColor(25,29,35);
            if(surface==Surface::Shell)base=QColor(33,39,47);
            if(surface==Surface::Section)base=QColor(30,35,42);
            if(surface==Surface::Heading||surface==Surface::Button)base=QColor(42,48,57);
            if(surface==Surface::Input||surface==Surface::Well)base=QColor(21,26,32);
            if(surface==Surface::Dialog)base=QColor(32,37,45);
            const auto theme=qApp->property("potatoTheme").toString();
            if(theme=="halloween")base=QColor(base.red()+3,base.green()-1,base.blue()+3);
            else if(theme=="autumn")base=QColor(base.red()+4,base.green()+1,base.blue()-3);
            else if(theme=="christmas")base=QColor(base.red()-1,base.green()+3,base.blue());
            if(state&1)base=base.lighter(116); // hover
            if(state&2)base=base.darker(116);  // pressed
            if(state&4){ // checked/default action: readable, restrained accent wash
                const QColor color=accent();base=QColor((base.red()*3+color.red())/4,(base.green()*3+color.green())/4,(base.blue()*3+color.blue())/4);
            }
            if(state&8)base=QColor(31,35,41); // disabled has no bright gloss
            painter.fillPath(path,base);
            QRadialGradient cool(logical.width()*.12,-logical.height()*.2,qMax(180,logical.width()));
            cool.setColorAt(0,QColor(154,174,196,surface==Surface::Shell?19:12));cool.setColorAt(1,Qt::transparent);
            painter.fillPath(path,cool);
            QRadialGradient warm(logical.width()*.95,logical.height()*.1,qMax(140.0,logical.width()*.7));
            warm.setColorAt(0,QColor(156,132,104,surface==Surface::Shell?14:7));warm.setColorAt(1,Qt::transparent);
            painter.fillPath(path,warm);
            QLinearGradient sheen(0,0,0,qMax(1,logical.height()));
            const bool inset=surface==Surface::Input||surface==Surface::Well;
            sheen.setColorAt(0,QColor(255,255,255,(state&8)?3:(inset?3:14)));
            sheen.setColorAt(.38,QColor(255,255,255,inset?0:3));
            sheen.setColorAt(1,QColor(0,0,0,inset?9:17));painter.fillPath(path,sheen);
        }
        // Full-surface low-amplitude grain, independent of canvas/output pixels.
        quint32 random=0x504f5441u+quint32(surface)*137u;
        for(int y=0;y<image.height();++y){
            auto *row=reinterpret_cast<QRgb*>(image.scanLine(y));
            for(int x=0;x<image.width();++x){
                random^=random<<13;random^=random>>17;random^=random<<5;
                const QRgb pixel=row[x];if(qAlpha(pixel)!=255)continue;
                const int delta=int((random>>24)%5)-2;
                row[x]=qRgb(qBound(0,qRed(pixel)+delta,255),qBound(0,qGreen(pixel)+delta,255),qBound(0,qBlue(pixel)+delta,255));
            }
        }
        QPixmap result=QPixmap::fromImage(image);
        m_cache.insert(key,new QPixmap(result),int(qMax<qsizetype>(1,(image.sizeInBytes()+1023)/1024)));
        return result;
    }
    void paint(QWidget *widget,QPaintEvent *event,Surface surface){
        if(widget->width()<2||widget->height()<2)return;
        if(surface==Surface::FocusOnly){
            if(widget->hasFocus()&&widget->isEnabled()){
                QPainter painter(widget);painter.setClipRegion(event->region());painter.setRenderHint(QPainter::Antialiasing);
                painter.setBrush(Qt::NoBrush);painter.setPen(QPen(accent().lighter(120),2));
                painter.drawRoundedRect(QRectF(widget->rect()).adjusted(1.5,1.5,-1.5,-1.5),3,3);
            }
            return;
        }
        int state=0;
        if(widget->underMouse()&&widget->isEnabled())state|=1;
        if(auto *button=qobject_cast<QAbstractButton*>(widget)){
            if(button->isDown())state|=2;
            if(button->isChecked()&&surface!=Surface::Heading)state|=4;
        }
        if(widget->objectName()=="outputStart")state|=4;
        if(!widget->isEnabled())state|=8;
        QPainter painter(widget);painter.setClipRegion(event->region());painter.setRenderHint(QPainter::Antialiasing);
        const auto material=plate(widget,surface,state);
        painter.drawPixmap(widget->rect(),material);
        const QRectF rect=QRectF(widget->rect()).adjusted(.5,.5,-.5,-.5);
        if(surface==Surface::Foundation)return;
        if(surface==Surface::Shell){
            painter.setPen(QColor(228,236,248,20));painter.drawLine(QPointF(0,.5),QPointF(widget->width(),.5));
            painter.setPen(QColor(0,0,0,90));painter.drawLine(QPointF(0,widget->height()-.5),QPointF(widget->width(),widget->height()-.5));return;
        }
        QLinearGradient rim(0,0,0,widget->height());
        rim.setColorAt(0,QColor(229,236,247,(state&8)?19:((state&1)?88:53)));
        rim.setColorAt(.5,QColor(210,224,242,(state&8)?12:27));
        rim.setColorAt(1,QColor(155,174,196,(state&8)?10:20));
        painter.setBrush(Qt::NoBrush);painter.setPen(QPen(QBrush(rim),1));painter.drawRoundedRect(rect,radius(surface),radius(surface));
        if(surface==Surface::Input||surface==Surface::Well){
            painter.setPen(QColor(0,0,0,75));painter.drawLine(QPointF(5,1.5),QPointF(widget->width()-5,1.5));
        }
        if(widget->hasFocus()&&widget->isEnabled()){
            painter.setPen(QPen(accent().lighter(120),2));
            painter.drawRoundedRect(QRectF(widget->rect()).adjusted(1.5,1.5,-1.5,-1.5),qMax(2.0,radius(surface)-1),qMax(2.0,radius(surface)-1));
        }
        // Preserve Qt's complex-control geometry and hit areas while giving the
        // arrows an explicit foreground. Native Fusion arrows can otherwise
        // retain a dark system palette over transparent stylesheet surfaces.
        if(auto *combo=qobject_cast<QComboBox*>(widget)){
            QStyleOptionComboBox option;option.initFrom(combo);option.editable=combo->isEditable();option.frame=combo->hasFrame();
            const QRect arrow=combo->style()->subControlRect(QStyle::CC_ComboBox,&option,QStyle::SC_ComboBoxArrow,combo);
            drawChevron(painter,arrow,false,combo->isEnabled());
        }else if(auto *spin=qobject_cast<QAbstractSpinBox*>(widget)){
            if(spin->buttonSymbols()!=QAbstractSpinBox::NoButtons){
                QStyleOptionSpinBox option;option.initFrom(spin);option.frame=spin->hasFrame();option.buttonSymbols=spin->buttonSymbols();
                bool up=spin->isEnabled()&&!spin->isReadOnly(),down=up;
                if(auto *integer=qobject_cast<QSpinBox*>(spin);integer&&!integer->wrapping()){
                    up=up&&integer->value()<integer->maximum();down=down&&integer->value()>integer->minimum();
                }else if(auto *decimal=qobject_cast<QDoubleSpinBox*>(spin);decimal&&!decimal->wrapping()){
                    up=up&&decimal->value()<decimal->maximum();down=down&&decimal->value()>decimal->minimum();
                }
                drawChevron(painter,spin->style()->subControlRect(QStyle::CC_SpinBox,&option,QStyle::SC_SpinBoxUp,spin),true,up);
                drawChevron(painter,spin->style()->subControlRect(QStyle::CC_SpinBox,&option,QStyle::SC_SpinBoxDown,spin),false,down);
            }
        }
    }
    static void drawChevron(QPainter &painter,const QRect &area,bool up,bool enabled){
        if(area.width()<4||area.height()<3)return;
        const QPointF center=QRectF(area).center();const qreal direction=up?-1:1;
        QPainterPath chevron;chevron.moveTo(center.x()-3,center.y()-1.5*direction);
        chevron.lineTo(center.x(),center.y()+1.5*direction);chevron.lineTo(center.x()+3,center.y()-1.5*direction);
        painter.setBrush(Qt::NoBrush);painter.setPen(QPen(enabled?QColor(214,224,237):QColor(104,117,132),1.5,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
        painter.drawPath(chevron);
    }
    QPointer<QWidget> m_editor;QCache<QString,QPixmap> m_cache;
};
}

inline void installPotatoMaterial(QWidget *mainWindow){
    if(!mainWindow||mainWindow->property("potatoMaterialInstalled").toBool())return;
    mainWindow->setProperty("potatoMaterialInstalled",true);
    new PotatoMaterial::ChromeMaterial(mainWindow);
}
