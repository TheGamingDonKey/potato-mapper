#pragma once
#include "model.h"
#include "ui.h"
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QListWidget>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTimer>

class DynamicPanel : public InspectorSection {
public:
    explicit DynamicPanel(Scene *s,QWidget *parent=nullptr):InspectorSection("Dynamic FX",parent),scene(s){
        heading->setChecked(false);
        auto *description=new QLabel("One coordinated effect across the surfaces you choose.");description->setWordWrap(true);content->addWidget(description);
        auto *form=new QFormLayout;
        effect=new QComboBox;effect->setObjectName("dynamicEffect");effect->addItems({"Off","Hot-Ass Potato","Potato Focus"});form->addRow("Effect",effect);
        auto spin=[form](const QString &name,const QString &id,int low,int high){auto *w=new QSpinBox;w->setObjectName(id);w->setRange(low,high);form->addRow(name,w);return w;};
        speed=spin("Speed","dynamicSpeed",10,300);speed->setSuffix(" %");density=spin("Shapes","dynamicDensity",8,80);glitch=spin("Glitch","dynamicGlitch",0,100);glitch->setSuffix(" %");
        snakeWidth=spin("Snake width","dynamicSnakeWidth",8,96);snakeWidth->setSuffix(" px");
        cellSize=spin("Cell size","dynamicCellSize",24,160);cellSize->setSuffix(" px");
        cellSize->setToolTip("Size in projector-output pixels. Spacing is 1.8 times this size; Shapes limits visible cells.");
        palette=new QComboBox;palette->setObjectName("dynamicPalette");palette->addItems({"Holographic","White","Cyan","Violet","Amber","Mint"});form->addRow("Colour",palette);
        pause=new QCheckBox("Pause performance");pause->setObjectName("dynamicPause");form->addRow(pause);content->addLayout(form);
        members=new QListWidget;members->setObjectName("dynamicMembers");members->setMaximumHeight(115);content->addWidget(members);
        auto *all=new QPushButton("Use all visible surfaces");content->addWidget(all);
        auto *buttons=new QHBoxLayout;auto *replay=new QPushButton("Replay intro");replay->setObjectName("dynamicReplay");auto *ambient=new QPushButton("Skip to ambient");ambient->setObjectName("dynamicAmbient");buttons->addWidget(replay);buttons->addWidget(ambient);content->addLayout(buttons);
        stage=new QLabel;stage->setWordWrap(true);stage->setStyleSheet("color:#a4d6e9;font-size:11px;");content->addWidget(stage);
        auto update=[this]{if(refreshing)return;auto settings=scene->dynamic;settings.effect=effect->currentIndex();settings.speed=speed->value();settings.density=density->value();settings.palette=palette->currentIndex();settings.glitch=glitch->value();settings.snakeWidth=snakeWidth->value();settings.cellSize=cellSize->value();settings.playing=!pause->isChecked();
            if(settings.effect&&settings.members.isEmpty()&&scene->dynamic.effect==0)for(const auto &surface:scene->surfaces)if(surface.visible)settings.members<<surface.id;
            scene->setDynamic(settings);
        };
        connect(effect,&QComboBox::currentIndexChanged,this,[update](int){update();});connect(palette,&QComboBox::currentIndexChanged,this,[update](int){update();});
        for(auto *w:{speed,density,glitch,snakeWidth,cellSize})connect(w,&QSpinBox::valueChanged,this,[update](int){update();});connect(pause,&QCheckBox::toggled,this,[update](bool){update();});
        connect(members,&QListWidget::itemChanged,this,[this](QListWidgetItem *){if(refreshing)return;auto settings=scene->dynamic;settings.members.clear();for(int i=0;i<members->count();++i)if(members->item(i)->checkState()==Qt::Checked)settings.members<<members->item(i)->data(Qt::UserRole).toString();scene->setDynamic(settings);});
        connect(all,&QPushButton::clicked,this,[this]{auto settings=scene->dynamic;settings.members.clear();for(const auto &surface:scene->surfaces)if(surface.visible)settings.members<<surface.id;scene->setDynamic(settings);});
        connect(replay,&QPushButton::clicked,this,[this]{scene->setDynamicTime(0);});connect(ambient,&QPushButton::clicked,this,[this]{scene->setDynamicTime(PotatoDynamic::ambientStart(scene->dynamic,int(scene->dynamicSurfaces.size())));});
        connect(scene,&Scene::structureChanged,this,[this]{refresh();});
        auto *timer=new QTimer(this);connect(timer,&QTimer::timeout,this,[this]{stage->setText(scene->dynamic.effect?QString("%1 · %2 surfaces%3").arg(scene->dynamicFrame.stage).arg(scene->dynamicSurfaces.size()).arg(scene->dynamic.playing?QString():QString(" · paused")):QString("Choose an effect to connect your surfaces. Original content returns when this is Off."));});timer->start(250);refresh();
    }
private:
    Scene *scene;bool refreshing=false;QComboBox *effect,*palette;QSpinBox *speed,*density,*glitch,*snakeWidth,*cellSize;QCheckBox *pause;QListWidget *members;QLabel *stage;
    void refresh(){
        refreshing=true;effect->setCurrentIndex(scene->dynamic.effect);speed->setValue(scene->dynamic.speed);density->setValue(scene->dynamic.density);glitch->setValue(scene->dynamic.glitch);palette->setCurrentIndex(scene->dynamic.palette);pause->setChecked(!scene->dynamic.playing);
        snakeWidth->setValue(scene->dynamic.snakeWidth);cellSize->setValue(scene->dynamic.cellSize);
        members->clear();for(const auto &s:scene->surfaces){auto *item=new QListWidgetItem(s.name+(s.visible?QString():QString(" · hidden")),members);item->setData(Qt::UserRole,s.id);item->setFlags(item->flags()|Qt::ItemIsUserCheckable);item->setCheckState(scene->dynamic.members.contains(s.id)?Qt::Checked:Qt::Unchecked);}
        const bool active=scene->dynamic.effect;speed->setEnabled(active);density->setEnabled(active);palette->setEnabled(active);pause->setEnabled(active);glitch->setEnabled(scene->dynamic.effect==1);snakeWidth->setEnabled(scene->dynamic.effect==1);cellSize->setEnabled(scene->dynamic.effect==1);refreshing=false;
    }
};
