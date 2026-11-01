#pragma once
#include "appearance.h"
#include <QApplication>
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QSignalBlocker>
#include <QSlider>
#include <QVBoxLayout>
#include <functional>

namespace PotatoAmbientControls {
using Preferences=PotatoAppearance::AmbientPreferences;
inline Preferences current(){
    return PotatoAppearance::normalizedAmbientPreferences({qApp->property("potatoAmbientGlass").toString(),
        qApp->property("potatoAmbientPaused").toBool(),qRound(qApp->property("potatoAmbientSpeed").toDouble()*100),
        qRound(qApp->property("potatoAmbientIntensity").toDouble()*100)});
}
inline void publish(const Preferences &requested){
    const auto value=PotatoAppearance::normalizedAmbientPreferences(requested);
    qApp->setProperty("potatoAmbientGlass",value.glass);qApp->setProperty("potatoAmbientPaused",value.paused);
    qApp->setProperty("potatoAmbientSpeed",value.speed/100.0);qApp->setProperty("potatoAmbientIntensity",value.intensity/100.0);
}
inline void syncDialog(QDialog *dialog){
    if(!dialog)return;
    const auto value=current();
    for(const auto &name:QStringList{"ambientGlassClear","ambientGlassSmoked"})if(auto *button=dialog->findChild<QRadioButton*>(name)){
        QSignalBlocker block(button);button->setChecked(name.endsWith("Clear")?value.glass=="clear":value.glass=="smoked");
    }
    if(auto *pause=dialog->findChild<QCheckBox*>("ambientPauseControl")){QSignalBlocker block(pause);pause->setChecked(value.paused);}
    for(const auto &name:QStringList{"ambientSpeed","ambientIntensity"})if(auto *slider=dialog->findChild<QSlider*>(name)){
        QSignalBlocker block(slider);slider->setValue(name=="ambientSpeed"?value.speed:value.intensity);
    }
    if(auto *label=dialog->findChild<QLabel*>("ambientSpeedValue"))label->setText(QString::number(value.speed/100.0,'f',1)+QString::fromUtf8("×"));
    if(auto *label=dialog->findChild<QLabel*>("ambientIntensityValue"))label->setText(QString::number(value.intensity)+"%");
}
inline QDialog *createDialog(QWidget *owner,const std::function<void(const Preferences&)> &apply){
    auto *dialog=new QDialog(owner);dialog->setObjectName("ambientBackgroundDialog");
    dialog->setWindowTitle("Ambient Glass background");dialog->setAttribute(Qt::WA_DeleteOnClose);dialog->setMinimumWidth(420);
    auto *layout=new QVBoxLayout(dialog);layout->setContentsMargins(22,22,22,18);layout->setSpacing(16);
    auto *title=new QLabel("Ambient Glass 1",dialog);auto font=title->font();font.setPixelSize(20);font.setWeight(QFont::DemiBold);title->setFont(font);layout->addWidget(title);
    auto *description=new QLabel("Original moving light behind your workspace.\nYour mapping and projected content stay unchanged.",dialog);description->setWordWrap(true);layout->addWidget(description);
    auto *form=new QFormLayout;form->setSpacing(14);layout->addLayout(form);
    auto *glassRow=new QHBoxLayout;
    auto *clear=new QRadioButton("Clear",dialog);clear->setObjectName("ambientGlassClear");
    auto *smoked=new QRadioButton("Smoked",dialog);smoked->setObjectName("ambientGlassSmoked");
    glassRow->addWidget(clear);glassRow->addWidget(smoked);glassRow->addStretch();form->addRow("Glass",glassRow);
    QObject::connect(clear,&QRadioButton::toggled,dialog,[apply](bool checked){if(checked){auto value=current();value.glass="clear";apply(value);}});
    QObject::connect(smoked,&QRadioButton::toggled,dialog,[apply](bool checked){if(checked){auto value=current();value.glass="smoked";apply(value);}});
    for(const bool speed:{true,false}){
        auto *row=new QHBoxLayout;auto *slider=new QSlider(Qt::Horizontal,dialog);slider->setObjectName(speed?"ambientSpeed":"ambientIntensity");
        slider->setRange(20,speed?120:100);slider->setSingleStep(speed?10:5);slider->setPageStep(speed?10:5);
        slider->setAccessibleName(speed?"Background speed":"Background light intensity");
        auto *valueLabel=new QLabel(dialog);valueLabel->setObjectName(speed?"ambientSpeedValue":"ambientIntensityValue");valueLabel->setMinimumWidth(42);valueLabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
        row->addWidget(slider,1);row->addWidget(valueLabel);form->addRow(speed?"Speed":"Light",row);
        QObject::connect(slider,&QSlider::valueChanged,dialog,[apply,speed](int number){auto value=current();if(speed)value.speed=number;else value.intensity=number;apply(value);});
    }
    auto *pause=new QCheckBox("Pause background motion",dialog);pause->setObjectName("ambientPauseControl");layout->addWidget(pause);
    QObject::connect(pause,&QCheckBox::toggled,dialog,[apply](bool checked){auto value=current();value.paused=checked;apply(value);});
    auto *note=new QLabel("Reduce interface motion also keeps this background still.\nHidden or minimized windows pause automatically.",dialog);note->setWordWrap(true);layout->addWidget(note);
    auto *buttons=new QDialogButtonBox(QDialogButtonBox::Close,dialog);
    auto *reset=buttons->addButton("Reset background",QDialogButtonBox::ResetRole);reset->setObjectName("ambientReset");
    QObject::connect(reset,&QPushButton::clicked,dialog,[apply]{apply(Preferences{});});
    QObject::connect(buttons,&QDialogButtonBox::rejected,dialog,&QDialog::close);layout->addWidget(buttons);
    syncDialog(dialog);return dialog;
}
}
