#include "model.h"
#include "canvas.h"
#include "branding.h"
#include "updates.h"
#include "ui.h"
#include "style.h"
#include "../tests/appearance-check.h"
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
#include <functional>
#include <memory>

class MainWindow : public QMainWindow {
    friend int main(int,char**);
public:
    Scene scene{this};
    Canvas *canvas,*output;
    bool smoke=false;
    explicit MainWindow(){
        setWindowTitle("Potato Mapper");resize(1280,800);setMinimumSize(760,540);
        canvas=new Canvas(&scene,true);output=new Canvas(&scene,false);
        output->setWindowTitle("Potato Mapper — Projector");output->setWindowFlag(Qt::FramelessWindowHint);
        output->installEventFilter(this);
        auto *bar=addToolBar("Project");bar->setMovable(false);
        auto *brandIcon=new QLabel;brandIcon->setPixmap(QPixmap(":/assets/potato-mapper.png").scaled(42,42,Qt::KeepAspectRatio,Qt::SmoothTransformation));bar->addWidget(brandIcon);
        auto *brandName=new QLabel("Potato Mapper");brandName->setStyleSheet("font-size:16px;font-weight:700;color:#f6d396;padding-right:12px;");bar->addWidget(brandName);
        auto action=[this,bar](const QString &name,const QKeySequence &key,std::function<void()> fn){auto *a=bar->addAction(name);if(!key.isEmpty())a->setShortcut(key);connect(a,&QAction::triggered,this,fn);return a;};
        action("New",QKeySequence::New,[this]{if(!canDiscard())return;scene.newProject();refresh();});
        action("Open",QKeySequence::Open,[this]{open();});action("Save",QKeySequence::Save,[this]{save(false);});
        action("Save as",QKeySequence("Ctrl+Shift+S"),[this]{save(true);});bar->addSeparator();
        action("Undo",QKeySequence::Undo,[this]{scene.undo();});action("Redo",QKeySequence::Redo,[this]{scene.redo();});bar->addSeparator();
        action("+ Surface",QKeySequence("Ctrl+N, S"),[this]{scene.add();});action("Load media",QKeySequence("Ctrl+I"),[this]{loadMedia();});
        action("Potato FX",{},[this]{if(!scene.current())scene.add();fxSection->heading->setChecked(true);inspector->ensureWidgetVisible(fxSection);patternPicker->setFocus();patternPicker->showPopup();});
        addToolBarBreak();
        auto *outputBar=addToolBar("Projector output");outputBar->setMovable(false);
        auto *outputPanel=new QWidget;auto *outputLayout=new QVBoxLayout(outputPanel);outputLayout->setContentsMargins(4,2,4,2);outputLayout->setSpacing(5);
        auto *outputRow=new QHBoxLayout;outputRow->setSpacing(8);
        outputRow->addWidget(new QLabel("PROJECTOR OUTPUT"));
        displays=new QComboBox;displays->setMinimumWidth(160);displays->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);outputRow->addWidget(displays,1);
        auto *refreshButton=new QPushButton("Refresh");outputRow->addWidget(refreshButton);
        auto *settingsButton=new QPushButton("Display settings");outputRow->addWidget(settingsButton);
        outputLayout->addLayout(outputRow);
        auto *outputControls=new QHBoxLayout;outputControls->setSpacing(8);
        startOutputButton=new QPushButton("Start output");startOutputButton->setStyleSheet("background:#286250;color:#eafff7;font-weight:600;");outputControls->addWidget(startOutputButton);
        stopOutputButton=new QPushButton("Stop");outputControls->addWidget(stopOutputButton);
        blackoutButton=new QPushButton("Blackout  [B]");blackoutButton->setCheckable(true);outputControls->addWidget(blackoutButton);
        outputInfo=new QLabel;outputInfo->setMinimumWidth(0);outputInfo->setWordWrap(true);outputInfo->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Preferred);outputControls->addWidget(outputInfo,1);
        outputLayout->addLayout(outputControls);outputBar->addWidget(outputPanel);outputPanel->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
        connect(refreshButton,&QPushButton::clicked,this,[this]{updateDisplays();});
        connect(startOutputButton,&QPushButton::clicked,this,[this]{showOutput();});
        connect(stopOutputButton,&QPushButton::clicked,this,[this]{output->hide();updateOutputStatus();});
        connect(blackoutButton,&QPushButton::toggled,this,[this](bool v){scene.blackout=v;emit scene.changed();updateOutputStatus();});
        connect(settingsButton,&QPushButton::clicked,this,[]{QDesktopServices::openUrl(QUrl("ms-settings:display"));});
        connect(displays,&QComboBox::currentIndexChanged,this,[this]{selectedDisplay=displays->currentData().toString();if(!selectedDisplay.isEmpty())QSettings().setValue("output/display",selectedDisplay);updateOutputStatus();});
        auto *blackShortcut=new QShortcut(QKeySequence("B"),this);connect(blackShortcut,&QShortcut::activated,blackoutButton,&QPushButton::click);
        auto *outBlack=new QShortcut(QKeySequence("B"),output);connect(outBlack,&QShortcut::activated,blackoutButton,&QPushButton::click);
        selectedDisplay=QSettings().value("output/display").toString();
        auto *split=new QSplitter;split->setObjectName("editorSplitter");split->setHandleWidth(6);split->setChildrenCollapsible(false);setCentralWidget(split);
        auto *panel=new QWidget;panel->setObjectName("surfaceSidebar");panel->setMinimumWidth(260);panel->setMaximumWidth(600);
        auto *layout=new QVBoxLayout(panel);layout->setContentsMargins(12,12,8,12);layout->setSpacing(8);
        auto *surfaceHeading=new QLabel("SURFACES");surfaceHeading->setStyleSheet("font-weight:600;color:#90a8b5;");layout->addWidget(surfaceHeading);
        list=new QListWidget;list->setObjectName("surfaceList");list->setContextMenuPolicy(Qt::CustomContextMenu);list->setToolTip("Right-click a surface for media actions. Double-click to rename.");
        list->setTextElideMode(Qt::ElideMiddle);list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);list->setMinimumHeight(100);list->setMaximumHeight(190);layout->addWidget(list,1);
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
        for(int i=0;i<surfaceButtons->count();++i)surfaceButtons->itemAt(i)->widget()->setProperty("compactSurface",true);layout->addLayout(surfaceButtons);
        visible=new QCheckBox("Visible");locked=new QCheckBox("Lock position");auto *flags=new QHBoxLayout;flags->addWidget(visible);flags->addWidget(locked);layout->addLayout(flags);
        connect(visible,&QCheckBox::toggled,this,[this](bool v){if(auto *s=scene.current()){scene.checkpoint();s->visible=v;scene.touch(true);}});
        connect(locked,&QCheckBox::toggled,this,[this](bool v){if(auto *s=scene.current()){scene.checkpoint();s->locked=v;scene.touch(true);}});
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
        auto *appearance=new InspectorSection("Appearance");detailLayout->addWidget(appearance);
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
        resetAppearance=button("Reset appearance",[this]{scene.setAppearance(scene.selected,100,100);});resetAppearance->setObjectName("resetAppearanceButton");appearance->content->addWidget(resetAppearance);
        fxSection=new InspectorSection("Potato FX");fxSection->heading->setChecked(false);detailLayout->addWidget(fxSection);
        auto *fxForm=new QFormLayout;patternPicker=new QComboBox;patternPicker->setObjectName("animationPicker");patternPicker->addItems({"Media / white grid","Animated dots"});fxForm->addRow("Animation",patternPicker);fxSection->content->addLayout(fxForm);
        connect(patternPicker,&QComboBox::currentIndexChanged,this,[this](int index){scene.setPattern(scene.selected,index);});
        patternControls=new QWidget;patternControls->setObjectName("patternControls");auto *patternForm=new QFormLayout(patternControls);patternForm->setContentsMargins(0,0,0,0);
        patternSpeed=new QSpinBox;patternSpeed->setRange(0,300);patternSpeed->setSuffix(" %");patternSpeed->setSingleStep(10);patternForm->addRow("Speed",patternSpeed);
        patternSize=new QSpinBox;patternSize->setRange(5,45);patternSize->setSuffix(" %");patternForm->addRow("Dot size",patternSize);
        patternPause=new QCheckBox("Pause animation");patternForm->addRow(patternPause);fxSection->content->addWidget(patternControls);
        connect(patternSpeed,&QSpinBox::valueChanged,this,[this](int v){if(auto *s=scene.current();s&&s->pattern){scene.checkpoint();s->patternSpeed=v;scene.touch();}});
        connect(patternSize,&QSpinBox::valueChanged,this,[this](int v){if(auto *s=scene.current();s&&s->pattern){scene.checkpoint();s->patternSize=v;scene.touch();}});
        connect(patternPause,&QCheckBox::toggled,this,[this](bool v){if(auto *s=scene.current();s&&s->pattern){scene.checkpoint();s->patternPlaying=!v;scene.touch();}});
        auto *mapping=new InspectorSection("Mapping");mapping->heading->setChecked(false);detailLayout->addWidget(mapping);auto *form=new QFormLayout;
        mode=new QComboBox;mode->addItems({"Corners","Mesh points"});form->addRow("Edit",mode);
        connect(mode,&QComboBox::currentIndexChanged,this,[this](int i){canvas->meshMode=i==1;canvas->update();});
        subdivisions=new QSpinBox;subdivisions->setRange(1,16);subdivisions->setSuffix(" × "+QString::number(2)+" cells");form->addRow("Grid",subdivisions);
        connect(subdivisions,&QSpinBox::valueChanged,this,[this](int n){if(auto *s=scene.current();s&&!s->locked){scene.checkpoint();Surface candidate=*s;candidate.subdivide(n);if(candidate.valid()){*s=candidate;scene.touch(true);}else refresh();}});
        fit=new QComboBox;fit->addItems({"Stretch","Fit whole image","Fill / crop"});form->addRow("Media",fit);
        connect(fit,&QComboBox::currentIndexChanged,this,[this](int i){if(auto *s=scene.current()){scene.checkpoint();s->fit=i;scene.touch();}});mapping->content->addLayout(form);
        auto *resetButtons=new QHBoxLayout;
        resetButtons->addWidget(button("Reset mesh",[this]{if(auto *s=scene.current();s&&!s->locked){scene.checkpoint();s->resetMesh();scene.touch();}}));
        resetButtons->addWidget(button("Fit view",[this]{canvas->resetView();}));mapping->content->addLayout(resetButtons);
        auto *help=new QLabel("Drag handles to map • drag inside to move\nWheel: zoom • middle mouse: pan\nArrow keys: nudge • Shift: 10 pixels\nEsc: close projector output");help->setWordWrap(true);help->setStyleSheet("color:#90a0ae;font-size:11px;margin-top:4px;");mapping->content->addWidget(help);detailLayout->addStretch();
        inspector=new QScrollArea;inspector->setObjectName("surfaceInspector");inspector->setWidget(details);inspector->setWidgetResizable(true);inspector->setFrameShape(QFrame::NoFrame);inspector->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);inspector->setMinimumHeight(90);layout->addWidget(inspector,3);
        split->addWidget(panel);split->addWidget(canvas);split->setStretchFactor(1,1);split->setSizes({310,970});
        auto sidebarSize=QSettings().value("ui/sidebarWidth",310).toInt();split->setSizes({std::clamp(sidebarSize,260,600),qMax(320,width()-sidebarSize)});
        connect(split,&QSplitter::splitterMoved,this,[split]{QSettings().setValue("ui/sidebarWidth",split->sizes().value(0));});
        connect(&scene,&Scene::structureChanged,this,[this]{refresh();});
        connect(&scene,&Scene::message,this,[this](const QString &m){statusBar()->showMessage(m,20000);});
        connect(canvas,&Canvas::graphicsInitialized,this,[this](const QString &m){statusBar()->showMessage(m,12000);});
        connect(qApp,&QGuiApplication::screenAdded,this,[this](QScreen *s){watchScreen(s);updateDisplays();});
        connect(qApp,&QGuiApplication::screenRemoved,this,[this](QScreen *s){if(activeOutputScreen==s){output->hide();activeOutputScreen.clear();statusBar()->showMessage("Projector disconnected. Reconnect it, select it, and start output again.");}watchedScreens.remove(s);updateDisplays();});
        connect(output,&Canvas::graphicsInitialized,this,[this](const QString &details){if(!output->graphicsReady){output->hide();outputInfo->setText("Output could not start: "+details);}});
        for(auto *s:QGuiApplication::screens())watchScreen(s);
        updateDisplays();
        auto *timer=new QTimer(this);connect(timer,&QTimer::timeout,this,[this]{auto *m=selectedMedia();bool video=m&&m->player;seek->setEnabled(video);playButton->setEnabled(video);restartButton->setEnabled(video);if(video&&!seek->isSliderDown()){qint64 d=m->player->duration();seek->setValue(d>0?int(m->player->position()*1000/d):0);}});timer->start(250);
        auto *fileMenu=menuBar()->addMenu("File");auto *editMenu=menuBar()->addMenu("Edit");
        for(auto *a:bar->actions()){if(QStringList{"New","Open","Save","Save as"}.contains(a->text()))fileMenu->addAction(a);if(QStringList{"Undo","Redo"}.contains(a->text()))editMenu->addAction(a);}
        fileMenu->addSeparator();fileMenu->addAction("Open app folder",this,[]{QDesktopServices::openUrl(QUrl::fromLocalFile(potatoInstallRoot()));});
        fileMenu->addAction("Exit",this,&QWidget::close);
        auto *helpMenu=menuBar()->addMenu("Help");
        helpMenu->addAction("Quick guide",this,[this]{QMessageBox::information(this,"Potato Mapper — Quick guide","1. Connect your projector and extend your desktop.\n2. Add a surface and load or drop media.\n3. Drag corners or choose Mesh points to align it.\n4. Choose your projector and Start output.\n\nRight-click a surface to clear media or rename it.\nClear media keeps the mesh and supports Undo.\nSave your project; media files stay linked on disk.\n\nB: blackout   Esc: stop output   F: fit editor view");});
        helpMenu->addAction("Check for updates…",this,[this]{if(updateDialog){updateDialog->raise();updateDialog->activateWindow();return;}updateDialog=new UpdateDialog(this,[this]{return prepareRestart();},[this]{return scene.projectPath;});updateDialog->show();});
        helpMenu->addAction("Use previous app version…",this,[this]{rollback();});
        helpMenu->addAction("GitHub releases",this,[]{QDesktopServices::openUrl(QUrl("https://github.com/TheGamingDonKey/potato-mapper/releases"));});
        helpMenu->addAction("About Potato Mapper",this,[this]{QMessageBox::about(this,"About Potato Mapper","Potato Mapper "+QCoreApplication::applicationVersion()+"\nA home projection mapper for one projector.\n\nMIT licensed. Qt and FFmpeg have their own licenses.\nYour projects and media are stored locally.\nUse Help → Check for updates to get new releases.");});
        connect(&scene,&Scene::changed,this,[this]{setWindowModified(scene.dirty);});
        scene.add();scene.dirty=false;refresh();
        statusBar()->showMessage("Add a surface, then drop an image or video onto it.");
    }
    ~MainWindow()override{delete output;}
    bool save(bool as){
        QString path=scene.projectPath;
        if(as||path.isEmpty())path=QFileDialog::getSaveFileName(this,"Save mapping",path.isEmpty()?QDir(projectFolder()).filePath("My mapping.pmap"):path,"Potato Mapper (*.pmap *.hmap)");
        if(path.isEmpty())return false;if(!path.endsWith(".pmap",Qt::CaseInsensitive)&&!path.endsWith(".hmap",Qt::CaseInsensitive))path+=".pmap";
        QString error;if(!scene.save(path,error)){QMessageBox::warning(this,"Save failed",error);return false;}QSettings().setValue("projects/folder",QFileInfo(path).absolutePath());statusBar()->showMessage("Saved "+QFileInfo(path).fileName(),5000);return true;
    }
    void open(){
        if(!canDiscard())return;auto path=QFileDialog::getOpenFileName(this,"Open mapping",projectFolder(),"Potato Mapper (*.pmap *.hmap)");if(path.isEmpty())return;
        QString error;if(!scene.load(path,error))QMessageBox::warning(this,"Open failed",error);else QSettings().setValue("projects/folder",QFileInfo(path).absolutePath());canvas->resetView();
    }
    void showOutput(){
        QScreen *target=nullptr;for(auto *s:QGuiApplication::screens())if(s->name()==selectedDisplay){target=s;break;}
        if(!target){updateDisplays();return;}
        output->hide();activeOutputScreen=target;
        const QSize size=target->size()*target->devicePixelRatio();
        if(scene.outputSize!=size){scene.outputSize=size;scene.touch();}else emit scene.changed();
        output->winId();output->windowHandle()->setScreen(target);output->setGeometry(target->geometry());output->showFullScreen();
        if(target==this->screen()){output->raise();output->activateWindow();}else{raise();activateWindow();}
        updateOutputStatus();
    }
protected:
    bool eventFilter(QObject *obj,QEvent *event)override{
        if(obj==output&&(event->type()==QEvent::Show||event->type()==QEvent::Hide))QTimer::singleShot(0,this,[this]{updateOutputStatus();});
        return QMainWindow::eventFilter(obj,event);
    }
    void closeEvent(QCloseEvent *e)override{if(smoke||property("potatoUpdateExit").toBool()||canDiscard()){output->close();e->accept();}else e->ignore();}
private:
    QListWidget *list;
    QComboBox *mode,*fit,*displays;
    QSpinBox *subdivisions;
    QComboBox *patternPicker;
    QWidget *patternControls,*videoControls;
    InspectorSection *mediaSection,*fxSection;
    QScrollArea *inspector;
    QSpinBox *patternSpeed,*patternSize;
    QCheckBox *patternPause;
    QCheckBox *visible,*locked,*mute;
    ElidedLabel *mediaLabel;
    QSlider *seek,*brightness,*opacity;
    QLabel *brightnessValue,*opacityValue;
    QPushButton *resetAppearance;
    QPointer<UpdateDialog> updateDialog;
    QPushButton *clearButton=nullptr,*playButton=nullptr,*restartButton=nullptr;
    QPushButton *startOutputButton=nullptr,*stopOutputButton=nullptr,*blackoutButton=nullptr;
    QLabel *outputInfo=nullptr;
    QString selectedDisplay;
    QPointer<QScreen> activeOutputScreen;
    QSet<QScreen*> watchedScreens;
    MediaSource *selectedMedia(){auto *s=scene.current();return s?scene.source(s->media):nullptr;}
    bool canDiscard(){
        if(!scene.dirty)return true;
        auto answer=QMessageBox::question(this,"Save mapping?","Save your changes before continuing?",QMessageBox::Save|QMessageBox::Discard|QMessageBox::Cancel);
        return answer==QMessageBox::Discard||(answer==QMessageBox::Save&&save(false));
    }
    QString projectFolder() const {auto folder=QSettings().value("projects/folder",potatoInstallRoot()).toString();return QDir(folder).exists()?folder:potatoInstallRoot();}
    void renameSelected(){auto *s=scene.current();if(!s)return;bool ok=false;const auto name=QInputDialog::getText(this,"Rename surface","Surface name:",QLineEdit::Normal,s->name,&ok);if(ok)scene.renameSurface(scene.selected,name);}
    void surfaceMenu(int index,const QPoint &pos){
        if(index<0||index>=scene.surfaces.size())return;scene.select(index);QMenu menu(this);
        menu.addAction("Load media…",this,[this]{loadMedia();});
        const auto *s=scene.current();auto *clear=menu.addAction("Clear media — white grid",this,[this]{scene.clearMedia(scene.selected);});clear->setEnabled(s&&(!s->media.isEmpty()||s->pattern));
        menu.addSeparator();menu.addAction("Rename surface…",this,[this]{renameSelected();});menu.exec(pos);
    }
    bool prepareRestart(){
        if(output->isVisible()&&QMessageBox::question(this,"Stop projector output?","Stop projector output and continue with the update?",QMessageBox::Yes|QMessageBox::Cancel,QMessageBox::Cancel)!=QMessageBox::Yes)return false;
        if(!canDiscard())return false;output->hide();updateOutputStatus();return true;
    }
    void rollback(){
        const auto root=potatoInstallRoot();if(!QFileInfo::exists(QDir(root).filePath("previous.txt"))){QMessageBox::information(this,"Previous version","There is no previous app version in this folder yet.");return;}
        if(QMessageBox::question(this,"Use previous app version?","Reopen the previous app version? This does not undo edits to saved projects.",QMessageBox::Yes|QMessageBox::Cancel,QMessageBox::Cancel)!=QMessageBox::Yes||!prepareRestart())return;
        QStringList args{"--rollback","--wait-pid",QString::number(QCoreApplication::applicationPid())};if(!scene.projectPath.isEmpty())args<<"--project"<<scene.projectPath;
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
            if(QGuiApplication::screens().size()<2)outputInfo->setText("Only one display detected. Connect your projector, then open Display settings and choose Extend these displays. Select a display above to test output; Esc returns to the editor.");
            else outputInfo->setText("OUTPUT OFF · Select your projector above, then Start output. Keep Windows set to Extend so the editor stays on your computer screen.");
        }
    }
    void refresh(){
        QSignalBlocker b1(list),b2(visible),b3(locked),b4(subdivisions),b5(fit),b6(mute),b7(patternSpeed),b8(patternSize),b9(patternPause),b10(patternPicker),b11(brightness),b12(opacity);
        const int listScroll=list->verticalScrollBar()->value();list->clear();for(const auto &s:scene.surfaces){const auto content=s.pattern?QString("Potato FX · Dots"):s.media.isEmpty()?QString("White grid"):QFileInfo(s.media).fileName();auto *item=new QListWidgetItem(s.name+"\n"+content+(s.visible?"":" · hidden"),list);item->setToolTip(s.name+"\n"+(s.media.isEmpty()?content:s.media));}list->setCurrentRow(scene.selected);list->verticalScrollBar()->setValue(listScroll);
        auto *s=scene.current();clearButton->setEnabled(s&&(!s->media.isEmpty()||s->pattern));visible->setEnabled(s);locked->setEnabled(s);subdivisions->setEnabled(s&&!s->locked);fit->setEnabled(s);
        patternPicker->setEnabled(s);if(s&&s->pattern&&patternPicker->currentIndex()!=s->pattern)fxSection->heading->setChecked(true);patternPicker->setCurrentIndex(s?s->pattern:0);patternControls->setVisible(s&&s->pattern);
        brightness->setEnabled(s);opacity->setEnabled(s);resetAppearance->setEnabled(s&&(s->brightness!=100||s->opacity!=100));
        brightness->setValue(s?s->brightness:100);opacity->setValue(s?s->opacity:100);brightnessValue->setText(QString::number(brightness->value())+" %");opacityValue->setText(QString::number(opacity->value())+" %");
        if(s){patternSpeed->setValue(s->patternSpeed);patternSize->setValue(s->patternSize);patternPause->setChecked(!s->patternPlaying);visible->setChecked(s->visible);locked->setChecked(s->locked);subdivisions->setValue(s->cells);subdivisions->setSuffix(" × "+QString::number(s->cells)+" cells");fit->setCurrentIndex(s->fit);fit->setEnabled(!s->pattern&&!s->media.isEmpty());mediaLabel->setFullText(s->pattern?"Animated dots · Potato FX":s->media.isEmpty()?"White grid — drop media here":QFileInfo(s->media).fileName(),s->media);auto *m=scene.source(s->media);videoControls->setVisible(m->player&&!s->pattern);mute->setEnabled(m->audio);mute->setChecked(!m->audio||m->audio->isMuted());}
        else {mediaLabel->setFullText("Add a surface to begin");mute->setEnabled(false);videoControls->hide();}
        setWindowTitle("Potato Mapper"+(scene.projectPath.isEmpty()?QString():" — "+QFileInfo(scene.projectPath).fileName())+" [*]");setWindowModified(scene.dirty);
    }
};

static void logMessage(QtMsgType,const QMessageLogContext &,const QString &message){
    QFile log(potatoInstallRoot()+"/mapper.log");if(log.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text)){QTextStream out(&log);out<<message<<Qt::endl;}
}
int main(int argc,char **argv){
    QSurfaceFormat format;format.setVersion(3,3);format.setProfile(QSurfaceFormat::CoreProfile);format.setSwapInterval(1);format.setDepthBufferSize(0);format.setStencilBufferSize(8);QSurfaceFormat::setDefaultFormat(format);
    QApplication app(argc,argv);app.setApplicationName("Potato Mapper");app.setOrganizationName("PotatoMapper");app.setApplicationVersion(POTATO_VERSION);app.setWindowIcon(QIcon(":/assets/potato-mapper.png"));
    qInstallMessageHandler(logMessage);app.setStyle("Fusion");
    app.setStyleSheet(potatoStyle());
    auto args=app.arguments();const bool diagnostic=std::any_of(args.begin(),args.end(),[](const QString &a){return a.startsWith("--smoke")||a.startsWith("--test-")||a=="--profile-media";});
    std::unique_ptr<QLockFile> runtimeLock;
    if(!diagnostic&&QFileInfo::exists(QDir(potatoInstallRoot()).filePath("installation.txt"))){runtimeLock=std::make_unique<QLockFile>(QDir(potatoInstallRoot()).filePath(".potato-runtime.lock"));runtimeLock->setStaleLockTime(0);if(!runtimeLock->tryLock()){QMessageBox::information(nullptr,"Potato Mapper","Potato Mapper is already open in this installation. Switch to its window to continue.");return 0;}}
    if(args.contains("--test-splash")){
        auto *splash=potatoSplash();splash->show();QTimer::singleShot(250,splash,[&app,args,splash]{
            auto frame=splash->grab();bool ok=!frame.isNull()&&splash->isVisible();
            if(auto *screen=splash->screen())ok=ok&&(splash->geometry().center()-screen->availableGeometry().center()).manhattanLength()<=2;
            int i=args.indexOf("--snapshot");if(i>=0&&i+1<args.size())ok=ok&&frame.save(args[i+1]);
            qInfo()<<"SPLASH CHECK"<<ok<<"DPI scale"<<splash->devicePixelRatioF();app.exit(ok?0:2);
        });return app.exec();
    }
    QSplashScreen *splash=nullptr;if(!diagnostic&&!args.contains("--no-splash")){splash=potatoSplash();splash->show();app.processEvents();}
    MainWindow window;window.show();
    if(splash)QTimer::singleShot(args.contains("--preview-splash")?10000:1100,&window,[splash,&window]{splash->finish(&window);splash->deleteLater();});
    const int projectArgument=args.indexOf("--project");
    if(projectArgument>=0&&projectArgument+1<args.size()){QString error;if(!window.scene.load(args[projectArgument+1],error))window.statusBar()->showMessage(error);}
    const int mediaArgument=args.indexOf("--media");
    if(mediaArgument>=0&&mediaArgument+1<args.size())window.scene.assignMedia(0,args[mediaArgument+1]);
    if(args.contains("--smoke-appearance")){window.smoke=true;runAppearanceChecks(window,app,args);}
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
