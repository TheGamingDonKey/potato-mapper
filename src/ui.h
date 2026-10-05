#pragma once
#include <QLabel>
#include <QResizeEvent>
#include <QToolButton>
#include <QVBoxLayout>

// Keep full filenames available without making the inspector wider.
class ElidedLabel : public QLabel {
public:
    explicit ElidedLabel(QWidget *parent=nullptr):QLabel(parent){
        setMinimumWidth(0);setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Preferred);
    }
    void setFullText(const QString &text,const QString &detail={}){
        fullText=text;setToolTip(detail.isEmpty()?text:detail);elide();
    }
protected:
    void resizeEvent(QResizeEvent *event)override{QLabel::resizeEvent(event);elide();}
private:
    QString fullText;
    void elide(){setText(fontMetrics().elidedText(fullText,Qt::ElideMiddle,qMax(0,contentsRect().width())));}
};

class InspectorSection : public QWidget {
public:
    QToolButton *heading;
    QWidget *body;
    QVBoxLayout *content;
    explicit InspectorSection(const QString &title,QWidget *parent=nullptr):QWidget(parent){
        auto *layout=new QVBoxLayout(this);layout->setContentsMargins(0,0,0,0);layout->setSpacing(8);
        heading=new QToolButton;heading->setObjectName("sectionHeading");heading->setText(title);
        heading->setCheckable(true);heading->setChecked(true);heading->setArrowType(Qt::DownArrow);
        heading->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);heading->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
        heading->setToolTip("Expand or collapse "+title.toLower());layout->addWidget(heading);
        body=new QWidget;content=new QVBoxLayout(body);content->setContentsMargins(2,0,2,4);content->setSpacing(8);layout->addWidget(body);
        connect(heading,&QToolButton::toggled,this,[this](bool open){body->setVisible(open);heading->setArrowType(open?Qt::DownArrow:Qt::RightArrow);});
    }
};
