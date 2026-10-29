#pragma once
#include <QString>
#include "appearance.h"

// Appearance changes editor chrome only; it never changes scene colours or data.
inline QString potatoStyle(const QString &theme=QStringLiteral("classic1")){
    QString style=QStringLiteral(R"QSS(
QWidget { background:#1B1D1F; color:#E7E8EA; font-family:'Segoe UI'; font-size:13px; }
QMainWindow { background:#101214; }
QLabel { background:transparent; border:0; }
QMenuBar { background:#141618; color:#B7BABF; border-bottom:1px solid #303336; padding:2px 10px; }
QMenuBar::item { background:transparent; padding:5px 10px; }
QMenuBar::item:selected,QMenuBar::item:pressed { background:#2B2E31; color:#FFFFFF; }
QMenu { background:#242729; border:1px solid #464A4E; padding:5px; }
QMenu::item { padding:7px 30px 7px 12px; border-radius:3px; }
QMenu::item:selected { background:#3C3427; color:#FFFFFF; }
QMenu::item:disabled { color:#767B80; }
QMenu::separator { height:1px; background:#383C40; margin:5px 8px; }
QToolBar { background:#17191B; border:0; border-bottom:1px solid #34373A; padding:8px 12px; spacing:10px; }
QToolBar::separator { width:1px; background:#383B3E; margin:5px 9px; }
QToolBar#projectToolbar,QWidget#projectHeader { background:#141618; border-bottom:1px solid #333639; padding:8px 12px; spacing:10px; }
QToolBar#outputToolbar { background:#1C1F21; padding:4px 12px; spacing:6px; }
QToolButton,QPushButton { background:#25282B; color:#E7E8EA; border:1px solid #41464B; border-radius:4px; padding:7px 12px; }
QToolButton { background:transparent; border-color:transparent; }
QToolButton:hover,QPushButton:hover { background:#303437; border-color:#62686E; color:#FFFFFF; }
QToolButton:pressed,QPushButton:pressed { background:#393D40; border-color:#F3B34C; }
QToolButton:disabled,QPushButton:disabled { background:transparent; border-color:#303437; color:#73787D; }
QToolButton:focus,QPushButton:focus { border-color:#D39A42; }
QToolButton:checked,QPushButton:checked { background:#F3B34C; border-color:#FFD185; color:#17191B; }
QToolButton:checked:hover,QPushButton:checked:hover { background:#FFC365; border-color:#FFD185; }
QPushButton[compactSurface="true"] { padding:5px 4px; font-size:12px; }
QPushButton#outputStart,QToolButton#outputStart { background:#F3B34C; border:1px solid #FFD185; color:#17191B; padding:8px 16px; font-weight:600; }
QPushButton#outputStart:hover,QToolButton#outputStart:hover { background:#FFC365; }
QPushButton#blackoutButton { background:#24272A; border-color:#464B50; padding:8px 14px; }
QPushButton#blackoutButton:checked { background:#4B3230; border:1px solid #E6A568; color:#FFE3C4; }
QLabel#brandName { font-size:18px; font-weight:700; color:#F1F2F3; padding-right:10px; }
QLabel#projectName { color:#C3C6CA; font-size:15px; }
QLabel#surfaceHeading,QLabel#inspectorHeading,QLabel#libraryHeading { color:#C8CBD0; font-size:12px; font-weight:500; }
QLabel#taskSurface { font-size:18px; font-weight:600; color:#F2F3F4; padding:8px 2px 12px; }
QLabel#canvasHint,QLabel#scopeLabel,QLabel#browserStatus,QLabel#outputStatus { color:#9EA3AA; font-size:12px; }
QWidget#surfaceSidebar,QWidget#inspectorPanel { background:#1C1F21; border:1px solid #303437; border-radius:4px; }
QWidget#canvasTools { background:#1C1F21; border:1px solid #303437; border-radius:4px; }
QWidget#canvasTools QPushButton,QWidget#canvasTools QToolButton { padding:7px 12px; }
QWidget#canvasTools QPushButton:!checked { background:transparent; border-color:#33373B; }
QWidget#libraryPanel,QWidget#libraryShelf { background:#191C1E; border:1px solid #33373B; border-radius:4px; }
QWidget#libraryHeader { background:#1C1F21; border-bottom:1px solid #303437; }
QWidget#stageTransport { background:#171A1C; border-top:1px solid #34373A; }
QWidget#stageTransport QToolButton { padding:8px 10px; }
QWidget#outputDetails { background:#202326; border-top:1px solid #33373B; }
QToolButton#sectionHeading { text-align:left; background:transparent; border:0; border-bottom:1px solid #35393D; border-radius:0; font-weight:600; color:#CDD0D4; padding:10px 2px; }
QToolButton#sectionHeading:hover { background:#262A2D; color:#F2F3F4; }
QToolButton#sectionHeading:checked { background:transparent; color:#E7E8EA; }
QListWidget { background:#171A1C; border:1px solid #34383C; border-radius:4px; outline:0; alternate-background-color:#1D2022; }
QListWidget::item { padding:7px; border-bottom:1px solid #303437; color:#E7E8EA; }
QListWidget::item:selected { background:#2C2B27; color:#FFFFFF; border:1px solid #D39A42; border-left:4px solid #F3B34C; }
QListWidget::item:hover:!selected { background:#262A2D; }
QListWidget#surfaceList { background:transparent; border:0; border-radius:0; }
QListWidget#animationLibrary { background:transparent; border:0; }
QListWidget#animationLibrary::item { border-bottom:0; padding:4px; }
QComboBox,QSpinBox,QDoubleSpinBox,QLineEdit { background:#171A1C; color:#DFE2E5; border:1px solid #41474D; border-radius:4px; padding:6px 8px; selection-background-color:#65502D; selection-color:#FFFFFF; }
QComboBox:hover,QSpinBox:hover,QDoubleSpinBox:hover,QLineEdit:hover { border-color:#62686F; }
QComboBox:focus,QSpinBox:focus,QDoubleSpinBox:focus,QLineEdit:focus { border-color:#D39A42; }
QComboBox:disabled,QSpinBox:disabled,QDoubleSpinBox:disabled,QLineEdit:disabled { color:#777C82; border-color:#303437; }
QComboBox::drop-down { width:24px; border:0; background:transparent; }
QComboBox QAbstractItemView { background:#25282B; color:#E7E8EA; selection-background-color:#51422C; selection-color:#FFFFFF; border:1px solid #464B50; outline:0; padding:3px; }
QSpinBox::up-button,QDoubleSpinBox::up-button,QSpinBox::down-button,QDoubleSpinBox::down-button { width:17px; border:0; background:transparent; }
QSlider::groove:horizontal { height:4px; background:#484D53; border-radius:2px; }
QSlider::sub-page:horizontal { background:#F3B34C; border-radius:2px; }
QSlider::handle:horizontal { background:#D7DAE0; width:13px; height:13px; margin:-5px 0; border:1px solid #F0F1F3; border-radius:7px; }
QSlider::handle:horizontal:hover { background:#FFFFFF; border-color:#FFD185; }
QSlider::handle:horizontal:disabled { background:#70767D; border-color:#80868D; }
QSlider::sub-page:horizontal:disabled { background:#666C72; }
QCheckBox { background:transparent; spacing:8px; color:#CDD0D4; }
QCheckBox::indicator { width:14px; height:14px; border:1px solid #636B73; border-radius:3px; background:#171A1C; }
QCheckBox::indicator:checked { background:#F3B34C; border:2px solid #F3B34C; image:url(:/assets/icons/check.svg); }
QCheckBox::indicator:hover { border-color:#D39A42; }
QRadioButton { background:transparent; spacing:7px; }
QScrollArea { background:transparent; border:0; }
QScrollArea > QWidget > QWidget { background:transparent; }
QScrollBar:vertical { background:#1B1D1F; width:10px; margin:3px 2px; border:0; }
QScrollBar::handle:vertical { background:#4C5157; min-height:28px; border-radius:3px; }
QScrollBar::handle:vertical:hover { background:#777E86; }
QScrollBar::handle:vertical:pressed { background:#D39A42; }
QScrollBar::add-line:vertical,QScrollBar::sub-line:vertical { height:0; }
QScrollBar::add-page:vertical,QScrollBar::sub-page:vertical { background:none; }
QScrollBar:horizontal { background:#1B1D1F; height:10px; margin:2px 3px; border:0; }
QScrollBar::handle:horizontal { background:#4C5157; min-width:28px; border-radius:3px; }
QScrollBar::handle:horizontal:hover { background:#777E86; }
QScrollBar::handle:horizontal:pressed { background:#D39A42; }
QScrollBar::add-line:horizontal,QScrollBar::sub-line:horizontal { width:0; }
QScrollBar::add-page:horizontal,QScrollBar::sub-page:horizontal { background:none; }
QTabWidget::pane { background:transparent; border:0; border-top:1px solid #34383C; }
QTabBar { background:transparent; }
QTabBar::tab { background:transparent; color:#AEB3B9; border:0; border-bottom:3px solid transparent; padding:10px 13px; min-width:42px; }
QTabBar::tab:selected { background:#222629; color:#F2F3F4; border-bottom:3px solid #F3B34C; font-weight:600; }
QTabBar::tab:hover:!selected { background:#292D30; color:#E7E8EA; }
QTabBar::tab:disabled { color:#73787D; }
QStatusBar { background:#15181A; color:#A7ADB4; border-top:1px solid #303437; padding:2px 8px; font-size:12px; }
QStatusBar::item { border:0; }
QSplitter::handle { background:#111416; width:5px; height:5px; }
QSplitter::handle:hover { background:#8B6A37; }
QFrame[frameShape="4"],QFrame[frameShape="5"] { color:#34383C; }
QLabel#connectionDescription { background:#24282B; border:1px solid #343A40; border-radius:4px; padding:10px; color:#BEC4CA; }
QToolTip { background:#303438; color:#F0F2F4; border:1px solid #626971; padding:7px; }
QProgressBar { border:1px solid #43494F; border-radius:3px; background:#171A1C; text-align:center; }
QProgressBar::chunk { background:#D39A42; }
)QSS");
    const auto id=PotatoAppearance::themeId(theme);
    // Event palettes tint only quiet chrome and interaction accents.
    if(id=="halloween"){
        style.replace("#1B1D1F","#201D23").replace("#1C1F21","#25212A").replace("#171A1C","#1B171F");
        style.replace("#F3B34C","#EAA15D").replace("#D39A42","#BE854D").replace("#FFD185","#FFD0A1");
    }else if(id=="autumn"){
        style.replace("#1B1D1F","#211E1B").replace("#1C1F21","#27231E").replace("#171A1C","#1B1815");
        style.replace("#F3B34C","#DEB878").replace("#D39A42","#BA965F").replace("#FFD185","#F1D7AF");
    }else if(id=="christmas"){
        style.replace("#1B1D1F","#1C211F").replace("#1C1F21","#202824").replace("#171A1C","#161D19");
        style.replace("#F3B34C","#D6BE85").replace("#D39A42","#B29B64").replace("#FFD185","#EEE0BA");
    }
    return style;
}
