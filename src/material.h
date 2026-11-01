#pragma once
#include <QAbstractButton>
#include <QAbstractItemView>
#include <QAbstractSpinBox>
#include <QApplication>
#include <QCache>
#include <QCheckBox>
#include <QComboBox>
#include <QCursor>
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
#include "liquid-glass.h"

namespace PotatoMaterial {
enum class Surface { None,Foundation,Shell,Panel,Section,Heading,Button,Input,Well,Dialog,FocusOnly };

// One bounded cache per selected profile for the editor and owned dialogs. Static material never
// samples the animated canvas and has no timer or graphics effect. Foundation,
// content, controls and transient surfaces have distinct, quiet material roles.
class ChromeMaterial final : public QObject {
public:
    explicit ChromeMaterial(QWidget *editor):QObject(editor),m_editor(editor),m_cache(16384){
        setObjectName("potatoChromeMaterial");
        setProperty("potatoMaterialPaintCount",QVariant::fromValue<qulonglong>(0));
        setProperty("potatoMaterialEnabled",enabled());
        qApp->installEventFilter(this);
        prepare(editor);
        for(auto *widget:editor->findChildren<QWidget*>())prepare(widget);
    }
    ~ChromeMaterial()override{if(qApp)qApp->removeEventFilter(this);}
    void syncTheme(){
        if(m_refreshing||!m_editor)return;
        m_refreshing=true;m_cache.clear();PotatoGlass::clearCaches();setProperty("potatoMaterialEnabled",enabled());
        prepare(m_editor);m_editor->update();
        for(auto *widget:m_editor->findChildren<QWidget*>()){
            if(excluded(widget))continue;
            prepare(widget);widget->update();
        }
        m_refreshing=false;
    }
protected:
    bool eventFilter(QObject *object,QEvent *event)override{
        if(object==qApp&&event->type()==QEvent::DynamicPropertyChange){
            const auto property=static_cast<QDynamicPropertyChangeEvent*>(event)->propertyName();
            if(property=="potatoInterfaceTheme"||property=="potatoTheme")syncTheme();
            else if(PotatoGlass::active()&&(property=="potatoReducedTransparency"||property=="potatoIncreasedContrast"||property=="potatoReducedMotion"))syncTheme();
            return false;
        }
        auto *widget=qobject_cast<QWidget*>(object);
        if(!widget||!owned(widget)||excluded(widget))return false;
        if(event->type()==QEvent::Polish||event->type()==QEvent::Show||event->type()==QEvent::StyleChange){prepare(widget);}
        if(!enabled()||qobject_cast<QMenu*>(widget))return false;
        if(PotatoGlass::active()&&event->type()==QEvent::Move){
            // Windows may move a top-level dialog by reusing its backing store.
            // Its editor-relative sample origin still changed: schedule each
            // owned chrome surface once, without capture or work in this event.
            auto repaintDialog=[&](QDialog *dialog){
                if(!dialog->isVisible())return;
                dialog->update();
                for(auto *child:dialog->findChildren<QWidget*>())if(!excluded(child))child->update();
            };
            if(widget==m_editor){
                for(auto *dialog:m_editor->findChildren<QDialog*>())repaintDialog(dialog);
            }else if(widget->isWindow()){
                if(auto *dialog=qobject_cast<QDialog*>(widget))repaintDialog(dialog);
            }
        }
        if(PotatoGlass::active()&&(event->type()==QEvent::HoverMove||event->type()==QEvent::WindowActivate||event->type()==QEvent::WindowDeactivate)){
            const auto surface=role(widget);
            if(surface==Surface::Button||surface==Surface::Heading||surface==Surface::Input)widget->update();
            if(event->type()!=QEvent::HoverMove){widget->update();for(auto *child:widget->findChildren<QWidget*>())if(!excluded(child))child->update();}
        }
        if(event->type()==QEvent::Paint){
            const auto surface=role(widget);
            if(surface!=Surface::None)paint(widget,static_cast<QPaintEvent*>(event),surface);
        }else if(event->type()==QEvent::Enter||event->type()==QEvent::Leave||
                 event->type()==QEvent::HoverEnter||event->type()==QEvent::HoverLeave||
                 event->type()==QEvent::FocusIn||event->type()==QEvent::FocusOut||
                 event->type()==QEvent::EnabledChange||event->type()==QEvent::MouseButtonPress||
                 event->type()==QEvent::MouseButtonRelease){
            for(auto *target=widget;target&&owned(target);target=target->parentWidget()){
                const auto surface=role(target);
                if(surface!=Surface::None){target->update();break;}
            }
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
        if(qobject_cast<QAbstractItemView*>(widget)){
            // The combo's elevated popup container already supplies its plate.
            if(widget->parentWidget()&&widget->parentWidget()->inherits("QComboBoxPrivateContainer"))return Surface::None;
            return Surface::Well;
        }
        return Surface::None;
    }
    void prepare(QWidget *widget){
        if(!owned(widget)||excluded(widget))return;
        if(auto *menu=qobject_cast<QMenu*>(widget)){installMenuMaterial(menu,m_editor);return;}
        if(widget->objectName()=="sectionHeading"&&widget->parentWidget()){
            widget->parentWidget()->setProperty("potatoMaterialSection",true);
            prepareBackground(widget->parentWidget(),false);
        }
        if(role(widget)!=Surface::None)prepareBackground(widget,true);
        if(auto *view=qobject_cast<QAbstractItemView*>(widget))prepareBackground(view->viewport(),false);
    }
    static void prepareBackground(QWidget *widget,bool hover){
        const bool prepared=widget->property("potatoMaterialPrepared").toBool();
        if(enabled()){
            if(!prepared){
                widget->setProperty("potatoMaterialAutoFill",widget->autoFillBackground());
                widget->setProperty("potatoMaterialHover",widget->testAttribute(Qt::WA_Hover));
                widget->setProperty("potatoMaterialPrepared",true);
            }
            widget->setAutoFillBackground(false);
            if(hover)widget->setAttribute(Qt::WA_Hover);
        }else if(prepared){
            widget->setAutoFillBackground(widget->property("potatoMaterialAutoFill").toBool());
            widget->setAttribute(Qt::WA_Hover,widget->property("potatoMaterialHover").toBool());
            widget->setProperty("potatoMaterialPrepared",false);
        }
    }
    static bool hasFocusWithin(QWidget *widget){
        auto *focus=QApplication::focusWidget();
        return widget->hasFocus()||(focus&&widget->isAncestorOf(focus));
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
            QColor base(35,39,46); // content layer sits above the quiet shell
            if(surface==Surface::Foundation)base=QColor(25,28,34);
            if(surface==Surface::Shell)base=QColor(29,33,40);
            if(surface==Surface::Section)base=QColor(36,40,47);
            if(surface==Surface::Heading)base=QColor(40,44,52);
            if(surface==Surface::Button)base=QColor(47,51,59);
            if(surface==Surface::Input||surface==Surface::Well)base=QColor(27,30,36);
            if(surface==Surface::Dialog)base=QColor(43,47,55);
            const auto theme=qApp->property("potatoTheme").toString();
            if(theme=="halloween")base=QColor(base.red()+3,base.green()-1,base.blue()+3);
            else if(theme=="autumn")base=QColor(base.red()+4,base.green()+1,base.blue()-3);
            else if(theme=="christmas")base=QColor(base.red()-1,base.green()+3,base.blue());
            if(state&1)base=base.lighter(112); // hover belongs to controls only
            if(state&2)base=base.darker(110);  // pressing lowers local elevation
            if(state&4){ // checked/default action: readable, restrained accent wash
                const QColor color=accent();base=QColor((base.red()*5+color.red())/6,(base.green()*5+color.green())/6,(base.blue()*5+color.blue())/6);
            }
            if(state&8)base=QColor(34,37,43); // disabled has no bright gloss
            painter.fillPath(path,base);
            // Broad static light belongs to the foundation, rather than making
            // each button and panel look like an independent glass tile.
            if(surface==Surface::Foundation||surface==Surface::Shell){
                QRadialGradient light(logical.width()*.15,-logical.height()*.5,qMax(220,logical.width()));
                light.setColorAt(0,QColor(154,174,196,8));light.setColorAt(1,Qt::transparent);
                painter.fillPath(path,light);
            }
            QLinearGradient sheen(0,0,0,qMax(1,logical.height()));
            const bool inset=surface==Surface::Input||surface==Surface::Well;
            const bool control=surface==Surface::Button||surface==Surface::Heading;
            sheen.setColorAt(0,QColor(255,255,255,(state&8)?2:(inset?2:(control?9:4))));
            sheen.setColorAt(1,QColor(0,0,0,(state&2)?10:(inset?5:(control?8:3))));painter.fillPath(path,sheen);
        }
        QPixmap result=QPixmap::fromImage(image);
        m_cache.insert(key,new QPixmap(result),int(qMax<qsizetype>(1,(image.sizeInBytes()+1023)/1024)));
        return result;
    }
    void paint(QWidget *widget,QPaintEvent *event,Surface surface){
        if(!enabled()||widget->width()<2||widget->height()<2)return;
        if(PotatoGlass::active()){paintGlass(widget,event,surface);return;}
        if(surface==Surface::FocusOnly){
            if(hasFocusWithin(widget)&&widget->isEnabled()){
                setProperty("potatoMaterialPaintCount",QVariant::fromValue(++m_paintCount));
                QPainter painter(widget);painter.setClipRegion(event->region());painter.setRenderHint(QPainter::Antialiasing);
                painter.setBrush(Qt::NoBrush);painter.setPen(QPen(accent().lighter(120),2));
                painter.drawRoundedRect(QRectF(widget->rect()).adjusted(1.5,1.5,-1.5,-1.5),3,3);
            }
            return;
        }
        setProperty("potatoMaterialPaintCount",QVariant::fromValue(++m_paintCount));
        int state=0;
        const bool interactive=surface==Surface::Button||surface==Surface::Heading||surface==Surface::Input;
        if(interactive&&widget->underMouse()&&widget->isEnabled())state|=1;
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
            painter.setPen(QColor(0,0,0,68));painter.drawLine(QPointF(0,widget->height()-.5),QPointF(widget->width(),widget->height()-.5));return;
        }
        QLinearGradient rim(0,0,0,widget->height());
        rim.setColorAt(0,QColor(229,236,247,(state&8)?14:((state&1)?62:(interactive?42:22))));
        rim.setColorAt(1,QColor(155,174,196,(state&8)?10:(interactive?24:14)));
        painter.setBrush(Qt::NoBrush);painter.setPen(QPen(QBrush(rim),1));painter.drawRoundedRect(rect,radius(surface),radius(surface));
        if(surface==Surface::Input){
            // Fluent fields use a quiet lower edge; focus gives it an explicit
            // accent while the separate ring remains a clear keyboard cue.
            painter.setPen(QPen(hasFocusWithin(widget)&&widget->isEnabled()?accent():QColor(180,190,204,(state&8)?30:85),
                                hasFocusWithin(widget)&&widget->isEnabled()?2:1));
            painter.drawLine(QPointF(5,widget->height()-1.5),QPointF(widget->width()-5,widget->height()-1.5));
        }
        if(hasFocusWithin(widget)&&widget->isEnabled()&&(interactive||surface==Surface::Well)){
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
    void paintGlass(QWidget *widget,QPaintEvent *event,Surface surface){
        using Role=PotatoGlass::Role;
        const bool focus=hasFocusWithin(widget)&&widget->isEnabled();
        if(surface==Surface::FocusOnly){
            if(focus){
                setProperty("potatoMaterialPaintCount",QVariant::fromValue(++m_paintCount));
                QPainter p(widget);p.setClipRegion(event->region());p.setRenderHint(QPainter::Antialiasing);
                p.setBrush(Qt::NoBrush);p.setPen(QPen(PotatoGlass::accent(),2));
                p.drawRoundedRect(QRectF(widget->rect()).adjusted(1.5,1.5,-1.5,-1.5),3,3);
            }
            return;
        }
        Role role=Role::Panel;
        switch(surface){
        case Surface::Foundation:role=Role::Foundation;break;
        case Surface::Shell:role=Role::Shell;break;
        case Surface::Section:role=Role::Section;break;
        case Surface::Heading:role=Role::Heading;break;
        case Surface::Button:role=Role::Button;break;
        case Surface::Input:role=Role::Input;break;
        case Surface::Well:role=Role::Well;break;
        case Surface::Dialog:role=Role::Dialog;break;
        default:break;
        }
        setProperty("potatoMaterialPaintCount",QVariant::fromValue(++m_paintCount));
        const bool interactive=surface==Surface::Button||surface==Surface::Heading||surface==Surface::Input;
        int state=interactive&&widget->underMouse()&&widget->isEnabled()?1:0;
        if(auto *button=qobject_cast<QAbstractButton*>(widget)){
            if(button->isDown())state|=2;
            if(button->isChecked()&&surface!=Surface::Heading)state|=4;
        }
        if(widget->objectName()=="outputStart")state|=4;
        if(!widget->isEnabled())state|=8;
        if(!widget->window()->isActiveWindow())state|=16;
        QPainter p(widget);p.setClipRegion(event->region());p.setRenderHint(QPainter::Antialiasing);
        const QRectF rect(widget->rect());
        const QPointF origin=m_editor->mapFromGlobal(widget->mapToGlobal(QPoint(0,0)));
        PotatoGlass::paintSurface(p,rect,role,origin,m_editor->size(),state);
        // Pointer light changes only this control's vector reflection. It never
        // invalidates the source/refraction cache and has no animation timer.
        if((state&1)&&!(state&8)&&!PotatoGlass::reducedMotion()&&!PotatoGlass::reducedTransparency()&&!PotatoGlass::increasedContrast()){
            const QPoint pointer=widget->mapFromGlobal(QCursor::pos());
            const qreal r=PotatoGlass::radius(role);QPainterPath shape;shape.addRoundedRect(rect.adjusted(1,1,-1,-1),qMax(1.0,r-1),qMax(1.0,r-1));
            p.save();p.setClipPath(shape,Qt::IntersectClip);
            const QPointF light(qBound(0,pointer.x(),widget->width()),qMin(widget->height()*.28,qreal(pointer.y())));
            QRadialGradient sheen(light,qMin<qreal>(110,qMax<qreal>(28,widget->width()*.55)));
            sheen.setColorAt(0,QColor(235,250,227,(state&2)?10:24));sheen.setColorAt(.45,QColor(193,235,210,7));sheen.setColorAt(1,Qt::transparent);
            p.fillPath(shape,sheen);p.restore();
        }
        if(focus&&(interactive||surface==Surface::Well)){
            p.setBrush(Qt::NoBrush);p.setPen(QPen(QColor(8,19,13),3));
            const QRectF ring=rect.adjusted(1.5,1.5,-1.5,-1.5);const qreal r=qMax(2.0,PotatoGlass::radius(role)-1);
            p.drawRoundedRect(ring,r,r);p.setPen(QPen(PotatoGlass::accent().lighter(110),1.7));p.drawRoundedRect(ring,r,r);
        }
        auto arrow=[&](const QRect &area,bool up,bool enabled){
            if(area.width()<4||area.height()<3)return;
            const QPointF c=QRectF(area).center();const qreal direction=up?-1:1;
            QPainterPath chevron;chevron.moveTo(c.x()-3,c.y()-1.5*direction);chevron.lineTo(c.x(),c.y()+1.5*direction);chevron.lineTo(c.x()+3,c.y()-1.5*direction);
            p.setBrush(Qt::NoBrush);p.setPen(QPen(enabled?QColor(232,238,232):QColor(110,124,115),1.5,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));p.drawPath(chevron);
        };
        if(auto *combo=qobject_cast<QComboBox*>(widget)){
            QStyleOptionComboBox option;option.initFrom(combo);option.editable=combo->isEditable();option.frame=combo->hasFrame();
            arrow(combo->style()->subControlRect(QStyle::CC_ComboBox,&option,QStyle::SC_ComboBoxArrow,combo),false,combo->isEnabled());
        }else if(auto *spin=qobject_cast<QAbstractSpinBox*>(widget)){
            if(spin->buttonSymbols()!=QAbstractSpinBox::NoButtons){
                QStyleOptionSpinBox option;option.initFrom(spin);option.frame=spin->hasFrame();option.buttonSymbols=spin->buttonSymbols();
                bool up=spin->isEnabled()&&!spin->isReadOnly(),down=up;
                if(auto *integer=qobject_cast<QSpinBox*>(spin);integer&&!integer->wrapping()){
                    up=up&&integer->value()<integer->maximum();down=down&&integer->value()>integer->minimum();
                }else if(auto *decimal=qobject_cast<QDoubleSpinBox*>(spin);decimal&&!decimal->wrapping()){
                    up=up&&decimal->value()<decimal->maximum();down=down&&decimal->value()>decimal->minimum();
                }
                arrow(spin->style()->subControlRect(QStyle::CC_SpinBox,&option,QStyle::SC_SpinBoxUp,spin),true,up);
                arrow(spin->style()->subControlRect(QStyle::CC_SpinBox,&option,QStyle::SC_SpinBoxDown,spin),false,down);
            }
        }
    }
    QPointer<QWidget> m_editor;QCache<QString,QPixmap> m_cache;
    bool m_refreshing=false;qulonglong m_paintCount=0;
};
}

inline void installPotatoMaterial(QWidget *mainWindow){
    if(!mainWindow)return;
    for(auto *child:mainWindow->children())if(child->objectName()=="potatoChromeMaterial"){
        static_cast<PotatoMaterial::ChromeMaterial*>(child)->syncTheme();return;
    }
    mainWindow->setProperty("potatoMaterialInstalled",true);
    new PotatoMaterial::ChromeMaterial(mainWindow);
}
