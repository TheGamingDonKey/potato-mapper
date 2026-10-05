#pragma once
#include <QApplication>
#include <QComboBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QListWidget>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QSlider>
#include <QSplitter>
#include <QTemporaryDir>
#include <QTimer>
#include <algorithm>
#include <cmath>
#include <memory>

// Exercise the actual widgets, saved projects and both OpenGL canvases.
template<class Window> void runAppearanceChecks(Window &window,QApplication &app,const QStringList &args){
    window.output->resize(640,360);window.output->show();
    QTimer::singleShot(650,&window,[&window,&app,args]{
        QStringList failures;auto require=[&](bool pass,const QString &name){if(!pass)failures<<name;};
        auto temp=std::make_shared<QTemporaryDir>();auto &scene=window.scene;
        QImage image(64,64,QImage::Format_RGBA8888);image.fill(QColor(200,120,80));
        const auto media=temp->filePath("A very long filename for a perfectly ordinary image used to check sidebar elision.png");require(image.save(media),"fixture image");
        scene.assignMedia(0,media);scene.surfaces[0].corners={QPointF(0,0),QPointF(1,0),QPointF(1,1),QPointF(0,1)};scene.touch(true);
        auto *brightness=window.template findChild<QSlider*>("brightnessSlider");auto *opacity=window.template findChild<QSlider*>("opacitySlider");
        auto *picker=window.template findChild<QComboBox*>("animationPicker");
        if(!brightness||!opacity||!picker){qInfo()<<"APPEARANCE FAIL: controls missing";app.exit(2);return;}
        auto sample=[](Canvas *canvas){const auto shot=canvas->grabFramebuffer();return shot.pixelColor(shot.width()/2,shot.height()/2);};
        auto closeColour=[](QColor a,QColor b){return std::max({std::abs(a.red()-b.red()),std::abs(a.green()-b.green()),std::abs(a.blue()-b.blue())})<=4;};
        require(closeColour(sample(window.output),QColor(200,120,80)),"default image colour");
        brightness->setSliderDown(true);brightness->setValue(75);brightness->setValue(50);brightness->setSliderDown(false);
        require(scene.surfaces[0].brightness==50&&scene.dirty,"brightness UI and dirty state");
        require(closeColour(sample(window.output),QColor(100,60,40)),"brightness pixels");
        require(closeColour(sample(window.canvas),sample(window.output)),"matching editor/output pixels");
        scene.undo();require(scene.surfaces[0].brightness==100,"one Undo for slider drag");scene.redo();require(scene.surfaces[0].brightness==50,"appearance Redo");
        opacity->setValue(50);qInfo()<<"OPACITY image"<<sample(window.output)<<"expected"<<QColor(50,30,20);require(closeColour(sample(window.output),QColor(50,30,20)),"combined opacity pixels");
        scene.add();const auto other=scene.surfaces[1].json();scene.select(0);brightness->setValue(40);require(scene.surfaces[1].json()==other,"other surface unchanged");
        QImage blue(64,64,QImage::Format_RGBA8888);blue.fill(QColor(0,0,200));const auto lower=temp->filePath("lower.png");require(blue.save(lower),"lower layer image");scene.assignMedia(1,lower);
        scene.surfaces[1].corners=scene.surfaces[0].corners;scene.surfaces.swapItemsAt(0,1);scene.select(1);scene.setAppearance(1,100,50);
        require(closeColour(sample(window.output),QColor(100,60,140)),"opacity reveals lower layer");
        scene.surfaces.swapItemsAt(0,1);scene.surfaces[1].visible=false;scene.select(0);scene.setAppearance(0,40,50);
        QString error;const auto saved=scene.json();require(scene.save(temp->filePath("appearance.pmap"),error)&&scene.load(temp->filePath("appearance.pmap"),error)&&scene.json()==saved,"save/reopen appearance");
        auto legacy=saved;legacy["format"]="HomeMapper";auto items=legacy["surfaces"].toArray();
        for(int i=0;i<items.size();++i){auto item=items[i].toObject();item.remove("brightness");item.remove("opacity");items[i]=item;}legacy["surfaces"]=items;
        require(scene.restore(legacy,{},error)&&scene.surfaces[0].brightness==100&&scene.surfaces[0].opacity==100,"old HomeMapper defaults");
        auto *video=window.template findChild<QWidget*>("videoControls");auto *patterns=window.template findChild<QWidget*>("patternControls");
        require(video&&video->isHidden()&&patterns&&patterns->isHidden(),"image hides irrelevant controls");
        picker->setCurrentIndex(1);require(scene.surfaces[0].pattern==1&&scene.surfaces[0].media.isEmpty()&&!patterns->isHidden(),"Potato FX picker");
        scene.surfaces[0].patternPlaying=false;scene.touch(true);scene.dirty=false;
        auto energy=[&]{const auto frame=window.output->grabFramebuffer();quint64 total=0;for(int y=0;y<frame.height();y+=2)for(int x=0;x<frame.width();x+=2)total+=frame.pixelColor(x,y).green();return total;};
        const auto full=energy();brightness->setValue(50);const auto dim=energy();require(full>0&&std::abs(double(dim)/full-.5)<.025,"dots brightness pixels");
        opacity->setValue(50);const auto faded=energy();qInfo()<<"OPACITY dots"<<full<<dim<<faded;require(std::abs(double(faded)/full-.25)<.025,"dots opacity pixels");
        picker->setCurrentIndex(0);require(!scene.surfaces[0].pattern&&scene.surfaces[0].media.isEmpty()&&patterns->isHidden(),"return to grid");
        auto *reset=window.template findChild<QPushButton*>("resetAppearanceButton");if(reset)reset->click();require(scene.surfaces[0].brightness==100&&scene.surfaces[0].opacity==100,"reset appearance");
        auto *source=scene.source("");QVideoFrameFormat format(QSize(66,34),QVideoFrameFormat::Format_NV12);format.setColorSpace(QVideoFrameFormat::ColorSpace_BT709);format.setColorRange(QVideoFrameFormat::ColorRange_Video);
        QVideoFrame frame(format);if(frame.map(QVideoFrame::WriteOnly)){
            for(int y=0;y<34;++y)std::fill_n(frame.bits(0)+y*frame.bytesPerLine(0),66,uchar(150));
            for(int y=0;y<17;++y)std::fill_n(frame.bits(1)+y*frame.bytesPerLine(1),66,uchar(128));frame.unmap();
            source->presentFrame(frame);const auto normal=sample(window.output);brightness->setValue(50);require(source->mappedVideo.isValid()&&closeColour(sample(window.output),QColor(normal.red()/2,normal.green()/2,normal.blue()/2)),"native NV12 brightness");
            opacity->setValue(50);qInfo()<<"OPACITY video"<<normal<<sample(window.output);require(closeColour(sample(window.output),QColor(normal.red()/4,normal.green()/4,normal.blue()/4)),"native NV12 opacity");
        }else require(false,"NV12 fixture map");
        scene.assignMedia(0,media);scene.setAppearance(0,100,100);auto *split=window.template findChild<QSplitter*>("editorSplitter");
        const auto sizes=split->sizes();split->setSizes({420,860});require(split->sizes()[0]>sizes[0],"resizable sidebar");split->setSizes({310,970});
        auto *list=window.template findChild<QListWidget*>("surfaceList");require(list&&list->horizontalScrollBarPolicy()==Qt::ScrollBarAlwaysOff&&!list->item(0)->toolTip().isEmpty(),"filename tooltip and no horizontal scroll");
        scene.dirty=false;window.resize(1024,640);app.processEvents();
        auto *scroll=window.template findChild<QScrollArea*>("surfaceInspector");require(scroll&&scroll->horizontalScrollBar()->maximum()==0&&list->isVisible(),"compact inspector layout");
        const int snapshot=args.indexOf("--snapshot");if(snapshot>=0&&snapshot+1<args.size())require(window.grab().save(args[snapshot+1]),"UI snapshot");
        qInfo().noquote()<<"APPEARANCE"<<(failures.isEmpty()?"PASS":"FAIL")<<failures.join(", ")<<error;
        app.exit(failures.isEmpty()?0:2);
    });
}
