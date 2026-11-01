#pragma once
#include <QString>
#include "appearance.h"

// Keep the familiar compact widget geometry. material.h paints the surfaces;
// transparent bases let Qt paint real text, icons, values and native interaction
// states without concealing the app-rendered grain, sheen and rounded rims.
inline QString potatoStyle(const QString &theme=QStringLiteral("classic1")){
    QString accent=QStringLiteral("#F3B34C"),focus=QStringLiteral("#FFD185");
    const auto id=PotatoAppearance::themeId(theme);
    if(id=="halloween"){accent="#EAA15D";focus="#FFD0A1";}
    else if(id=="autumn"){accent="#DEB878";focus="#F1D7AF";}
    else if(id=="christmas"){accent="#D6BE85";focus="#EEE0BA";}
    QString style=QStringLiteral(R"QSS(
QWidget { background:transparent; color:#E8EDF3; font-family:'Segoe UI Variable','Segoe UI'; font-size:12px; }
QLabel { background:transparent; border:0; }
QLabel:disabled { color:#818B97; }
QMainWindow,QDialog { background:transparent; }
QToolBar { background:transparent; spacing:7px; padding:9px; border:0; }
QToolBar::separator { width:1px; background:rgba(219,230,245,25); margin:5px 7px; }
QToolBar#projectToolbar { spacing:5px; padding:8px; }
QToolBar#projectToolbar::separator { margin:5px; }
QToolBar#projectToolbar QToolButton { padding:7px 6px; }
QMenuBar { background:transparent; color:#CDD5DF; border:0; padding:2px 0; }
QMenuBar::item { background:transparent; padding:4px 9px; border-radius:4px; }
QMenuBar::item:selected,QMenuBar::item:pressed { background:rgba(154,172,194,32); color:#FFFFFF; }
QMenu { background:transparent; color:#F2F4F7; border:0; padding:9px; font-size:13px; }
QMenu::item { background:transparent; padding:9px 28px 9px 17px; border-radius:4px; }
QMenu::item:selected { background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 rgba(133,145,160,65),stop:1 rgba(104,115,130,48)); color:#FFFFFF; }
QMenu::item:disabled { color:#929BA6; }
QMenu::separator { height:1px; background:rgba(224,230,240,24); margin:5px 9px; }
QToolButton,QPushButton { background:transparent; color:#E8EDF3; border:1px solid transparent; border-radius:5px; padding:7px; }
QToolButton:hover,QPushButton:hover { background:transparent; color:#FFFFFF; }
QToolButton:pressed,QPushButton:pressed { background:transparent; color:#FFFFFF; }
QToolButton:disabled,QPushButton:disabled { background:transparent; color:#7D8996; border-color:transparent; }
QToolButton:focus,QPushButton:focus { background:transparent; border-color:transparent; }
QToolButton:checked,QPushButton:checked { background:transparent; color:#FFF4DE; }
QPushButton[compactSurface="true"] { padding:5px 3px; font-size:12px; }
QPushButton#outputStart,QToolButton#outputStart { font-weight:600; color:#FFF1D9; }
QPushButton#blackoutButton:checked { color:#FFE1CC; }
QPushButton#outputStart:disabled,QToolButton#outputStart:disabled,QPushButton#blackoutButton:disabled { color:#7D8996; }
QToolButton#sectionHeading { text-align:left; background:transparent; border:0; border-radius:5px; font-weight:600; color:#DDE5EE; padding:8px; }
QToolButton#sectionHeading:hover { background:transparent; color:#FFFFFF; }
QToolButton#sectionHeading:checked { background:transparent; color:#E8EDF3; }
QToolButton#sectionHeading:disabled { color:#7D8996; }
QLabel#brandName { font-size:17px; font-weight:700; color:#F4DCAD; }
QLabel#projectName,QLabel#projectTitle { color:#CAD3DF; }
QLabel#surfaceHeading,QLabel#inspectorHeading,QLabel#libraryHeading { color:#C8D3E0; font-size:12px; font-weight:600; }
QLabel#taskSurface { font-size:17px; font-weight:600; color:#F2F4F7; }
QLabel#canvasHint,QLabel#scopeLabel,QLabel#browserStatus,QLabel#outputStatus,QLabel#entranceInfo { color:#AFBBC8; font-size:12px; }
QLabel#connectionDescription { background:transparent; border:1px solid transparent; border-radius:6px; padding:9px; color:#C5CED9; }
QWidget#surfaceSidebar,QWidget#inspectorPanel,QWidget#outputToolbar,QWidget#projectHeader { background:transparent; border:0; }
QListWidget,QListView,QTreeView,QTableView { background:transparent; alternate-background-color:rgba(190,207,228,5); color:#E8EDF3; border:1px solid transparent; border-radius:6px; outline:0; }
QAbstractItemView::item { color:#E8EDF3; }
QListWidget::item { padding:8px; border:1px solid transparent; border-radius:3px; }
QListWidget::item:selected,QAbstractItemView::item:selected { background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 rgba(133,119,89,155),stop:1 rgba(90,83,68,145)); color:#FFFFFF; border:1px solid rgba(255,225,165,35); border-left:3px solid @accent@; }
QListWidget::item:hover:!selected,QAbstractItemView::item:hover:!selected { background:rgba(147,171,198,24); }
QListWidget::item:disabled,QAbstractItemView::item:disabled { color:#7D8996; }
QListWidget#animationLibrary::item { padding:4px; }
QComboBox,QSpinBox,QDoubleSpinBox,QLineEdit,QTextEdit,QPlainTextEdit { background:transparent; color:#E5EBF2; border:1px solid transparent; border-radius:4px; padding:5px; selection-background-color:#655333; selection-color:#FFFFFF; }
QComboBox:disabled,QSpinBox:disabled,QDoubleSpinBox:disabled,QLineEdit:disabled { color:#7D8996; }
QComboBox::drop-down { width:21px; border:0; background:transparent; }
QComboBox::down-arrow { image:none; width:0; height:0; }
QComboBox QAbstractItemView { background:transparent; color:#E8EDF3; selection-background-color:#51483A; selection-color:#FFFFFF; border:1px solid transparent; outline:0; padding:3px; }
QComboBox QAbstractItemView::item { padding:6px 9px; min-height:18px; }
QSpinBox::up-button,QDoubleSpinBox::up-button,QSpinBox::down-button,QDoubleSpinBox::down-button { width:17px; border:0; background:transparent; }
QSpinBox::up-arrow,QDoubleSpinBox::up-arrow,QSpinBox::down-arrow,QDoubleSpinBox::down-arrow { image:none; width:0; height:0; }
QSlider { background:transparent; }
QSlider::groove:horizontal { height:5px; background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #26313C,stop:1 #4C5865); border:1px solid #26303A; border-radius:3px; }
QSlider::sub-page:horizontal { background:@accent@; border-radius:2px; }
QSlider::handle:horizontal { background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #FBFCFE,stop:1 #CBD5E0); width:14px; height:14px; margin:-5px 0; border:1px solid #F0F4F9; border-radius:7px; }
QSlider::handle:horizontal:hover { background:#FFFFFF; border-color:@focus@; }
QSlider::handle:horizontal:pressed { background:@focus@; border-color:@accent@; }
QSlider::handle:horizontal:disabled { background:#697786; border-color:#83909E; }
QSlider::sub-page:horizontal:disabled { background:#65717E; }
QCheckBox { background:transparent; spacing:5px; color:#DBE3ED; }
QCheckBox:disabled,QRadioButton:disabled { color:#7D8996; }
QCheckBox::indicator { width:14px; height:14px; border:1px solid #7D8A99; border-radius:3px; background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #252F3A,stop:1 #1B222B); }
QCheckBox::indicator:checked { background:@accent@; border-color:@focus@; image:url(:/assets/icons/check.svg); }
QCheckBox::indicator:hover { border-color:@focus@; }
QCheckBox::indicator:disabled { border-color:#4B5664; background:#242C35; }
QRadioButton { background:transparent; spacing:6px; }
QRadioButton::indicator { width:14px; height:14px; border:1px solid #7D8A99; border-radius:7px; background:#1B222B; }
QRadioButton::indicator:checked { border:4px solid @accent@; background:#1B222B; }
QScrollArea { background:transparent; border:0; }
QScrollArea > QWidget > QWidget { background:transparent; }
QScrollBar:vertical { background:rgba(8,13,19,28); width:12px; margin:2px; border:0; }
QScrollBar::handle:vertical { background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #627180,stop:1 #4A5867); min-height:30px; border:1px solid rgba(180,201,224,18); border-radius:4px; }
QScrollBar::handle:vertical:hover { background:#8698AB; }
QScrollBar::handle:vertical:pressed { background:@accent@; }
QScrollBar::add-line:vertical,QScrollBar::sub-line:vertical { height:0; }
QScrollBar::add-page:vertical,QScrollBar::sub-page:vertical { background:none; }
QScrollBar:horizontal { background:rgba(8,13,19,28); height:12px; margin:2px; border:0; }
QScrollBar::handle:horizontal { background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #627180,stop:1 #4A5867); min-width:30px; border:1px solid rgba(180,201,224,18); border-radius:4px; }
QScrollBar::handle:horizontal:hover { background:#8698AB; }
QScrollBar::handle:horizontal:pressed { background:@accent@; }
QScrollBar::add-line:horizontal,QScrollBar::sub-line:horizontal { width:0; }
QScrollBar::add-page:horizontal,QScrollBar::sub-page:horizontal { background:none; }
QTabWidget::pane { background:transparent; border:0; }
QTabBar { background:transparent; }
QTabBar::tab { background:rgba(190,211,235,6); color:#AEBBCB; border:0; border-bottom:2px solid transparent; padding:8px 10px; }
QTabBar::tab:selected { background:rgba(190,211,235,17); color:#F2F4F7; border-bottom-color:@accent@; }
QTabBar::tab:hover:!selected { background:rgba(190,211,235,12); color:#E8EDF3; }
QStatusBar { background:transparent; color:#AEBCCB; border:0; font-size:12px; }
QStatusBar::item { border:0; }
QSplitter::handle { background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #1C242E,stop:0.5 #35404D,stop:1 #1C242E); }
QSplitter::handle:hover { background:#78684C; }
QFrame[frameShape="4"],QFrame[frameShape="5"] { color:rgba(208,225,244,25); }
QToolTip { background:#303946; color:#F0F4FA; border:1px solid #718095; border-radius:4px; padding:7px; }
QProgressBar { border:1px solid #596778; border-radius:4px; background:#1B242E; text-align:center; }
QProgressBar::chunk { background:@accent@; border-radius:3px; }
)QSS");
    style.replace(QStringLiteral("@accent@"),accent);style.replace(QStringLiteral("@focus@"),focus);
    return style;
}
