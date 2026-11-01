#pragma once
#include <QString>
#include "appearance.h"
#include "classic-style.h"

// Keep the familiar compact widget geometry. material.h paints the surfaces;
// transparent bases let Qt paint real text, icons, values and native interaction
// states without concealing the app-rendered layers, light and rounded rims.
inline QString potatoStyle(const QString &theme=QStringLiteral("classic1"),const QString &interfaceTheme=QStringLiteral("material")){
    if(interfaceTheme==QStringLiteral("classic1"))return potatoClassicStyle();
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
QMenu::item:selected { background:rgba(160,174,192,38); color:#FFFFFF; }
QMenu::item:disabled { color:#929BA6; }
QMenu::separator { height:1px; background:rgba(224,230,240,24); margin:5px 9px; }
QToolButton,QPushButton { background:transparent; color:#E8EDF3; border:1px solid transparent; border-radius:5px; padding:7px; }
QToolButton:hover,QPushButton:hover { background:transparent; color:#FFFFFF; }
QToolButton:pressed,QPushButton:pressed { background:transparent; color:#D4DDE8; }
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
QListWidget::item:selected,QAbstractItemView::item:selected { background:rgba(163,151,126,42); color:#FFFFFF; border:1px solid rgba(255,225,165,24); border-left:3px solid @accent@; }
QListWidget::item:selected:!active,QAbstractItemView::item:selected:!active { background:rgba(151,161,174,26); color:#DCE3EC; border-left-color:#8B97A6; }
QListWidget::item:hover:!selected,QAbstractItemView::item:hover:!selected { background:rgba(147,171,198,24); }
QListWidget::item:disabled,QAbstractItemView::item:disabled { color:#7D8996; }
QListWidget#animationLibrary::item { padding:4px; }
QComboBox,QSpinBox,QDoubleSpinBox,QLineEdit,QTextEdit,QPlainTextEdit { background:transparent; color:#E5EBF2; border:1px solid transparent; border-radius:4px; padding:5px; selection-background-color:#655333; selection-color:#FFFFFF; }
QComboBox:disabled,QSpinBox:disabled,QDoubleSpinBox:disabled,QLineEdit:disabled,QTextEdit:disabled,QPlainTextEdit:disabled { color:#7D8996; }
QLineEdit:read-only,QTextEdit:read-only,QPlainTextEdit:read-only { color:#C3CDD9; }
QComboBox::drop-down { width:21px; border:0; background:transparent; }
QComboBox::down-arrow { image:none; width:0; height:0; }
QComboBox QAbstractItemView { background:transparent; color:#E8EDF3; selection-background-color:#504C43; selection-color:#FFFFFF; border:1px solid transparent; outline:0; padding:3px; }
QComboBox QAbstractItemView::item { padding:6px 9px; min-height:18px; }
QSpinBox::up-button,QDoubleSpinBox::up-button,QSpinBox::down-button,QDoubleSpinBox::down-button { width:17px; border:0; background:transparent; }
QSpinBox::up-button:hover,QDoubleSpinBox::up-button:hover,QSpinBox::down-button:hover,QDoubleSpinBox::down-button:hover { background:rgba(205,218,235,18); border-radius:3px; }
QSpinBox::up-button:pressed,QDoubleSpinBox::up-button:pressed,QSpinBox::down-button:pressed,QDoubleSpinBox::down-button:pressed { background:rgba(0,0,0,38); }
QSpinBox::up-button:disabled,QDoubleSpinBox::up-button:disabled,QSpinBox::down-button:disabled,QDoubleSpinBox::down-button:disabled { background:transparent; }
QSpinBox::up-arrow,QDoubleSpinBox::up-arrow,QSpinBox::down-arrow,QDoubleSpinBox::down-arrow { image:none; width:0; height:0; }
QSlider { background:transparent; }
QSlider::groove:horizontal { height:5px; background:#4A535F; border:1px solid #333B46; border-radius:3px; }
QSlider::sub-page:horizontal { background:@accent@; border-radius:2px; }
QSlider::handle:horizontal { background:#E0E7EF; width:14px; height:14px; margin:-5px 0; border:1px solid #F0F4F9; border-radius:7px; }
QSlider::handle:horizontal:hover { background:#FFFFFF; border-color:@focus@; }
QSlider::handle:horizontal:pressed { background:@focus@; border-color:@accent@; }
QSlider::handle:horizontal:disabled { background:#697786; border-color:#83909E; }
QSlider::sub-page:horizontal:disabled { background:#65717E; }
QCheckBox { background:transparent; spacing:5px; color:#DBE3ED; }
QCheckBox:disabled,QRadioButton:disabled { color:#7D8996; }
QCheckBox::indicator { width:14px; height:14px; border:1px solid #7D8A99; border-radius:3px; background:#1F252D; }
QCheckBox::indicator:checked { background:@accent@; border-color:@focus@; image:url(:/assets/icons/check.svg); }
QCheckBox::indicator:hover { border-color:@focus@; }
QCheckBox::indicator:pressed { background:#39414C; border-color:@accent@; }
QCheckBox::indicator:checked:pressed { background:@focus@; }
QCheckBox::indicator:disabled { border-color:#4B5664; background:#242C35; }
QRadioButton { background:transparent; spacing:6px; }
QRadioButton::indicator { width:14px; height:14px; border:1px solid #7D8A99; border-radius:7px; background:#1B222B; }
QRadioButton::indicator:checked { border:4px solid @accent@; background:#1B222B; }
QRadioButton::indicator:hover { border-color:@focus@; }
QRadioButton::indicator:pressed { background:#39414C; }
QRadioButton::indicator:disabled { border-color:#4B5664; background:#242C35; }
QScrollArea { background:transparent; border:0; }
QScrollArea > QWidget > QWidget { background:transparent; }
QScrollBar:vertical { background:rgba(8,13,19,28); width:12px; margin:2px; border:0; }
QScrollBar::handle:vertical { background:#657384; min-height:30px; border:1px solid rgba(180,201,224,18); border-radius:4px; }
QScrollBar::handle:vertical:hover { background:#8698AB; }
QScrollBar::handle:vertical:pressed { background:@accent@; }
QScrollBar::add-line:vertical,QScrollBar::sub-line:vertical { height:0; }
QScrollBar::add-page:vertical,QScrollBar::sub-page:vertical { background:none; }
QScrollBar:horizontal { background:rgba(8,13,19,28); height:12px; margin:2px; border:0; }
QScrollBar::handle:horizontal { background:#657384; min-width:30px; border:1px solid rgba(180,201,224,18); border-radius:4px; }
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
QSplitter::handle { background:#303944; }
QSplitter::handle:hover { background:#78684C; }
QFrame[frameShape="4"],QFrame[frameShape="5"] { color:rgba(208,225,244,25); }
QToolTip { background:#303946; color:#F0F4FA; border:1px solid #718095; border-radius:4px; padding:7px; }
QProgressBar { border:1px solid #596778; border-radius:4px; background:#1B242E; text-align:center; }
QProgressBar::chunk { background:@accent@; border-radius:3px; }
)QSS");
    style.replace(QStringLiteral("@accent@"),accent);style.replace(QStringLiteral("@focus@"),focus);
    return style;
}
