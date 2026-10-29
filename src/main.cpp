#include "model.h"
#include "canvas.h"
#include "branding.h"
#include "updates.h"
#include "ui.h"
#include "style.h"
#include "studio-widgets.h"
#include "distribution.h"
#include "../tests/appearance-theme-check.h"
#include "../tests/seasonal-check.h"
#include "../tests/connections-check.h"
#include "../tests/studio-check.h"
#include "dynamic-ui.h"
#include "projects.h"
#include "launcher-refresh.h"
#include "entrance.h"
#include "../tests/appearance-check.h"
#include "../tests/fx-check.h"
#include "../tests/harvest-check.h"
#include "../tests/fall-check.h"
#include "../tests/fx-profile.h"
#include "../tests/dynamic-check.h"
#include "../tests/entrance-check.h"
#include <QMenuBar>
#include <QMenu>
#include <QInputDialog>
#include <QProgressBar>
#include <QProcess>
#include <QLockFile>
#include <QDir>
#include <QContextMenuEvent>
#include <QApplication>
#include <QMainWindow>
#include <QToolBar>
#include <QLabel>
#include <QListWidget>
#include <QSplitter>
#include <QScrollArea>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QSlider>
#include <QFileDialog>
#include <QMessageBox>
#include <QStatusBar>
#include <QScreen>
#include <QWindow>
#include <QCloseEvent>
#include <QShortcut>
#include <QSignalBlocker>
#include <QTimer>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QJsonDocument>
#include <QSurfaceFormat>
#include <QFile>
#include <QTextStream>
#include <QDesktopServices>
#include <QPointer>
#include <QSet>
#include <QSettings>
#include <QIcon>
#include <QActionGroup>
#include <QTabWidget>
#include <QTabBar>
#include <QLineEdit>
#include <QButtonGroup>
#include <functional>
#include <memory>

class MainWindow : public QMainWindow {
    friend int main(int,char**);
public:
    Scene scene{this};
    Canvas *canvas,*output;
    bool smoke=false;
    explicit MainWindow(){
        setWindowTitle("Potato Mapper");setMinimumSize(960,640);
        const auto desktop=screen()->availableGeometry().size();resize(qMin(1280,desktop.width()-48),qMin(800,desktop.height()-48));
        canvas=new Canvas(&scene,true);output=new Canvas(&scene,false);
        output->setWindowTitle("Potato Mapper — Projector");output->setWindowFlag(Qt::FramelessWindowHint);
        output->installEventFilter(this);
        auto *bar=addToolBar("Project");bar->setObjectName("projectToolbar");bar->setMovable(false);bar->setFloatable(false);bar->setFixedHeight(56);bar->setIconSize({18,18});bar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);bar->setStyleSheet("QToolBar{padding:4px 6px;spacing:4px;} QToolButton{padding:5px 6px;}");bar->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
        auto *brandIcon=new QLabel;brandIcon->setPixmap(QPixmap(":/assets/potato-mapper.png").scaled(28,28,Qt::KeepAspectRatio,Qt::SmoothTransformation));brandIcon->setContentsMargins(8,0,5,0);bar->addWidget(brandIcon);
        projectTitle=new ElidedLabel;projectTitle->setObjectName("projectTitle");projectTitle->setMinimumWidth(90);projectTitle->setMaximumWidth(300);projectTitle->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);bar->addWidget(projectTitle);
        auto action=[this,bar](const QString &name,const QKeySequence &key,std::function<void()> fn){auto *a=bar->addAction(name);if(!key.isEmpty())a->setShortcut(key);connect(a,&QAction::triggered,this,fn);return a;};
        action("New",QKeySequence::New,[this]{if(!canDiscard())return;scene.newProject();refresh();});
        action("Open",QKeySequence::Open,[this]{open();})->setIcon(studioIcon("open"));action("Save",QKeySequence::Save,[this]{save(false);})->setIcon(studioIcon("save"));
        action("Save as",QKeySequence("Ctrl+Shift+S"),[this]{save(true);});
        auto *undoAction=action("Undo",QKeySequence::Undo,[this]{scene.undo();});undoAction->setIcon(studioIcon("undo"));
        auto *redoAction=action("Redo",QKeySequence::Redo,[this]{scene.redo();});redoAction->setIcon(studioIcon("redo"));
        action("+ Surface",QKeySequence("Ctrl+N, S"),[this]{scene.add();});action("Load media",QKeySequence("Ctrl+I"),[this]{loadMedia();});
        action("Potato FX",{},[this]{showAnimationBrowser();});action("Dynamic FX",{},[this]{showDynamicControls();});bar->addSeparator();
        auto *outputPanel=new QWidget;outputPanel->setObjectName("outputToolbar");auto *outputRow=new QHBoxLayout(outputPanel);outputRow->setContentsMargins(2,0,5,0);outputRow->setSpacing(5);
        displays=new QComboBox;displays->setObjectName("outputDisplays");displays->setMinimumWidth(130);displays->setMaximumWidth(180);displays->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);displays->setMinimumContentsLength(13);displays->setToolTip("Projection destination");outputRow->addWidget(displays);
        startOutputButton=new QPushButton("Start output");startOutputButton->setIcon(studioIcon("play"));startOutputButton->setObjectName("outputStart");startOutputButton->setStyleSheet("padding:7px 10px;");outputRow->addWidget(startOutputButton);
        stopOutputButton=new QPushButton;stopOutputButton->setIcon(studioIcon("stop"));stopOutputButton->setToolTip("Stop projector output (Esc)");stopOutputButton->setAccessibleName("Stop output");stopOutputButton->setObjectName("outputStop");stopOutputButton->setFixedWidth(34);outputRow->addWidget(stopOutputButton);
        blackoutButton=new QPushButton("Blackout");blackoutButton->setToolTip("Blackout projector output [B]");blackoutButton->setObjectName("blackoutButton");blackoutButton->setStyleSheet("padding:7px 8px;");blackoutButton->setCheckable(true);outputRow->addWidget(blackoutButton);
        auto *outputDetailsToggle=new QToolButton;outputDetailsToggle->setIcon(studioIcon("settings"));outputDetailsToggle->setToolTip("Output details and Potato Scan");outputDetailsToggle->setAccessibleName("Output details");outputDetailsToggle->setObjectName("outputDetailsToggle");outputDetailsToggle->setFixedSize(32,34);outputDetailsToggle->setIconSize({20,20});outputDetailsToggle->setStyleSheet("padding:4px;");outputRow->addWidget(outputDetailsToggle);outputPanel->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Fixed);
        auto *outputDetails=new QWidget(this,Qt::Popup);outputDetails->setObjectName("outputDetails");outputDetails->setMinimumWidth(440);auto *detailsLayout=new QVBoxLayout(outputDetails);detailsLayout->setContentsMargins(14,14,14,14);detailsLayout->setSpacing(12);
        auto *outputHeading=new QLabel("PROJECTOR OUTPUT");outputHeading->setObjectName("surfaceHeading");detailsLayout->addWidget(outputHeading);
        outputInfo=new QLabel;outputInfo->setObjectName("outputStatus");outputInfo->setWordWrap(true);detailsLayout->addWidget(outputInfo);
        auto *displayActions=new QHBoxLayout;auto *refreshButton=new QPushButton("Refresh displays");displayActions->addWidget(refreshButton);
        auto *settingsButton=new QPushButton("Display settings");displayActions->addWidget(settingsButton);detailsLayout->addLayout(displayActions);
        auto *displayHelp=new QLabel("Use Windows + P → Extend. Choose the projector in the header. Output on the editor display opens fullscreen; Esc returns to the editor.");displayHelp->setWordWrap(true);detailsLayout->addWidget(displayHelp);
        connect(outputDetailsToggle,&QToolButton::clicked,this,[outputDetails,outputDetailsToggle]{outputDetails->adjustSize();outputDetails->move(outputDetailsToggle->mapToGlobal(QPoint(outputDetailsToggle->width()-outputDetails->width(),outputDetailsToggle->height()+4)));outputDetails->show();});
        auto *entranceControls=new QVBoxLayout;entranceControls->setSpacing(8);
        auto *scanLabel=new QLabel("POTATO SCAN");scanLabel->setStyleSheet("color:#91d6d8;font-size:11px;font-weight:600;");entranceControls->addWidget(scanLabel);
        entrancePlay=new QPushButton("Play entrance");entrancePlay->setObjectName("entrancePlay");entrancePlay->setToolTip("Turn off blackout and trace your mapped surfaces, build a scan grid, then reveal their content.");entranceControls->addWidget(entrancePlay);
        entranceSkip=new QPushButton("Skip");entranceSkip->setObjectName("entranceSkip");entranceControls->addWidget(entranceSkip);
        entranceAutomatic=new QCheckBox("On output start");entranceAutomatic->setObjectName("entranceAutomatic");entranceAutomatic->setChecked(QSettings().value("output/entrance",true).toBool());entranceControls->addWidget(entranceAutomatic);
        entranceInfo=new QLabel("8-second mapping reveal");entranceInfo->setObjectName("entranceInfo");entranceInfo->setStyleSheet("color:#9caab6;font-size:11px;");entranceControls->addWidget(entranceInfo);detailsLayout->addLayout(entranceControls);
        connect(entrancePlay,&QPushButton::clicked,this,[this]{scene.playEntrance();updateOutputStatus();});
        connect(entranceSkip,&QPushButton::clicked,this,[this]{scene.skipEntrance();});
        connect(entranceAutomatic,&QCheckBox::toggled,this,[this](bool enabled){if(!smoke)QSettings().setValue("output/entrance",enabled);});
        connect(&scene,&Scene::changed,this,[this]{updateEntranceStatus();});
        connect(refreshButton,&QPushButton::clicked,this,[this]{updateDisplays();});
        connect(startOutputButton,&QPushButton::clicked,this,[this]{showOutput();});
        connect(stopOutputButton,&QPushButton::clicked,this,[this]{output->hide();updateOutputStatus();});
        connect(blackoutButton,&QPushButton::toggled,this,[this](bool v){scene.blackout=v;emit scene.changed();updateOutputStatus();});
        connect(settingsButton,&QPushButton::clicked,this,[]{QDesktopServices::openUrl(QUrl("ms-settings:display"));});
        connect(displays,&QComboBox::currentIndexChanged,this,[this]{selectedDisplay=displays->currentData().toString();if(!smoke&&!selectedDisplay.isEmpty())QSettings().setValue("output/display",selectedDisplay);updateOutputStatus();});
        auto *blackShortcut=new QShortcut(QKeySequence("B"),this);connect(blackShortcut,&QShortcut::activated,blackoutButton,&QPushButton::click);
        auto *outBlack=new QShortcut(QKeySequence("B"),output);connect(outBlack,&QShortcut::activated,blackoutButton,&QPushButton::click);
        selectedDisplay=QSettings().value("output/display").toString();
        auto *split=new QSplitter;split->setObjectName("editorSplitter");split->setHandleWidth(6);split->setChildrenCollapsible(false);
        auto *editorRoot=new QWidget;auto *editorLayout=new QVBoxLayout(editorRoot);editorLayout->setContentsMargins(0,0,0,0);editorLayout->setSpacing(0);
        auto *header=new QWidget;header->setObjectName("projectHeader");header->setFixedHeight(56);auto *headerLayout=new QHBoxLayout(header);headerLayout->setContentsMargins(0,0,5,0);headerLayout->setSpacing(3);removeToolBar(bar);headerLayout->addWidget(bar,1);bar->show();headerLayout->addWidget(outputPanel);editorLayout->addWidget(header);editorLayout->addWidget(split,1);setCentralWidget(editorRoot);
        auto *panel=new QWidget;panel->setObjectName("surfaceSidebar");panel->setMinimumWidth(180);panel->setMaximumWidth(420);
        auto *layout=new QVBoxLayout(panel);layout->setContentsMargins(5,8,5,8);layout->setSpacing(6);
        auto *surfaceHeadingRow=new QHBoxLayout;auto *surfaceHeading=new QLabel("SURFACES");surfaceHeading->setObjectName("surfaceHeading");surfaceHeadingRow->addWidget(surfaceHeading,1);
        auto *addSurface=new QPushButton("Surface");addSurface->setObjectName("addSurface");addSurface->setIcon(studioIcon("add"));connect(addSurface,&QPushButton::clicked,this,[this]{scene.add();});surfaceHeadingRow->addWidget(addSurface);layout->addLayout(surfaceHeadingRow);
        list=new QListWidget;list->setObjectName("surfaceList");list->setContextMenuPolicy(Qt::CustomContextMenu);list->setToolTip("Right-click a surface for media actions. Double-click to rename.");
        list->setTextElideMode(Qt::ElideMiddle);list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);list->setMinimumHeight(100);list->setSpacing(0);list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);list->setItemDelegate(new StudioSurfaceDelegate(list));list->viewport()->installEventFilter(this);layout->addWidget(list,1);
        connect(list,&QListWidget::currentRowChanged,this,[this](int row){scene.select(row);});
        connect(list,&QListWidget::customContextMenuRequested,this,[this](const QPoint &pos){const auto *item=list->itemAt(pos);if(item)surfaceMenu(list->row(item),list->viewport()->mapToGlobal(pos));});
        connect(list,&QListWidget::itemDoubleClicked,this,[this]{renameSelected();});
        connect(canvas,&Canvas::surfaceContextMenuRequested,this,[this](int index,const QPoint &pos){surfaceMenu(index,pos);});
        auto *surfaceButtons=new QHBoxLayout;
        auto button=[this](const QString &text,std::function<void()> fn){auto *b=new QPushButton(text);connect(b,&QPushButton::clicked,this,fn);return b;};
        surfaceButtons->addWidget(button("Duplicate",[this]{if(auto *s=scene.current()){scene.checkpoint();Surface copy=*s;copy.id=QUuid::createUuid().toString(QUuid::WithoutBraces);copy.name+=" copy";for(auto &p:copy.corners)p+=QPointF(.025,.025);scene.surfaces.append(copy);scene.selected=int(scene.surfaces.size())-1;scene.touch(true);}}));
        surfaceButtons->addWidget(button("Delete",[this]{if(scene.current()){scene.checkpoint();scene.surfaces.removeAt(scene.selected);scene.selected=std::min(scene.selected,int(scene.surfaces.size())-1);scene.touch(true);}}));
        auto *orderButtons=surfaceButtons;
        orderButtons->addWidget(button("Back",[this]{int i=scene.selected;if(i>0){scene.checkpoint();scene.surfaces.swapItemsAt(i,i-1);scene.selected--;scene.touch(true);}}));
        orderButtons->addWidget(button("Forward",[this]{int i=scene.selected;if(i>=0&&i+1<scene.surfaces.size()){scene.checkpoint();scene.surfaces.swapItemsAt(i,i+1);scene.selected++;scene.touch(true);}}));
        const QStringList surfaceIcons{"duplicate","trash","undo","redo"};
        for(int i=0;i<surfaceButtons->count();++i){auto *b=qobject_cast<QPushButton*>(surfaceButtons->itemAt(i)->widget());b->setProperty("compactSurface",true);b->setToolTip(b->text());b->setAccessibleName(b->text());b->setIcon(studioIcon(surfaceIcons.at(i)));b->setText({});b->setFixedWidth(35);}layout->addLayout(surfaceButtons);
        visible=new QCheckBox("Visible");locked=new QCheckBox("Lock position");auto *flags=new QHBoxLayout;flags->addWidget(visible);flags->addWidget(locked);
        connect(visible,&QCheckBox::toggled,this,[this](bool v){if(auto *s=scene.current()){scene.checkpoint();s->visible=v;scene.touch(true);}});
        connect(locked,&QCheckBox::toggled,this,[this](bool v){if(auto *s=scene.current()){scene.checkpoint();s->locked=v;scene.touch(true);}});
        auto *inspectorPanel=new QWidget;inspectorPanel->setObjectName("inspectorPanel");inspectorPanel->setMinimumWidth(250);inspectorPanel->setMaximumWidth(420);auto *inspectorLayout=new QVBoxLayout(inspectorPanel);inspectorLayout->setContentsMargins(10,10,10,8);inspectorLayout->setSpacing(10);
        auto *inspectorHeading=new QLabel("INSPECTOR");inspectorHeading->setObjectName("surfaceHeading");inspectorLayout->addWidget(inspectorHeading);
        taskSurface=new ElidedLabel;taskSurface->setObjectName("taskSurface");inspectorLayout->addWidget(taskSurface);
        contentPreview=new QLabel;contentPreview->setObjectName("contentPreview");contentPreview->setAlignment(Qt::AlignCenter);contentPreview->setFixedHeight(116);contentPreview->setMinimumWidth(0);contentPreview->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Fixed);inspectorLayout->addWidget(contentPreview);
        contentTitle=new ElidedLabel;contentTitle->setObjectName("contentTitle");inspectorLayout->addWidget(contentTitle);
        tasks=new QTabWidget;tasks->setObjectName("taskTabs");tasks->tabBar()->setStyleSheet("QTabBar::tab{padding:9px 5px;min-width:0px;}");inspectorLayout->addWidget(tasks,1);
        auto *details=new QWidget;auto *detailLayout=new QVBoxLayout(details);detailLayout->setContentsMargins(0,0,6,0);detailLayout->setSpacing(10);
        mediaSection=new InspectorSection("Media");detailLayout->addWidget(mediaSection);auto *mediaLayout=mediaSection->content;
        mediaLabel=new ElidedLabel;mediaLabel->setStyleSheet("color:#b9c5cd;");mediaLayout->addWidget(mediaLabel);
        auto *mediaActions=new QHBoxLayout;
        mediaActions->addWidget(button("Load media…",[this]{loadMedia();}));
        clearButton=button("Clear media",[this]{scene.clearMedia(scene.selected);});clearButton->setObjectName("clearMediaButton");clearButton->setToolTip("Return this surface to the white grid. Keeps your mesh and original file. Undo restores the content.");mediaActions->addWidget(clearButton);mediaLayout->addLayout(mediaActions);
        videoControls=new QWidget;videoControls->setObjectName("videoControls");auto *videoLayout=new QVBoxLayout(videoControls);videoLayout->setContentsMargins(0,0,0,0);videoLayout->setSpacing(8);mediaLayout->addWidget(videoControls);
        auto *playButtons=new QHBoxLayout;
        playButton=button("Play / pause",[this]{if(auto *m=selectedMedia();m&&m->player){if(m->player->playbackState()==QMediaPlayer::PlayingState)m->player->pause();else m->player->play();}});playButtons->addWidget(playButton);
        restartButton=button("Restart",[this]{if(auto *m=selectedMedia();m&&m->player){m->player->setPosition(0);m->player->play();}});playButtons->addWidget(restartButton);videoLayout->addLayout(playButtons);
        seek=new QSlider(Qt::Horizontal);seek->setRange(0,1000);videoLayout->addWidget(seek);
        connect(seek,&QSlider::sliderReleased,this,[this]{if(auto *m=selectedMedia();m&&m->player)m->player->setPosition(m->player->duration()*seek->value()/1000);});
        mute=new QCheckBox("Mute audio");mute->setChecked(true);videoLayout->addWidget(mute);
        connect(mute,&QCheckBox::toggled,this,[this](bool v){if(auto *m=selectedMedia();m&&m->audio)m->audio->setMuted(v);});
        auto *appearance=new InspectorSection("Surface appearance");appearance->heading->setChecked(false);detailLayout->addWidget(appearance);
        auto appearanceControl=[this,appearance](const QString &name,QSlider *&slider,QLabel *&value){
            auto *row=new QHBoxLayout;auto *label=new QLabel(name);label->setMinimumWidth(64);row->addWidget(label);
            slider=new QSlider(Qt::Horizontal);slider->setMinimumWidth(60);slider->setObjectName(name.toLower()+"Slider");slider->setRange(0,100);slider->setValue(100);slider->setAccessibleName(name);row->addWidget(slider,1);
            value=new QLabel("100 %");value->setMinimumWidth(42);value->setAlignment(Qt::AlignRight);row->addWidget(value);appearance->content->addLayout(row);
            const bool dim=name=="Brightness";
            connect(slider,&QSlider::sliderPressed,this,[slider]{slider->setProperty("appearanceEdit",false);});
            connect(slider,&QSlider::valueChanged,this,[this,slider,dim](int v){if(auto *s=scene.current()){
                const bool checkpoint=!slider->isSliderDown()||!slider->property("appearanceEdit").toBool();
                scene.setAppearance(scene.selected,dim?v:s->brightness,dim?s->opacity:v,checkpoint);slider->setProperty("appearanceEdit",true);
            }});
        };
        appearanceControl("Brightness",brightness,brightnessValue);appearanceControl("Opacity",opacity,opacityValue);
        brightness->setToolTip("Dim this surface. 100% keeps its original brightness.");opacity->setToolTip("Fade this surface to reveal layers underneath. 0% is transparent.");
        auto *blendForm=new QFormLayout;blendPicker=new QComboBox;blendPicker->setObjectName("blendPicker");blendPicker->addItems({"Normal","Screen","Add"});blendForm->addRow("Blend",blendPicker);appearance->content->addLayout(blendForm);
        blendPicker->setToolTip("Normal covers lower layers. Screen and Add keep black transparent over other surfaces; Add can reach white more quickly.");
        connect(blendPicker,&QComboBox::currentIndexChanged,this,[this](int index){scene.setBlend(scene.selected,index);});
        resetAppearance=button("Reset appearance",[this]{scene.resetAppearance(scene.selected);});resetAppearance->setObjectName("resetAppearanceButton");appearance->content->addWidget(resetAppearance);
        fxSection=new InspectorSection("Potato FX");detailLayout->addWidget(fxSection);
        auto *scope=new QLabel("This surface · media or built-in animation");scope->setObjectName("scopeLabel");scope->setWordWrap(true);fxSection->content->addWidget(scope);
        auto *fxForm=new QFormLayout;patternPicker=new QComboBox;patternPicker->setObjectName("animationPicker");for(int id=0;id<potatoPatterns().size();++id)patternPicker->addItem(potatoPatterns().at(id),id);patternPicker->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);patternPicker->setMinimumContentsLength(8);fxForm->addRow("Animation",patternPicker);fxSection->content->addLayout(fxForm);
        connect(patternPicker,&QComboBox::currentIndexChanged,this,[this](int index){if(index>=0)scene.setPattern(scene.selected,patternPicker->itemData(index).toInt());});
        auto *browseButton=button("Browse animations…",[this]{showAnimationBrowser();});browseButton->setObjectName("browseAnimations");fxSection->content->addWidget(browseButton);
        browserSection=new InspectorSection("Library");browserSection->setObjectName("animationBrowser");browserSection->heading->hide();browserSection->setMinimumHeight(176);
        auto *libraryBar=new QHBoxLayout;auto *libraryHeading=new QLabel("LIBRARY");libraryHeading->setObjectName("surfaceHeading");libraryBar->addWidget(libraryHeading);
        animationSearch=new QLineEdit;animationSearch->setObjectName("animationSearch");animationSearch->setPlaceholderText("Search content…");animationSearch->setClearButtonEnabled(true);animationSearch->setAccessibleName("Search animations");animationSearch->setMaximumWidth(280);libraryBar->addWidget(animationSearch,1);
        animationCategory=new QComboBox;animationCategory->setObjectName("animationCategory");animationCategory->setAccessibleName("Animation category");
        for(const auto &entry:QList<QPair<QString,QString>>{{"All","all"},{"Patterns","patterns"},{"Autumn","autumn"},{"Halloween","halloween"},{"Christmas","christmas"}})animationCategory->addItem(entry.first,entry.second);libraryBar->addWidget(animationCategory);
        browserStatus=new QLabel;browserStatus->setObjectName("browserStatus");browserStatus->setMinimumWidth(0);browserStatus->setMaximumWidth(120);browserStatus->setMaximumHeight(34);browserStatus->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Preferred);libraryBar->addWidget(browserStatus,1);
        applyAnimation=button("Apply to surface",[this]{if(auto *item=animationList->currentItem())scene.setPattern(scene.selected,item->data(Qt::UserRole).toInt());});applyAnimation->setObjectName("applyAnimation");applyAnimation->setIcon(studioIcon("add"));libraryBar->addWidget(applyAnimation);browserSection->content->addLayout(libraryBar);
        animationList=new StudioLibraryWidget;animationList->setObjectName("animationLibrary");animationList->setMinimumHeight(90);animationList->setViewMode(QListView::IconMode);animationList->setItemDelegate(new StudioLibraryDelegate(animationList));animationList->setIconSize({160,90});animationList->setGridSize({178,124});animationList->setResizeMode(QListView::Adjust);animationList->setMovement(QListView::Static);animationList->setWrapping(false);animationList->setFlow(QListView::LeftToRight);animationList->setWordWrap(false);animationList->setTextElideMode(Qt::ElideRight);animationList->setSpacing(4);animationList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);animationList->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);animationList->setDragEnabled(true);animationList->setDragDropMode(QAbstractItemView::DragOnly);browserSection->content->addWidget(animationList,1);
        connect(animationSearch,&QLineEdit::textChanged,this,[this]{filterAnimations();});connect(animationCategory,&QComboBox::currentIndexChanged,this,[this]{filterAnimations();});
        connect(animationList,&QListWidget::currentRowChanged,this,[this]{applyAnimation->setEnabled(scene.current()&&animationList->currentItem());});filterAnimations();
        patternControls=new QWidget;patternControls->setObjectName("patternControls");auto *patternForm=new QFormLayout(patternControls);patternForm->setContentsMargins(0,0,0,0);
        patternSpeed=new QSpinBox;patternSpeed->setRange(0,300);patternSpeed->setSuffix(" %");patternSpeed->setSingleStep(10);patternForm->addRow("Speed",patternSpeed);
        patternSize=new QSpinBox;patternSize->setObjectName("patternSize");patternSize->setRange(5,45);patternSize->setSuffix(" %");patternSizeLabel=new QLabel("Dot size");patternForm->addRow(patternSizeLabel,patternSize);
        patternSpeed->setObjectName("patternSpeed");
        auto *advancedFx=new InspectorSection("More controls");advancedFx->heading->setChecked(false);auto *advancedForm=new QFormLayout;advancedForm->setContentsMargins(0,0,0,0);advancedFx->content->addLayout(advancedForm);
        auto fxSpin=[advancedForm](const QString &label,const QString &id,int min,int max){auto *spin=new QSpinBox;spin->setRange(min,max);spin->setObjectName(id);advancedForm->addRow(label,spin);return spin;};
        patternDensity=fxSpin("Density","patternDensity",4,80);patternDensity->setToolTip("Pattern density. Higher values add finer or more closely spaced detail; adjust size or line width separately.");
        patternFlow=fxSpin("Flow","patternFlow",0,100);patternFlow->setSuffix(" %");patternFlow->setToolTip("Amount of variation in the flowing cell effects.");
        patternAngle=fxSpin("Angle","patternAngle",-180,180);patternAngle->setSuffix(" °");
        patternPalette=new QComboBox;patternPalette->setObjectName("patternPalette");patternPalette->addItems({"Original palette","White","Cyan","Purple","Amber","Mint"});advancedForm->addRow("Colour",patternPalette);
        patternEdge=fxSpin("Soft edge","patternEdge",0,25);patternEdge->setSuffix(" %");patternEdge->setToolTip("Optional fade near the mapped boundary. 0% keeps the whole surface active.");
        patternReverse=new QCheckBox("Reverse movement");patternReverse->setObjectName("patternReverse");advancedForm->addRow(patternReverse);
        auto updateLook=[this]{if(auto *s=scene.current();s&&s->pattern){auto look=s->fx;look.density=patternDensity->value();look.flow=patternFlow->value();look.angle=patternAngle->value();look.palette=patternPalette->currentIndex();look.edge=patternEdge->value();look.reverse=patternReverse->isChecked();scene.setFxLook(scene.selected,look);}};
        for(auto *spin:{patternDensity,patternFlow,patternAngle,patternEdge})connect(spin,qOverload<int>(&QSpinBox::valueChanged),this,[updateLook](int){updateLook();});
        connect(patternPalette,&QComboBox::currentIndexChanged,this,[updateLook](int){updateLook();});connect(patternReverse,&QCheckBox::toggled,this,[updateLook](bool){updateLook();});
        patternPause=new QCheckBox("Pause animation");patternPause->setObjectName("patternPause");patternForm->addRow(patternPause);patternForm->addRow(advancedFx);fxSection->content->addWidget(patternControls);
        connect(patternSpeed,&QSpinBox::valueChanged,this,[this](int v){if(auto *s=scene.current();s&&s->pattern){scene.checkpoint();s->patternSpeed=v;scene.touch();}});
        connect(patternSize,&QSpinBox::valueChanged,this,[this](int v){if(auto *s=scene.current();s&&s->pattern){scene.checkpoint();s->patternSize=v;scene.touch();}});
        connect(patternPause,&QCheckBox::toggled,this,[this](bool v){if(auto *s=scene.current();s&&s->pattern){scene.checkpoint();s->patternPlaying=!v;scene.touch();}});
        dynamicSection=new DynamicPanel(&scene);
        // Content selection and its playback controls come before secondary adjustments.
        detailLayout->removeWidget(fxSection);detailLayout->insertWidget(0,fxSection);fxSection->heading->hide();scope->hide();
        inspectorLayout->removeWidget(contentPreview);inspectorLayout->removeWidget(contentTitle);detailLayout->insertWidget(0,contentTitle);detailLayout->insertWidget(0,contentPreview);
        if(auto *label=fxForm->labelForField(patternPicker)){fxForm->removeWidget(label);label->deleteLater();}fxForm->removeWidget(patternPicker);advancedForm->insertRow(0,"Animation",patternPicker);browseButton->setText("Replace from library…");browseButton->setIcon(studioIcon("library"));
        auto *mapDetails=new QWidget;auto *mapLayout=new QVBoxLayout(mapDetails);mapLayout->setContentsMargins(0,0,6,0);mapLayout->setSpacing(10);
        auto *mapping=new InspectorSection("Mapping");mapLayout->addWidget(mapping);mapping->content->addLayout(flags);auto *form=new QFormLayout;
        mode=new QComboBox;mode->setObjectName("mapEditMode");mode->addItems({"Corners","Mesh points"});form->addRow("Edit",mode);
        connect(mode,&QComboBox::currentIndexChanged,this,[this](int i){setEditingTool(i==1?EditingTool::Mesh:EditingTool::Corners);});
        subdivisions=new QSpinBox;subdivisions->setRange(1,16);subdivisions->setSuffix(" × "+QString::number(2)+" cells");form->addRow("Grid",subdivisions);
        connect(subdivisions,&QSpinBox::valueChanged,this,[this](int n){if(auto *s=scene.current();s&&!s->locked){scene.checkpoint();Surface candidate=*s;candidate.subdivide(n);if(candidate.valid()){*s=candidate;scene.touch(true);}else refresh();}});
        fit=new QComboBox;fit->addItems({"Stretch","Fit whole image","Fill / crop"});form->addRow("Media",fit);
        connect(fit,&QComboBox::currentIndexChanged,this,[this](int i){if(auto *s=scene.current()){scene.checkpoint();s->fit=i;scene.touch();}});mapping->content->addLayout(form);
        auto *resetButtons=new QHBoxLayout;
        resetButtons->addWidget(button("Reset mesh",[this]{if(auto *s=scene.current();s&&!s->locked){scene.checkpoint();s->resetMesh();scene.touch();}}));
        resetButtons->addWidget(button("Fit view",[this]{canvas->resetView();}));mapping->content->addLayout(resetButtons);
        auto *help=new QLabel("Drag corners to match your surface.\nDrag inside to move. Wheel zooms; middle mouse pans.\nArrow keys nudge; Shift moves 10 pixels.\nEsc closes projector output.");help->setWordWrap(true);help->setObjectName("canvasHint");mapping->content->addWidget(help);mapLayout->addStretch();detailLayout->addStretch();
        auto makeScroll=[](QWidget *contents){auto *scroll=new QScrollArea;scroll->setWidget(contents);scroll->setWidgetResizable(true);scroll->setFrameShape(QFrame::NoFrame);scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);scroll->setMinimumHeight(90);return scroll;};
        tasks->addTab(makeScroll(mapDetails),"Map");
        inspector=makeScroll(details);inspector->setObjectName("surfaceInspector");tasks->addTab(inspector,"Content");
        auto *connectionDetails=new QWidget;auto *connectionLayout=new QVBoxLayout(connectionDetails);connectionLayout->setContentsMargins(2,8,6,0);
        auto *connectionIntro=new QLabel("Join nearby surface edges, then let a skeleton walk their boundary. Connections keep each surface's content.");connectionIntro->setWordWrap(true);connectionLayout->addWidget(connectionIntro);
        connectionInfo=new QLabel;connectionInfo->setObjectName("connectionDescription");connectionInfo->setWordWrap(true);connectionLayout->addWidget(connectionInfo);
        fitConnection=new QCheckBox("Fit selected edge");fitConnection->setObjectName("fitConnection");fitConnection->setToolTip("Off keeps mapped positions. Fit may move, rotate and resize this surface to match the proposed edge; the confirmation shows what will change.");connectionLayout->addWidget(fitConnection);
        auto *connectionActions=new QHBoxLayout;connectButton=button("Connect…",[this]{connectSelected();});connectButton->setObjectName("connectSurface");connectionActions->addWidget(connectButton);
        disconnectButton=button("Disconnect",[this]{scene.disconnectSurface(scene.selected);});disconnectButton->setObjectName("disconnectSurface");connectionActions->addWidget(disconnectButton);connectionLayout->addLayout(connectionActions);
        tieConnections=new QCheckBox("Tie group while moving");tieConnections->setObjectName("tieConnections");connectionLayout->addWidget(tieConnections);connect(tieConnections,&QCheckBox::toggled,this,[this](bool tied){scene.setConnectionsTied(scene.selected,tied);});
        boundaryActors=new QCheckBox("Walking skeleton");boundaryActors->setObjectName("boundaryActors");connectionLayout->addWidget(boundaryActors);connect(boundaryActors,&QCheckBox::toggled,this,[this](bool enabled){scene.setBoundaryActors(enabled);});
        wizardActors=new QCheckBox("Original wizard duel");wizardActors->setObjectName("wizardActors");connectionLayout->addWidget(wizardActors);connect(wizardActors,&QCheckBox::toggled,this,[this](bool enabled){scene.setWizardActors(enabled);});
        auto *actorScope=new QLabel("Scene-wide actors · skeleton follows mapped boundaries; wizards duel on mapped surfaces. Projector output uses the same scene clock.");actorScope->setObjectName("scopeLabel");actorScope->setWordWrap(true);connectionLayout->addWidget(actorScope);connectionLayout->addStretch();tasks->addTab(makeScroll(connectionDetails),"Connections");
        auto *sceneDetails=new QWidget;auto *sceneLayout=new QVBoxLayout(sceneDetails);sceneLayout->setContentsMargins(0,0,6,0);sceneLayout->addWidget(dynamicSection);
        auto *actorSection=new InspectorSection("Whole scene actors");sceneLayout->addWidget(actorSection);auto *actorForm=new QFormLayout;
        actorSpeed=new QSpinBox;actorSpeed->setObjectName("actorSpeed");actorSpeed->setRange(10,300);actorSpeed->setSuffix(" %");actorForm->addRow("Speed",actorSpeed);
        actorSize=new QSpinBox;actorSize->setObjectName("actorSize");actorSize->setRange(50,200);actorSize->setSuffix(" %");actorForm->addRow("Size",actorSize);actorSection->content->addLayout(actorForm);connectionLayout->removeWidget(boundaryActors);connectionLayout->removeWidget(wizardActors);actorSection->content->insertWidget(0,wizardActors);actorSection->content->insertWidget(0,boundaryActors);
        connect(actorSpeed,&QSpinBox::valueChanged,this,[this](int v){scene.setActorSpeed(v);});connect(actorSize,&QSpinBox::valueChanged,this,[this](int v){scene.setActorSize(v);});
        auto *actorHint=new QLabel("Speed and size affect the walking skeleton and wizard duel. Surface animations and media keep their own controls.");actorHint->setWordWrap(true);actorSection->content->addWidget(actorHint);sceneLayout->addStretch();sceneInspector=makeScroll(sceneDetails);tasks->addTab(sceneInspector,"Scene");
        auto *canvasPanel=new QWidget;canvasPanel->setObjectName("canvasPanel");canvasPanel->setMinimumWidth(280);auto *canvasLayout=new QVBoxLayout(canvasPanel);canvasLayout->setContentsMargins(0,0,0,0);canvasLayout->setSpacing(0);
        auto *canvasTools=new QWidget;canvasToolsWidget=canvasTools;canvasTools->setObjectName("canvasTools");canvasTools->installEventFilter(this);auto *canvasToolsLayout=new QHBoxLayout(canvasTools);canvasToolsLayout->setContentsMargins(8,5,8,5);canvasToolsLayout->setSpacing(5);
        auto *toolGroup=new QButtonGroup(this);toolGroup->setExclusive(true);
        auto tool=[&,this](const QString &text,const QString &icon,std::function<void()> fn){auto *b=button(text,fn);b->setIcon(studioIcon(icon));b->setProperty("studioToolText",text);b->setToolTip(text);b->setAccessibleName(text);b->setCheckable(true);toolGroup->addButton(b);canvasToolsLayout->addWidget(b);return b;};
        selectTool=tool("Select","select",[this]{setEditingTool(EditingTool::Select);});selectTool->setObjectName("selectTool");
        cornersTool=tool("Corners","corners",[this]{setEditingTool(EditingTool::Corners);});cornersTool->setObjectName("cornersTool");
        meshTool=tool("Mesh","mesh",[this]{setEditingTool(EditingTool::Mesh);});meshTool->setObjectName("meshTool");setEditingTool(EditingTool::Corners);
        canvasConnect=button("Connect",[this]{tasks->setCurrentIndex(2);});canvasConnect->setIcon(studioIcon("connect"));canvasConnect->setProperty("studioToolText","Connect");canvasConnect->setAccessibleName("Connect");canvasConnect->setObjectName("canvasConnect");canvasConnect->setToolTip("Open connection preview and fitting controls");canvasToolsLayout->addWidget(canvasConnect);canvasToolsLayout->addStretch();
        auto *fitView=button("Fit view",[this]{canvas->resetView();});fitView->setIcon(studioIcon("fit"));fitView->setObjectName("fitView");fitView->setProperty("studioToolText","Fit view");fitView->setToolTip("Fit view [F]");canvasToolsLayout->addWidget(fitView);
        auto *zoomOut=button("−",[this]{canvas->setViewZoom(canvas->viewZoom()/1.2);});zoomOut->setToolTip("Zoom out");zoomOut->setFixedWidth(29);canvasToolsLayout->addWidget(zoomOut);
        auto *zoomIn=button("+",[this]{canvas->setViewZoom(canvas->viewZoom()*1.2);});zoomIn->setToolTip("Zoom in");zoomIn->setFixedWidth(29);canvasToolsLayout->addWidget(zoomIn);
        zoomPercent=new QSpinBox;zoomPercent->setObjectName("canvasZoom");zoomPercent->setRange(25,800);zoomPercent->setSuffix(" %");zoomPercent->setValue(100);zoomPercent->setFixedWidth(74);zoomPercent->setAccessibleName("Canvas zoom");canvasToolsLayout->addWidget(zoomPercent);
        connect(zoomPercent,&QSpinBox::valueChanged,this,[this](int value){canvas->setViewZoom(value/100.0);});connect(canvas,&Canvas::viewChanged,this,[this](double value){QSignalBlocker block(zoomPercent);zoomPercent->setValue(qRound(value*100));});
        canvasLayout->addWidget(canvasTools);canvasLayout->addWidget(canvas,1);
        auto *stageSplit=new QSplitter(Qt::Horizontal);stageSplit->setObjectName("stageSplitter");stageSplit->setHandleWidth(5);stageSplit->setChildrenCollapsible(false);stageSplit->addWidget(canvasPanel);stageSplit->addWidget(inspectorPanel);stageSplit->setStretchFactor(0,1);stageSplit->setSizes({760,280});
        librarySplit=new QSplitter(Qt::Vertical);librarySplit->setObjectName("librarySplitter");librarySplit->setHandleWidth(5);librarySplit->setChildrenCollapsible(false);librarySplit->addWidget(stageSplit);librarySplit->addWidget(browserSection);librarySplit->setStretchFactor(0,1);librarySplit->setSizes({560,196});
        auto *workspace=new QWidget;auto *workspaceLayout=new QVBoxLayout(workspace);workspaceLayout->setContentsMargins(0,0,0,0);workspaceLayout->setSpacing(0);workspaceLayout->addWidget(librarySplit,1);
        auto *transport=new QWidget;transport->setObjectName("sceneTransport");auto *transportLayout=new QHBoxLayout(transport);transportLayout->setContentsMargins(12,4,12,4);
        auto *sceneLabel=new QLabel("Whole scene actors");transportLayout->addWidget(sceneLabel);actorPause=button("Pause",[this]{scene.setActorPlayback(!scene.actorsPlaying);});actorPause->setObjectName("actorPause");actorPause->setIcon(studioIcon("pause"));transportLayout->addWidget(actorPause);
        auto *sceneControls=button("Scene controls",[this]{tasks->setCurrentIndex(3);});sceneControls->setObjectName("sceneControls");sceneControls->setIcon(studioIcon("chevron"));transportLayout->addWidget(sceneControls);transportLayout->addStretch();
        outputBadge=new QLabel("Output off");outputBadge->setObjectName("outputBadge");transportLayout->addWidget(outputBadge);workspaceLayout->addWidget(transport);
        split->addWidget(panel);split->addWidget(workspace);split->setStretchFactor(1,1);
        const auto sidebarSize=std::clamp(QSettings().value("ui/sidebarWidth",220).toInt(),180,300);split->setSizes({sidebarSize,qMax(500,width()-sidebarSize)});
        connect(split,&QSplitter::splitterMoved,this,[this,split]{if(!smoke)QSettings().setValue("ui/sidebarWidth",split->sizes().value(0));});
        connect(&scene,&Scene::structureChanged,this,[this]{refresh();});
        connect(&scene,&Scene::message,this,[this](const QString &m){statusBar()->showMessage(m,20000);});
        connect(canvas,&Canvas::graphicsInitialized,this,[this](const QString &m){statusBar()->showMessage(m,12000);});
        connect(qApp,&QGuiApplication::screenAdded,this,[this](QScreen *s){watchScreen(s);updateDisplays();});
        connect(qApp,&QGuiApplication::screenRemoved,this,[this](QScreen *s){if(activeOutputScreen==s){output->hide();activeOutputScreen.clear();statusBar()->showMessage("Projector disconnected. Reconnect it, select it, and start output again.");}watchedScreens.remove(s);updateDisplays();});
        connect(output,&Canvas::graphicsInitialized,this,[this](const QString &details){if(!output->graphicsReady){output->hide();outputInfo->setText("Output could not start: "+details);}});
        for(auto *s:QGuiApplication::screens())watchScreen(s);
        updateDisplays();
        auto *timer=new QTimer(this);connect(timer,&QTimer::timeout,this,[this]{actorPause->setText(scene.actorsPlaying?"Pause":"Play");actorPause->setIcon(studioIcon(scene.actorsPlaying?"pause":"play"));auto *m=selectedMedia();bool video=m&&m->player;seek->setEnabled(video);playButton->setEnabled(video);restartButton->setEnabled(video);if(video&&!seek->isSliderDown()){qint64 d=m->player->duration();seek->setValue(d>0?int(m->player->position()*1000/d):0);}});timer->start(250);
        auto *fileMenu=menuBar()->addMenu("File");auto *editMenu=menuBar()->addMenu("Edit");
        const auto projectActions=bar->actions();
        for(auto *a:projectActions){if(QStringList{"New","Open","Save","Save as","Load media"}.contains(a->text()))fileMenu->addAction(a);if(QStringList{"Undo","Redo","+ Surface","Potato FX","Dynamic FX"}.contains(a->text()))editMenu->addAction(a);if(QStringList{"New","Save as","Load media","+ Surface","Potato FX","Dynamic FX"}.contains(a->text())){addAction(a);bar->removeAction(a);}}
        fileMenu->addSeparator();fileMenu->addAction("Open app folder",this,[]{QDesktopServices::openUrl(QUrl::fromLocalFile(potatoInstallRoot()));});
        fileMenu->addAction("Open Projects folder",this,[]{QDesktopServices::openUrl(QUrl::fromLocalFile(PotatoProjects::directory(potatoInstallRoot())));});
        auto *recentMenu=fileMenu->addMenu("Recent projects");
        connect(recentMenu,&QMenu::aboutToShow,this,[this,recentMenu]{recentMenu->clear();for(const auto &path:PotatoProjects::recent(potatoInstallRoot())){auto *a=recentMenu->addAction(QFileInfo(path).fileName());a->setToolTip(path);connect(a,&QAction::triggered,this,[this,path]{openProject(path);});}if(recentMenu->isEmpty())recentMenu->addAction("No recent projects")->setEnabled(false);});
        auto *examplesMenu=fileMenu->addMenu("Examples");
        for(const auto &name:QStringList{"Hot-Ass Potato","Potato Focus","Maple Supper","Harvest Lanterns","Pumpkin Mischief","Ghost Parade","Rainlit Window","October Porch","Seasonal Studio","Connected Skeleton","Wizard Duel","Shared Snowfall"}){auto *example=examplesMenu->addAction(name);connect(example,&QAction::triggered,this,[this,name]{if(!openProject(QDir(QCoreApplication::applicationDirPath()).filePath("examples/"+name+".pmap")))return;if(name=="Hot-Ass Potato"||name=="Potato Focus")showDynamicControls();else if(name=="Connected Skeleton"||name=="Wizard Duel")tasks->setCurrentIndex(2);else{tasks->setCurrentIndex(1);fxSection->heading->setChecked(true);inspector->ensureWidgetVisible(fxSection);}});}
        fileMenu->addSeparator();fileMenu->addAction("Exit",this,&QWidget::close);
        auto *settingsMenu=menuBar()->addMenu("Settings");
        auto *appearanceMenu=settingsMenu->addMenu("Appearance");auto *themeGroup=new QActionGroup(this);themeGroup->setExclusive(true);
        const auto selectedTheme=PotatoAppearance::themeId(QSettings().value("ui/theme","classic1").toString());
        for(const auto &entry:QList<QPair<QString,QString>>{{"Classic 1","classic1"},{"Halloween","halloween"},{"Autumn","autumn"},{"Christmas","christmas"}}){
            auto *themeAction=appearanceMenu->addAction(entry.first);themeAction->setObjectName("theme_"+entry.second);themeAction->setData(entry.second);themeAction->setCheckable(true);themeAction->setChecked(entry.second==selectedTheme);themeGroup->addAction(themeAction);
            connect(themeAction,&QAction::triggered,this,[this,id=entry.second]{qApp->setStyleSheet(potatoStyle(id));qApp->setProperty("potatoTheme",id);if(!smoke)QSettings().setValue("ui/theme",id);});
        }
        appearanceMenu->addSeparator();auto *reduceMotion=appearanceMenu->addAction("Reduce interface motion");reduceMotion->setObjectName("reduceInterfaceMotion");reduceMotion->setCheckable(true);reduceMotion->setChecked(QSettings().value("ui/reducedMotion",false).toBool());reduceMotion->setToolTip("Editor chrome only. These themes are static; scene animations retain their own playback controls.");
        qApp->setProperty("potatoReducedMotion",reduceMotion->isChecked());connect(reduceMotion,&QAction::toggled,this,[this](bool reduced){qApp->setProperty("potatoReducedMotion",reduced);if(!smoke)QSettings().setValue("ui/reducedMotion",reduced);});
        auto *specialEvents=settingsMenu->addAction("Special events…",this,[this]{showAnimationBrowser(true);});specialEvents->setObjectName("specialEvents");
        auto *helpMenu=menuBar()->addMenu("Help");
        helpMenu->addAction("Quick guide",this,[this]{QMessageBox::information(this,"Potato Mapper — Quick guide","1. Connect your projector and extend your desktop.\n2. Add a surface and load or drop media.\n3. Drag corners or choose Mesh points to align it.\n4. Choose your projector and Start output.\n\nRight-click a surface to clear media or rename it.\nClear media keeps the mesh and supports Undo.\nSave your project; media files stay linked on disk.\n\nB: blackout   Esc: stop output   F: fit editor view");});
        helpMenu->addAction("Check for updates…",this,[this]{if(updateDialog){updateDialog->raise();updateDialog->activateWindow();return;}updateDialog=new UpdateDialog(this,[this]{return prepareRestart();},[this]{return scene.projectPath;});updateDialog->show();});
        helpMenu->addAction("Use previous app version…",this,[this]{rollback();});
        helpMenu->addAction("GitHub releases",this,[]{QDesktopServices::openUrl(QUrl("https://github.com/TheGamingDonKey/potato-mapper/releases"));});
        helpMenu->addAction("About Potato Mapper",this,[this]{QMessageBox::about(this,"About Potato Mapper","Potato Mapper "+QCoreApplication::applicationVersion()+"\nA home projection mapper for one projector.\n\nMIT licensed. Qt and FFmpeg have their own licenses.\nYour projects and media are stored locally.\nUse Help → Check for updates to get new releases.");});
        for(auto *form:tasks->findChildren<QFormLayout*>()){form->setRowWrapPolicy(QFormLayout::WrapLongRows);form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);}
        for(auto *a:{undoAction,redoAction})if(auto *b=qobject_cast<QToolButton*>(bar->widgetForAction(a)))b->setToolButtonStyle(Qt::ToolButtonIconOnly);
        connect(&scene,&Scene::changed,this,[this]{setWindowModified(scene.dirty);projectTitle->setFullText((scene.projectPath.isEmpty()?QString("Untitled mapping"):QFileInfo(scene.projectPath).fileName())+(scene.dirty?" •":""));});
        scene.add();scene.dirty=false;refresh();tasks->setCurrentIndex(1);
        statusBar()->showMessage("Add a surface, then drop an image or video onto it.");
    }
    ~MainWindow()override{delete output;}
    void showDynamicControls(){
        tasks->setCurrentIndex(3);
        dynamicSection->heading->setChecked(true);
        // Scroll after the expanded section has received its layout geometry.
        QTimer::singleShot(0,this,[this]{
            sceneInspector->widget()->layout()->activate();sceneInspector->ensureWidgetVisible(dynamicSection);
        });
    }
    bool save(bool as){
        QString path=scene.projectPath;
        if(as||path.isEmpty())path=QFileDialog::getSaveFileName(this,"Save mapping",path.isEmpty()?QDir(projectFolder()).filePath("My mapping.pmap"):path,"Potato Mapper (*.pmap *.hmap)");
        if(path.isEmpty())return false;if(!path.endsWith(".pmap",Qt::CaseInsensitive)&&!path.endsWith(".hmap",Qt::CaseInsensitive))path+=".pmap";
        QString error;if(!scene.save(path,error)){QMessageBox::warning(this,"Save failed",path+"\n\n"+error);return false;}PotatoProjects::remember(potatoInstallRoot(),path);statusBar()->showMessage("Saved "+QFileInfo(path).fileName(),5000);return true;
    }
    void open(){
        if(!canDiscard())return;auto path=QFileDialog::getOpenFileName(this,"Open mapping",projectFolder(),"Potato Mapper (*.pmap *.hmap)");if(path.isEmpty())return;
        openProject(path,false);
    }
    bool openProject(const QString &path,bool ask=true){
        if(ask&&!canDiscard())return false;
        QString error;if(!scene.load(path,error)){QMessageBox::warning(this,"Open failed",path+"\n\n"+error+"\n\nYour current mapping was kept.");return false;}
        const auto collected=PotatoProjects::collect(path,potatoInstallRoot(),error);
        if(collected.isEmpty())QMessageBox::warning(this,"Project opened — collection needs attention",error+"\n\nThe original project remains open. Use Save as to choose another location.");
        else if(collected!=path){if(!scene.load(collected,error))QMessageBox::warning(this,"Collected project could not open",collected+"\n\n"+error);}
        PotatoProjects::remember(potatoInstallRoot(),scene.projectPath);canvas->resetView();return true;
    }
    void showOutput(){
        QScreen *target=nullptr;for(auto *s:QGuiApplication::screens())if(s->name()==selectedDisplay){target=s;break;}
        if(!target){updateDisplays();return;}
        if(!smoke&&target==this->screen()&&QMessageBox::question(this,"Start output on the editor display?","This display will show the projection fullscreen. Press Esc to return to the editor.\n\nFor a separate projector, connect it and choose Windows + P → Extend.",QMessageBox::Yes|QMessageBox::Cancel,QMessageBox::Cancel)!=QMessageBox::Yes)return;
        output->hide();activeOutputScreen=target;
        const QSize size=target->size()*target->devicePixelRatio();
        if(scene.outputSize!=size){scene.outputSize=size;scene.touch();}else emit scene.changed();
        output->winId();output->windowHandle()->setScreen(target);output->setGeometry(target->geometry());
        if(entranceAutomatic->isChecked())scene.playEntrance();else scene.skipEntrance();
        output->showFullScreen();
        if(target==this->screen()){output->raise();output->activateWindow();}else{raise();activateWindow();}
        updateOutputStatus();
    }
protected:
    bool eventFilter(QObject *obj,QEvent *event)override{
        if(list&&obj==list->viewport()&&event->type()==QEvent::MouseButtonRelease){
            auto *mouse=static_cast<QMouseEvent*>(event);auto *item=list->itemAt(mouse->position().toPoint());
            if(item&&mouse->button()==Qt::LeftButton){const auto rect=list->visualItemRect(item);const int row=list->row(item);
                const bool lock=studioSurfaceIndicatorRect(rect,true).contains(mouse->position().toPoint());const bool eye=studioSurfaceIndicatorRect(rect,false).contains(mouse->position().toPoint());
                if(lock||eye){scene.select(row);scene.checkpoint();auto &surface=scene.surfaces[row];if(lock)surface.locked=!surface.locked;else surface.visible=!surface.visible;scene.touch(true);return true;}
            }
        }
        if(obj==canvasToolsWidget&&event->type()==QEvent::Resize){const bool compact=canvasToolsWidget->width()<610;for(auto *b:canvasToolsWidget->findChildren<QPushButton*>()){const auto label=b->property("studioToolText").toString();if(!label.isEmpty())b->setText(compact?QString():label);}}
        if(obj==output&&event->type()==QEvent::Hide)scene.skipEntrance();
        if(obj==output&&(event->type()==QEvent::Show||event->type()==QEvent::Hide))QTimer::singleShot(0,this,[this]{updateOutputStatus();});
        return QMainWindow::eventFilter(obj,event);
    }
    void closeEvent(QCloseEvent *e)override{if(smoke||property("potatoUpdateExit").toBool()||canDiscard()){output->close();e->accept();}else e->ignore();}
private:
    enum class EditingTool {Select,Corners,Mesh};
    QPushButton *selectTool=nullptr,*cornersTool=nullptr,*meshTool=nullptr;
    void setEditingTool(EditingTool tool){
        const QSignalBlocker modeBlock(mode);
        mode->setCurrentIndex(tool==EditingTool::Mesh?1:0);
        canvas->setProperty("selectionOnly",tool==EditingTool::Select);canvas->meshMode=tool==EditingTool::Mesh;
        if(selectTool&&cornersTool&&meshTool){const QSignalBlocker selectBlock(selectTool),cornersBlock(cornersTool),meshBlock(meshTool);selectTool->setChecked(tool==EditingTool::Select);cornersTool->setChecked(tool==EditingTool::Corners);meshTool->setChecked(tool==EditingTool::Mesh);}
        canvas->update();
    }
    QListWidget *list=nullptr;
    QWidget *canvasToolsWidget=nullptr;
    QSpinBox *zoomPercent=nullptr;
    QComboBox *mode,*fit,*displays;
    QSpinBox *subdivisions;
    QComboBox *patternPicker,*blendPicker;
    QLabel *patternSizeLabel;
    QWidget *patternControls,*videoControls;
    InspectorSection *mediaSection,*fxSection;
    DynamicPanel *dynamicSection;
    QScrollArea *inspector;
    QTabWidget *tasks;
    ElidedLabel *taskSurface,*projectTitle,*contentTitle;
    QLabel *contentPreview,*outputBadge=nullptr;
    QSplitter *librarySplit;
    QScrollArea *sceneInspector;
    QSpinBox *actorSpeed,*actorSize;
    QPushButton *actorPause;
    InspectorSection *browserSection;
    QLineEdit *animationSearch;
    QComboBox *animationCategory;
    QListWidget *animationList;
    QString librarySurfaceUid;
    int libraryAssignedPattern=-1;
    bool librarySelectionPending=false;
    QLabel *browserStatus,*connectionInfo;
    QPushButton *applyAnimation,*connectButton,*disconnectButton,*canvasConnect;
    QCheckBox *fitConnection,*tieConnections,*boundaryActors,*wizardActors;
    QSpinBox *patternSpeed,*patternSize;
    QSpinBox *patternDensity,*patternFlow,*patternAngle,*patternEdge;
    QComboBox *patternPalette;
    QCheckBox *patternReverse;
    QCheckBox *patternPause;
    QCheckBox *visible,*locked,*mute;
    ElidedLabel *mediaLabel;
    QSlider *seek,*brightness,*opacity;
    QLabel *brightnessValue,*opacityValue;
    QPushButton *resetAppearance;
    QPointer<UpdateDialog> updateDialog;
    QPushButton *clearButton=nullptr,*playButton=nullptr,*restartButton=nullptr;
    QPushButton *startOutputButton=nullptr,*stopOutputButton=nullptr,*blackoutButton=nullptr;
    QPushButton *entrancePlay=nullptr,*entranceSkip=nullptr;
    QCheckBox *entranceAutomatic=nullptr;
    QLabel *entranceInfo=nullptr;
    QLabel *outputInfo=nullptr;
    QString selectedDisplay;
    QPointer<QScreen> activeOutputScreen;
    QSet<QScreen*> watchedScreens;
    QIcon surfaceThumbnail(const Surface &surface){
        if(surface.pattern||surface.media.isEmpty())return studioThumbnail(surface.pattern);
        auto *media=scene.source(surface.media);if(media->player)return studioIcon("play");if(!media->image.isNull())return QIcon(QPixmap::fromImage(media->image.scaled(272,153,Qt::KeepAspectRatio,Qt::SmoothTransformation)));
        return studioIcon("play");
    }
    void syncLibrarySelection(){
        if(!librarySelectionPending)return;
        for(int row=0;row<animationList->count();++row)if(auto *item=animationList->item(row);item->data(Qt::UserRole).toInt()==libraryAssignedPattern){
            animationList->setCurrentItem(item);animationList->scrollToItem(item,QAbstractItemView::PositionAtCenter);librarySelectionPending=false;break;
        }
    }
    void filterAnimations(){
        const auto selected=librarySelectionPending?libraryAssignedPattern:animationList->currentItem()?animationList->currentItem()->data(Qt::UserRole).toInt():scene.current()?scene.current()->pattern:0;
        QSignalBlocker block(animationList);animationList->clear();
        const auto category=animationCategory->currentData().toString();
        for(int id=0;id<potatoPatterns().size();++id)if(PotatoAppearance::matches(id,potatoPatterns().at(id),category,animationSearch->text())){
            auto *item=new QListWidgetItem(studioThumbnail(id),potatoPatterns().at(id).section(" · ",0,0),animationList);item->setData(Qt::UserRole,id);item->setData(StudioRoles::Metadata,id>=14?QString("Native animation · %1 s loop").arg(potatoPatternPeriod(id),0,'f',0):id?QString("Native pattern"):QString("Media or alignment grid"));item->setToolTip(potatoPatterns().at(id)+"\nSelect, then Apply to surface. Browsing keeps the current content.");if(id==selected)animationList->setCurrentItem(item);
        }
        if(!animationList->currentItem()&&animationList->count())animationList->setCurrentRow(0);
        browserStatus->setText(animationList->count()?QString("%1 installed").arg(animationList->count()):QString("No results"));
        browserStatus->setToolTip(animationList->count()?QString("%1 installed animations match this search. Select one, then Apply to surface.").arg(animationList->count()):category=="christmas"?QString("No matching Christmas scenes. Try another search. The Christmas editor theme is available in Settings → Appearance."):QString("No matching animations. Try another search or category."));
        syncLibrarySelection();applyAnimation->setEnabled(scene.current()&&animationList->currentItem());
    }
    void showAnimationBrowser(bool seasonal=false){
        if(!scene.current())scene.add();tasks->setCurrentIndex(1);fxSection->heading->setChecked(true);browserSection->heading->setChecked(true);librarySplit->setSizes({qMax(240,librarySplit->height()-200),200});
        if(seasonal){animationSearch->clear();animationCategory->setCurrentIndex(animationCategory->findData("halloween"));}
        QTimer::singleShot(0,this,[this]{animationSearch->setFocus();});
    }
    void connectSelected(){
        if(!scene.current())return;const auto preview=scene.connectionPreview(scene.selected);
        if(preview.isEmpty()){QMessageBox::information(this,"Connect surfaces","No compatible nearby edge was found. Add or move another surface nearby, then try again.");return;}
        const bool fitEdge=fitConnection->isChecked();
        const auto explanation=preview+(fitEdge?"\n\nFit selected edge is enabled: the selected surface may move, rotate and resize. Undo restores its previous mapping.":"\n\nKeep existing mapped positions. Add a logical boundary crossing; no surface is resized or moved.");
        canvas->setProperty("connectionPreview",true);canvas->repaint();
        const auto answer=QMessageBox::question(this,"Preview connection",explanation+"\n\nConnect these surfaces?",QMessageBox::Yes|QMessageBox::Cancel,QMessageBox::Cancel);
        canvas->setProperty("connectionPreview",false);canvas->update();
        if(answer!=QMessageBox::Yes)return;
        QString error;if(!scene.connectNearest(scene.selected,fitEdge,error))QMessageBox::warning(this,"Connection could not be made",error);else statusBar()->showMessage("Surfaces connected. Use Disconnect to remove the link or Undo to restore the prior mapping.",7000);
    }
    MediaSource *selectedMedia(){auto *s=scene.current();return s?scene.source(s->media):nullptr;}
    bool canDiscard(){
        if(!scene.dirty)return true;
        auto answer=QMessageBox::question(this,"Save mapping?","Save your changes before continuing?",QMessageBox::Save|QMessageBox::Discard|QMessageBox::Cancel);
        return answer==QMessageBox::Discard||(answer==QMessageBox::Save&&save(false));
    }
    QString projectFolder() const {return PotatoProjects::directory(potatoInstallRoot());}
    void renameSelected(){auto *s=scene.current();if(!s)return;bool ok=false;const auto name=QInputDialog::getText(this,"Rename surface","Surface name:",QLineEdit::Normal,s->name,&ok);if(ok)scene.renameSurface(scene.selected,name);}
    void surfaceMenu(int index,const QPoint &pos){
        if(index<0||index>=scene.surfaces.size())return;scene.select(index);QMenu menu(this);
        menu.addAction("Load media…",this,[this]{loadMedia();});
        menu.addAction("Choose animation…",this,[this]{showAnimationBrowser();});
        const auto *s=scene.current();auto *clear=menu.addAction("Clear media — white grid",this,[this]{scene.clearMedia(scene.selected);});clear->setEnabled(s&&(!s->media.isEmpty()||s->pattern));
        menu.addSeparator();menu.addAction("Connect…",this,[this]{tasks->setCurrentIndex(2);connectSelected();});
        auto *disconnect=menu.addAction("Disconnect",this,[this]{scene.disconnectSurface(scene.selected);});disconnect->setEnabled(scene.hasConnections(scene.selected));
        menu.addSeparator();menu.addAction("Rename surface…",this,[this]{renameSelected();});menu.exec(pos);
    }
    bool prepareRestart(){
        if(output->isVisible()&&QMessageBox::question(this,"Stop projector output?","Stop projector output and continue with the update?",QMessageBox::Yes|QMessageBox::Cancel,QMessageBox::Cancel)!=QMessageBox::Yes)return false;
        if(!canDiscard())return false;output->hide();updateOutputStatus();return true;
    }
    void rollback(){
        if(scene.json()["version"].toInt()>1){QMessageBox::information(this,"Keep the newer project safe","This project uses the newer format. Close it with File → New before changing app versions. The old app can open its separate .v1-recovery.pmap copy; it cannot open this newer project.");return;}
        if(PotatoDistribution::isStorePackage()){QMessageBox::information(this,"Store updates","This edition is maintained by Microsoft Store. Use the direct-download edition for retained portable runtimes.");return;}
        const auto root=potatoInstallRoot();if(!QFileInfo::exists(QDir(root).filePath("previous.txt"))){QMessageBox::information(this,"Previous version","There is no previous app version in this folder yet.");return;}
        if(QMessageBox::question(this,"Use previous app version?","Reopen the previous app version? This does not undo edits to saved projects.",QMessageBox::Yes|QMessageBox::Cancel,QMessageBox::Cancel)!=QMessageBox::Yes||!prepareRestart())return;
        QStringList args{"--rollback","--wait-pid",QString::number(QCoreApplication::applicationPid())};
        if(!scene.projectPath.isEmpty()){
            QFile saved(scene.projectPath);const bool compatible=saved.open(QIODevice::ReadOnly)&&QJsonDocument::fromJson(saved.readAll()).object()["version"].toInt()==1;
            if(compatible)args<<"--project"<<scene.projectPath;
            else QMessageBox::information(this,"Newer project retained","The previous app will open without this project because its saved format is newer. The project and its recovery copies remain unchanged.");
        }
        if(!QProcess::startDetached(QDir(root).filePath("PotatoMapper.exe"),args,root)){QMessageBox::warning(this,"Previous version","Could not start the launcher. Open the app through PotatoMapper.exe in the installation folder.");return;}
        setProperty("potatoUpdateExit",true);QCoreApplication::quit();
    }
    void loadMedia(){
        auto files=QFileDialog::getOpenFileNames(this,"Choose images or videos",{},"Media (*.png *.jpg *.jpeg *.bmp *.webp *.mp4 *.mov *.mkv *.avi *.webm);;All files (*)");
        bool first=true;for(const auto &path:files){if(!first||!scene.current())scene.add();scene.assignMedia(scene.selected,path);first=false;}
    }
    void updateDisplays(){
        QSignalBlocker block(displays);displays->clear();displays->addItem("Choose your projector / display…",QString());
        const auto screens=QGuiApplication::screens();int chosen=0,external=0;
        for(int i=0;i<screens.size();i++){
            auto *s=screens[i];const QSize pixels=s->size()*s->devicePixelRatio();QString name=s->model().trimmed();if(name.isEmpty())name=s->name();
            const QString label=QString("Display %1 · %2 · %3 × %4%5").arg(i+1).arg(name).arg(pixels.width()).arg(pixels.height()).arg(s==this->screen()?" (editor screen)":"");
            displays->addItem(label,s->name());if(s->name()==selectedDisplay)chosen=i+1;if(!external&&s!=this->screen())external=i+1;
        }
        if(!chosen&&external)chosen=external;displays->setCurrentIndex(chosen);selectedDisplay=displays->currentData().toString();updateOutputStatus();
    }
    void watchScreen(QScreen *s){
        if(watchedScreens.contains(s))return;watchedScreens.insert(s);
        connect(s,&QScreen::geometryChanged,this,[this,s]{if(output->isVisible()&&activeOutputScreen==s){output->setGeometry(s->geometry());scene.outputSize=s->size()*s->devicePixelRatio();scene.touch();}updateDisplays();});
    }
    void updateOutputStatus(){
        if(!outputInfo)return;const bool live=output->isVisible();startOutputButton->setEnabled(!selectedDisplay.isEmpty());stopOutputButton->setEnabled(live);
        {QSignalBlocker block(blackoutButton);blackoutButton->setChecked(scene.blackout);}
        if(live&&activeOutputScreen){outputInfo->setText(QString("%1 · %2 · %3 × %4 — Esc stops output. B toggles blackout.").arg(scene.blackout?"BLACKOUT":"LIVE").arg(activeOutputScreen->name()).arg(scene.outputSize.width()).arg(scene.outputSize.height()));outputInfo->setStyleSheet(QString("color:%1;font-size:12px;padding:2px;").arg(scene.blackout?"#ffce85":"#91efd1"));}
        else{
            outputInfo->setStyleSheet("color:#b5c6d1;font-size:12px;padding:2px;");
            if(QGuiApplication::screens().size()<2)outputInfo->setText("OUTPUT OFF · One display detected. Connect projector → Windows + P → Extend. Details has help.");
            else outputInfo->setText("OUTPUT OFF · Select a projector and Start output. The canvas is an editor preview.");
        }
        outputInfo->setToolTip(outputInfo->text());
        if(outputBadge){outputBadge->setText(live?(scene.blackout?"Blackout":"Output live"):"Output off");outputBadge->setToolTip(outputInfo->text());}
    }
    void updateEntranceStatus(){
        if(!entrancePlay)return;
        entrancePlay->setEnabled(std::any_of(scene.surfaces.begin(),scene.surfaces.end(),[](const Surface &s){return s.visible&&s.opacity>0;}));
        entranceSkip->setEnabled(scene.entranceActive);
        const auto label=scene.entranceActive?(scene.blackout?QString("Paused for blackout"):PotatoEntrance::stage(scene.entranceElapsed)):QString("8-second mapping reveal");
        if(entranceInfo->text()!=label)entranceInfo->setText(label);
    }
    void refresh(){
        QSignalBlocker b1(list),b2(visible),b3(locked),b4(subdivisions),b5(fit),b6(mute),b7(patternSpeed),b8(patternSize),b9(patternPause),b10(patternPicker),b11(brightness),b12(opacity),b13(blendPicker),b14(patternDensity),b15(patternFlow),b16(patternAngle),b17(patternPalette),b18(patternEdge),b19(patternReverse);
        const int listScroll=list->verticalScrollBar()->value();const auto previousId=list->currentItem()?list->currentItem()->data(Qt::UserRole).toString():QString();
        QStringList previousOrder,nextOrder;for(int i=0;i<list->count();++i)previousOrder<<list->item(i)->data(Qt::UserRole).toString();
        list->clear();for(const auto &s:scene.surfaces){const auto content=scene.dynamicSurface(s.id)?QString("Dynamic FX · ")+(scene.dynamic.effect==1?"Hot-Ass Potato":"Potato Focus"):s.pattern?potatoPatterns().value(s.pattern):s.media.isEmpty()?QString("White grid"):QFileInfo(s.media).fileName();
            const auto thumb=surfaceThumbnail(s);const auto bounds=s.corners.boundingRect();
            const auto dimensions=QString("%1 × %2").arg(qRound(bounds.width()*scene.outputSize.width())).arg(qRound(bounds.height()*scene.outputSize.height()));
            const auto shortContent=content.section(" · ",0,0);const QString type=scene.dynamicSurface(s.id)?"Dynamic FX":s.pattern?"Animation":s.media.isEmpty()?"Grid":scene.source(s.media)->player?"Video":"Image";
            const auto metadata=s.name.compare(shortContent,Qt::CaseInsensitive)==0?dimensions+" · "+type:shortContent+" · "+dimensions;
            auto *item=new QListWidgetItem(thumb,s.name+"\n"+metadata,list);item->setData(Qt::UserRole,s.id);item->setData(StudioRoles::Visible,s.visible);item->setData(StudioRoles::Locked,s.locked);nextOrder<<s.id;item->setToolTip(s.name+"\n"+(s.media.isEmpty()?content:s.media)+"\nMapped bounds: "+dimensions+" output pixels\nEye: show/hide · Lock: lock mapping");}list->setCurrentRow(scene.selected);
        if(list->currentItem()){if(previousId==list->currentItem()->data(Qt::UserRole).toString()&&previousOrder==nextOrder)list->verticalScrollBar()->setValue(listScroll);else list->scrollToItem(list->currentItem());}
        auto *s=scene.current();clearButton->setEnabled(s&&(!s->media.isEmpty()||s->pattern));visible->setEnabled(s);locked->setEnabled(s);subdivisions->setEnabled(s&&!s->locked);fit->setEnabled(s);
        if(s&&(librarySurfaceUid!=s->routeUid||libraryAssignedPattern!=s->pattern)){librarySurfaceUid=s->routeUid;libraryAssignedPattern=s->pattern;librarySelectionPending=true;syncLibrarySelection();}
        taskSurface->setFullText(s?s->name:"No surface selected");
        contentTitle->setFullText(s?(s->pattern?potatoPatterns().value(s->pattern):s->media.isEmpty()?QString("White grid"):QFileInfo(s->media).fileName()):QString("Select a surface"));
        contentPreview->setPixmap(s?surfaceThumbnail(*s).pixmap(272,153):QPixmap());
        projectTitle->setFullText((scene.projectPath.isEmpty()?QString("Untitled mapping"):QFileInfo(scene.projectPath).fileName())+(scene.dirty?" •":""));
        {QSignalBlocker speedBlock(actorSpeed),sizeBlock(actorSize);actorSpeed->setValue(scene.actorSpeed);actorSize->setValue(scene.actorSize);}
        actorPause->setText(scene.actorsPlaying?"Pause":"Play");actorPause->setIcon(studioIcon(scene.actorsPlaying?"pause":"play"));
        {QSignalBlocker tieBlock(tieConnections),actorBlock(boundaryActors),wizardBlock(wizardActors);connectionInfo->setText(s?scene.connectionDescription(scene.selected):"Select a surface to see its connections.");tieConnections->setChecked(s&&scene.connectionsTied(scene.selected));boundaryActors->setChecked(scene.boundaryActorsEnabled());wizardActors->setChecked(scene.wizardActorsEnabled());}
        connectButton->setEnabled(s&&scene.surfaces.size()>1);canvasConnect->setEnabled(s&&scene.surfaces.size()>1);disconnectButton->setEnabled(s&&scene.hasConnections(scene.selected));tieConnections->setEnabled(s&&scene.hasConnections(scene.selected));fitConnection->setEnabled(s&&!s->locked);if(s&&s->locked)fitConnection->setChecked(false);applyAnimation->setEnabled(s&&animationList->currentItem());
        patternPicker->setEnabled(s);if(s&&s->pattern&&patternPicker->currentData().toInt()!=s->pattern)fxSection->heading->setChecked(true);patternPicker->setCurrentIndex(patternPicker->findData(s?s->pattern:0));patternControls->setVisible(s&&s->pattern);
        brightness->setEnabled(s);opacity->setEnabled(s);blendPicker->setEnabled(s);blendPicker->setCurrentIndex(s?s->blend:0);patternSizeLabel->setText(potatoPatternSizeLabel(s?s->pattern:0));resetAppearance->setEnabled(s&&(s->brightness!=100||s->opacity!=100||s->blend));
        if(auto *patternForm=patternDensity->parentWidget()->findChild<QFormLayout*>()){
            if(auto *label=qobject_cast<QLabel*>(patternForm->labelForField(patternDensity)))label->setText(s&&s->pattern==17?"Ghost count":potatoPatternDensityLabel(s?s->pattern:0));
            if(auto *label=qobject_cast<QLabel*>(patternForm->labelForField(patternFlow)))label->setText(s&&s->pattern==17?"Flutter":potatoPatternFlowLabel(s?s->pattern:0));
        }
        patternDensity->setToolTip(s&&s->pattern==17?"Ghost count control: larger values add followers (two to four ghosts).":"Amount of detail or elements in this scene.");
        patternFlow->setToolTip(s&&s->pattern==17?"Flutter and expressive movement. Basic walking continues at 0%.":"Amount of variation or expressive movement in this scene.");
        brightness->setValue(s?s->brightness:100);opacity->setValue(s?s->opacity:100);brightnessValue->setText(QString::number(brightness->value())+" %");opacityValue->setText(QString::number(opacity->value())+" %");
        if(s){patternDensity->setValue(s->fx.density);patternFlow->setValue(s->fx.flow);patternAngle->setValue(s->fx.angle);patternPalette->setCurrentIndex(s->fx.palette);patternEdge->setValue(s->fx.edge);patternReverse->setChecked(s->fx.reverse);}
        const bool flowing=s&&s->pattern>=5;patternFlow->setEnabled(flowing);patternReverse->setEnabled(s&&s->pattern);patternDensity->setEnabled(s&&s->pattern);
        if(s){patternSpeed->setValue(s->patternSpeed);patternSize->setValue(s->patternSize);patternPause->setChecked(!s->patternPlaying);visible->setChecked(s->visible);locked->setChecked(s->locked);subdivisions->setValue(s->cells);subdivisions->setSuffix(" × "+QString::number(s->cells)+" cells");fit->setCurrentIndex(s->fit);fit->setEnabled(!s->pattern&&!s->media.isEmpty());mediaLabel->setFullText(s->pattern?potatoPatterns().value(s->pattern)+" · Potato FX":s->media.isEmpty()?"White grid — drop media here":QFileInfo(s->media).fileName(),s->media);auto *m=scene.source(s->media);videoControls->setVisible(m->player&&!s->pattern);mute->setEnabled(m->audio);mute->setChecked(!m->audio||m->audio->isMuted());}
        else {mediaLabel->setFullText("Add a surface to begin");mute->setEnabled(false);videoControls->hide();}
        setWindowTitle("Potato Mapper"+(scene.projectPath.isEmpty()?QString():" — "+QFileInfo(scene.projectPath).fileName())+" [*]");setWindowModified(scene.dirty);
    }
};

static void logMessage(QtMsgType,const QMessageLogContext &,const QString &message){
    const auto logRoot=PotatoDistribution::isStorePackage()?QStandardPaths::writableLocation(QStandardPaths::CacheLocation):potatoInstallRoot();QDir().mkpath(logRoot);QFile log(logRoot+"/mapper.log");if(log.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text)){QTextStream out(&log);out<<message<<Qt::endl;}
}
int main(int argc,char **argv){
    QSurfaceFormat format;format.setVersion(3,3);format.setProfile(QSurfaceFormat::CoreProfile);format.setSwapInterval(1);format.setDepthBufferSize(0);format.setStencilBufferSize(8);QSurfaceFormat::setDefaultFormat(format);
    QApplication app(argc,argv);app.setApplicationName("Potato Mapper");app.setOrganizationName("PotatoMapper");app.setApplicationVersion(POTATO_VERSION);app.setWindowIcon(QIcon(":/assets/potato-mapper.png"));
    qInstallMessageHandler(logMessage);app.setStyle("Fusion");
    const auto theme=PotatoAppearance::themeId(QSettings().value("ui/theme","classic1").toString());app.setStyleSheet(potatoStyle(theme));app.setProperty("potatoTheme",theme);
    auto args=app.arguments();const bool diagnostic=std::any_of(args.begin(),args.end(),[](const QString &a){return a.startsWith("--smoke")||a.startsWith("--test-")||a=="--profile-media"||a=="--profile-fx"||a=="--check-project"||a=="--seasonal-check";});
    if(args.contains("--check-project")){const int i=args.indexOf("--check-project");Scene scene;QString error;const bool ok=i+1<args.size()&&scene.load(args[i+1],error);qInfo()<<"PROJECT LOAD"<<ok<<"surfaces"<<scene.surfaces.size()<<error;return ok?0:2;}
    QString maintenanceNotice;
    std::unique_ptr<QLockFile> runtimeLock;
    if(!diagnostic&&!PotatoDistribution::isStorePackage()&&QFileInfo::exists(QDir(potatoInstallRoot()).filePath("installation.txt"))){runtimeLock=std::make_unique<QLockFile>(QDir(potatoInstallRoot()).filePath(".potato-runtime.lock"));runtimeLock->setStaleLockTime(0);if(!runtimeLock->tryLock()){QMessageBox::information(nullptr,"Potato Mapper","Potato Mapper is already open in this installation. Switch to its window to continue.");return 0;}}
    if(!diagnostic&&!PotatoDistribution::isStorePackage()){
        QString error;PotatoProjects::organizeLegacy(potatoInstallRoot(),error);if(!error.isEmpty())maintenanceNotice="Project organization: "+error;
        if(!requestPotatoLauncherRefresh(potatoInstallRoot(),&error))maintenanceNotice+="\n"+error;
        QFile refreshReport(QDir(potatoInstallRoot()).filePath(".maintenance/launcher-refresh-error.txt"));if(refreshReport.open(QIODevice::ReadOnly))maintenanceNotice+="\n"+QString::fromUtf8(refreshReport.readAll());
    }
    if(args.contains("--test-splash")){
        auto *splash=potatoSplash();splash->show();QTimer::singleShot(250,splash,[&app,args,splash]{
            auto frame=splash->grab();bool ok=!frame.isNull()&&splash->isVisible();
            if(auto *screen=splash->screen())ok=ok&&(splash->geometry().center()-screen->availableGeometry().center()).manhattanLength()<=2;
            int i=args.indexOf("--snapshot");if(i>=0&&i+1<args.size())ok=ok&&frame.save(args[i+1]);
            qInfo()<<"SPLASH CHECK"<<ok<<"DPI scale"<<splash->devicePixelRatioF();app.exit(ok?0:2);
        });return app.exec();
    }
    QSplashScreen *splash=nullptr;if(!diagnostic&&!args.contains("--no-splash")){splash=potatoSplash();splash->show();app.processEvents();}
    MainWindow window;window.smoke=qEnvironmentVariableIsSet("POTATO_VERIFICATION_SESSION");window.show();
    if(!maintenanceNotice.trimmed().isEmpty())QTimer::singleShot(1500,&window,[&window,maintenanceNotice]{window.statusBar()->showMessage(maintenanceNotice.trimmed(),30000);});
    if(splash)QTimer::singleShot(args.contains("--preview-splash")?10000:1100,&window,[splash,&window]{splash->finish(&window);splash->deleteLater();});
    const int projectArgument=args.indexOf("--project");
    if(projectArgument>=0&&projectArgument+1<args.size())window.openProject(args[projectArgument+1],false);
    else if(!diagnostic)for(int i=1;i<args.size();++i)if(args[i].endsWith(".pmap",Qt::CaseInsensitive)||args[i].endsWith(".hmap",Qt::CaseInsensitive)){window.openProject(args[i],false);break;}
    const int mediaArgument=args.indexOf("--media");
    if(mediaArgument>=0&&mediaArgument+1<args.size())window.scene.assignMedia(0,args[mediaArgument+1]);
    if(args.contains("--smoke-dynamic"))runDynamicChecks(window,app,args);
    if(args.contains("--smoke-entrance"))runEntranceChecks(window,app,args);
    if(args.contains("--test-dynamic-motion"))runDynamicChecks(window,app,args);
    if(args.contains("--smoke-fx")){window.smoke=true;runFxChecks(window,app,args);}
    if(args.contains("--smoke-harvest")||args.contains("--test-harvest-motion")){window.smoke=true;runHarvestChecks(window,app,args);}
    if(args.contains("--smoke-fall")||args.contains("--test-fall-motion")){window.smoke=true;runFallChecks(window,app,args);}
    if(args.contains("--profile-fx")){window.smoke=true;profileFx(window,app,args);}
    if(args.contains("--test-fx-motion")){window.smoke=true;const int i=args.indexOf("--frames"),batch=args.indexOf("--first-pattern");renderFxMotion(window,app,i>=0&&i+1<args.size()?args[i+1]:QString(),batch>=0&&batch+1<args.size()?args[batch+1].toInt():5);}
    if(args.contains("--smoke-appearance")){window.smoke=true;runAppearanceChecks(window,app,args);}
    if(args.contains("--smoke-themes")){window.smoke=true;runAppearanceThemeChecks(window,app,args);}
    if(args.contains("--smoke-connections")){window.smoke=true;runConnectionActorChecks(window,app,args);}
    if(args.contains("--smoke-studio")){window.smoke=true;runStudioChecks(window,app,args);}
    if(args.contains("--seasonal-check")){window.smoke=true;runSeasonalChecks(window,app,args);}
    if(args.contains("--test-restart")){
        window.smoke=true;QTimer::singleShot(300,&window,[&window,&app]{
            auto answer=[](QMessageBox::StandardButton choice){QTimer::singleShot(0,[choice]{for(auto *w:QApplication::topLevelWidgets())if(auto *box=qobject_cast<QMessageBox*>(w))if(auto *b=box->button(choice))b->click();});};
            window.scene.dirty=true;const auto before=window.scene.json();answer(QMessageBox::Cancel);
            const bool cancelled=!window.prepareRestart()&&window.scene.dirty&&window.scene.json()==before;
            window.output->resize(320,180);window.output->show();answer(QMessageBox::Cancel);
            const bool projecting=!window.prepareRestart()&&window.output->isVisible();
            QTemporaryDir temp;window.scene.projectPath=temp.filePath("saved.pmap");window.output->hide();answer(QMessageBox::Save);
            const bool saved=window.prepareRestart()&&!window.scene.dirty&&QFileInfo::exists(window.scene.projectPath);
            qInfo()<<"RESTART CHECK unsaved cancel"<<cancelled<<"active output cancel"<<projecting<<"save before restart"<<saved;app.exit(cancelled&&projecting&&saved?0:2);
        });
    }
    if(args.contains("--smoke-clear")){
        window.smoke=true;QTimer::singleShot(650,&window,[&window,&app]{
            QTemporaryDir temp;QImage image(64,64,QImage::Format_RGBA8888);image.fill(Qt::blue);const auto media=temp.filePath("clip.png");image.save(media);
            window.scene.assignMedia(0,media);window.scene.surfaces[0].mesh[4]=QPointF(.55,.45);window.scene.add();window.scene.surfaces[1].pattern=1;window.scene.select(0);
            const auto before=window.scene.json();auto *button=window.findChild<QPushButton*>("clearMediaButton");bool ok=button&&button->isEnabled();if(button)button->click();
            auto cleared=window.scene.json();ok=ok&&window.scene.surfaces[0].media.isEmpty()&&window.scene.surfaces[0].pattern==0&&window.scene.surfaces[0].mesh[4]==QPointF(.55,.45)&&window.scene.surfaces[1].pattern==1;
            window.scene.undo();ok=ok&&before==window.scene.json();window.scene.redo();ok=ok&&cleared==window.scene.json();
            QString error;ok=ok&&window.scene.save(temp.filePath("cleared.pmap"),error)&&window.scene.load(temp.filePath("cleared.pmap"),error)&&cleared==window.scene.json()&&QFileInfo::exists(media);
            window.scene.clearMedia(1);ok=ok&&!window.scene.surfaces[1].pattern;window.scene.undo();ok=ok&&window.scene.surfaces[1].pattern==1;
            qInfo()<<"CLEAR CHECK"<<ok<<error;app.exit(ok?0:2);
        });
    }
    if(args.contains("--profile-media")){
        window.smoke=true;window.scene.newProject();
        for(int i=args.indexOf("--profile-media")+1;i<args.size();++i){if(i>args.indexOf("--profile-media")+1)window.scene.add();window.scene.assignMedia(window.scene.selected,args[i]);}
        window.output->resize(1280,720);window.output->show();
        QTimer::singleShot(3000,&window,[&window,&app]{
            auto elapsed=std::make_shared<QElapsedTimer>();elapsed->start();
            QVector<quint64> frames;QVector<qint64> costs;
            for(const auto &s:window.scene.surfaces){auto *m=window.scene.source(s.media);frames<<m->revision;costs<<m->conversionNs;}
            const auto paints=window.output->paintedFrames;
            QTimer::singleShot(6000,&window,[&window,&app,elapsed,frames,costs,paints]{
                const double seconds=elapsed->elapsed()/1000.0;bool ok=window.output->graphicsReady;
                for(int i=0;i<frames.size();++i){auto *m=window.scene.source(window.scene.surfaces[i].media);const auto n=m->revision-frames[i];qInfo()<<"PROFILE clip"<<i<<"fps"<<n/seconds<<"frame preparation ms/frame"<<(n?(m->conversionNs-costs[i])/1e6/n:0)<<"size"<<m->frameSize();ok=ok&&n>0;}
                qInfo()<<"PROFILE output fps"<<(window.output->paintedFrames-paints)/seconds<<"seconds"<<seconds;app.exit(ok?0:2);
            });
        });
    }
    if(args.contains("--smoke-video-colour")){
        window.smoke=true;window.scene.newProject();
        window.scene.surfaces[0].corners={QPointF(0,0),QPointF(1,0),QPointF(1,1),QPointF(0,1)};
        window.output->resize(640,360);window.output->show();
        QTimer::singleShot(500,&window,[&window,&app]{
            auto *source=window.scene.source("");bool ok=true;
            auto check=[&](const QVideoFrame &frame,bool native){
                const auto reference=frame.toImage();source->presentFrame(frame);
                const auto rendered=window.output->grabFramebuffer();
                const auto expected=reference.pixelColor(reference.width()/2,reference.height()/2),actual=rendered.pixelColor(rendered.width()/2,rendered.height()/2);
                const int difference=std::max({std::abs(expected.red()-actual.red()),std::abs(expected.green()-actual.green()),std::abs(expected.blue()-actual.blue())});
                const bool pass=source->mappedVideo.isValid()==native&&difference<=5;ok=ok&&pass;
                qInfo()<<"VIDEO COLOUR"<<pass<<"native"<<native<<"channel error"<<difference<<"expected"<<expected<<"actual"<<actual;
            };
            for(auto space:{QVideoFrameFormat::ColorSpace_BT601,QVideoFrameFormat::ColorSpace_BT709})for(auto range:{QVideoFrameFormat::ColorRange_Video,QVideoFrameFormat::ColorRange_Full}){
                QVideoFrameFormat format(QSize(66,34),QVideoFrameFormat::Format_NV12);format.setColorSpace(space);format.setColorRange(range);
                QVideoFrame frame(format);if(!frame.map(QVideoFrame::WriteOnly)){ok=false;continue;}
                for(int y=0;y<34;y++)std::fill_n(frame.bits(0)+y*frame.bytesPerLine(0),66,uchar(110));
                for(int y=0;y<17;y++)for(int x=0;x<33;x++){auto *uv=frame.bits(1)+y*frame.bytesPerLine(1)+x*2;uv[0]=90;uv[1]=190;}
                frame.unmap();check(frame,space==QVideoFrameFormat::ColorSpace_BT709||range!=QVideoFrameFormat::ColorRange_Full);
            }
            QImage image(66,34,QImage::Format_RGBA8888);image.fill(QColor(30,180,80));check(QVideoFrame(image),false);
            app.exit(ok?0:2);
        });
    }
    if(args.contains("--smoke-dots")){
        window.smoke=true;auto &s=window.scene.surfaces[0];s.pattern=1;s.patternSpeed=140;s.patternSize=24;window.scene.touch(true);
        window.output->resize(640,360);window.output->show();
        QTimer::singleShot(500,&window,[&window,&app]{
            const auto editorBefore=window.canvas->grabFramebuffer(),outputBefore=window.output->grabFramebuffer();
            const double phaseBefore=window.scene.surfaces[0].patternPhase;
            QTimer::singleShot(400,&window,[&window,&app,editorBefore,outputBefore,phaseBefore]{
                auto &s=window.scene.surfaces[0];
                const bool moving=window.canvas->graphicsReady&&window.output->graphicsReady&&!editorBefore.isNull()&&!outputBefore.isNull()&&s.patternPhase>phaseBefore&&window.canvas->grabFramebuffer()!=editorBefore&&window.output->grabFramebuffer()!=outputBefore;
                s.patternPlaying=false;const double frozenPhase=s.patternPhase;
                const auto frozenFrame=window.output->grabFramebuffer();
                QTimer::singleShot(250,&window,[&window,&app,moving,frozenPhase,frozenFrame]{
                    const bool paused=window.scene.surfaces[0].patternPhase==frozenPhase&&window.output->grabFramebuffer()==frozenFrame;
                    QTemporaryDir temp;QString error;const auto before=window.scene.json();
                    const bool saved=window.scene.save(temp.filePath("dots.pmap"),error)&&window.scene.load(temp.filePath("dots.pmap"),error)&&window.scene.json()==before;
                    qInfo()<<"DOTS CHECK: animated editor and output"<<moving<<"pause"<<paused<<"saved settings"<<saved<<error;
                    app.exit(moving&&paused&&saved?0:2);
                });
            });
        });
    }
    if(args.contains("--smoke")){
        window.smoke=true;
        window.output->resize(640,360);window.output->show();
        QTimer::singleShot(3000,&window,[&window,&app]{
            bool ok=window.canvas->graphicsReady;QStringList failures;
            if(!ok)failures<<"editor GL";
            const auto oldCorner=window.scene.surfaces[0].corners[1];const auto area=window.canvas->canvasRect();
            const QPointF press(area.x()+area.width()*oldCorner.x(),area.y()+area.height()*oldCorner.y()),release=press+QPointF(30,20);
            auto event=[&](QEvent::Type type,QPointF pos,Qt::MouseButton button,Qt::MouseButtons buttons){QMouseEvent e(type,pos,window.canvas->mapToGlobal(pos),button,buttons,Qt::NoModifier);QApplication::sendEvent(window.canvas,&e);};
            event(QEvent::MouseButtonPress,press,Qt::LeftButton,Qt::LeftButton);event(QEvent::MouseMove,release,Qt::NoButton,Qt::LeftButton);event(QEvent::MouseButtonRelease,release,Qt::LeftButton,Qt::NoButton);
            auto &surface=window.scene.surfaces[0];if(surface.corners[1]==oldCorner)failures<<"corner drag";ok=ok&&surface.corners[1]!=oldCorner;surface.mesh[4]=QPointF(.56,.42);ok=ok&&surface.valid();window.canvas->meshMode=true;window.canvas->repaint();
            if(app.arguments().contains("--media")){auto *source=window.scene.source(surface.media);ok=ok&&(!source->player||source->revision>2);qInfo()<<"MEDIA frames"<<source->revision<<"size"<<source->frameSize();}
            QTemporaryDir temp;QString error;const auto before=window.scene.json();
            const bool roundtrip=window.scene.save(temp.filePath("roundtrip.hmap"),error)&&window.scene.load(temp.filePath("roundtrip.hmap"),error)&&before==window.scene.json();if(!roundtrip)failures<<"project roundtrip";ok=ok&&roundtrip;
            auto shot=window.canvas->grabFramebuffer();auto rect=window.canvas->canvasRect();const double dpi=window.canvas->devicePixelRatioF();
            const QRect content=QRect(qRound(rect.x()*dpi),qRound(rect.y()*dpi),qRound(rect.width()*dpi),qRound(rect.height()*dpi)).intersected(shot.rect()).adjusted(3,3,-3,-3);
            int litPixels=0;for(int y=content.top();y<content.bottom();y+=3)for(int x=content.left();x<content.right();x+=3)if(shot.pixelColor(x,y).green()>35)++litPixels;
            const bool rendered=!shot.isNull()&&(app.arguments().contains("--media")||litPixels>30);if(!rendered)failures<<"rendered content";ok=ok&&rendered;
            window.output->repaint();auto projected=window.output->grabFramebuffer();if(!window.output->graphicsReady||projected.isNull())failures<<"output GL";ok=ok&&window.output->graphicsReady&&!projected.isNull();
            auto args=app.arguments();int i=args.indexOf("--snapshot");if(i>=0&&i+1<args.size())window.grab().save(args[i+1]);
            qInfo().noquote()<<"SMOKE"<<(ok?"PASS":"FAIL")<<window.canvas->graphicsDescription<<error<<failures.join(", ");
            app.exit(ok?0:2);
        });
    }
    return app.exec();
}
