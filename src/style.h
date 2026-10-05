#pragma once
#include <QString>

inline QString potatoStyle(){return QStringLiteral(R"QSS(
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
QComboBox,QSpinBox{background:#222d37;border:1px solid #3c4e5b;border-radius:4px;padding:5px;}
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
)QSS");}
