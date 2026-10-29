#pragma once
#include <QString>
#include "appearance.h"

inline QString potatoStyle(const QString &theme="classic1"){
    QString style=QStringLiteral(R"QSS(
QWidget{background:#171c24;color:#e7edf2;font-family:'Segoe UI';font-size:12px;}
QToolBar{spacing:7px;padding:9px;background:#202833;border:0;}
QToolButton,QPushButton{background:#2b3743;border:1px solid #3c4e5b;border-radius:5px;padding:7px;}
QToolButton:hover,QPushButton:hover{background:#354957;border-color:#78cdb8;}
QToolButton:pressed,QPushButton:pressed{background:#456456;}
QPushButton[compactSurface="true"]{padding:5px 3px;font-size:11px;}
QPushButton:disabled,QToolButton:disabled{background:#202833;color:#6e7d89;border-color:#2c3741;}
QToolButton#sectionHeading{text-align:left;background:#202833;border:0;border-radius:5px;font-weight:600;color:#c8d8e0;padding:8px;}
QToolButton#sectionHeading:hover{background:#2b3945;color:#bbffeb;}
QMenu::item:selected{background:#284b45;}
QMenu::item:disabled{color:#6e7d89;}
QListWidget{background:#11161d;border:1px solid #34424e;border-radius:6px;}
QListWidget::item{padding:8px;}
QListWidget::item:selected{background:#284b45;color:#bbffeb;}
QListWidget::item:hover{background:#202f37;}
QComboBox,QSpinBox,QLineEdit{background:#222d37;border:1px solid #3c4e5b;border-radius:4px;padding:5px;}
QLineEdit:focus,QComboBox:focus,QSpinBox:focus,QPushButton:focus,QToolButton:focus{border:1px solid #78cdb8;}
QComboBox:disabled,QSpinBox:disabled{color:#6e7d89;border-color:#2c3741;}
QSlider::groove:horizontal{height:5px;background:#35434d;border-radius:2px;}
QSlider::handle:horizontal{background:#78efce;width:14px;margin:-5px 0;border-radius:7px;}
QSlider::handle:horizontal:hover{background:#b4ffe9;}
QSlider::handle:horizontal:disabled{background:#60717b;}
QScrollArea{border:0;}
QScrollBar:vertical{background:#171c24;width:12px;margin:2px;}
QScrollBar::handle:vertical{background:#526371;min-height:30px;border-radius:4px;}
QScrollBar::handle:vertical:hover{background:#78cdb8;}
QScrollBar::handle:vertical:pressed{background:#a6efda;}
QScrollBar::add-line:vertical,QScrollBar::sub-line:vertical{height:0px;}
QScrollBar::add-page:vertical,QScrollBar::sub-page:vertical{background:none;}
QScrollBar:horizontal{background:#171c24;height:12px;margin:2px;}
QScrollBar::handle:horizontal{background:#526371;min-width:30px;border-radius:4px;}
QScrollBar::handle:horizontal:hover{background:#78cdb8;}
QScrollBar::handle:horizontal:pressed{background:#a6efda;}
QScrollBar::add-line:horizontal,QScrollBar::sub-line:horizontal{width:0px;}
QScrollBar::add-page:horizontal,QScrollBar::sub-page:horizontal{background:none;}
QStatusBar{background:#10161d;color:#a9bbc7;}
QSplitter::handle{background:#293542;}
QSplitter::handle:hover{background:#78cdb8;}
QCheckBox{spacing:5px;}
QTabWidget::pane{border:0;}
QTabBar::tab{background:#202833;border:1px solid #34424e;padding:8px 12px;min-width:44px;}
QTabBar::tab:selected{background:#284b45;color:#bbffeb;border-bottom:2px solid #78cdb8;}
QTabBar::tab:hover{background:#354957;}
QPushButton:checked,QToolButton:checked{background:#284b45;border-color:#78cdb8;color:#bbffeb;}
QPushButton#outputStart{background:#286250;color:#eafff7;font-weight:600;}
QPushButton#blackoutButton:checked{background:#4b2930;color:#ffe5c0;border:2px solid #f1b474;}
QLabel#brandName{font-size:16px;font-weight:700;color:#f6d396;padding-right:12px;}
QLabel#surfaceHeading,QLabel#taskSurface{color:#a9bbc7;font-weight:600;}
QWidget#canvasTools{background:#202833;}
QWidget#canvasTools QPushButton{padding:5px 10px;}
QLabel#canvasHint{color:#a9bbc7;font-size:11px;}
QLabel#scopeLabel,QLabel#browserStatus{color:#a9bbc7;font-size:11px;}
QLabel#connectionDescription{background:#202833;border-radius:5px;padding:8px;}
QToolBar#projectToolbar{padding:5px;spacing:5px;}
QToolBar#outputToolbar{padding:3px 8px;}
)QSS");
    // Only widget chrome uses these tokens. Canvas/projector paint is unchanged.
    const auto id=PotatoAppearance::themeId(theme);
    if(id=="halloween"){
        style.replace("#171c24","#1d1925").replace("#202833","#2b2235").replace("#11161d","#15111c");
        style.replace("#78cdb8","#e7a760").replace("#78efce","#f3b565").replace("#284b45","#55402e").replace("#bbffeb","#ffdfb1");
    }else if(id=="autumn"){
        style.replace("#171c24","#211e1b").replace("#202833","#302a24").replace("#11161d","#181512");
        style.replace("#78cdb8","#d9ab74").replace("#78efce","#e5b878").replace("#284b45","#57412e").replace("#bbffeb","#ffe4bd");
    }else if(id=="christmas"){
        style.replace("#171c24","#172321").replace("#202833","#20332f").replace("#11161d","#111b19");
        style.replace("#78cdb8","#dcbf7c").replace("#78efce","#e5cd93").replace("#284b45","#385349").replace("#bbffeb","#fff0c9");
    }
    return style;
}
