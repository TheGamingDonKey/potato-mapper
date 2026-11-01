#pragma once
#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QDialog>
#include <QDebug>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QLabel>
#include <QImage>
#include <QListWidget>
#include <QMenu>
#include <QPushButton>
#include <QScrollArea>
#include <QSplitter>
#include <QTemporaryDir>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>

namespace PotatoMaterialUiCheck {
inline void settle(QApplication &app,int milliseconds=0){
    if(milliseconds){QEventLoop events;QTimer::singleShot(milliseconds,&events,&QEventLoop::quit);events.exec();}
    app.processEvents();
}
inline QRect bounds(QWidget *widget,QWidget *relative){return QRect(widget->mapTo(relative,QPoint()),widget->size());}
inline bool visible(QWidget *widget,QWidget *window){
    return widget&&widget->isVisible()&&!widget->visibleRegion().isEmpty()&&window->rect().contains(bounds(widget,window));
}
// Open the real collapsible sections, then use their real scroll area. No
// production state or test-only widget properties are introduced.
inline void reveal(QWidget *widget,QApplication &app){
    if(!widget)return;
    for(auto *parent=widget->parentWidget();parent;parent=parent->parentWidget())
        if(auto *heading=parent->findChild<QToolButton*>("sectionHeading",Qt::FindDirectChildrenOnly))heading->setChecked(true);
    app.processEvents();
    for(auto *parent=widget->parentWidget();parent;parent=parent->parentWidget())
        if(auto *scroll=qobject_cast<QScrollArea*>(parent))scroll->ensureWidgetVisible(widget,6,6);
    app.processEvents();
}
}

// Catches displaced/hidden legacy controls, lost access to later features,
// wrong context targets, destructive media handling and editor paint leaking
// into output. All saved data is synthetic and scoped to --install-root.
template<class Window> void runMaterialUiCheck(Window &window,QApplication &app,QStringList args){
    QTimer::singleShot(650,&window,[&window,&app,args]{
        using namespace PotatoMaterialUiCheck;
        QStringList failures;auto require=[&](bool ok,const QString &label){if(!ok)failures<<label;};
        auto finish=[&]{window.output->hide();qInfo().noquote()<<"MATERIAL UI CHECK"<<(failures.isEmpty()?"PASS":"FAIL")<<failures.join("; ");app.exit(failures.isEmpty()?0:2);};
        const int rootArg=args.indexOf("--install-root");
        require(rootArg>=0&&rootArg+1<args.size(),"explicit disposable --install-root supplied");
        if(!failures.isEmpty()){finish();return;}
        const auto root=QFileInfo(args[rootArg+1]).absoluteFilePath();
        require(QFileInfo(root).isDir(),"diagnostic install root exists");
        if(!failures.isEmpty()){finish();return;}
        QTemporaryDir fixture(QDir(root).filePath("material-ui-XXXXXX"));require(fixture.isValid(),"disposable fixture directory created inside install root");
        auto *sidebar=window.template findChild<QWidget*>("surfaceSidebar");
        auto *inspector=window.template findChild<QScrollArea*>("surfaceInspector");
        auto *list=window.template findChild<QListWidget*>("surfaceList");
        auto *splitter=window.template findChild<QSplitter*>("editorSplitter");
        auto *projectBar=window.template findChild<QToolBar*>("projectToolbar");
        auto *outputBar=window.template findChild<QToolBar*>("outputToolbar");
        auto *browser=window.template findChild<QDialog*>("animationBrowserDialog");
        require(sidebar&&inspector&&list&&splitter&&projectBar&&outputBar&&browser,"familiar native editor and temporary browser exist");
        require(!window.template findChild<QWidget*>("inspectorPanel")&&!window.template findChild<QWidget*>("rightInspector")&&!window.template findChild<QWidget*>("persistentLibrary")&&!window.template findChild<QWidget*>("librarySplitter"),"rejected persistent inspector and library columns absent");
        if(!failures.isEmpty()){finish();return;}
        require(splitter->count()==2&&splitter->widget(0)==sidebar&&splitter->widget(1)==window.canvas,"splitter retains left sidebar and right canvas only");
        require(sidebar->isAncestorOf(inspector)&&sidebar->isAncestorOf(list),"surface list and inspector remain in left sidebar");
        require(!browser->isVisible(),"animation chooser is temporary rather than persistent");
        auto &scene=window.scene;scene.newProject();scene.setPattern(0,17);scene.dirty=false;
        const auto clean=scene.json();
        const QStringList commands{"New","Open","Save","Save as","Undo","Redo","+ Surface","Load media","Potato FX","Dynamic FX"};
        const QStringList outputNames{"outputDisplays","outputRefresh","outputSettings","outputStart","outputStop","blackoutButton","outputStatus","entrancePlay","entranceSkip","entranceAutomatic","entranceInfo"};
        auto rowOrder=[&](const QStringList &names,const QString &label){
            QRect previous;
            for(const auto &name:names){
                auto *widget=window.template findChild<QWidget*>(name);require(widget,label+" control "+name);
                if(!widget)continue;const auto box=bounds(widget,&window);
                if(previous.isValid())require(box.left()>previous.right()&&qAbs(box.center().y()-previous.center().y())<=4,label+" original order: "+name);
                previous=box;
            }
        };
        for(const auto size:{QSize(1280,800),QSize(1024,700),QSize(960,640)}){
            window.resize(size);settle(app,80);const auto label=QString::number(size.width())+"x"+QString::number(size.height());
            require(visible(sidebar,&window)&&visible(list,&window)&&visible(window.canvas,&window),label+" visible sidebar, list and canvas");
            require(bounds(sidebar,&window).right()<bounds(window.canvas,&window).left(),label+" sidebar stays left of canvas");
            require(window.canvas->width()>=240&&window.canvas->height()>=170,label+" usable canvas");
            require(visible(projectBar,&window)&&visible(outputBar,&window)&&bounds(projectBar,&window).bottom()<bounds(outputBar,&window).top(),label+" separate project and output bands");
            QStringList foundCommands;
            for(auto *action:projectBar->actions())if(commands.contains(action->text())){
                foundCommands<<action->text();require(visible(projectBar->widgetForAction(action),&window),label+" labelled project action visible: "+action->text());
            }
            require(foundCommands==commands,label+" original labelled project command order retained");
            for(const auto &name:outputNames)require(visible(window.template findChild<QWidget*>(name),&window),label+" visible output control "+name);
            rowOrder({"outputDisplays","outputRefresh","outputSettings"},label+" destination row");
            rowOrder({"outputStart","outputStop","blackoutButton","outputStatus"},label+" output row");
            rowOrder({"entrancePlay","entranceSkip","entranceAutomatic","entranceInfo"},label+" Scan row");
            auto *destination=window.template findChild<QWidget*>("outputDisplays");auto *start=window.template findChild<QWidget*>("outputStart");auto *scan=window.template findChild<QWidget*>("entrancePlay");
            if(destination&&start&&scan)require(bounds(destination,&window).bottom()<bounds(start,&window).top()&&bounds(start,&window).bottom()<bounds(scan,&window).top(),label+" original output row sequence");
        }
        require(scene.json()==clean&&!scene.dirty,"resizing preserves clean project");
        window.resize(1280,800);settle(app,50);
        for(const auto &name:QStringList{"brightnessSlider","opacitySlider","blendPicker","animationPicker","browseAnimations","patternSpeed","patternSize","patternDensity","patternFlow","patternAngle","patternPalette","patternEdge","patternReverse","patternPause","dynamicEffect","dynamicMembers","dynamicSnakeWidth","dynamicCellSize","mapEditMode","canvasZoom","fitView","fitConnection","connectSurface","disconnectSurface","tieConnections","boundaryActors","wizardActors","actorSpeed","actorSize","actorPause"}){
            auto *widget=window.template findChild<QWidget*>(name);reveal(widget,app);
            require(widget&&visible(widget,&window)&&sidebar->isAncestorOf(widget),"feature reachable in familiar sidebar: "+name);
        }
        auto *browse=window.template findChild<QPushButton*>("browseAnimations");reveal(browse,app);if(browse)browse->click();settle(app,60);
        require(browser->isVisible(),"real Browse action opens temporary animation chooser");
        for(const auto &name:QStringList{"animationSearch","animationCategory","animationLibrary","applyAnimation"})require(visible(browser->template findChild<QWidget*>(name),browser),"chooser control reachable: "+name);
        browser->close();settle(app);require(!browser->isVisible()&&scene.json()==clean&&!scene.dirty,"chooser cancellation preserves clean mapping");

        // A right click over the second panel must not act on the first panel,
        // which is deliberately selected before each real context event.
        scene.newProject();auto &first=scene.surfaces[0];first.name="Porch window";first.corners={{.08,.16},{.47,.16},{.47,.84},{.08,.84}};first.subdivide(3);
        scene.add();auto &second=scene.surfaces[1];second.name="Door panel";second.corners={{.55,.16},{.92,.16},{.92,.84},{.55,.84}};second.subdivide(3);
        const auto mediaPath=fixture.filePath("original.png");QImage image(32,32,QImage::Format_RGB32);image.fill(QColor(180,85,55));require(image.save(mediaPath),"real disposable media written");
        scene.assignMedia(1,mediaPath);scene.select(0);scene.touch(true);window.canvas->resetView();
        QString error;const auto projectPath=fixture.filePath("fixture.pmap");require(scene.save(projectPath,error),"synthetic project saved: "+error);
        auto bytes=[&](const QString &path){QFile file(path);require(file.open(QIODevice::ReadOnly),"read synthetic fixture: "+QFileInfo(path).fileName());return file.readAll();};
        const auto mediaBytes=bytes(mediaPath),projectBytes=bytes(projectPath);const auto original=scene.json();const auto retainedId=scene.surfaces[1].id;
        window.output->resize(480,270);window.output->show();settle(app,120);const auto outputBefore=window.output->grabFramebuffer();require(!outputBefore.isNull(),"actual projector framebuffer available");
        auto findMenu=[]()->QMenu*{for(auto *widget:QApplication::topLevelWidgets())if(widget->isVisible()&&widget->objectName()=="materialSurfaceMenu")return qobject_cast<QMenu*>(widget);return nullptr;};
        const auto stage=window.canvas->canvasRect();const QPoint local(qRound(stage.x()+stage.width()*.72),qRound(stage.y()+stage.height()*.5));
        auto context=[&](bool remove){
            scene.select(0);bool sawMenu=false;QTimer inspect,watchdog;inspect.setSingleShot(true);watchdog.setSingleShot(true);
            QObject::connect(&inspect,&QTimer::timeout,&window,[&]{
                auto *menu=findMenu();require(menu,"real material context menu opened");if(!menu)return;sawMenu=true;
                require(scene.current()&&scene.current()->name=="Door panel","right click selects pointer target rather than old selection");
                require(window.output->grabFramebuffer()==outputBefore,"material menu leaves projector pixels unchanged");
                auto *removeAction=menu->template findChild<QAction*>("deleteSurfaceAction");require(removeAction&&removeAction->isEnabled(),"real enabled contextual Delete action");
                if(remove&&removeAction)removeAction->trigger();menu->close();
            });
            QObject::connect(&watchdog,&QTimer::timeout,&window,[&]{if(auto *menu=qobject_cast<QMenu*>(QApplication::activePopupWidget()))menu->close();});
            inspect.start(120);watchdog.start(1800);QContextMenuEvent event(QContextMenuEvent::Mouse,local,window.canvas->mapToGlobal(local));QApplication::sendEvent(window.canvas,&event);inspect.stop();watchdog.stop();
            require(sawMenu,"context inspection executed");
        };
        context(false);require(scene.json()==original&&!scene.dirty,"opening and cancelling context menu preserves clean project");
        context(true);require(scene.surfaces.size()==1&&scene.surfaces[0].name=="Porch window","Delete removes only right-clicked surface");
        scene.undo();require(scene.json()==original&&scene.surfaces.size()==2&&scene.surfaces[1].id==retainedId,"one Undo restores exact surface data, order and identity");
        require(window.output->grabFramebuffer()==outputBefore,"Undo restores exact projector output");
        scene.redo();require(scene.surfaces.size()==1,"Redo reapplies contextual deletion");scene.undo();
        require(bytes(mediaPath)==mediaBytes&&bytes(projectPath)==projectBytes,"Delete, Undo and Redo preserve saved project and original media bytes");
        require(window.output->grabFramebuffer()==outputBefore,"final Undo preserves original projector pixels");finish();
    });
}
