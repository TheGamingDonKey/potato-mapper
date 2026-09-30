#include "model.h"
#include "canvas.h"
#include <QApplication>
#include <QMainWindow>
#include <QToolBar>
#include <QLabel>
#include <QListWidget>
#include <QSplitter>
#include <QScrollArea>
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

class MainWindow : public QMainWindow {
public:
    Scene scene{this};
    Canvas *canvas,*output;
    bool smoke=false;
    explicit MainWindow(){
        setWindowTitle("Potato Mapper");resize(1280,800);
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
        action("Animated dots",{},[this]{if(!scene.current())scene.add();scene.checkpoint();auto *s=scene.current();s->pattern=1;s->media.clear();s->patternPhase=0;s->patternPlaying=true;scene.touch(true);});
        addToolBarBreak();
        patternBar=addToolBar("Animated dots");patternBar->setMovable(false);
        patternBar->addWidget(new QLabel("ANIMATED DOTS   Speed "));
        patternSpeed=new QSpinBox;patternSpeed->setRange(0,300);patternSpeed->setSuffix(" %");patternSpeed->setSingleStep(10);patternBar->addWidget(patternSpeed);
        patternBar->addWidget(new QLabel("  Dot size "));
        patternSize=new QSpinBox;patternSize->setRange(5,45);patternSize->setSuffix(" %");patternBar->addWidget(patternSize);
        patternPause=new QCheckBox("Pause");patternBar->addWidget(patternPause);
        auto *gridAction=patternBar->addAction("Back to white grid");
        connect(patternSpeed,&QSpinBox::valueChanged,this,[this](int v){if(auto *s=scene.current();s&&s->pattern){scene.checkpoint();s->patternSpeed=v;scene.touch();}});
        connect(patternSize,&QSpinBox::valueChanged,this,[this](int v){if(auto *s=scene.current();s&&s->pattern){scene.checkpoint();s->patternSize=v;scene.touch();}});
        connect(patternPause,&QCheckBox::toggled,this,[this](bool v){if(auto *s=scene.current();s&&s->pattern){scene.checkpoint();s->patternPlaying=!v;scene.touch();}});
        connect(gridAction,&QAction::triggered,this,[this]{if(auto *s=scene.current()){scene.checkpoint();s->pattern=0;s->media.clear();scene.touch(true);}});
        addToolBarBreak();
        auto *outputBar=addToolBar("Projector output");outputBar->setMovable(false);
        auto *outputPanel=new QWidget;auto *outputLayout=new QVBoxLayout(outputPanel);outputLayout->setContentsMargins(4,2,4,2);outputLayout->setSpacing(5);
        auto *outputRow=new QHBoxLayout;outputRow->setSpacing(8);
        outputRow->addWidget(new QLabel("PROJECTOR OUTPUT"));
        displays=new QComboBox;displays->setMinimumWidth(320);displays->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);outputRow->addWidget(displays,1);
        auto *refreshButton=new QPushButton("Refresh");outputRow->addWidget(refreshButton);
        startOutputButton=new QPushButton("Start output");startOutputButton->setStyleSheet("background:#286250;color:#eafff7;font-weight:600;");outputRow->addWidget(startOutputButton);
        stopOutputButton=new QPushButton("Stop");outputRow->addWidget(stopOutputButton);
        blackoutButton=new QPushButton("Blackout  [B]");blackoutButton->setCheckable(true);outputRow->addWidget(blackoutButton);
        auto *settingsButton=new QPushButton("Display settings");outputRow->addWidget(settingsButton);
        outputLayout->addLayout(outputRow);
        outputInfo=new QLabel;outputInfo->setWordWrap(true);outputInfo->setStyleSheet("color:#b5c6d1;font-size:12px;padding:2px;");outputLayout->addWidget(outputInfo);outputBar->addWidget(outputPanel);
        connect(refreshButton,&QPushButton::clicked,this,[this]{updateDisplays();});
        connect(startOutputButton,&QPushButton::clicked,this,[this]{showOutput();});
        connect(stopOutputButton,&QPushButton::clicked,this,[this]{output->hide();updateOutputStatus();});
        connect(blackoutButton,&QPushButton::toggled,this,[this](bool v){scene.blackout=v;emit scene.changed();updateOutputStatus();});
        connect(settingsButton,&QPushButton::clicked,this,[]{QDesktopServices::openUrl(QUrl("ms-settings:display"));});
        connect(displays,&QComboBox::currentIndexChanged,this,[this]{selectedDisplay=displays->currentData().toString();if(!selectedDisplay.isEmpty())QSettings().setValue("output/display",selectedDisplay);updateOutputStatus();});
        auto *blackShortcut=new QShortcut(QKeySequence("B"),this);connect(blackShortcut,&QShortcut::activated,blackoutButton,&QPushButton::click);
        auto *outBlack=new QShortcut(QKeySequence("B"),output);connect(outBlack,&QShortcut::activated,blackoutButton,&QPushButton::click);
        selectedDisplay=QSettings().value("output/display").toString();
        auto *split=new QSplitter;setCentralWidget(split);
        auto *panel=new QWidget;panel->setMinimumWidth(230);panel->setMaximumWidth(310);
        auto *layout=new QVBoxLayout(panel);layout->setContentsMargins(16,18,16,16);layout->setSpacing(10);
        auto heading=[layout](const QString &text){auto *l=new QLabel(text);l->setStyleSheet("font-weight:600;color:#90a8b5;margin-top:8px;");layout->addWidget(l);};
        heading("SURFACES");list=new QListWidget;list->setMinimumHeight(145);layout->addWidget(list,1);
        connect(list,&QListWidget::currentRowChanged,this,[this](int row){scene.select(row);});
        auto *surfaceButtons=new QHBoxLayout;
        auto button=[this](const QString &text,std::function<void()> fn){auto *b=new QPushButton(text);connect(b,&QPushButton::clicked,this,fn);return b;};
        surfaceButtons->addWidget(button("Duplicate",[this]{if(auto *s=scene.current()){scene.checkpoint();Surface copy=*s;copy.id=QUuid::createUuid().toString(QUuid::WithoutBraces);copy.name+=" copy";for(auto &p:copy.corners)p+=QPointF(.025,.025);scene.surfaces.append(copy);scene.selected=int(scene.surfaces.size())-1;scene.touch(true);}}));
        surfaceButtons->addWidget(button("Delete",[this]{if(scene.current()){scene.checkpoint();scene.surfaces.removeAt(scene.selected);scene.selected=std::min(scene.selected,int(scene.surfaces.size())-1);scene.touch(true);}}));layout->addLayout(surfaceButtons);
        auto *orderButtons=new QHBoxLayout;
        orderButtons->addWidget(button("Back",[this]{int i=scene.selected;if(i>0){scene.checkpoint();scene.surfaces.swapItemsAt(i,i-1);scene.selected--;scene.touch(true);}}));
        orderButtons->addWidget(button("Forward",[this]{int i=scene.selected;if(i>=0&&i+1<scene.surfaces.size()){scene.checkpoint();scene.surfaces.swapItemsAt(i,i+1);scene.selected++;scene.touch(true);}}));layout->addLayout(orderButtons);
        visible=new QCheckBox("Visible");locked=new QCheckBox("Lock position");auto *flags=new QHBoxLayout;flags->addWidget(visible);flags->addWidget(locked);layout->addLayout(flags);
        connect(visible,&QCheckBox::toggled,this,[this](bool v){if(auto *s=scene.current()){scene.checkpoint();s->visible=v;scene.touch(true);}});
        connect(locked,&QCheckBox::toggled,this,[this](bool v){if(auto *s=scene.current()){scene.checkpoint();s->locked=v;scene.touch(true);}});
        heading("MAPPING");auto *form=new QFormLayout;
        mode=new QComboBox;mode->addItems({"Corners","Mesh points"});form->addRow("Edit",mode);
        connect(mode,&QComboBox::currentIndexChanged,this,[this](int i){canvas->meshMode=i==1;canvas->update();});
        subdivisions=new QSpinBox;subdivisions->setRange(1,16);subdivisions->setSuffix(" × "+QString::number(2)+" cells");form->addRow("Grid",subdivisions);
        connect(subdivisions,&QSpinBox::valueChanged,this,[this](int n){if(auto *s=scene.current();s&&!s->locked){scene.checkpoint();Surface candidate=*s;candidate.subdivide(n);if(candidate.valid()){*s=candidate;scene.touch(true);}else refresh();}});
        fit=new QComboBox;fit->addItems({"Stretch","Fit whole image","Fill / crop"});form->addRow("Media",fit);
        connect(fit,&QComboBox::currentIndexChanged,this,[this](int i){if(auto *s=scene.current()){scene.checkpoint();s->fit=i;scene.touch();}});layout->addLayout(form);
        auto *resetButtons=new QHBoxLayout;
        resetButtons->addWidget(button("Reset mesh",[this]{if(auto *s=scene.current();s&&!s->locked){scene.checkpoint();s->resetMesh();scene.touch();}}));
        resetButtons->addWidget(button("Fit view",[this]{canvas->resetView();}));layout->addLayout(resetButtons);
        heading("MEDIA");mediaLabel=new QLabel("Drop an image or video onto a surface");mediaLabel->setWordWrap(true);mediaLabel->setStyleSheet("color:#b9c5cd;");layout->addWidget(mediaLabel);
        auto *playButtons=new QHBoxLayout;
        playButtons->addWidget(button("Play / pause",[this]{if(auto *m=selectedMedia();m&&m->player){if(m->player->playbackState()==QMediaPlayer::PlayingState)m->player->pause();else m->player->play();}}));
        playButtons->addWidget(button("Restart",[this]{if(auto *m=selectedMedia();m&&m->player){m->player->setPosition(0);m->player->play();}}));layout->addLayout(playButtons);
        seek=new QSlider(Qt::Horizontal);seek->setRange(0,1000);layout->addWidget(seek);
        connect(seek,&QSlider::sliderReleased,this,[this]{if(auto *m=selectedMedia();m&&m->player)m->player->setPosition(m->player->duration()*seek->value()/1000);});
        mute=new QCheckBox("Mute audio");mute->setChecked(true);layout->addWidget(mute);
        connect(mute,&QCheckBox::toggled,this,[this](bool v){if(auto *m=selectedMedia();m&&m->audio)m->audio->setMuted(v);});
        auto *help=new QLabel("Drag handles to map • drag inside to move\nWheel: zoom • middle mouse: pan\nArrow keys: nudge • Shift: 10 pixels\nEsc: close projector output");help->setWordWrap(true);help->setStyleSheet("color:#90a0ae;font-size:11px;margin-top:8px;");layout->addWidget(help);
        auto *scroll=new QScrollArea;scroll->setWidget(panel);scroll->setWidgetResizable(true);scroll->setFrameShape(QFrame::NoFrame);scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);scroll->setMinimumWidth(250);scroll->setMaximumWidth(320);
        split->addWidget(scroll);split->addWidget(canvas);split->setStretchFactor(1,1);
        connect(&scene,&Scene::structureChanged,this,[this]{refresh();});
        connect(&scene,&Scene::message,this,[this](const QString &m){statusBar()->showMessage(m,20000);});
        connect(canvas,&Canvas::graphicsInitialized,this,[this](const QString &m){statusBar()->showMessage(m,12000);});
        connect(qApp,&QGuiApplication::screenAdded,this,[this](QScreen *s){watchScreen(s);updateDisplays();});
        connect(qApp,&QGuiApplication::screenRemoved,this,[this](QScreen *s){if(activeOutputScreen==s){output->hide();activeOutputScreen.clear();statusBar()->showMessage("Projector disconnected. Reconnect it, select it, and start output again.");}watchedScreens.remove(s);updateDisplays();});
        connect(output,&Canvas::graphicsInitialized,this,[this](const QString &details){if(!output->graphicsReady){output->hide();outputInfo->setText("Output could not start: "+details);}});
        for(auto *s:QGuiApplication::screens())watchScreen(s);
        updateDisplays();
        auto *timer=new QTimer(this);connect(timer,&QTimer::timeout,this,[this]{auto *m=selectedMedia();bool video=m&&m->player;seek->setEnabled(video);if(video&&!seek->isSliderDown()){qint64 d=m->player->duration();seek->setValue(d>0?int(m->player->position()*1000/d):0);}});timer->start(250);
        scene.add();scene.dirty=false;refresh();
        statusBar()->showMessage("Add a surface, then drop an image or video onto it.");
    }
    ~MainWindow()override{delete output;}
    bool save(bool as){
        QString path=scene.projectPath;
        if(as||path.isEmpty())path=QFileDialog::getSaveFileName(this,"Save mapping",path.isEmpty()?"My mapping.pmap":path,"Potato Mapper (*.pmap *.hmap)");
        if(path.isEmpty())return false;if(!path.endsWith(".pmap",Qt::CaseInsensitive)&&!path.endsWith(".hmap",Qt::CaseInsensitive))path+=".pmap";
        QString error;if(!scene.save(path,error)){QMessageBox::warning(this,"Save failed",error);return false;}statusBar()->showMessage("Saved "+QFileInfo(path).fileName(),5000);return true;
    }
    void open(){
        if(!canDiscard())return;auto path=QFileDialog::getOpenFileName(this,"Open mapping",{},"Potato Mapper (*.pmap *.hmap)");if(path.isEmpty())return;
        QString error;if(!scene.load(path,error))QMessageBox::warning(this,"Open failed",error);canvas->resetView();
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
    void closeEvent(QCloseEvent *e)override{if(smoke||canDiscard()){output->close();e->accept();}else e->ignore();}
private:
    QListWidget *list;
    QComboBox *mode,*fit,*displays;
    QSpinBox *subdivisions;
    QToolBar *patternBar;
    QSpinBox *patternSpeed,*patternSize;
    QCheckBox *patternPause;
    QCheckBox *visible,*locked,*mute;
    QLabel *mediaLabel;
    QSlider *seek;
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
        QSignalBlocker b1(list),b2(visible),b3(locked),b4(subdivisions),b5(fit),b6(mute),b7(patternSpeed),b8(patternSize),b9(patternPause);
        list->clear();for(const auto &s:scene.surfaces)list->addItem(s.name+(s.visible?"":"  (hidden)"));list->setCurrentRow(scene.selected);
        auto *s=scene.current();visible->setEnabled(s);locked->setEnabled(s);subdivisions->setEnabled(s&&!s->locked);fit->setEnabled(s);
        patternBar->setVisible(s&&s->pattern);if(s){patternSpeed->setValue(s->patternSpeed);patternSize->setValue(s->patternSize);patternPause->setChecked(!s->patternPlaying);}
        if(s){visible->setChecked(s->visible);locked->setChecked(s->locked);subdivisions->setValue(s->cells);subdivisions->setSuffix(" × "+QString::number(s->cells)+" cells");fit->setCurrentIndex(s->fit);mediaLabel->setText(s->media.isEmpty()?"Alignment grid — drop media here":QFileInfo(s->media).fileName());mediaLabel->setToolTip(s->media);auto *m=scene.source(s->media);mute->setEnabled(m->audio);mute->setChecked(!m->audio||m->audio->isMuted());}
        else {mediaLabel->setText("Add a surface to begin");mute->setEnabled(false);}
        if(s&&s->pattern)mediaLabel->setText("Animated dots — use the controls above");
        setWindowTitle("Potato Mapper"+(scene.projectPath.isEmpty()?QString():" — "+QFileInfo(scene.projectPath).fileName()));
    }
};

static void logMessage(QtMsgType,const QMessageLogContext &,const QString &message){
    QFile log(QCoreApplication::applicationDirPath()+"/mapper.log");if(log.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text)){QTextStream out(&log);out<<message<<Qt::endl;}
}
int main(int argc,char **argv){
    QSurfaceFormat format;format.setVersion(3,3);format.setProfile(QSurfaceFormat::CoreProfile);format.setSwapInterval(1);format.setDepthBufferSize(0);format.setStencilBufferSize(8);QSurfaceFormat::setDefaultFormat(format);
    QApplication app(argc,argv);app.setApplicationName("Potato Mapper");app.setOrganizationName("PotatoMapper");app.setWindowIcon(QIcon(":/assets/potato-mapper.png"));
    qInstallMessageHandler(logMessage);app.setStyle("Fusion");
    app.setStyleSheet("QWidget{background:#171c24;color:#e7edf2;font-family:'Segoe UI';font-size:12px;} QToolBar{spacing:7px;padding:9px;background:#202833;border:0;} QToolButton,QPushButton{background:#2b3743;border:1px solid #3c4e5b;border-radius:5px;padding:7px;} QToolButton:hover,QPushButton:hover{background:#354957;border-color:#78cdb8;} QToolButton:pressed,QPushButton:pressed{background:#456456;} QListWidget{background:#11161d;border:1px solid #34424e;border-radius:5px;} QListWidget::item{padding:9px;} QListWidget::item:selected{background:#284b45;color:#bbffeb;} QComboBox,QSpinBox{background:#222d37;border:1px solid #3c4e5b;border-radius:4px;padding:5px;} QSlider::groove:horizontal{height:5px;background:#35434d;} QSlider::handle:horizontal{background:#78efce;width:12px;margin:-4px 0;border-radius:5px;} QStatusBar{background:#10161d;color:#a9bbc7;} QSplitter::handle{background:#293542;width:2px;} QCheckBox{spacing:5px;} ");
    MainWindow window;window.show();
    auto args=app.arguments();
    const int projectArgument=args.indexOf("--project");
    if(projectArgument>=0&&projectArgument+1<args.size()){QString error;if(!window.scene.load(args[projectArgument+1],error))window.statusBar()->showMessage(error);}
    const int mediaArgument=args.indexOf("--media");
    if(mediaArgument>=0&&mediaArgument+1<args.size())window.scene.assignMedia(0,args[mediaArgument+1]);
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
            if(app.arguments().contains("--media")){auto *source=window.scene.source(surface.media);ok=ok&&(!source->player||source->revision>2);qInfo()<<"MEDIA frames"<<source->revision<<"size"<<source->image.size();}
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
