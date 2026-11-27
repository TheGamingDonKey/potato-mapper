#pragma once
#include "material-ui-check.h"
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
#include <QMessageBox>
#include <QMouseEvent>
#include <QPushButton>
#include <QScreen>
#include <QSpinBox>
#include <QDialog>
#include <QScrollArea>
#include <QToolButton>
#include <QTemporaryDir>
#include <QTimer>
#include <QWheelEvent>

// Real native-widget integration check. Only an in-memory scene and disposable
// files are changed. Keep the existing seasonal/output pixel checks separate:
// these assertions establish layout and wiring, not artistic acceptance.
// The historical --smoke-studio flag retains the newer-feature regression
// coverage while following the restored sidebar and temporary chooser.
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
        auto visible=[&](QWidget *widget){return widget&&PotatoMaterialUiCheck::visible(widget,widget->window());};
        auto click=[&](QWidget *widget,const QString &label){
            PotatoMaterialUiCheck::reveal(widget,app);
            require(visible(widget)&&widget->isEnabled(),label+" visible and enabled");
            if(!widget||!widget->isVisible()||!widget->isEnabled())return;
            const QPointF local(widget->rect().center()),global(widget->mapToGlobal(local.toPoint()));
            QMouseEvent down(QEvent::MouseButtonPress,local,global,Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);
            QMouseEvent up(QEvent::MouseButtonRelease,local,global,Qt::LeftButton,Qt::NoButton,Qt::NoModifier);
            QApplication::sendEvent(widget,&down);QApplication::sendEvent(widget,&up);app.processEvents();
        };
        auto *sidebar=window.template findChild<QWidget*>("surfaceSidebar");
        auto *inspector=window.template findChild<QScrollArea*>("surfaceInspector");
        auto *browser=window.template findChild<QWidget*>("animationBrowser");
        auto *browserDialog=window.template findChild<QDialog*>("animationBrowserDialog");
        auto *browse=window.template findChild<QPushButton*>("browseAnimations");
        auto *openPotatoFx=window.template findChild<QAction*>("openPotatoFx");
        auto *fxSection=window.template findChild<QWidget*>("fxSection");
        auto *fxHeading=fxSection?fxSection->template findChild<QToolButton*>("sectionHeading"):nullptr;
        auto *stage=window.canvas;
        auto *list=window.template findChild<QListWidget*>("surfaceList");
        auto *library=window.template findChild<QListWidget*>("animationLibrary");
        auto *category=window.template findChild<QComboBox*>("animationCategory");
        auto *search=window.template findChild<QLineEdit*>("animationSearch");
        auto *apply=window.template findChild<QPushButton*>("applyAnimation");
        auto *actorPause=window.template findChild<QPushButton*>("actorPause");
        auto *actorSpeed=window.template findChild<QSpinBox*>("actorSpeed");
        auto *actorSize=window.template findChild<QSpinBox*>("actorSize");
        auto *zoom=window.template findChild<QSpinBox*>("canvasZoom");
        auto *fit=window.template findChild<QPushButton*>("fitView");
        auto *editMode=window.template findChild<QComboBox*>("mapEditMode");
        auto *connectTool=window.template findChild<QPushButton*>("connectSurface");
        auto *actorsSection=window.template findChild<QWidget*>("actorsSection");
        require(sidebar&&inspector&&browser&&browserDialog&&browse&&openPotatoFx&&fxHeading&&stage&&list&&library&&category&&search&&apply&&actorPause&&actorSpeed&&actorSize&&zoom&&fit&&editMode&&connectTool&&actorsSection,"retained native feature controls exist");
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
            require(visible(sidebar)&&visible(stage)&&visible(list)&&visible(inspector),label+" left navigation, inspector and canvas visible");
            require(bounds(sidebar).right()<bounds(stage).left(),label+" navigator sits left of stage");
            require(sidebar->isAncestorOf(inspector)&&!browserDialog->isVisible(),label+" inspector stays left and library is temporary");
            require(window.canvas->width()>=240&&window.canvas->height()>=170,label+" usable canvas area");
            for(const auto &name:QStringList{"outputDisplays","outputStart","outputStop","blackoutButton"})
                require(visible(window.template findChild<QWidget*>(name)),label+" visible output control "+name);
        }
        fxHeading->setChecked(false);openPotatoFx->trigger();settle(35);
        require(fxHeading->isChecked()&&visible(fxHeading)&&!browserDialog->isVisible(),"Potato FX toolbar action reveals sidebar FX without opening browser");
        require(scene.json()==unchanged&&!scene.dirty,"Potato FX sidebar navigation preserves clean mapping");
        click(browse,"Browse animations");settle(35);require(browserDialog->isVisible()&&visible(library),"Browse opens reachable native library");
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
        browserDialog->close();settle(35);

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

        PotatoMaterialUiCheck::reveal(editMode,app);
        editMode->setCurrentIndex(1);require(window.canvas->meshMode&&!window.canvas->property("selectionOnly").toBool(),"Map Edit Mesh activates canvas mesh handles");
        editMode->setCurrentIndex(0);require(!window.canvas->meshMode&&!window.canvas->property("selectionOnly").toBool(),"Corners activates corner editing");
        editMode->setCurrentIndex(2);require(!window.canvas->meshMode&&window.canvas->property("selectionOnly").toBool(),"Select activates selection-only mode");
        editMode->setCurrentIndex(1);require(window.canvas->meshMode&&!window.canvas->property("selectionOnly").toBool(),"Map Edit clears stale Select mode");
        require(!connectTool->isCheckable(),"Connect is a command rather than an editing mode");
        scene.add();
        scene.surfaces[0].corners={{.1,.3},{.3,.3},{.3,.5},{.1,.5}};
        scene.surfaces[1].corners={{.32,.1},{.62,.1},{.62,.7},{.32,.7}};
        scene.select(0);scene.touch(true);scene.dirty=false;
        PotatoMaterialUiCheck::reveal(connectTool,app);require(visible(connectTool)&&connectTool->isEnabled()&&editMode->currentIndex()==1&&window.canvas->meshMode,"opening Connections preserves active Mesh tool");
        if(auto *fitConnection=window.template findChild<QCheckBox*>("fitConnection"))fitConnection->setChecked(false);
        const auto beforeConnection=scene.json();
        const auto leftGeometry=scene.surfaces[0].corners,rightGeometry=scene.surfaces[1].corners;
        auto answerConnection=[&](QMessageBox::StandardButton answer){
            bool sawPreview=false,answered=false;QTimer inspect,watchdog;inspect.setSingleShot(true);watchdog.setSingleShot(true);
            QObject::connect(&inspect,&QTimer::timeout,&window,[&]{
                auto *dialog=qobject_cast<QMessageBox*>(QApplication::activeModalWidget());
                require(dialog&&dialog->windowTitle()=="Preview connection","Connect button opens real Preview connection confirmation");
                if(!dialog)return;
                if(dialog->windowTitle()!="Preview connection"){dialog->reject();return;}
                sawPreview=true;require(window.canvas->property("connectionPreview").toBool(),"connection geometry preview active during confirmation");
                auto *button=dialog->button(answer);require(button,"requested confirmation button exists");
                if(button){answered=true;button->click();}else dialog->reject();
            });
            QObject::connect(&watchdog,&QTimer::timeout,&window,[&]{if(auto *dialog=qobject_cast<QMessageBox*>(QApplication::activeModalWidget()))dialog->reject();});
            inspect.start(120);watchdog.start(1800);click(connectTool,"Connect confirmation command");inspect.stop();watchdog.stop();
            require(sawPreview&&answered,"native connection confirmation answered");
            require(!window.canvas->property("connectionPreview").toBool()&&editMode->currentIndex()==1&&window.canvas->meshMode,"confirmation closes preview and preserves active Mesh mode");
        };
        answerConnection(QMessageBox::Cancel);
        require(scene.json()==beforeConnection&&!scene.dirty&&scene.connections.isEmpty(),"Cancel preserves exact clean mapping without creating a link");
        answerConnection(QMessageBox::Yes);
        require(scene.connections.size()==1&&scene.hasConnections(0)&&scene.hasConnections(1),"Yes from actual Connect button creates one panel link");
        require(scene.surfaces[0].corners==leftGeometry&&scene.surfaces[1].corners==rightGeometry,"logical Connect keeps both panel geometries");
        scene.undo();require(scene.json()==beforeConnection&&scene.connections.isEmpty(),"one Undo restores exact pre-Connect mapping");
        scene.undo();
        require(scene.surfaces.size()==surfaceCount,"temporary connection test surface removed by Undo");editMode->setCurrentIndex(0);

        window.canvas->setViewZoom(1.75);require(zoom->value()==175,"canvas change synchronizes zoom display");
        zoom->setValue(125);require(qAbs(window.canvas->viewZoom()-1.25)<.001,"zoom control changes canvas");
        const auto canvasCentre=QPointF(window.canvas->rect().center());
        QWheelEvent wheel(canvasCentre,window.canvas->mapToGlobal(canvasCentre.toPoint()),{},QPoint(0,120),Qt::NoButton,Qt::NoModifier,Qt::NoScrollPhase,false);
        QApplication::sendEvent(window.canvas,&wheel);
        require(zoom->value()==qRound(window.canvas->viewZoom()*100),"wheel zoom synchronizes display");
        click(fit,"Fit view");require(zoom->value()==100&&qAbs(window.canvas->viewZoom()-1)<.001,"Fit view resets canvas and zoom display");
        PotatoMaterialUiCheck::reveal(actorPause,app);require(visible(actorPause)&&sidebar->isAncestorOf(actorsSection),"whole-scene actor controls reachable in sidebar");
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
        QTemporaryDir temp(QDir(potatoInstallRoot()).filePath("studio-check-XXXXXX"));require(temp.isValid(),"disposable feature-check project folder");QString error;
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
                scene.select(0);category->setCurrentIndex(category->findData("halloween"));search->clear();
                if(scene.current())for(int row=0;row<library->count();++row)if(library->item(row)->data(Qt::UserRole).toInt()==scene.current()->pattern){library->setCurrentRow(row);library->scrollToItem(library->item(row),QAbstractItemView::PositionAtCenter);break;}
                browserDialog->close();window.resize(1440,1024);PotatoMaterialUiCheck::reveal(window.template findChild<QWidget*>("animationPicker"),app);settle(150);
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
