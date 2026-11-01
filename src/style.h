#pragma once
#include <QApplication>
#include <QString>
#include <QVariant>
#include "appearance.h"
#include "classic-style.h"

// Geometry matches the existing Material sheet. The shared glass painter owns
// plate depth/refraction; Qt retains foregrounds, focus and all input semantics.
inline QString potatoLiquidGlassStyle(){
    QString style=QStringLiteral(R"QSS(
QWidget { background:transparent; color:#F2F3EF; font-family:'Segoe UI Variable','Segoe UI'; font-size:12px; }
QLabel { background:transparent; border:0; }
QLabel:disabled { color:#7F8A83; }
QMainWindow,QDialog { background:transparent; }
QToolBar { background:transparent; spacing:7px; padding:9px; border:0; }
QToolBar::separator { width:1px; background:rgba(232,238,232,44); margin:5px 7px; }
QToolBar#projectToolbar { spacing:5px; padding:8px; }
QToolBar#projectToolbar::separator { margin:5px; }
QToolBar#projectToolbar QToolButton { padding:7px 6px; }
QMenuBar { background:transparent; color:#D1DBD4; border:0; padding:2px 0; }
QMenuBar::item { background:transparent; padding:4px 9px; border-radius:4px; }
QMenuBar::item:selected,QMenuBar::item:pressed { background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 rgba(232,238,232,49),stop:.35 rgba(143,208,178,26),stop:1 rgba(143,208,178,15)); color:#FFFFFF; }
QMenu { background:transparent; color:#F2F3EF; border:0; padding:9px; font-size:13px; }
QMenu::item { background:transparent; padding:9px 28px 9px 17px; border-radius:4px; }
QMenu::item:selected { background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 rgba(195,231,209,62),stop:.22 rgba(143,208,178,42),stop:1 rgba(143,208,178,23)); color:#FFFFFF; }
QMenu::item:disabled { color:#87938B; }
QMenu::separator { height:1px; background:rgba(232,238,232,40); margin:5px 9px; }
QToolButton,QPushButton { background:transparent; color:#F2F3EF; border:1px solid transparent; border-radius:5px; padding:7px; }
QToolButton:hover,QPushButton:hover { background:transparent; color:#FFFFFF; }
QToolButton:pressed,QPushButton:pressed { background:transparent; color:#D7E5DB; }
QToolButton:disabled,QPushButton:disabled { background:transparent; color:#7F8A83; border-color:transparent; }
QToolButton:focus,QPushButton:focus { background:transparent; border-color:#BCE8CF; }
QToolButton:checked,QPushButton:checked { background:transparent; color:#C8F0DB; }
QPushButton:default { border-color:rgba(143,208,178,110); }
QPushButton[compactSurface="true"] { padding:5px 3px; font-size:12px; }
QPushButton#outputStart,QToolButton#outputStart { font-weight:600; color:#C8F0DB; }
QPushButton#blackoutButton:checked { color:#FFE1CC; }
QPushButton#outputStart:disabled,QToolButton#outputStart:disabled,QPushButton#blackoutButton:disabled { color:#7F8A83; }
QToolButton#sectionHeading { text-align:left; background:transparent; border:0; border-radius:5px; font-weight:600; color:#E2EAE3; padding:8px; }
QToolButton#sectionHeading:hover { background:transparent; color:#FFFFFF; }
QToolButton#sectionHeading:checked { background:transparent; color:#F2F3EF; }
QToolButton#sectionHeading:disabled { color:#7F8A83; }
QLabel#brandName { font-size:17px; font-weight:700; color:#E8EEE8; }
QLabel#projectName,QLabel#projectTitle { color:#C8D4CB; }
QLabel#surfaceHeading,QLabel#inspectorHeading,QLabel#libraryHeading { color:#D2DFD5; font-size:12px; font-weight:600; }
QLabel#taskSurface { font-size:17px; font-weight:600; color:#F2F3EF; }
QLabel#canvasHint,QLabel#scopeLabel,QLabel#browserStatus,QLabel#outputStatus,QLabel#entranceInfo { color:#AFB8B3; font-size:12px; }
QLabel#connectionDescription { background:transparent; border:1px solid transparent; border-radius:6px; padding:9px; color:#C8D4CB; }
QWidget#surfaceSidebar,QWidget#inspectorPanel,QWidget#outputToolbar,QWidget#projectHeader { background:transparent; border:0; }
QListWidget,QListView,QTreeView,QTableView { background:transparent; alternate-background-color:rgba(232,238,232,7); color:#F2F3EF; border:1px solid transparent; border-radius:6px; outline:0; }
QAbstractItemView::item { color:#F2F3EF; }
QListWidget::item { padding:8px; border:1px solid transparent; border-radius:3px; }
QListWidget::item:selected,QAbstractItemView::item:selected { background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 rgba(180,225,197,52),stop:.2 rgba(143,208,178,37),stop:1 rgba(112,170,140,26)); color:#FFFFFF; border:1px solid rgba(205,242,219,60); border-left:3px solid #8FD0B2; }
QListWidget::item:selected:!active,QAbstractItemView::item:selected:!active { background:rgba(175,184,179,29); color:#D3DDD6; border-left-color:#94A59A; }
QListWidget::item:hover:!selected,QAbstractItemView::item:hover:!selected { background:rgba(232,238,232,23); }
QListWidget::item:disabled,QAbstractItemView::item:disabled { color:#7F8A83; }
QListWidget:focus,QListView:focus,QTreeView:focus,QTableView:focus { border-color:#BCE8CF; }
QListWidget#animationLibrary::item { padding:4px; }
QComboBox,QSpinBox,QDoubleSpinBox,QLineEdit,QTextEdit,QPlainTextEdit { background:transparent; color:#F2F3EF; border:1px solid transparent; border-radius:4px; padding:5px; selection-background-color:#8FD0B2; selection-color:#11241C; }
QComboBox:disabled,QSpinBox:disabled,QDoubleSpinBox:disabled,QLineEdit:disabled,QTextEdit:disabled,QPlainTextEdit:disabled { color:#7F8A83; }
QLineEdit:read-only,QTextEdit:read-only,QPlainTextEdit:read-only { color:#C3D0C7; }
QComboBox:focus,QSpinBox:focus,QDoubleSpinBox:focus,QLineEdit:focus,QTextEdit:focus,QPlainTextEdit:focus { border-color:#BCE8CF; }
QComboBox::drop-down { width:21px; border:0; background:transparent; }
QComboBox::down-arrow { image:none; width:0; height:0; }
QComboBox QAbstractItemView { background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #36423A,stop:.13 #272F2A,stop:1 #202722); color:#F2F3EF; selection-background-color:#3C5948; selection-color:#FFFFFF; border:1px solid rgba(232,238,232,120); outline:0; padding:3px; }
QComboBox QAbstractItemView::item { padding:6px 9px; min-height:18px; }
QSpinBox::up-button,QDoubleSpinBox::up-button,QSpinBox::down-button,QDoubleSpinBox::down-button { width:17px; border:0; background:transparent; }
QSpinBox::up-button:hover,QDoubleSpinBox::up-button:hover,QSpinBox::down-button:hover,QDoubleSpinBox::down-button:hover { background:rgba(232,238,232,25); border-radius:3px; }
QSpinBox::up-button:pressed,QDoubleSpinBox::up-button:pressed,QSpinBox::down-button:pressed,QDoubleSpinBox::down-button:pressed { background:rgba(0,0,0,50); }
QSpinBox::up-button:disabled,QDoubleSpinBox::up-button:disabled,QSpinBox::down-button:disabled,QDoubleSpinBox::down-button:disabled { background:transparent; }
QSpinBox::up-arrow,QDoubleSpinBox::up-arrow,QSpinBox::down-arrow,QDoubleSpinBox::down-arrow { image:none; width:0; height:0; }
QSlider { background:transparent; }
QSlider::groove:horizontal { height:5px; background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #17201B,stop:1 #46564B); border:1px solid #53675B; border-radius:3px; }
QSlider::sub-page:horizontal { background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #BCE8CF,stop:.42 #8FD0B2,stop:1 #6EAD8D); border-radius:2px; }
QSlider::handle:horizontal { background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #FFFFFF,stop:.3 #E8EEE8,stop:1 #A6BCAF); width:14px; height:14px; margin:-5px 0; border:1px solid #F2F6F0; border-radius:7px; }
QSlider::handle:horizontal:hover { background:#FFFFFF; border-color:#BCE8CF; }
QSlider::handle:horizontal:pressed { background:#BCE8CF; border-color:#8FD0B2; }
QSlider::handle:horizontal:disabled { background:#6A7B70; border-color:#89988F; }
QSlider::sub-page:horizontal:disabled { background:#68776E; }
QSlider::handle:horizontal:focus { border-color:#8FD0B2; }
QCheckBox { background:transparent; spacing:5px; color:#DEE8E0; }
QCheckBox:disabled,QRadioButton:disabled { color:#7F8A83; }
QCheckBox:focus,QRadioButton:focus { text-decoration:underline; }
QCheckBox::indicator { width:14px; height:14px; border:1px solid #8DAB98; border-radius:3px; background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #17201B,stop:.75 #2B382F,stop:1 #566C5D); }
QCheckBox::indicator:checked { background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #C1E9D1,stop:.25 #8FD0B2,stop:1 #73B493); border-color:#C9F0D9; image:url(:/assets/icons/check.svg); }
QCheckBox::indicator:hover { border-color:#E8EEE8; }
QCheckBox::indicator:pressed { background:#34483A; border-color:#8FD0B2; }
QCheckBox::indicator:checked:pressed { background:#BCE8CF; }
QCheckBox::indicator:disabled { border-color:#526559; background:#28332C; }
QRadioButton { background:transparent; spacing:6px; }
QRadioButton::indicator { width:14px; height:14px; border:1px solid #8DAB98; border-radius:7px; background:#1D2220; }
QRadioButton::indicator:checked { border:4px solid #8FD0B2; background:#1D2220; }
QRadioButton::indicator:hover { border-color:#C9F0D9; }
QRadioButton::indicator:pressed { background:#34483A; }
QRadioButton::indicator:disabled { border-color:#526559; background:#28332C; }
QScrollArea { background:transparent; border:0; }
QScrollArea > QWidget > QWidget { background:transparent; }
QScrollBar:vertical { background:rgba(7,16,10,35); width:12px; margin:2px; border:0; }
QScrollBar::handle:vertical { background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #B4C6B9,stop:.25 #7E9586,stop:1 #536B5C); min-height:30px; border:1px solid rgba(232,238,232,65); border-radius:4px; }
QScrollBar::handle:vertical:hover { background:#ADC9B8; }
QScrollBar::handle:vertical:pressed { background:#8FD0B2; }
QScrollBar::add-line:vertical,QScrollBar::sub-line:vertical { height:0; }
QScrollBar::add-page:vertical,QScrollBar::sub-page:vertical { background:none; }
QScrollBar:horizontal { background:rgba(7,16,10,35); height:12px; margin:2px; border:0; }
QScrollBar::handle:horizontal { background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #B4C6B9,stop:.25 #7E9586,stop:1 #536B5C); min-width:30px; border:1px solid rgba(232,238,232,65); border-radius:4px; }
QScrollBar::handle:horizontal:hover { background:#ADC9B8; }
QScrollBar::handle:horizontal:pressed { background:#8FD0B2; }
QScrollBar::add-line:horizontal,QScrollBar::sub-line:horizontal { width:0; }
QScrollBar::add-page:horizontal,QScrollBar::sub-page:horizontal { background:none; }
QTabWidget::pane { background:transparent; border:0; }
QTabBar { background:transparent; }
QTabBar::tab { background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 rgba(232,238,232,21),stop:.3 rgba(232,238,232,5),stop:1 rgba(0,0,0,12)); color:#AFB8B3; border:0; border-bottom:2px solid transparent; padding:8px 10px; }
QTabBar::tab:selected { background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 rgba(232,238,232,43),stop:.24 rgba(143,208,178,23),stop:1 rgba(143,208,178,14)); color:#F2F3EF; border-bottom-color:#8FD0B2; }
QTabBar::tab:hover:!selected { background:rgba(232,238,232,23); color:#F2F3EF; }
QStatusBar { background:transparent; color:#AFB8B3; border:0; font-size:12px; }
QStatusBar::item { border:0; }
QSplitter::handle { background:#34443A; }
QSplitter::handle:hover { background:#8FD0B2; }
QFrame[frameShape="4"],QFrame[frameShape="5"] { color:rgba(232,238,232,42); }
QToolTip { background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #526357,stop:.13 #343F37,stop:.75 #252E28,stop:1 #1D2520); color:#F2F3EF; border:1px solid #9AAC9E; border-radius:4px; padding:7px; }
QProgressBar { border:1px solid #728B7B; border-radius:4px; background:#1D2220; text-align:center; color:#F2F3EF; }
QProgressBar::chunk { background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #8FD0B2,stop:.16 #5E8D71,stop:.3 #355140,stop:1 #233D2E); border-radius:3px; }
)QSS");
    if(qApp&&qApp->property("potatoIncreasedContrast").toBool())style+=QStringLiteral(R"QSS(
QWidget { color:#FFFFFF; }
QLabel#canvasHint,QLabel#scopeLabel,QLabel#browserStatus,QLabel#outputStatus,QLabel#entranceInfo,QStatusBar { color:#DCE9DF; }
QMenu::item:selected { background:#3C5948; color:#FFFFFF; }
QMenu::separator,QToolBar::separator { background:#A2B7A9; }
QComboBox QAbstractItemView { background:#1D2620; border-color:#C7DCCF; }
QComboBox,QSpinBox,QDoubleSpinBox,QLineEdit,QTextEdit,QPlainTextEdit,QListWidget,QListView,QTreeView,QTableView { border-color:#91AA9A; }
QToolButton,QPushButton { border-color:#81998A; }
QToolButton:focus,QPushButton:focus,QComboBox:focus,QSpinBox:focus,QDoubleSpinBox:focus,QLineEdit:focus,QTextEdit:focus,QPlainTextEdit:focus,QListWidget:focus,QListView:focus,QTreeView:focus,QTableView:focus { border-color:#E8FFF1; }
QToolTip { background:#1D2620; color:#FFFFFF; border-color:#C7DCCF; }
)QSS");
    return style;
}

// The existing optical sheet supplies native control metrics. These profiles
// replace its palette while the shared painter supplies the approved clear
// material, so no opaque QSS layer hides the continuous backdrop.
inline QString potatoAuroraStyle(bool transparentGlass=false){
    auto style=potatoLiquidGlassStyle();
    const QList<QPair<QString,QString>> colours{
        {"#F2F3EF","#F7FAFF"},{"#E8EEE8","#D7F1FF"},{"#AFB8B3","#B7CDE9"},
        {"#7F8A83","#A0B7D1"},{"#87938B","#A0B7D1"},{"#8FD0B2","#8CE8E6"},
        {"#BCE8CF","#A1F2F2"},{"#C8F0DB","#F7FAFF"},{"#D1DBD4","#E5EEFF"},
        {"#D7E5DB","#E5EEFF"},{"#E2EAE3","#F7FAFF"},{"#C8D4CB","#E5EEFF"},
        {"#D2DFD5","#E5EEFF"},{"#C3D0C7","#E5EEFF"},{"#D3DDD6","#E5EEFF"},
        {"#94A59A","#78A3CE"},{"#DEE8E0","#F7FAFF"},{"#DCE9DF","#F7FAFF"},
        {"#91AA9A","#9CC9F0"},{"#81998A","#8EB9E5"},{"#E8FFF1","#C5FFFF"},
        {"#C7DCCF","#C8EAFF"},{"#A2B7A9","#BCDFF7"},
        {"232,238,232","215,241,255"},{"143,208,178","140,232,230"},
        {"195,231,209","117,210,235"},{"180,225,197","92,186,230"},
        {"112,170,140","59,119,197"},{"205,242,219","179,232,255"},
        {"175,184,179","156,184,220"}
    };
    for(const auto &colour:colours)style.replace(colour.first,colour.second);
    style+=QStringLiteral(R"QSS(
QToolButton,QPushButton { border-radius:8px; color:#F7FAFF; }
QToolButton:focus,QPushButton:focus,QComboBox:focus,QSpinBox:focus,QDoubleSpinBox:focus,QLineEdit:focus,QTextEdit:focus,QPlainTextEdit:focus { border-color:#A1F2F2; }
QPushButton:default { border-color:rgba(161,242,242,170); }
QToolButton#sectionHeading { border-radius:8px; color:#F7FAFF; }
QToolButton#sectionHeading:checked { color:#F7FAFF; }
QPushButton#outputStart,QToolButton#outputStart { color:#F7FAFF; }
QPushButton#blackoutButton:checked { color:#FFF0F6; }
QLabel#dynamicStage { color:#D3EEFF; font-size:11px; }
QLabel#updateDialogTitle { color:#F7FAFF; font-size:20px; font-weight:700; }
QLabel#surfaceMenuTargetLabel { color:#F7FAFF; font:600 13px 'Segoe UI'; }
QListWidget::item:selected:!active,QAbstractItemView::item:selected:!active { background:rgba(104,171,218,38); color:#F7FAFF; border-left-color:#8BC6E6; }
QListWidget::item:disabled,QAbstractItemView::item:disabled { color:#A0B7D1; }
QComboBox,QSpinBox,QDoubleSpinBox,QLineEdit,QTextEdit,QPlainTextEdit { border-radius:8px; selection-background-color:#237B9A; selection-color:#FFFFFF; }
QComboBox QAbstractItemView { background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #29528B,stop:.5 #374A84,stop:1 #25647B); color:#F7FAFF; selection-background-color:#287E9C; selection-color:#FFFFFF; border-color:#97C6E5; }
QSlider::groove:horizontal { background:rgba(156,200,246,40); border-color:rgba(156,200,246,45); }
QSlider::sub-page:horizontal { background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #8CE8E6,stop:1 #58BDE0); }
QSlider::handle:horizontal { background:#FAFCFF; border-color:#B8DAEC; }
QSlider::handle:horizontal:hover { background:#FFFFFF; border-color:#A1F2F2; }
QSlider::handle:horizontal:pressed { background:#A1F2F2; border-color:#8CE8E6; }
QSlider::handle:horizontal:disabled { background:#8FA9C5; border-color:#7890AC; }
QSlider::sub-page:horizontal:disabled { background:#7396BA; }
QCheckBox::indicator { border-color:#A5CCE8; background:rgba(40,77,123,80); }
QCheckBox::indicator:checked { background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #147B81,stop:1 #225FA5); border-color:#9DD5ED; }
QCheckBox::indicator:hover { border-color:#D7F1FF; }
QCheckBox::indicator:pressed { background:#246887; border-color:#8CE8E6; }
QCheckBox::indicator:checked:pressed { background:#247D9D; }
QCheckBox::indicator:disabled { border-color:#688BAD; background:#3D587C; }
QRadioButton::indicator { border-color:#A5CCE8; background:#294B77; }
QRadioButton::indicator:checked { border-color:#8CE8E6; background:#294B77; }
QRadioButton::indicator:hover { border-color:#D7F1FF; }
QRadioButton::indicator:pressed { background:#246887; }
QRadioButton::indicator:disabled { border-color:#688BAD; background:#3D587C; }
QScrollBar:vertical,QScrollBar:horizontal { background:rgba(34,63,114,30); }
QScrollBar::handle:vertical,QScrollBar::handle:horizontal { background:rgba(162,212,239,110); border-color:rgba(215,241,255,50); }
QScrollBar::handle:vertical:hover,QScrollBar::handle:horizontal:hover { background:#91CCE4; }
QScrollBar::handle:vertical:pressed,QScrollBar::handle:horizontal:pressed { background:#8CE8E6; }
QSplitter::handle { background:rgba(102,181,215,45); }
QSplitter::handle:hover { background:#8CE8E6; }
QToolTip { background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #2B5088,stop:.5 #384980,stop:1 #256479); color:#F7FAFF; border-color:#98C7E6; border-radius:8px; }
QProgressBar { background:rgba(34,63,114,110); color:#F7FAFF; border-color:#79A4CE; }
QProgressBar::chunk { background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #147B81,stop:1 #225FA5); }
)QSS");
    if(!transparentGlass)style+=QStringLiteral("QCheckBox::indicator:checked { image:url(:/assets/icons/check-light.svg); }\n");
    if(transparentGlass)style+=QStringLiteral(R"QSS(
QPushButton#outputStart,QToolButton#outputStart { color:#0D2734; }
QPushButton#outputStart:disabled,QToolButton#outputStart:disabled { color:#A0B7D1; }
QCheckBox::indicator:checked { background:#80E2D7; border-color:#B4FFF4; }
QComboBox QAbstractItemView { background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #485E73,stop:.5 #3F526A,stop:1 #265563); }
QToolTip { background:#354D62; }
)QSS");
    if(qApp&&qApp->property("potatoIncreasedContrast").toBool())style+=QStringLiteral(R"QSS(
QWidget { color:#FFFFFF; }
QLabel#canvasHint,QLabel#scopeLabel,QLabel#browserStatus,QLabel#outputStatus,QLabel#entranceInfo,QLabel#dynamicStage,QStatusBar { color:#F0F7FF; }
QComboBox QAbstractItemView,QToolTip { background:#1E395F; color:#FFFFFF; border-color:#D2F1FF; }
QMenu::item:selected { background:#246B91; color:#FFFFFF; }
)QSS");
    return style;
}

inline QString potatoAmbientStyle(){
    // Preserve established metrics and input behaviour. Ambient's shared
    // procedural environment supplies the moving material, not this stylesheet.
    auto style=potatoAuroraStyle(false);
    style+=QStringLiteral(R"QSS(
QWidget { color:#EDF1F7; }
QToolButton,QPushButton { color:#EDF1F7; }
QPushButton#outputStart,QToolButton#outputStart { color:#F7FAFF; }
QLabel:disabled,QToolButton:disabled,QPushButton:disabled,QMenu::item:disabled { color:#8E9CA8; }
QMenu::item:selected { background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 rgba(169,210,202,65),stop:1 rgba(146,183,199,30)); color:#FFFFFF; }
QListWidget::item:selected,QAbstractItemView::item:selected { background:rgba(141,189,202,45); border-left-color:#A9D2CA; }
QComboBox QAbstractItemView { background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #1B353B,stop:.5 #252B37,stop:1 #302735); color:#EDF1F7; selection-background-color:#355C63; border-color:#8AA9AE; }
QToolTip { background:#16252E; color:#EDF1F7; border-color:#8AA9AE; }
QSlider::groove:horizontal { background:rgba(160,185,197,40); border-color:rgba(185,211,218,45); }
QSlider::sub-page:horizontal { background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #A9D2CA,stop:1 #8DB5C4); }
QSlider::handle:horizontal { background:#EFF7F4; border-color:#BCD5D1; }
QCheckBox::indicator { background:rgba(7,17,25,100); border-color:#8AA9AE; }
QCheckBox::indicator:checked { background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #476C76,stop:1 #355384); border-color:#A9D2CA; }
QRadioButton::indicator { background:#13242C; border-color:#8AA9AE; }
QRadioButton::indicator:checked { background:#13242C; border-color:#A9D2CA; }
QScrollBar:vertical,QScrollBar:horizontal { background:rgba(8,16,23,32); }
QScrollBar::handle:vertical,QScrollBar::handle:horizontal { background:rgba(180,207,212,100); border-color:rgba(220,240,248,35); }
QSplitter::handle { background:rgba(138,169,174,38); }
QSplitter::handle:hover { background:#A9D2CA; }
QProgressBar { background:#14242D; border-color:#8AA9AE; }
)QSS");
    if(qApp&&qApp->property("potatoIncreasedContrast").toBool())style+=QStringLiteral("QWidget { color:#FFFFFF; } QComboBox QAbstractItemView,QToolTip { background:#101B24; color:#FFFFFF; border-color:#E2F2F0; }\n");
    return style;
}

// Keep the familiar compact widget geometry. material.h paints the surfaces;
// transparent bases let Qt paint real text, icons, values and native interaction
// states without concealing the app-rendered layers, light and rounded rims.
inline QString potatoStyle(const QString &theme=QStringLiteral("classic1"),const QString &interfaceTheme=QStringLiteral("material")){
    if(interfaceTheme==QStringLiteral("classic1"))return potatoClassicStyle();
    if(interfaceTheme==QStringLiteral("liquidglass1"))return potatoLiquidGlassStyle();
    if(interfaceTheme==QStringLiteral("ambientglass1"))return potatoAmbientStyle();
    if(PotatoAppearance::clearGlass(interfaceTheme))return potatoAuroraStyle(interfaceTheme=="transparentglass1");
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
