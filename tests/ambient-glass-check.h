#pragma once
#include "material-ui-check.h"
#include "../src/appearance.h"
#include <QElapsedTimer>
#include <QRadioButton>
#include <QSettings>
#include <QSlider>
#include <QVariantMap>
#include <cmath>

namespace PotatoAmbientCheck {
inline double difference(const QImage &a,const QImage &b){
    if(a.isNull()||a.size()!=b.size())return -1;
    double total=0;quint64 samples=0;
    for(int y=2;y<a.height()-2;y+=3)for(int x=2;x<a.width()-2;x+=3){
        const auto first=a.pixel(x,y),second=b.pixel(x,y);
        total+=qAbs(qRed(first)-qRed(second))+qAbs(qGreen(first)-qGreen(second))+qAbs(qBlue(first)-qBlue(second));samples+=3;
    }
    return samples?total/samples:0;
}
inline QVariantMap storedAppearance(){
    QSettings settings;QVariantMap values;
    for(const auto &key:settings.allKeys())if(key.startsWith("ui/"))values.insert(key,settings.value(key));
    return values;
}
}

// Native controls, real composed chrome and OpenGL output, with a disposable
// scene and INI preferences. The caller sets window.smoke=true before running.
template<class Window> void runAmbientGlassChecks(Window &window,QApplication &app,const QStringList &args){
    QTimer::singleShot(450,&window,[&window,&app,args]{
        using namespace PotatoMaterialUiCheck;
        QStringList failures;int assertions=0;
        auto require=[&](bool ok,const QString &label){++assertions;if(!ok)failures<<label;};
        const auto userPreferences=PotatoAmbientCheck::storedAppearance();
        auto finish=[&]{
            window.output->hide();
            require(PotatoAmbientCheck::storedAppearance()==userPreferences,"diagnostic does not write actual user appearance preferences");
            qInfo().noquote()<<"AMBIENT GLASS"<<(failures.isEmpty()?"PASS":"FAIL")<<assertions<<"checks"<<failures.join("; ");
            app.exit(failures.isEmpty()?0:2);
        };
        auto *ambient=window.template findChild<QAction*>("interfaceTheme_ambientglass1");
        require(ambient,"native Ambient Glass 1 profile action exists");
        if(!ambient){finish();return;}
        auto *smoked=window.template findChild<QAction*>("ambientGlass_smoked");
        auto *clear=window.template findChild<QAction*>("ambientGlass_clear");
        auto *pause=window.template findChild<QAction*>("ambientPause");
        auto *adjust=window.template findChild<QAction*>("ambientSettings");
        auto *reduceMotion=window.template findChild<QAction*>("reduceInterfaceMotion");
        auto *reduceTransparency=window.template findChild<QAction*>("reduceInterfaceTransparency");
        auto *increaseContrast=window.template findChild<QAction*>("increaseInterfaceContrast");
        auto *projectBar=window.template findChild<QToolBar*>("projectToolbar");
        auto *outputBar=window.template findChild<QToolBar*>("outputToolbar");
        auto *sidebar=window.template findChild<QWidget*>("surfaceSidebar");
        auto *splitter=window.template findChild<QSplitter*>("editorSplitter");
        require(smoked&&clear&&pause&&adjust&&reduceMotion&&reduceTransparency&&increaseContrast&&projectBar&&outputBar&&sidebar&&splitter,"glass controls, accessibility and familiar workspace exist");
        if(!failures.isEmpty()){finish();return;}

        QTemporaryDir preferencesDirectory;
        require(preferencesDirectory.isValid(),"temporary preferences folder created");
        if(!preferencesDirectory.isValid()){finish();return;}
        const auto preferencesPath=preferencesDirectory.filePath("ambient.ini");
        QSettings preferences(preferencesPath,QSettings::IniFormat);
        auto settings=PotatoAppearance::readAmbientPreferences(preferences);
        require(settings.glass=="smoked"&&!settings.paused&&settings.speed==50&&settings.intensity==65&&preferences.allKeys().isEmpty(),"unset ambient preferences have defaults without writing");
        PotatoAppearance::writePreferences(preferences,{"ambientglass1","autumn"});
        PotatoAppearance::writeAmbientPreferences(preferences,{"clear",true,90,80});preferences.sync();
        QSettings reopened(preferencesPath,QSettings::IniFormat);settings=PotatoAppearance::readAmbientPreferences(reopened);
        require(reopened.status()==QSettings::NoError&&PotatoAppearance::readPreferences(reopened).interfaceTheme=="ambientglass1"&&settings.glass=="clear"&&settings.paused&&settings.speed==90&&settings.intensity==80,"ambient profile and all motion preferences survive reopen");
        preferences.setValue("ui/ambientGlass","unknown");preferences.setValue("ui/ambientSpeed",999);preferences.setValue("ui/ambientIntensity",-8);
        settings=PotatoAppearance::readAmbientPreferences(preferences);
        require(settings.glass=="smoked"&&settings.speed==240&&settings.intensity==20,"invalid glass and out-of-range values normalize safely");

        reduceMotion->setChecked(false);reduceTransparency->setChecked(false);increaseContrast->setChecked(false);
        ambient->trigger();pause->setChecked(true);smoked->trigger();window.resize(1280,800);settle(app,100);
        auto &scene=window.scene;scene.newProject();scene.setPattern(0,1);scene.current()->patternPlaying=false;
        const auto undoState=scene.json();scene.setPattern(0,17);scene.current()->patternPlaying=false;const auto saved=scene.json();
        scene.setPattern(0,18);scene.current()->patternPlaying=false;const auto redoState=scene.json();scene.undo();scene.dirty=false;
        const int selected=scene.selected;const auto phase=scene.current()->patternPhase;
        const auto projectActions=projectBar->actions(),outputActions=outputBar->actions();
        const auto splitterSizes=splitter->sizes();const auto windowSize=window.size();
        require(splitter->count()==2&&splitter->widget(0)==sidebar&&splitter->widget(1)==window.canvas,"8.8 left sidebar and right canvas retained");
        QStringList found;
        const QStringList commandOrder{"New","Open","Save","Save as","Undo","Redo","+ Surface","Load media","Potato FX","Dynamic FX"};
        for(auto *action:projectActions)if(commandOrder.contains(action->text()))found<<action->text();
        require(found==commandOrder,"original project toolbar command order retained");
        window.output->resize(480,270);window.output->show();settle(app,100);
        const auto outputBefore=window.output->grabFramebuffer(),canvasBefore=window.canvas->grabFramebuffer();
        require(!outputBefore.isNull()&&!canvasBefore.isNull(),"real editor and output framebuffer samples available");
        auto seconds=[&]{return app.property("potatoAmbientSeconds").toDouble();};
        auto frames=[&]{return app.property("potatoAmbientFrames").toULongLong();};
        auto chromeImage=[&]{return projectBar->grab().toImage().convertToFormat(QImage::Format_RGB32);};
        auto unchangedScene=[&](const QString &label){
            require(scene.json()==saved&&!scene.dirty&&scene.selected==selected&&scene.current()->patternPhase==phase,label+" retains clean selected scene");
            require(window.output->grabFramebuffer()==outputBefore&&window.canvas->grabFramebuffer()==canvasBefore,label+" retains exact editor/output pixels");
        };

        const auto smokedFrame=chromeImage();clear->trigger();settle(app,50);const auto clearFrame=chromeImage();
        const double glassDifference=PotatoAmbientCheck::difference(smokedFrame,clearFrame);
        require(app.property("potatoAmbientGlass").toString()=="clear"&&clear->isChecked()&&!smoked->isChecked()&&glassDifference>.25,"Clear and Smoked visibly differ through real exclusive actions");
        pause->setChecked(false);settle(app,80);
        const auto movingBefore=chromeImage();const auto frameStart=frames();QElapsedTimer interval;interval.start();
        settle(app,950);const auto movingAfter=chromeImage();const auto elapsed=interval.elapsed();const auto frameDelta=frames()-frameStart;
        const double motionDifference=PotatoAmbientCheck::difference(movingBefore,movingAfter);
        require(app.property("potatoAmbientRunning").toBool()&&frameDelta>0&&motionDifference>.01,"actual chrome changes while ambient motion plays");
        require(frameDelta<=qulonglong(elapsed/33+4),"shared ambient rendering remains at or below 30fps");
        const auto pixels=app.property("potatoAmbientEnvironmentPixels").toLongLong();
        require(pixels>0&&pixels<=400000,"ambient environment stays within 800x500 pixel budget");
        unchangedScene("Playing ambient background");

        pause->setChecked(true);settle(app,100);const auto pausedBefore=chromeImage();const auto pausedSeconds=seconds();const auto pausedFrames=frames();
        settle(app,240);const auto pausedAfter=chromeImage();
        require(!app.property("potatoAmbientRunning").toBool()&&seconds()==pausedSeconds&&frames()==pausedFrames&&pausedBefore==pausedAfter,"Pause freezes simulation and actual chrome exactly");
        adjust->trigger();settle(app,80);
        auto *dialog=window.template findChild<QDialog*>("ambientBackgroundDialog");
        auto *speed=window.template findChild<QSlider*>("ambientSpeed");
        auto *intensity=window.template findChild<QSlider*>("ambientIntensity");
        auto *dialogPause=window.template findChild<QCheckBox*>("ambientPauseControl");
        require(dialog&&dialog->isVisible()&&speed&&intensity&&dialogPause,"real adjustment dialog exposes motion controls");
        if(!dialog||!speed||!intensity||!dialogPause){finish();return;}
        require(speed->minimum()==20&&speed->maximum()==240&&intensity->minimum()==20&&intensity->maximum()==100,"speed and light controls have the approved bounds");
        speed->setValue(70);intensity->setValue(85);settle(app,50);
        require(qAbs(app.property("potatoAmbientSpeed").toDouble()-.7)<.001&&qAbs(app.property("potatoAmbientIntensity").toDouble()-.85)<.001,"native sliders update live speed and intensity");
        intensity->setValue(20);settle(app,40);const auto dim=chromeImage();intensity->setValue(100);settle(app,40);const auto bright=chromeImage();
        require(PotatoAmbientCheck::difference(dim,bright)>.25&&seconds()==pausedSeconds,"Light changes actual chrome while motion stays paused");
        dialogPause->setChecked(false);settle(app,50);
        require(!pause->isChecked()&&app.property("potatoAmbientRunning").toBool(),"dialog Play synchronizes the menu and resumes motion");
        speed->setValue(20);settle(app,50);QElapsedTimer rateTimer;const auto slowStart=seconds();rateTimer.start();settle(app,340);
        const double slowRate=(seconds()-slowStart)/qMax<qint64>(1,rateTimer.elapsed());
        speed->setValue(120);settle(app,50);const auto fastStart=seconds();rateTimer.restart();settle(app,340);
        const double fastRate=(seconds()-fastStart)/qMax<qint64>(1,rateTimer.elapsed());
        require(slowRate>0&&fastRate>slowRate*2,"speed adjustment changes real simulation rate");
        speed->setValue(50);intensity->setValue(65);dialog->close();settle(app,60);
        unchangedScene("Ambient settings dialog");

        for(auto *option:{reduceMotion,reduceTransparency,increaseContrast}){
            option->setChecked(true);settle(app,100);const auto frozen=seconds();const auto count=frames();settle(app,160);
            require(!app.property("potatoAmbientRunning").toBool()&&seconds()==frozen&&frames()==count,"accessibility option freezes ambient renderer: "+option->objectName());
            option->setChecked(false);settle(app,110);
            require(app.property("potatoAmbientRunning").toBool()&&seconds()>frozen,"motion resumes after accessibility override: "+option->objectName());
        }
        window.hide();settle(app,80);const auto hiddenSeconds=seconds();settle(app,220);
        require(!app.property("potatoAmbientRunning").toBool()&&seconds()==hiddenSeconds,"hidden editor stops background work");
        window.show();settle(app,100);require(app.property("potatoAmbientRunning").toBool()&&seconds()>hiddenSeconds&&seconds()-hiddenSeconds<.15,"show resumes from frozen time without a hidden-time jump");
        window.showMinimized();settle(app,100);const auto minimizedSeconds=seconds();settle(app,180);
        require(!app.property("potatoAmbientRunning").toBool()&&seconds()==minimizedSeconds,"minimized editor stops background work");
        window.showNormal();settle(app,100);require(app.property("potatoAmbientRunning").toBool()&&seconds()>minimizedSeconds,"restoring editor resumes ambient motion");
        for(const auto &id:QStringList{"classic1","material","auroraclear1"}){
            auto *action=window.template findChild<QAction*>("interfaceTheme_"+id);require(action,"retained profile action: "+id);if(!action)continue;
            action->trigger();settle(app,80);const auto frozen=seconds();settle(app,80);
            require(app.property("potatoInterfaceTheme").toString()==id&&!app.property("potatoAmbientRunning").toBool()&&seconds()==frozen,"old profile remains usable and stops ambient work: "+id);
        }
        ambient->trigger();settle(app,100);
        require(app.property("potatoAmbientRunning").toBool()&&ambient->isChecked(),"returning to Ambient restores live rendering");
        require(window.size()==windowSize&&splitter->sizes()==splitterSizes&&projectBar->actions()==projectActions&&outputBar->actions()==outputActions,"controls, window size and splitter retained through settings and themes");
        unchangedScene("All ambient and profile operations");
        scene.undo();require(scene.json()==undoState,"ambient changes did not insert an Undo checkpoint");
        scene.redo();require(scene.json()==saved,"Redo restores selected content");scene.redo();require(scene.json()==redoState,"future Redo checkpoint survived");scene.undo();scene.dirty=false;

        pause->setChecked(true);clear->trigger();settle(app,100);
        const int snapshot=args.indexOf("--snapshot");
        if(snapshot>=0&&snapshot+1<args.size()){
            const QFileInfo path(args[snapshot+1]);require(QDir().mkpath(path.absolutePath()),"capture directory created");
            require(window.grab().save(path.absoluteFilePath()),"actual native Ambient screenshot saved");
        }
        qInfo()<<"AMBIENT METRICS"<<"glassDelta"<<glassDifference<<"motionDelta"<<motionDifference<<"frames"<<frameDelta<<"elapsedMs"<<elapsed<<"environmentPixels"<<pixels;
        finish();
    });
}
