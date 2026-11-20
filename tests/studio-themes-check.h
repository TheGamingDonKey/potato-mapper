#pragma once
#include "appearance-theme-check.h"
#include "../src/studio-themes.h"
#include <QCheckBox>
#include <QSlider>
#include <QVariantMap>

namespace PotatoStudioCheck {
inline double difference(const QImage &a,const QImage &b){
    if(a.isNull()||a.size()!=b.size())return -1;
    double sum=0;quint64 count=0;
    for(int y=2;y<a.height()-2;y+=3)for(int x=2;x<a.width()-2;x+=3){
        const auto first=a.pixel(x,y),second=b.pixel(x,y);
        sum+=qAbs(qRed(first)-qRed(second))+qAbs(qGreen(first)-qGreen(second))+qAbs(qBlue(first)-qBlue(second));count+=3;
    }
    return count?sum/count:0;
}
inline QVariantMap storedSettings(){
    QSettings settings;QVariantMap values;
    for(const auto &key:settings.allKeys())values.insert(key,settings.value(key));
    return values;
}
}

// Only disposable media/project files and INI settings are written. Production
// actions run with smoke=true so their normal user-preference writes are skipped.
template<class Window> void runStudioThemesChecks(Window &window,QApplication &app,const QStringList &args){
    QTimer::singleShot(450,&window,[&window,&app,args]{
        using namespace PotatoMaterialUiCheck;
        QStringList failures;int assertions=0;
        auto require=[&](bool ok,const QString &label){++assertions;if(!ok)failures<<label;};
        const auto userSettings=PotatoStudioCheck::storedSettings();
        auto finish=[&]{
            window.output->hide();
            require(PotatoStudioCheck::storedSettings()==userSettings,"no actual user settings changed");
            qInfo().noquote()<<"STUDIO THEMES"<<(failures.isEmpty()?"PASS":"FAIL")<<assertions<<"checks"<<failures.join("; ");
            app.exit(failures.isEmpty()?0:2);
        };
        require(window.smoke,"diagnostic has preference-write suppression");
        if(!window.smoke){finish();return;}
        const QStringList ids{"lumenatelier1","nocturnecinema1","siennastudio1","tidallens1"};
        const QStringList names{"Lumen Atelier","Nocturne Cinema","Sienna Studio","Tidal Lens"};
        QMap<QString,QAction*> actions;
        for(int i=0;i<ids.size();++i){
            auto *action=window.template findChild<QAction*>("interfaceTheme_"+ids[i]);
            require(action&&action->isCheckable()&&action->text()==names[i],"actual named studio menu action: "+ids[i]);
            if(action){
                actions.insert(ids[i],action);bool inMenu=false;
                for(auto *menu:window.template findChildren<QMenu*>())if(menu->actions().contains(action))inMenu=true;
                require(inMenu,"studio action belongs to actual native menu: "+ids[i]);
            }
        }
        for(const auto &id:QStringList{"classic1","material","auroraclear1","ambientglass1"})
            require(window.template findChild<QAction*>("interfaceTheme_"+id),"retained interface action: "+id);
        require(!window.template findChild<QAction*>("interfaceTheme_liquidglass1")&&!window.template findChild<QAction*>("interfaceTheme_transparentglass1"),"rejected menu choices removed");
        auto *pause=window.template findChild<QAction*>("themeMotionPause");
        auto *ambientPause=window.template findChild<QAction*>("ambientPause");
        auto *adjust=window.template findChild<QAction*>("themeMotionSettings");
        auto *reduceMotion=window.template findChild<QAction*>("reduceInterfaceMotion");
        auto *reduceTransparency=window.template findChild<QAction*>("reduceInterfaceTransparency");
        auto *increaseContrast=window.template findChild<QAction*>("increaseInterfaceContrast");
        auto *projectBar=window.template findChild<QToolBar*>("projectToolbar");
        auto *outputBar=window.template findChild<QToolBar*>("outputToolbar");
        auto *sidebar=window.template findChild<QWidget*>("surfaceSidebar");
        auto *splitter=window.template findChild<QSplitter*>("editorSplitter");
        require(pause&&ambientPause&&adjust&&reduceMotion&&reduceTransparency&&increaseContrast&&projectBar&&outputBar&&sidebar&&splitter,"motion, accessibility and original workspace controls retained");
        if(!failures.isEmpty()){finish();return;}
        QTemporaryDir fixture;require(fixture.isValid(),"disposable fixture directory");
        if(!fixture.isValid()){finish();return;}
        const auto iniPath=fixture.filePath("preferences.ini");QSettings ini(iniPath,QSettings::IniFormat);
        for(const auto &migration:QList<QPair<QString,QString>>{{"transparentglass1","lumenatelier1"},{"liquidglass1","nocturnecinema1"}}){
            ini.clear();ini.setValue("ui/interfaceTheme",migration.first);ini.setValue("ui/accent","autumn");
            const auto keys=ini.allKeys();const auto preference=PotatoAppearance::readPreferences(ini);
            require(preference.interfaceTheme==migration.second&&preference.accent=="autumn"&&ini.allKeys()==keys&&ini.value("ui/interfaceTheme")==migration.first,"legacy migration is correct and read-only: "+migration.first);
            PotatoAppearance::writePreferences(ini,preference);ini.sync();QSettings reopened(iniPath,QSettings::IniFormat);
            require(reopened.status()==QSettings::NoError&&reopened.value("ui/interfaceTheme")==migration.second&&PotatoAppearance::readPreferences(reopened).interfaceTheme==migration.second,"migrated theme survives INI reopen: "+migration.first);
        }
        for(const auto &id:ids){
            PotatoAppearance::writePreferences(ini,{id,"christmas"});PotatoAppearance::writeAmbientPreferences(ini,{"clear",true,90,80});ini.sync();
            QSettings reopened(iniPath,QSettings::IniFormat);const auto motion=PotatoAppearance::readAmbientPreferences(reopened);
            require(reopened.status()==QSettings::NoError&&PotatoAppearance::readPreferences(reopened).interfaceTheme==id&&motion.glass=="clear"&&motion.paused&&motion.speed==90&&motion.intensity==80,"theme and shared motion settings survive reopen: "+id);
        }
        ini.setValue("ui/interfaceTheme","unknown");require(PotatoAppearance::readPreferences(ini).interfaceTheme=="material","unknown interface remains Material");

        reduceMotion->setChecked(false);reduceTransparency->setChecked(false);increaseContrast->setChecked(false);pause->setChecked(true);
        window.resize(1280,800);actions.value(ids.first())->trigger();settle(app,100);
        auto &scene=window.scene;scene.newProject();scene.surfaces[0].name="Pattern panel";
        scene.surfaces[0].corners={{.08,.16},{.47,.16},{.47,.84},{.08,.84}};scene.surfaces[0].subdivide(3);
        scene.add();scene.surfaces[1].name="Media panel";scene.surfaces[1].corners={{.55,.16},{.92,.16},{.92,.84},{.55,.84}};scene.surfaces[1].subdivide(3);
        const auto mediaPath=fixture.filePath("original.png"),projectPath=fixture.filePath("fixture.pmap");
        QImage media(32,32,QImage::Format_RGB32);media.fill(QColor(180,85,55));require(media.save(mediaPath),"synthetic media written");scene.assignMedia(1,mediaPath);scene.select(0);
        scene.setPattern(0,1);scene.current()->patternPlaying=false;const auto undoState=scene.json();
        scene.setPattern(0,17);scene.current()->patternPlaying=false;const auto saved=scene.json();
        scene.setPattern(0,18);scene.current()->patternPlaying=false;const auto redoState=scene.json();scene.undo();scene.dirty=false;
        QString error;require(scene.save(projectPath,error),"synthetic project saved: "+error);
        auto bytes=[&](const QString &path){QFile file(path);require(file.open(QIODevice::ReadOnly),"read disposable fixture");return file.readAll();};
        const auto mediaBytes=bytes(mediaPath),projectBytes=bytes(projectPath);const auto selected=scene.selected;const auto phase=scene.current()->patternPhase;
        const auto projectActions=projectBar->actions(),outputActions=outputBar->actions();const auto splitterSizes=splitter->sizes();const auto windowSize=window.size();
        const auto nativeWindowId=window.winId();
        struct Identity {QString name;QPointer<QWidget> widget,parent;QLayout *layout;};QList<Identity> identities;
        for(const auto &name:QStringList{"surfaceSidebar","surfaceInspector","surfaceList","editorSplitter","projectToolbar","outputToolbar","animationBrowserDialog","animationPicker","connectionsSection"}){
            auto *widget=window.template findChild<QWidget*>(name);require(widget,"retained widget: "+name);if(widget)identities.append({name,widget,widget->parentWidget(),widget->layout()});
        }
        window.canvas->resetView();window.output->resize(480,270);window.output->show();settle(app,120);
        const auto outputBefore=window.output->grabFramebuffer();require(!outputBefore.isNull(),"actual projector framebuffer available");
        auto seconds=[&]{return app.property("potatoStudioThemeSeconds").toDouble();};
        auto frames=[&]{return app.property("potatoStudioThemeFrames").toULongLong();};
        // Sienna intentionally has an opaque ink project deck; the output band
        // carries the environment in all four themes and excludes OpenGL content.
        auto composedChrome=[&](QWidget *widget){
            const auto frame=window.grab().toImage();const auto origin=widget->mapTo(&window,QPoint());const auto dpr=frame.devicePixelRatio();
            return frame.copy(QRect(qRound(origin.x()*dpr),qRound(origin.y()*dpr),qRound(widget->width()*dpr),qRound(widget->height()*dpr)));
        };
        // Native glass inherits the editor's reflection. A child-only grab
        // omits that backing, so inspect the actual composed window region.
        auto chrome=[&]{return composedChrome(outputBar).convertToFormat(QImage::Format_RGB32);};
        auto unchanged=[&](const QString &label){
            require(scene.json()==saved&&!scene.dirty&&scene.selected==selected&&scene.current()->patternPhase==phase&&scene.projectPath==projectPath,label+" preserves clean scene, current project, selection and phase");
            require(window.output->grabFramebuffer()==outputBefore,label+" preserves exact projector pixels");
        };
        const QStringList commandOrder{"New","Open","Save","Save as","Undo","Redo","+ Surface","Load media","Potato FX","Dynamic FX"};
        auto *browse=window.template findChild<QPushButton*>("browseAnimations");
        auto *browser=window.template findChild<QDialog*>("animationBrowserDialog");
        auto *search=window.template findChild<QLineEdit*>("animationSearch");
        require(browse&&browser&&search,"real animation browser input available for readability checks");
        if(!browse||!browser||!search){finish();return;}
        reveal(browse,app);browse->click();settle(app,80);search->setText("Ghost");search->clearFocus();
        require(browser->isVisible(),"real labelled animation browser opens");
        QMap<QString,QImage> stills;
        for(const auto &id:ids){
            actions.value(id)->trigger();settle(app,100);int checked=0;for(auto *action:window.template findChildren<QAction*>())if(action->objectName().startsWith("interfaceTheme_")&&action->isChecked())++checked;
            require(app.property("potatoInterfaceTheme").toString()==id&&PotatoThemes::isTheme(id)&&checked==1&&actions.value(id)->isChecked(),"exclusive live theme selection: "+id);
            require(window.winId()==nativeWindowId&&!(window.windowFlags()&Qt::FramelessWindowHint),"theme switch retains native window and standard Windows frame: "+id);
            if(id=="lumenatelier1"){
                auto *primary=window.template findChild<QPushButton*>("outputStart");
                auto *displayPicker=window.template findChild<QComboBox*>("outputDisplays");
                const int previousDisplay=displayPicker?displayPicker->currentIndex():-1;
                // With no saved display, the real Start button is disabled.
                // Select a display without starting output so the palette and
                // painted material both describe the enabled state under test.
                if(displayPicker&&displayPicker->count()>1)displayPicker->setCurrentIndex(1);
                settle(app,30);
                require(primary,"actual primary output control available for contrast check");
                require(primary&&primary->isEnabled(),"real display selection enables primary output control");
                if(primary)for(const bool hover:{false,true}){
                    // Render the production material without text, then inspect
                    // the full font-height bed; hover must retain normal-text contrast.
                    QImage bed(primary->size(),QImage::Format_RGB32);bed.fill(Qt::black);
                    {QPainter painter(&bed);PotatoThemes::paintSurface(painter,QRectF(bed.rect()),PotatoThemes::Role::Button,
                        primary->mapTo(&window,QPoint()),window.size(),32|(hover?1:0),QString(),PotatoDesktopGlass::active(&window));}
                    const int top=qMax(2,(bed.height()-primary->fontMetrics().height())/2);
                    const QRect textBed(bed.width()/4,top,qMax(1,bed.width()/2),qMin(primary->fontMetrics().height(),bed.height()-top-2));
                    const double text=PotatoThemeCheck::luminance(primary->palette().color(QPalette::ButtonText));double minimum=100;
                    for(int y=textBed.top();y<=textBed.bottom();++y)for(int x=textBed.left();x<=textBed.right();++x){
                        const double background=PotatoThemeCheck::luminance(bed.pixelColor(x,y));
                        minimum=qMin(minimum,(qMax(text,background)+.05)/(qMin(text,background)+.05));
                    }
                    require(!textBed.isEmpty()&&minimum>=4.5,QString("Lumen primary %1 painted label bed has normal-text contrast: %2").arg(hover?"hover":"normal").arg(minimum));
                    qInfo()<<"LUMEN PRIMARY CONTRAST"<<(hover?"hover":"normal")<<primary->palette().color(QPalette::ButtonText).name()<<minimum;
                }
                if(displayPicker)displayPicker->setCurrentIndex(previousDisplay);
                settle(app,30);
            }
            if(id=="lumenatelier1"&&window.testAttribute(Qt::WA_TranslucentBackground)&&window.property("potatoDesktopGlassSystemAllowed").toBool()){
                require(PotatoDesktopGlass::active(&window),"Lumen activates real desktop alpha composition on this host");
                require(window.property("potatoDesktopGlassBackdropRequested").toBool()&&window.property("potatoDesktopGlassBackdropAccepted").toBool()&&
                    window.property("potatoDesktopGlassBackdropReadback").toInt()==1&&window.property("potatoDesktopGlassAccentState").toInt()==4,
                    "Windows accepted low-tint desktop Acrylic with the opaque system material disabled (not a visual blur assertion)");
                const auto nativeFrame=window.grab().toImage();int translucentPixels=0,transmittingPixels=0,sampledPixels=0;
                const auto band=QRect(projectBar->mapTo(&window,QPoint()),projectBar->size());
                const auto scale=window.devicePixelRatioF();
                for(int y=band.top()+4;y<band.bottom()-4;y+=3)for(int x=band.left()+4;x<band.right()-4;x+=3){
                    const int a=nativeFrame.pixelColor(qRound(x*scale),qRound(y*scale)).alpha();if(a>0&&a<245)++translucentPixels;
                    ++sampledPixels;if(a>0&&a<=210)++transmittingPixels;
                }
                require(translucentPixels>100,"Lumen chrome contains real translucent pixels, not an opaque painted background");
                require(sampledPixels>0&&transmittingPixels>sampledPixels*.40,"Lumen broad chrome leaves meaningful desktop transmission instead of stacking opaque veils");
                qInfo()<<"DESKTOP GLASS"<<window.property("potatoDesktopGlassMode")<<"tint"<<window.property("potatoDesktopGlassTintAlpha")<<"transmitting samples"<<transmittingPixels<<"of"<<sampledPixels;
                auto checkLabelContrast=[&](QLabel *label,const QString &name){
                    const auto ink=label->palette().color(QPalette::WindowText);const auto style=label->styleSheet();
                    label->setStyleSheet(style+";color:transparent;");const auto bed=composedChrome(label);label->setStyleSheet(style);
                    double minimum=100;QPoint weakestPoint;QColor weakestBackground;
                    for(const QColor backdrop:{QColor(Qt::black),QColor(Qt::white)}){
                        QImage composed(bed.size(),QImage::Format_RGB32);composed.setDevicePixelRatio(bed.devicePixelRatio());composed.fill(backdrop);{QPainter p(&composed);p.drawImage(0,0,bed);}
                        const double text=PotatoThemeCheck::luminance(ink);
                        for(int y=2;y<composed.height()-2;y+=3)for(int x=2;x<composed.width()-2;x+=3){
                            const double background=PotatoThemeCheck::luminance(composed.pixelColor(x,y));
                            const double contrast=(qMax(text,background)+.05)/(qMin(text,background)+.05);
                            if(contrast<minimum){minimum=contrast;weakestPoint=QPoint(x,y);weakestBackground=composed.pixelColor(x,y);}
                        }
                    }
                    qInfo()<<"LUMEN LABEL CONTRAST"<<name<<ink.name()<<minimum<<weakestPoint<<weakestBackground.name()<<label->size();
                    require(minimum>=4.5,"actual translucent label backing readable over black and white desktop: "+name+" "+QString::number(minimum));
                };
                for(const auto &name:QStringList{"outputStatus","surfaceHeading"})if(auto *label=window.template findChild<QLabel*>(name))checkLabelContrast(label,name);
                // The output status uses primary ink while live and secondary
                // ink while stopped; both must survive the same desktop bed.
                window.output->hide();settle(app,40);
                if(auto *label=window.template findChild<QLabel*>("outputStatus"))checkLabelContrast(label,"outputStatus stopped");
                window.output->show();settle(app,40);
            }else require(!PotatoDesktopGlass::active(&window),"other studio materials retain opaque window composition: "+id);
            require(splitter->count()==2&&splitter->widget(0)==sidebar&&splitter->widget(1)==window.canvas&&bounds(sidebar,&window).right()<bounds(window.canvas,&window).left(),"original left sidebar/right canvas: "+id);
            require(window.size()==windowSize&&splitter->sizes()==splitterSizes&&projectBar->actions()==projectActions&&outputBar->actions()==outputActions,"same window, splitter and action order: "+id);
            for(const auto &entry:identities)require(entry.widget&&window.template findChild<QWidget*>(entry.name)==entry.widget&&entry.widget->parentWidget()==entry.parent&&entry.widget->layout()==entry.layout,"same native widget/parent/layout: "+entry.name+" "+id);
            QStringList found;
            for(auto *action:projectBar->actions())if(commandOrder.contains(action->text())){
                found<<action->text();auto *button=qobject_cast<QToolButton*>(projectBar->widgetForAction(action));
                const bool labelled=button&&(button->toolButtonStyle()!=Qt::ToolButtonIconOnly||(button->icon().isNull()&&button->arrowType()==Qt::NoArrow));
                require(button&&visible(button,&window)&&labelled&&button->text()==action->text(),"visible labelled command: "+action->text()+" "+id);
            }
            require(found==commandOrder,"labelled project command sequence: "+id);
            require(PotatoThemeCheck::textBedContrast(search)>=4.5,"actual composed search input has readable text: "+id);
            stills.insert(id,chrome());
            const int themeSnapshot=args.indexOf("--snapshot");
            if(themeSnapshot>=0&&themeSnapshot+1<args.size()){
                const QFileInfo path(args[themeSnapshot+1]);
                require(QDir().mkpath(path.absolutePath()),"studio capture directory created: "+id);
                require(window.grab().save(path.dir().filePath(path.completeBaseName()+"-"+id+".png")),"actual paused native theme screenshot saved: "+id);
            }
            pause->setChecked(false);settle(app,80);const auto before=chrome();const auto start=frames();QElapsedTimer elapsed;elapsed.start();settle(app,950);
            const double moving=PotatoStudioCheck::difference(before,chrome());const auto delta=frames()-start;
            require(app.property("potatoAmbientRunning").toBool()&&delta>0&&moving>.01,"real chrome moves: "+id);
            require(delta<=qulonglong(elapsed.elapsed()/33+4),"shared chrome clock stays <=30fps: "+id);
            const auto pixels=app.property("potatoStudioThemeEnvironmentPixels").toLongLong();require(pixels>0&&pixels<=800*500,"bounded original studio environment: "+id);
            pause->setChecked(true);settle(app,100);const auto frozen=seconds();const auto count=frames();const auto paused=chrome();settle(app,200);
            require(ambientPause->isChecked()&&!app.property("potatoAmbientRunning").toBool()&&seconds()==frozen&&frames()==count&&chrome()==paused,"Pause freezes exact chrome and clock and synchronizes Ambient: "+id);
            PotatoThemes::clearCaches();window.repaint();settle(app,40);require(seconds()==frozen,"cache refresh preserves phase: "+id);
            unchanged(id);
            for(auto *option:{reduceMotion,reduceTransparency,increaseContrast}){
                pause->setChecked(false);option->setChecked(true);settle(app,100);const auto staticSeconds=seconds();const auto staticFrames=frames();const auto staticChrome=chrome();settle(app,130);
                if(option==reduceTransparency||option==increaseContrast)require(!PotatoDesktopGlass::active(&window),"readability preference disables actual desktop translucency: "+id);
                require(!app.property("potatoAmbientRunning").toBool()&&seconds()==staticSeconds&&frames()==staticFrames&&chrome()==staticChrome,"accessibility freezes real chrome: "+option->objectName()+" "+id);
                require(!staticChrome.isNull()&&PotatoThemeCheck::textBedContrast(search)>=4.5,"static fallback has readable real search text: "+id);option->setChecked(false);pause->setChecked(true);settle(app,60);
            }
            qInfo()<<"STUDIO METRICS"<<id<<"motionDelta"<<moving<<"frames"<<delta<<"environmentPixels"<<pixels;
        }
        for(int i=0;i<ids.size();++i)for(int j=i+1;j<ids.size();++j)
            require(PotatoStudioCheck::difference(stills.value(ids[i]),stills.value(ids[j]))>.25,"distinct native theme chrome: "+ids[i]+" / "+ids[j]);

        browser->close();settle(app,50);
        adjust->trigger();settle(app,80);
        auto *dialog=window.template findChild<QDialog*>("themeMotionDialog");auto *speed=window.template findChild<QSlider*>("ambientSpeed");auto *intensity=window.template findChild<QSlider*>("ambientIntensity");auto *dialogPause=window.template findChild<QCheckBox*>("ambientPauseControl");
        require(dialog&&dialog->isVisible()&&speed&&intensity&&dialogPause,"Theme motion opens shared real controls");
        if(dialog&&speed&&intensity&&dialogPause){
            require(speed->minimum()==20&&speed->maximum()==240&&intensity->minimum()==20&&intensity->maximum()==100,"shared control bounds retained");
            speed->setValue(70);intensity->setValue(85);settle(app,60);
            require(qAbs(app.property("potatoAmbientSpeed").toDouble()-.7)<.001&&qAbs(app.property("potatoAmbientIntensity").toDouble()-.85)<.001,"studio controls use persisted shared speed/light");
            dialogPause->setChecked(false);speed->setValue(240);settle(app,70);const auto fastStart=seconds();QElapsedTimer fastTimer;fastTimer.start();settle(app,500);
            const double fastRate=(seconds()-fastStart)/(fastTimer.elapsed()/1000.0);
            require(fastRate>1.8&&fastRate<2.9,"expanded 2.4x control advances the real theme clock above the former 1.2x cap");
            dialogPause->setChecked(true);settle(app,50);
            const auto frozen=seconds();intensity->setValue(20);settle(app,50);const auto dim=chrome();intensity->setValue(100);settle(app,50);
            require(PotatoStudioCheck::difference(dim,chrome())>.25&&seconds()==frozen,"studio Light changes actual paused chrome");
            auto *clear=window.template findChild<QAction*>("ambientGlass_clear");auto *reset=dialog->template findChild<QPushButton*>("ambientReset");
            if(clear&&reset){clear->trigger();reset->click();settle(app,60);}
            require(clear&&reset&&clear->isChecked()&&app.property("potatoAmbientGlass").toString()=="clear"&&speed->value()==50&&intensity->value()==65&&!dialogPause->isChecked(),"Theme motion Reset restores motion defaults while retaining Ambient Clear");
            dialogPause->setChecked(false);settle(app,80);require(!pause->isChecked()&&!ambientPause->isChecked()&&app.property("potatoAmbientRunning").toBool(),"shared dialog Play synchronizes both menus");
            pause->setChecked(true);speed->setValue(50);intensity->setValue(65);dialog->close();settle(app,60);
        }
        pause->setChecked(false);settle(app,100);window.showMinimized();settle(app,100);
        const auto minimizedSeconds=seconds();const auto minimizedFrames=frames();settle(app,300);
        require(window.isMinimized()&&!app.property("potatoAmbientRunning").toBool()&&seconds()==minimizedSeconds&&frames()==minimizedFrames,"minimized studio theme stops its clock and rendering");
        window.showNormal();settle(app,120);
        require(app.property("potatoAmbientRunning").toBool()&&seconds()>minimizedSeconds&&seconds()-minimizedSeconds<.15,"studio restore resumes without advancing through minimized time");
        pause->setChecked(true);settle(app,60);
        unchanged("shared Theme motion settings");
        scene.undo();require(scene.json()==undoState,"theme operations preserve previous Undo checkpoint");scene.redo();require(scene.json()==saved,"Redo returns to unchanged selected content");scene.redo();require(scene.json()==redoState,"pre-existing future Redo survives");scene.undo();scene.dirty=false;

        // Exercise the real canvas event and production Delete action with the
        // first surface selected and the pointer over the second media surface.
        auto findMenu=[]()->QMenu*{for(auto *widget:QApplication::topLevelWidgets())if(widget->isVisible()&&widget->objectName()=="materialSurfaceMenu")return qobject_cast<QMenu*>(widget);return nullptr;};
        const auto stage=window.canvas->canvasRect();const QPoint local(qRound(stage.x()+stage.width()*.72),qRound(stage.y()+stage.height()*.5));
        const auto retainedId=scene.surfaces[1].id;bool sawMenu=false;QTimer inspect,watchdog;inspect.setSingleShot(true);watchdog.setSingleShot(true);
        QObject::connect(&inspect,&QTimer::timeout,&window,[&]{
            auto *menu=findMenu();require(menu,"real studio context menu opens");if(!menu)return;sawMenu=true;
            require(scene.current()&&scene.current()->name=="Media panel","context menu targets pointed media surface");
            auto *remove=menu->template findChild<QAction*>("deleteSurfaceAction");require(remove&&remove->isEnabled(),"contextual Delete remains enabled");
            require(!menu->grab().isNull()&&window.output->grabFramebuffer()==outputBefore,"real menu paints without projector contamination");
            if(remove)remove->trigger();menu->close();
        });
        QObject::connect(&watchdog,&QTimer::timeout,&window,[&]{if(auto *menu=findMenu())menu->close();});
        inspect.start(120);watchdog.start(1800);QContextMenuEvent context(QContextMenuEvent::Mouse,local,window.canvas->mapToGlobal(local));QApplication::sendEvent(window.canvas,&context);inspect.stop();watchdog.stop();
        require(sawMenu&&scene.surfaces.size()==1&&scene.surfaces[0].name=="Pattern panel","real Delete removes only pointer target");
        scene.undo();require(scene.json()==saved&&scene.surfaces.size()==2&&scene.surfaces[1].id==retainedId,"one Undo restores exact media surface identity and order");
        require(window.output->grabFramebuffer()==outputBefore,"Undo restores exact projector output");
        require(bytes(mediaPath)==mediaBytes&&bytes(projectPath)==projectBytes,"all appearance and Delete/Undo operations preserve saved media/project bytes");
        const int snapshot=args.indexOf("--snapshot");if(snapshot>=0&&snapshot+1<args.size()){
            const QFileInfo path(args[snapshot+1]);require(QDir().mkpath(path.absolutePath()),"capture directory created");require(window.grab().save(path.absoluteFilePath()),"actual native studio screenshot saved");
        }
        finish();
    });
}
