#pragma once
#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QDir>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QEventLoop>
#include <QFileInfo>
#include <QLineEdit>
#include <QListWidget>
#include <QMimeData>
#include <QMouseEvent>
#include <QPushButton>
#include <QScreen>
#include <QSpinBox>
#include <QTabWidget>
#include <QTemporaryDir>
#include <QTimer>
#include <QWheelEvent>

// Real native-widget integration check. Only an in-memory scene and disposable
// files are changed. Keep the existing seasonal/output pixel checks separate:
// these assertions establish layout and wiring, not artistic acceptance.
template<class Window> void runStudioChecks(Window &window,QApplication &app,QStringList args){
    QTimer::singleShot(650,&window,[&window,&app,args]{
        window.smoke=true;
        QStringList failures;
        auto require=[&](bool ok,const QString &label){if(!ok)failures<<label;};
        auto finish=[&]{qInfo().noquote()<<"STUDIO CHECK"<<(failures.isEmpty()?"PASS":"FAIL")<<failures.join("; ");app.exit(failures.isEmpty()?0:2);};
        auto settle=[&](int milliseconds=0){
            if(milliseconds){QEventLoop events;QTimer::singleShot(milliseconds,&events,&QEventLoop::quit);events.exec();}
            app.processEvents();
        };
        auto bounds=[&](QWidget *widget){return QRect(widget->mapTo(&window,QPoint()),widget->size());};
        auto visible=[&](QWidget *widget){return widget&&widget->isVisible()&&!widget->visibleRegion().isEmpty()&&window.rect().contains(bounds(widget));};
        auto click=[&](QWidget *widget,const QString &label){
            require(visible(widget)&&widget->isEnabled(),label+" visible and enabled");
            if(!widget||!widget->isVisible()||!widget->isEnabled())return;
            const QPointF local(widget->rect().center()),global(widget->mapToGlobal(local.toPoint()));
            QMouseEvent down(QEvent::MouseButtonPress,local,global,Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);
            QMouseEvent up(QEvent::MouseButtonRelease,local,global,Qt::LeftButton,Qt::NoButton,Qt::NoModifier);
            QApplication::sendEvent(widget,&down);QApplication::sendEvent(widget,&up);app.processEvents();
        };
        auto *sidebar=window.template findChild<QWidget*>("surfaceSidebar");
        auto *inspector=window.template findChild<QWidget*>("inspectorPanel");
        auto *browser=window.template findChild<QWidget*>("animationBrowser");
        auto *stage=window.template findChild<QWidget*>("canvasPanel");
        auto *tabs=window.template findChild<QTabWidget*>("taskTabs");
        auto *list=window.template findChild<QListWidget*>("surfaceList");
        auto *library=window.template findChild<QListWidget*>("animationLibrary");
        auto *category=window.template findChild<QComboBox*>("animationCategory");
        auto *search=window.template findChild<QLineEdit*>("animationSearch");
        auto *apply=window.template findChild<QPushButton*>("applyAnimation");
        auto *sceneControls=window.template findChild<QPushButton*>("sceneControls");
        auto *actorPause=window.template findChild<QPushButton*>("actorPause");
        auto *actorSpeed=window.template findChild<QSpinBox*>("actorSpeed");
        auto *actorSize=window.template findChild<QSpinBox*>("actorSize");
        auto *zoom=window.template findChild<QSpinBox*>("canvasZoom");
        auto *fit=window.template findChild<QPushButton*>("fitView");
        auto *editMode=window.template findChild<QComboBox*>("mapEditMode");
        auto *selectTool=window.template findChild<QPushButton*>("selectTool");
        auto *cornersTool=window.template findChild<QPushButton*>("cornersTool");
        auto *meshTool=window.template findChild<QPushButton*>("meshTool");
        auto *connectTool=window.template findChild<QPushButton*>("canvasConnect");
        require(sidebar&&inspector&&browser&&stage&&tabs&&list&&library&&category&&search&&apply&&sceneControls&&actorPause&&actorSpeed&&actorSize&&zoom&&fit&&editMode&&selectTool&&cornersTool&&meshTool&&connectTool,"native Studio controls exist");
        if(!failures.isEmpty()){finish();return;}
        auto *theme=window.template findChild<QAction*>("theme_classic1");require(theme,"Classic 1 theme action exists");if(theme)theme->trigger();
        auto &scene=window.scene;scene.newProject();scene.setPattern(0,17);scene.dirty=false;
        const auto unchanged=scene.json();
        for(const auto size:{QSize(1280,800),QSize(1440,1024),QSize(960,640)}){
            window.resize(size);settle(150);const auto label=QString::number(size.width())+"x"+QString::number(size.height());
            const auto screen=window.screen()?window.screen()->availableGeometry():QRect();
            const auto dimensions=QString("%1 requested, actual %2x%3, minimum %4x%5, available screen %6x%7, DPR %8").arg(label).arg(window.width()).arg(window.height()).arg(window.minimumWidth()).arg(window.minimumHeight()).arg(screen.width()).arg(screen.height()).arg(window.devicePixelRatioF());
            qInfo().noquote()<<"STUDIO SIZE"<<dimensions;
            const bool heightFits=!screen.isValid()||size.height()<=screen.height();
            const bool supportedHeight=heightFits?window.height()==size.height():window.height()>=640&&window.height()<=size.height();
            require(window.width()==size.width()&&supportedHeight,dimensions+(heightFits?" exact fitting size is supported":" oversized height may be constrained by Windows"));
            require(visible(sidebar)&&visible(stage)&&visible(browser)&&visible(list)&&visible(library),label+" navigation, stage and library visible");
            require(bounds(sidebar).right()<bounds(stage).left(),label+" navigator sits left of stage");
            require(bounds(browser).top()>bounds(stage).bottom(),label+" library sits below stage");
            if(inspector->isVisible())require(visible(inspector)&&bounds(inspector).left()>bounds(stage).right(),label+" inspector sits separately to the right");
            else require(size.width()<1280,label+" inspector is available at regular sizes");
            require(window.canvas->width()>=240&&window.canvas->height()>=170,label+" usable canvas area");
            for(const auto &name:QStringList{"outputDisplays","outputStart","outputStop","blackoutButton"})
                require(visible(window.template findChild<QWidget*>(name)),label+" visible output control "+name);
            require(visible(sceneControls)&&visible(actorPause),label+" visible whole-scene transport");
        }
        category->setCurrentIndex(category->findData("christmas"));search->setText("no-such-studio-animation-92473");settle(35);
        require(library->count()==0,"impossible search produces an empty library");
        require(window.width()==960,"empty Christmas library does not expand compact window");
        for(const auto &name:QStringList{"outputDisplays","outputStart","outputStop","blackoutButton"})
            require(visible(window.template findChild<QWidget*>(name)),"empty compact library keeps output control visible: "+name);
        category->setCurrentIndex(category->findData("all"));search->clear();settle(35);
        require(window.width()==960&&library->count()>0,"clearing empty search restores library without resizing");
        require(scene.json()==unchanged&&!scene.dirty,"layout changes preserve clean mapping");
        window.resize(1440,1024);settle(35);
        category->setCurrentIndex(category->findData("all"));search->clear();
        require(library->dragEnabled(),"library supports dragging content");
        for(int id:{17,19,22}){
            QListWidgetItem *found=nullptr;for(int row=0;row<library->count();++row)if(library->item(row)->data(Qt::UserRole).toInt()==id)found=library->item(row);
            require(found&&!found->icon().isNull()&&!found->icon().pixmap(160,90).isNull(),"actual loaded thumbnail for stable ID "+QString::number(id));
        }
        category->setCurrentIndex(category->findData("halloween"));search->setText("Ghost Parade");
        require(library->count()==1&&library->item(0)->data(Qt::UserRole).toInt()==17,"filtered library retains stable Ghost Parade ID 17");
        require(scene.json()==unchanged&&!scene.dirty,"browsing and filtering preserve assignment");
        scene.setPattern(0,1);if(library->count())library->setCurrentRow(0);click(apply,"Apply to surface");
        require(scene.current()&&scene.current()->pattern==17,"Apply uses stable ID rather than filtered row");
        scene.undo();require(scene.current()&&scene.current()->pattern==1,"Apply supports Undo");

        // A real drop event must hit the existing mapped surface and preserve
        // its geometry. No synthetic media paths or user's files are involved.
        window.canvas->resetView();const auto geometry=scene.current()->corners;const int surfaceCount=scene.surfaces.size();
        const auto centre=window.canvas->canvasRect().center();QMimeData mime;mime.setData("application/x-potato-pattern",QByteArray::number(22));
        QDragEnterEvent enter(centre.toPoint(),Qt::CopyAction,&mime,Qt::LeftButton,Qt::NoModifier);QApplication::sendEvent(window.canvas,&enter);
        QDropEvent drop(centre,Qt::CopyAction,&mime,Qt::LeftButton,Qt::NoModifier);QApplication::sendEvent(window.canvas,&drop);
        require(enter.isAccepted()&&drop.isAccepted(),"canvas accepts native library MIME drop");
        require(scene.surfaces.size()==surfaceCount&&scene.current()&&scene.current()->pattern==22&&scene.current()->corners==geometry,"drop applies stable content ID to existing surface, preserving geometry");
        scene.undo();require(scene.current()&&scene.current()->pattern==1,"content drop supports Undo");
        const auto beforeInvalid=scene.json();QMimeData invalid;invalid.setData("application/x-potato-pattern","99999");
        QDragEnterEvent invalidEnter(centre.toPoint(),Qt::CopyAction,&invalid,Qt::LeftButton,Qt::NoModifier);QApplication::sendEvent(window.canvas,&invalidEnter);
        QDropEvent invalidDrop(centre,Qt::CopyAction,&invalid,Qt::LeftButton,Qt::NoModifier);QApplication::sendEvent(window.canvas,&invalidDrop);
        require(!invalidDrop.isAccepted()&&scene.json()==beforeInvalid,"invalid content drop leaves mapping unchanged");

        editMode->setCurrentIndex(1);require(window.canvas->meshMode&&meshTool->isChecked()&&!selectTool->isChecked(),"Map Edit Mesh synchronizes canvas and toolbar");
        click(cornersTool,"Corners tool");require(editMode->currentIndex()==0&&!window.canvas->meshMode&&!window.canvas->property("selectionOnly").toBool(),"Corners toolbar synchronizes Map Edit and corner mode");
        click(selectTool,"Select tool");require(selectTool->isChecked()&&!window.canvas->meshMode&&window.canvas->property("selectionOnly").toBool(),"Select toolbar activates selection-only mode");
        editMode->setCurrentIndex(1);require(meshTool->isChecked()&&!selectTool->isChecked()&&window.canvas->meshMode&&!window.canvas->property("selectionOnly").toBool(),"Map Edit clears stale Select mode");
        require(!connectTool->isCheckable(),"Connect is a command rather than an editing mode");
        scene.add();click(connectTool,"Connect command");require(tabs->currentIndex()==2&&meshTool->isChecked()&&window.canvas->meshMode,"Connect opens inspector without changing active Mesh tool");scene.undo();
        require(scene.surfaces.size()==surfaceCount,"temporary connection test surface removed by Undo");click(cornersTool,"Restore Corners tool");

        window.canvas->setViewZoom(1.75);require(zoom->value()==175,"canvas change synchronizes zoom display");
        zoom->setValue(125);require(qAbs(window.canvas->viewZoom()-1.25)<.001,"zoom control changes canvas");
        const auto canvasCentre=QPointF(window.canvas->rect().center());
        QWheelEvent wheel(canvasCentre,window.canvas->mapToGlobal(canvasCentre.toPoint()),{},QPoint(0,120),Qt::NoButton,Qt::NoModifier,Qt::NoScrollPhase,false);
        QApplication::sendEvent(window.canvas,&wheel);
        require(zoom->value()==qRound(window.canvas->viewZoom()*100),"wheel zoom synchronizes display");
        click(fit,"Fit view");require(zoom->value()==100&&qAbs(window.canvas->viewZoom()-1)<.001,"Fit view resets canvas and zoom display");
        click(sceneControls,"Scene controls");require(tabs->currentIndex()==3&&tabs->tabText(3)=="Scene","visible Scene controls opens Scene inspector");
        const int originalSpeed=scene.actorSpeed,originalSize=scene.actorSize;
        actorSpeed->setValue(140);require(scene.actorSpeed==140,"whole-scene speed control edits model");
        scene.undo();require(scene.actorSpeed==originalSpeed&&actorSpeed->value()==originalSpeed,"whole-scene speed Undo refreshes control");actorSpeed->setValue(140);
        actorSize->setValue(125);require(scene.actorSize==125,"whole-scene size control edits model");
        scene.undo();require(scene.actorSize==originalSize&&actorSize->value()==originalSize,"whole-scene size Undo refreshes control");actorSize->setValue(125);
        scene.setBoundaryActors(true);scene.actorElapsed=3;scene.setActorPlayback(true);settle(90);
        require(scene.actorElapsed>3,"whole-scene actor clock advances while playing");
        click(actorPause,"Pause whole-scene actors");const double paused=scene.actorElapsed;const auto pausedDocument=scene.json();scene.dirty=false;settle(100);
        require(!scene.actorsPlaying&&scene.actorElapsed==paused,"Pause freezes actual actor clock");
        require(scene.json()==pausedDocument&&!scene.dirty,"paused timer does not dirty project or settings");
        QTemporaryDir temp;require(temp.isValid(),"disposable Studio project folder");QString error;
        if(temp.isValid()){
            const auto document=scene.json();const auto path=temp.filePath("studio.pmap");
            require(scene.save(path,error),"save Studio controls: "+error);scene.newProject();
            require(scene.load(path,error),"reopen Studio controls: "+error);
            require(scene.json()==document&&scene.actorSpeed==140&&scene.actorSize==125&&!scene.actorsPlaying,"whole-scene playback, speed and size survive save/reopen");
            require(actorSpeed->value()==140&&actorSize->value()==125,"reopened scene refreshes actual controls");
        }
        click(actorPause,"Play whole-scene actors");const double restarted=scene.actorElapsed;settle(90);
        require(scene.actorsPlaying&&scene.actorElapsed>restarted,"Play resumes actual actor clock");

        // Capture the running app, using a read-only example load. The concept
        // image is a layout reference, never evidence of rendered UI quality.
        const int snapshot=args.indexOf("--snapshot");if(snapshot>=0){
            require(snapshot+1<args.size(),"snapshot path provided");
            if(snapshot+1<args.size()){
                const int fixture=args.indexOf("--studio-scene");const auto example=fixture>=0&&fixture+1<args.size()?args[fixture+1]:QDir(QCoreApplication::applicationDirPath()).filePath("examples/Seasonal Studio.pmap");
                require(scene.load(example,error),"load Studio snapshot example: "+error);
                scene.select(0);tabs->setCurrentIndex(1);category->setCurrentIndex(category->findData("halloween"));search->clear();
                if(scene.current())for(int row=0;row<library->count();++row)if(library->item(row)->data(Qt::UserRole).toInt()==scene.current()->pattern){library->setCurrentRow(row);library->scrollToItem(library->item(row),QAbstractItemView::PositionAtCenter);break;}
                window.resize(1440,1024);settle(150);
                require(QDir().mkpath(QFileInfo(args[snapshot+1]).absolutePath()),"create Studio snapshot folder");
                const auto canvasFrame=window.canvas->grabFramebuffer();require(canvasFrame.save(args[snapshot+1]+".canvas.png"),"save actual editor framebuffer companion");
                const auto windowFrame=window.grab().toImage();require(windowFrame.save(args[snapshot+1]),"save actual native Studio snapshot");
                // Catch QWidget-grab redirection losing the QPainter handles
                // while retaining the OpenGL media. Inspect both real images.
                auto amberNear=[](const QImage &frame,QPointF logicalPoint,QSize logicalSize){
                    if(frame.isNull()||logicalSize.isEmpty())return false;
                    const double sx=double(frame.width())/logicalSize.width(),sy=double(frame.height())/logicalSize.height();
                    const QPoint centre(qRound(logicalPoint.x()*sx),qRound(logicalPoint.y()*sy));const int radius=qMax(1,qRound(5*qMax(sx,sy)));int amber=0;
                    for(int y=qMax(0,centre.y()-radius);y<=qMin(frame.height()-1,centre.y()+radius);++y)for(int x=qMax(0,centre.x()-radius);x<=qMin(frame.width()-1,centre.x()+radius);++x){const auto c=frame.pixelColor(x,y);if(qAbs(c.red()-230)<35&&qAbs(c.green()-173)<35&&qAbs(c.blue()-87)<35)++amber;}
                    return amber>=2;
                };
                if(scene.current()&&!scene.current()->corners.isEmpty()){
                    const auto corner=scene.current()->corners.first();const auto r=window.canvas->canvasRect();const QPointF handle(r.left()+r.width()*corner.x(),r.top()+r.height()*corner.y());
                    require(amberNear(canvasFrame,handle,window.canvas->size()),"real editor framebuffer contains selected amber corner handle");
                    require(amberNear(windowFrame,handle+window.canvas->mapTo(&window,QPoint()),window.size()),"real whole-window capture retains selected amber corner handle");
                }else require(false,"selected surface exists for capture handle regression");
            }
        }
        finish();
    });
}
