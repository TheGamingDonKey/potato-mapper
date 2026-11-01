#pragma once
#include "material-ui-check.h"
#include "../src/appearance.h"
#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QDialog>
#include <QElapsedTimer>
#include <QLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QMenuBar>
#include <QPointer>
#include <QPushButton>
#include <QScrollBar>
#include <QSettings>
#include <QTemporaryDir>
#include <QToolButton>
#include <QTimer>
#include <cmath>

namespace PotatoThemeCheck {
// Remove average luminance before comparing real chrome. A flat recolour
// disappears here; depth, grain and edge treatment remain.
inline double detailDifference(const QImage &a,const QImage &b){
    if(a.size()!=b.size()||a.isNull())return 0;
    const QRect sample(12,8,qMax(1,a.width()/5),qMax(1,a.height()-16));
    double meanA=0,meanB=0;int count=0;
    for(int y=sample.top();y<=sample.bottom();++y)for(int x=sample.left();x<=sample.right();++x){meanA+=qGray(a.pixel(x,y));meanB+=qGray(b.pixel(x,y));++count;}
    if(!count)return 0;meanA/=count;meanB/=count;double difference=0;
    for(int y=sample.top();y<=sample.bottom();++y)for(int x=sample.left();x<=sample.right();++x)
        difference+=qAbs((qGray(a.pixel(x,y))-meanA)-(qGray(b.pixel(x,y))-meanB));
    return difference/count;
}
inline double edgeDifference(const QImage &a,const QImage &b){
    if(a.size()!=b.size()||a.isNull())return 0;double difference=0;int count=0;
    // Border strips exclude the centred label. Local luminance gradients
    // detect the actual edge treatment independently of a flat tint shift.
    for(int y=1;y<a.height()-1;++y)for(int x=1;x<a.width()-1;++x){
        if(x>8&&x<a.width()-9&&y>8&&y<a.height()-9)continue;
        const int ax=qGray(a.pixel(x+1,y))-qGray(a.pixel(x-1,y)),ay=qGray(a.pixel(x,y+1))-qGray(a.pixel(x,y-1));
        const int bx=qGray(b.pixel(x+1,y))-qGray(b.pixel(x-1,y)),by=qGray(b.pixel(x,y+1))-qGray(b.pixel(x,y-1));
        difference+=qAbs(ax-bx)+qAbs(ay-by);++count;
    }
    return count?difference/count:0;
}
inline double luminance(const QColor &color){
    auto linear=[](double channel){channel/=255.;return channel<=.04045?channel/12.92:std::pow((channel+.055)/1.055,2.4);};
    return .2126*linear(color.red())+.7152*linear(color.green())+.0722*linear(color.blue());
}
// The short search term stays left of this region; exclude the clear button,
// cursor, selected text and optical perimeter. Read actual composited pixels,
// so a bright reflection leaking into the text bed can fail this check.
inline double textBedContrast(QLineEdit *field){
    const auto image=field->grab().toImage().convertToFormat(QImage::Format_RGB32);
    if(image.isNull())return 0;
    const QRect bed(image.width()*3/5,image.height()/3,qMax(1,image.width()/5),qMax(1,image.height()/3));
    const double text=luminance(field->palette().color(QPalette::Text));double minimum=100.;
    for(int y=bed.top();y<=bed.bottom();++y)for(int x=bed.left();x<=bed.right();++x){
        const double background=luminance(QColor::fromRgb(image.pixel(x,y)));
        minimum=qMin(minimum,(qMax(text,background)+.05)/(qMin(text,background)+.05));
    }
    return minimum;
}
}

// Synthetic scene only. The caller sets window.smoke=true, so live actions
// cannot overwrite user preferences. Persistence uses disposable INI files.
template<class Window> void runAppearanceThemeChecks(Window &window,QApplication &app,const QStringList &args){
    QTimer::singleShot(400,&window,[&window,&app,args]{
        using namespace PotatoMaterialUiCheck;
        QStringList failures;auto require=[&](bool ok,const QString &label){if(!ok)failures<<label;};
        auto finish=[&]{window.output->hide();qInfo().noquote()<<"THEMES"<<(failures.isEmpty()?"PASS":"FAIL")<<failures.join("; ");app.exit(failures.isEmpty()?0:2);};
        QTemporaryDir preferencesDirectory;require(preferencesDirectory.isValid(),"disposable preferences directory");
        if(!preferencesDirectory.isValid()){finish();return;}
        const auto preferencesPath=preferencesDirectory.filePath("appearance.ini");QSettings preferences(preferencesPath,QSettings::IniFormat);
        auto read=PotatoAppearance::readPreferences(preferences);
        require(read.interfaceTheme=="material"&&read.accent=="classic1","unset preferences keep installed Material default");
        for(const auto &id:QStringList{"classic1","halloween","autumn","christmas"}){
            preferences.setValue("ui/theme",id);const auto keys=preferences.allKeys();read=PotatoAppearance::readPreferences(preferences);
            require(read.interfaceTheme=="material"&&read.accent==id&&preferences.allKeys()==keys,"legacy accent migration is read-only: "+id);
        }
        preferences.setValue("ui/interfaceTheme","classic1");preferences.setValue("ui/accent","autumn");read=PotatoAppearance::readPreferences(preferences);
        require(read.interfaceTheme=="classic1"&&read.accent=="autumn","explicit interface and accent override legacy preference");
        preferences.setValue("ui/interfaceTheme","unknown");preferences.setValue("ui/accent","unknown");read=PotatoAppearance::readPreferences(preferences);
        require(read.interfaceTheme=="material"&&read.accent=="classic1","unknown preferences normalize safely");
        preferences.setValue("ui/reducedTransparency",true);preferences.setValue("ui/increasedContrast",true);
        for(const auto &id:QStringList{"classic1","material","liquidglass1"}){
            PotatoAppearance::writePreferences(preferences,{id,"christmas"});preferences.sync();QSettings reopened(preferencesPath,QSettings::IniFormat);read=PotatoAppearance::readPreferences(reopened);
            require(reopened.status()==QSettings::NoError&&read.interfaceTheme==id&&read.accent=="christmas"&&reopened.value("ui/theme").toString()=="christmas","new keys survive reopen and retain legacy accent: "+id);
            require(reopened.value("ui/reducedTransparency").toBool()&&reopened.value("ui/increasedContrast").toBool(),"saving interface does not discard independent glass accessibility preferences: "+id);
        }

        auto *browser=window.template findChild<QDialog*>("animationBrowserDialog");
        auto *browse=window.template findChild<QPushButton*>("browseAnimations");
        auto *closeButton=window.template findChild<QPushButton*>("animationBrowserClose");
        auto *connections=window.template findChild<QWidget*>("connectionsSection");
        auto *picker=window.template findChild<QComboBox*>("animationPicker");
        auto *category=window.template findChild<QComboBox*>("animationCategory");
        auto *search=window.template findChild<QLineEdit*>("animationSearch");
        auto *library=window.template findChild<QListWidget*>("animationLibrary");
        auto *apply=window.template findChild<QPushButton*>("applyAnimation");
        auto *classic=window.template findChild<QAction*>("interfaceTheme_classic1");
        auto *material=window.template findChild<QAction*>("interfaceTheme_material");
        auto *glass=window.template findChild<QAction*>("interfaceTheme_liquidglass1");
        auto *reduceTransparency=window.template findChild<QAction*>("reduceInterfaceTransparency");
        auto *increaseContrast=window.template findChild<QAction*>("increaseInterfaceContrast");
        auto *splitter=window.template findChild<QSplitter*>("editorSplitter");
        auto *projectBar=window.template findChild<QToolBar*>("projectToolbar");
        auto *outputBar=window.template findChild<QToolBar*>("outputToolbar");
        auto *chrome=window.template findChild<QObject*>("potatoChromeMaterial");
        QMenu *existingMenu=nullptr;for(auto *action:window.menuBar()->actions())if(action->text()=="Settings")existingMenu=action->menu();
        require(browser&&browse&&closeButton&&connections&&picker&&category&&search&&library&&apply&&classic&&material&&glass&&reduceTransparency&&increaseContrast&&splitter&&projectBar&&outputBar&&chrome&&existingMenu,"native three-profile themes, accessibility, layout, browser and material controls exist");
        if(!failures.isEmpty()){finish();return;}
        reduceTransparency->setChecked(false);increaseContrast->setChecked(false);

        // Both history directions are populated. Later real Undo/Redo catches
        // lost checkpoints or preference changes inserted into project history.
        auto &scene=window.scene;scene.newProject();scene.setPattern(0,1);scene.current()->patternPlaying=false;const auto undoState=scene.json();
        scene.setPattern(0,17);scene.current()->patternPlaying=false;const auto saved=scene.json();
        scene.setPattern(0,18);scene.current()->patternPlaying=false;const auto redoState=scene.json();scene.undo();scene.dirty=false;
        const auto pattern=scene.current()->pattern,selected=scene.selected;const auto phase=scene.current()->patternPhase;
        const auto *scenePointer=&scene;const auto *canvasPointer=window.canvas;const auto *outputPointer=window.output;
        struct Identity {QString name;QPointer<QWidget> widget,parent;QLayout *layout;};QList<Identity> identities;
        for(const auto &name:QStringList{"surfaceSidebar","surfaceInspector","surfaceList","editorSplitter","projectToolbar","outputToolbar","animationBrowserDialog","animationBrowserClose","animationPicker","connectionsSection"}){
            auto *widget=window.template findChild<QWidget*>(name);require(widget,"retained control exists: "+name);if(widget)identities.append(Identity{name,widget,widget->parentWidget(),widget->layout()});
        }
        const auto projectActions=projectBar->actions(),outputActions=outputBar->actions();
        window.resize(1280,800);material->trigger();settle(app,80);reveal(browse,app);browse->click();settle(app,60);
        require(browser->isVisible()&&library->isVisible(),"animation browser reachable from sidebar");
        const auto oldMinimum=closeButton->minimumSize(),oldMaximum=closeButton->maximumSize();closeButton->setFixedSize(196,42);closeButton->clearFocus();
        browser->resize(760,520);browser->move(window.mapToGlobal(QPoint(70,40)));settle(app,40);
        auto *inspector=window.template findChild<QScrollArea*>("surfaceInspector");
        require(inspector,"native sidebar scroll area retained");
        if(inspector)inspector->verticalScrollBar()->setValue(qMin(80,inspector->verticalScrollBar()->maximum()/2));
        const auto browserGeometry=browser->geometry();const auto splitterSizes=splitter->sizes();
        const int sidebarScroll=inspector?inspector->verticalScrollBar()->value():0;
        window.output->resize(480,270);window.output->show();settle(app,100);const auto windowGeometry=window.geometry();const auto outputBefore=window.output->grabFramebuffer();require(!outputBefore.isNull(),"actual paused projector framebuffer available");
        // Keep the actual production context-menu class alive across switches
        // to catch private Material QSS or flags accidentally overriding Classic.
        MaterialSurfaceMenu surfaceMenu(&window);surfaceMenu.addAction("Keep mapped surface");surfaceMenu.addAction("Choose animation…");
        const QList<QMenu*> menus{existingMenu,&surfaceMenu};QImage classicImage,materialImage,glassImage,materialBand,glassBand;
        for(const auto &id:QStringList{"material","classic1","liquidglass1","material","liquidglass1","classic1","material","liquidglass1"}){
            auto *action=id=="classic1"?classic:id=="material"?material:glass;action->trigger();settle(app,70);const bool useMaterial=id!="classic1";
            require(app.property("potatoInterfaceTheme").toString()==id&&int(classic->isChecked())+int(material->isChecked())+int(glass->isChecked())==1&&action->isChecked(),"live exclusive interface action: "+id);
            require(chrome->property("potatoMaterialEnabled").toBool()==useMaterial,"chrome renderer follows interface: "+id);
            const auto paints=chrome->property("potatoMaterialPaintCount").toULongLong();
            window.repaint();browser->repaint();const auto image=closeButton->grab().toImage().convertToFormat(QImage::Format_RGB32);settle(app);
            require(!image.isNull(),"actual fixed native button captured: "+id);
            require(useMaterial?chrome->property("potatoMaterialPaintCount").toULongLong()>paints:chrome->property("potatoMaterialPaintCount").toULongLong()==paints,"custom chrome paints only for optical interfaces: "+id);
            if(id=="liquidglass1"){glassImage=image;glassBand=projectBar->grab().toImage();}
            else if(id=="material"){materialImage=image;materialBand=projectBar->grab().toImage();}else classicImage=image;
            for(auto *menu:menus){
                auto *renderer=menu->template findChild<QObject*>("potatoMenuMaterial");require(renderer,"existing menu renderer retained: "+id);if(!renderer)continue;
                const auto captures=renderer->property("potatoMaterialCaptureCount").toULongLong(),menuPaints=renderer->property("potatoMaterialPaintCount").toULongLong();
                menu->popup(window.mapToGlobal(QPoint(400,160)));settle(app,45);require(menu->isVisible()&&!menu->grab().isNull(),"real popup opens after switch: "+id);
                require(renderer->property("potatoMaterialEnabled").toBool()==useMaterial,"existing popup material state follows interface: "+id);
                require(useMaterial?renderer->property("potatoMaterialCaptureCount").toULongLong()>captures:renderer->property("potatoMaterialCaptureCount").toULongLong()==captures,"menu samples editor only for optical interfaces: "+id);
                require(useMaterial?renderer->property("potatoMaterialPaintCount").toULongLong()>menuPaints:renderer->property("potatoMaterialPaintCount").toULongLong()==menuPaints,"menu paints glass only for optical interfaces: "+id);
                if(id=="liquidglass1"){
                    const auto openingCaptures=renderer->property("potatoMaterialCaptureCount").toULongLong();
                    menu->repaint();settle(app,60);menu->repaint();settle(app,60);
                    require(renderer->property("potatoMaterialCaptureCount").toULongLong()==openingCaptures,"visible glass menu does not resample on repaint or idle");
                }
                if(!useMaterial)require(!menu->testAttribute(Qt::WA_TranslucentBackground)&&!menu->windowFlags().testFlag(Qt::FramelessWindowHint),"Classic restores opaque ordinary popup flags");
                menu->close();settle(app);require(!menu->isVisible(),"popup dismissal survives switch: "+id);
            }
            for(const auto &identity:identities)require(identity.widget&&window.template findChild<QWidget*>(identity.name)==identity.widget&&identity.widget->parentWidget()==identity.parent&&identity.widget->layout()==identity.layout,"same control, parent and layout: "+identity.name+" "+id);
            require(&window.scene==scenePointer&&window.canvas==canvasPointer&&window.output==outputPointer&&projectBar->actions()==projectActions&&outputBar->actions()==outputActions,"scene, canvas, output and command identities retained: "+id);
            require(splitter->count()==2&&splitter->widget(0)==window.template findChild<QWidget*>("surfaceSidebar")&&splitter->widget(1)==window.canvas&&bounds(splitter->widget(0),&window).right()<bounds(window.canvas,&window).left(),"familiar left sidebar and right canvas retained: "+id);
            require(window.geometry()==windowGeometry&&browser->geometry()==browserGeometry&&splitter->sizes()==splitterSizes&&inspector&&inspector->verticalScrollBar()->value()==sidebarScroll,"window and moved browser geometry, splitter sizes and sidebar scroll retained: "+id);
            require(browser->isVisible()&&scene.json()==saved&&!scene.dirty&&scene.selected==selected&&scene.current()->patternPhase==phase,"live browser and clean selected mapping retained: "+id);
            require(window.output->grabFramebuffer()==outputBefore,"interface leaves exact paused projector pixels unchanged: "+id);
        }
        const double detail=PotatoThemeCheck::detailDifference(classicImage,materialImage),edges=PotatoThemeCheck::edgeDifference(classicImage,materialImage);
        qInfo()<<"THEME DESIGN fixed-button luminance detail"<<detail<<"border gradient difference"<<edges;
        require(classicImage.size()==materialImage.size()&&classicImage!=materialImage&&detail>.35&&edges>.5,"Classic and Material change real chrome detail and edges beyond a flat recolour");
        const double glassDetail=PotatoThemeCheck::detailDifference(materialImage,glassImage),glassEdges=PotatoThemeCheck::edgeDifference(materialImage,glassImage),bandDetail=PotatoThemeCheck::detailDifference(materialBand,glassBand);
        qInfo()<<"GLASS DESIGN button luminance detail"<<glassDetail<<"border gradient difference"<<glassEdges<<"command-band detail"<<bandDetail;
        require(materialImage.size()==glassImage.size()&&glassDetail>.35&&glassEdges>.5&&bandDetail>.35,"Liquid Glass changes real control rims and persistent command-band structure beyond a Material recolour");
        // An already-open native popup must safely leave the custom paint path,
        // then restore its ordinary flags when hidden and reopen optically.
        auto *popupRenderer=surfaceMenu.template findChild<QObject*>("potatoMenuMaterial");
        surfaceMenu.popup(window.mapToGlobal(QPoint(400,160)));settle(app,45);require(surfaceMenu.isVisible(),"glass popup visible before a real live profile switch");
        classic->trigger();settle(app,45);require(popupRenderer&&!popupRenderer->property("potatoMaterialEnabled").toBool(),"visible popup immediately leaves glass rendering on Classic switch");
        surfaceMenu.close();settle(app,30);
        require(!surfaceMenu.isVisible()&&!surfaceMenu.testAttribute(Qt::WA_TranslucentBackground)&&!surfaceMenu.windowFlags().testFlag(Qt::FramelessWindowHint),"live-switched popup restores ordinary Classic native flags after hide");
        glass->trigger();surfaceMenu.popup(window.mapToGlobal(QPoint(400,160)));settle(app,45);
        require(surfaceMenu.isVisible()&&popupRenderer&&popupRenderer->property("potatoMaterialEnabled").toBool()&&!surfaceMenu.grab().isNull(),"live-switched popup reopens with actual glass content");surfaceMenu.close();settle(app,30);
        require(scene.json()==saved&&!scene.dirty&&window.output->grabFramebuffer()==outputBefore&&browser->geometry()==browserGeometry&&window.geometry()==windowGeometry,"visible popup switching preserves mapping, projector pixels and window geometry");
        // Observe a small warm batch, without a hardware-dependent FPS gate.
        // A capture-on-paint or continually rebuilt cache must still fail.
        window.repaint();browser->repaint();settle(app,40);window.repaint();browser->repaint();
        const auto environmentBuilds=app.property("potatoGlassEnvironmentBuilds").toULongLong(),plateBuilds=app.property("potatoGlassPlateBuilds").toULongLong();
        QList<QPair<QObject*,qulonglong>> idleCaptures;for(auto *menu:menus)if(auto *renderer=menu->template findChild<QObject*>("potatoMenuMaterial"))idleCaptures.append({renderer,renderer->property("potatoMaterialCaptureCount").toULongLong()});
        QElapsedTimer repaintTime;repaintTime.start();constexpr int repaintBatch=8;
        for(int i=0;i<repaintBatch;++i){window.repaint();browser->repaint();}
        const double milliseconds=repaintTime.nsecsElapsed()/1000000.;settle(app,80);
        const auto environmentBytes=app.property("potatoGlassEnvironmentBytes").toLongLong(),cacheBytes=app.property("potatoGlassCacheBytes").toLongLong();
        const auto environmentBuildsAfter=app.property("potatoGlassEnvironmentBuilds").toULongLong(),plateBuildsAfter=app.property("potatoGlassPlateBuilds").toULongLong();
        qInfo()<<"GLASS WARM REPAINT batch"<<repaintBatch<<"milliseconds"<<milliseconds<<"per editor+dialog repaint"<<milliseconds/repaintBatch<<"environment bytes"<<environmentBytes<<"cache bytes"<<cacheBytes<<"environment builds before/after"<<environmentBuilds<<environmentBuildsAfter<<"plate builds before/after"<<plateBuilds<<plateBuildsAfter;
        require(environmentBytes>0&&environmentBytes<=4000000&&cacheBytes>0&&cacheBytes<=16777216,"observed glass environment and plate cache stay within bounded byte budgets");
        require(environmentBuildsAfter==environmentBuilds&&plateBuildsAfter==plateBuilds,"warm repaint and idle reuse the optical environment and plates");
        for(const auto &entry:idleCaptures)require(entry.first->property("potatoMaterialCaptureCount").toULongLong()==entry.second,"closed menus do not capture during editor repaint or idle");

        // A stylesheet refresh must preserve an in-progress input operation,
        // as well as the mapping and floating dialog it belongs to.
        search->setText("Ghost");search->setSelection(1,3);browser->raise();browser->activateWindow();settle(app,30);search->setFocus(Qt::OtherFocusReason);settle(app,30);
        const auto inputText=search->text(),inputSelection=search->selectedText();const int inputCursor=search->cursorPosition(),inputSelectionStart=search->selectionStart();
        require(search->hasFocus(),"real search input focused before glass accessibility changes");
        QImage normalGlass,reducedGlass,contrastGlass;
        for(const auto flags:QList<QPair<bool,bool>>{{false,false},{true,false},{true,true},{false,true},{false,false}}){
            reduceTransparency->setChecked(flags.first);increaseContrast->setChecked(flags.second);settle(app,70);
            require(app.property("potatoInterfaceTheme").toString()=="liquidglass1"&&glass->isChecked()&&app.property("potatoReducedTransparency").toBool()==flags.first&&app.property("potatoIncreasedContrast").toBool()==flags.second,"glass accessibility actions apply their independent live state");
            require(search->hasFocus()&&search->text()==inputText&&search->selectedText()==inputSelection&&search->cursorPosition()==inputCursor&&search->selectionStart()==inputSelectionStart,"glass accessibility changes retain edit text, selection, cursor and keyboard focus");
            require(scene.json()==saved&&!scene.dirty&&scene.selected==selected&&scene.current()->patternPhase==phase&&window.output->grabFramebuffer()==outputBefore,"glass accessibility changes preserve clean selected mapping and exact projector pixels");
            require(browser->isVisible()&&window.geometry()==windowGeometry&&browser->geometry()==browserGeometry&&splitter->sizes()==splitterSizes&&inspector&&inspector->verticalScrollBar()->value()==sidebarScroll,"glass accessibility changes retain window/open dialog geometry, splitter and sidebar scroll");
            const double contrast=PotatoThemeCheck::textBedContrast(search);qInfo()<<"GLASS TEXT BED reduced"<<flags.first<<"contrast"<<flags.second<<"ratio"<<contrast;
            require(contrast>=4.5,"actual composited glass search bed meets normal-text contrast");
            const auto frame=closeButton->grab().toImage();if(!flags.first&&!flags.second)normalGlass=frame;else if(flags.first&&!flags.second)reducedGlass=frame;else if(!flags.first&&flags.second)contrastGlass=frame;
        }
        require(!normalGlass.isNull()&&normalGlass!=reducedGlass&&normalGlass!=contrastGlass,"reduced transparency and stronger contrast visibly affect actual glass chrome");
        for(const auto flags:QList<QPair<bool,bool>>{{true,false},{false,true}}){
            reduceTransparency->setChecked(flags.first);increaseContrast->setChecked(flags.second);settle(app,40);
            const auto captures=popupRenderer?popupRenderer->property("potatoMaterialCaptureCount").toULongLong():0;
            surfaceMenu.popup(window.mapToGlobal(QPoint(400,160)));settle(app,45);
            require(surfaceMenu.isVisible()&&!surfaceMenu.grab().isNull(),"real fallback glass popup opens and paints");
            require(popupRenderer&&popupRenderer->property("potatoMaterialCaptureCount").toULongLong()==captures&&popupRenderer->property("potatoGlassMenuLensedPixels").toLongLong()==0,"reduced transparency or increased contrast popup performs no editor capture or lensing");
            surfaceMenu.close();settle(app,25);require(scene.json()==saved&&!scene.dirty&&window.output->grabFramebuffer()==outputBefore,"fallback popup preserves mapping and exact projector output");
        }
        reduceTransparency->setChecked(false);increaseContrast->setChecked(false);settle(app,40);
        closeButton->setMinimumSize(oldMinimum);closeButton->setMaximumSize(oldMaximum);
        scene.undo();require(scene.json()==undoState,"interface switches preserve the prior real Undo checkpoint");scene.redo();require(scene.json()==saved,"interface switches preserve Redo back to selected content");scene.redo();require(scene.json()==redoState,"interface switches preserve the existing future checkpoint");scene.undo();scene.dirty=false;

        material->trigger();QStringList styles;
        for(const auto &id:QStringList{"classic1","halloween","autumn","christmas"}){
            auto *action=window.template findChild<QAction*>("theme_"+id);require(action,"accent action "+id);
            if(action){action->trigger();styles<<app.styleSheet();require(app.property("potatoTheme").toString()==id&&app.property("potatoInterfaceTheme").toString()=="material","accent applies independently: "+id);}
            require(scene.json()==saved&&!scene.dirty,"accent keeps mapping and clean state: "+id);
        }
        styles.removeDuplicates();require(styles.size()==4,"four distinct Material accent palettes");
        classic->trigger();const auto classicStyle=app.styleSheet();auto *autumn=window.template findChild<QAction*>("theme_autumn");if(autumn)autumn->trigger();require(app.styleSheet()==classicStyle&&app.property("potatoInterfaceTheme").toString()=="classic1"&&app.property("potatoTheme").toString()=="autumn","Classic stays original while retaining accent preference");material->trigger();
        glass->trigger();const auto glassStyle=app.styleSheet();auto *halloween=window.template findChild<QAction*>("theme_halloween");if(halloween)halloween->trigger();require(app.styleSheet()==glassStyle&&app.property("potatoInterfaceTheme").toString()=="liquidglass1"&&app.property("potatoTheme").toString()=="halloween","Liquid Glass retains its own optical palette while remembering Material accent");material->trigger();
        category->setCurrentIndex(category->findData("christmas"));search->setText("Snowfall");
        require(scene.current()->pattern==pattern&&scene.json()==saved&&!scene.dirty,"category and search preserve assigned Halloween content");
        require(picker->findData(17)>=0,"assigned content remains editable across categories");
        for(int row=0;row<library->count();++row)require(library->item(row)->data(Qt::UserRole).toInt()>=21,"Christmas category contains Christmas IDs");
        category->setCurrentIndex(category->findData("halloween"));search->setText("Ghost Parade");
        require(library->count()==1&&library->item(0)->data(Qt::UserRole).toInt()==17,"filtered Ghost Parade keeps stable ID 17");
        scene.setPattern(scene.selected,1);scene.dirty=false;apply->click();require(scene.current()->pattern==17,"Apply resolves filtered item data, not row zero");scene.undo();require(scene.current()->pattern==1,"Apply supports Undo");
        browser->close();auto *connectionHeading=connections->template findChild<QToolButton*>("sectionHeading");require(connectionHeading,"direct Connections section navigation");
        if(connectionHeading){connectionHeading->setChecked(true);scene.select(0);require(connectionHeading->isChecked(),"selection preserves expanded Connections section");}
        require(!window.template findChild<QWidget*>("taskTabs"),"restored sidebar does not require Studio task tabs");
        window.resize(1024,640);reveal(picker,app);app.processEvents();
        const int snapshot=args.indexOf("--snapshot");if(snapshot>=0&&snapshot+1<args.size()){
            const QFileInfo path(args[snapshot+1]);require(window.grab().save(path.absoluteFilePath()),"save actual task UI snapshot");
            require(classicImage.save(path.dir().filePath(path.completeBaseName()+"-classic-control.png"))&&materialImage.save(path.dir().filePath(path.completeBaseName()+"-material-control.png"))&&glassImage.save(path.dir().filePath(path.completeBaseName()+"-liquidglass-control.png")),"save real three-profile control evidence");
        }
        finish();
    });
}
