#pragma once
#include "updates.h"
#include <QApplication>
#include <QCryptographicHash>
#include <QContextMenuEvent>
#include <QFile>
#include <QJsonDocument>
#include <QMenu>
#include <QTemporaryDir>
#include <QTimer>

// The only scene used automatically by this proof is created here, in memory.
// --proof-check exercises the actual context-menu signal and real Scene Undo.
template<class Window> void runSurfaceMaterialProof(Window &window,QApplication &app,const QStringList &args){
    QTimer::singleShot(650,&window,[&window,&app,args]{
        auto &scene=window.scene;scene.newProject();
        auto &first=scene.surfaces[0];first.name="Porch window";first.corners={{.08,.16},{.47,.16},{.47,.84},{.08,.84}};first.subdivide(3);first.pattern=17;first.patternPlaying=false;first.patternPhase=3;
        scene.add();auto &second=scene.surfaces[1];second.name="Door panel";second.corners={{.55,.16},{.92,.16},{.92,.84},{.55,.84}};second.subdivide(3);second.pattern=15;second.patternPlaying=false;second.patternPhase=4;
        scene.select(0);scene.touch(true);scene.dirty=false;window.setWindowModified(false);window.canvas->resetView();
        window.statusBar()->showMessage("ONE-PIECE PROOF: right-click a surface. Only its menu is restyled; the 0.8.8 layout is unchanged.");
        if(!args.contains("--proof-check"))return;
        QStringList failures;auto require=[&](bool ok,const QString &label){if(!ok)failures<<label;};
        QTemporaryDir mediaFolder(QDir(potatoInstallRoot()).filePath("fixture-media-XXXXXX"));
        const auto mediaPath=mediaFolder.filePath("original.png");QImage image(32,32,QImage::Format_RGB32);image.fill(QColor(180,85,55));
        require(mediaFolder.isValid()&&image.save(mediaPath),"real disposable media fixture created");scene.assignMedia(1,mediaPath);scene.select(0);scene.dirty=false;
        QFile media(mediaPath);require(media.open(QIODevice::ReadOnly),"read original fixture bytes");const auto mediaBytes=media.readAll();media.close();
        window.output->resize(480,270);window.output->show();const auto outputBefore=window.output->grabFramebuffer();
        const auto original=scene.json();const auto retainedId=scene.surfaces[1].id;
        auto findMenu=[]()->QMenu*{for(auto *widget:QApplication::topLevelWidgets())if(widget->isVisible()&&widget->objectName()=="materialSurfaceMenu")return qobject_cast<QMenu*>(widget);return nullptr;};
        const auto stage=window.canvas->canvasRect();const QPoint local(qRound(stage.x()+stage.width()*.72),qRound(stage.y()+stage.height()*.5));
        auto open=[&](bool remove){
            bool sawMenu=false;
            QTimer::singleShot(180,&window,[&]{
                auto *popup=findMenu();
                require(popup,"real popup opened");if(!popup)return;sawMenu=true;
                require(popup->actions().first()->text()=="Door panel","context menu targets pointer surface, not old selection");
                qInfo()<<"MATERIAL PROOF"<<popup->property("nativeBackdropHresult")<<popup->property("nativeBackdropLayered")<<popup->property("nativeBackdropStatus");
                require(window.output->grabFramebuffer()==outputBefore,"UI material does not alter projector pixels");
                if(remove){auto *action=popup->template findChild<QAction*>("proofDeleteSurface");require(action,"real Delete action exists");if(action)action->trigger();}
                popup->close();
            });
            QTimer::singleShot(2000,&window,[findMenu]{if(auto *popup=findMenu())popup->close();});
            QContextMenuEvent event(QContextMenuEvent::Mouse,local,window.canvas->mapToGlobal(local));QApplication::sendEvent(window.canvas,&event);
            require(sawMenu,"popup check executed");
        };
        open(false);require(scene.json()==original&&!scene.dirty,"opening and cancelling menu preserves clean scene");
        scene.select(0);open(true);require(scene.surfaces.size()==1&&scene.surfaces[0].name=="Porch window","Delete removes only right-clicked Door panel");
        scene.undo();require(scene.json()==original&&scene.surfaces.size()==2&&scene.surfaces[1].id==retainedId,"one Undo restores exact surface data/order/identity");
        scene.redo();require(scene.surfaces.size()==1,"Redo reapplies deletion");scene.undo();
        require(media.open(QIODevice::ReadOnly)&&media.readAll()==mediaBytes,"Delete and Undo preserve original media bytes");media.close();
        qInfo().noquote()<<"SURFACE MATERIAL CHECK"<<(failures.isEmpty()?"PASS":"FAIL")<<failures.join("; ");
        app.exit(failures.isEmpty()?0:2);
    });
}
