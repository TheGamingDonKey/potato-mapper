#pragma once
#include "material-ui-check.h"
#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QJsonDocument>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QDialog>
#include <QToolButton>
#include <QTimer>

// Synthetic scene only. The caller sets window.smoke=true before this check,
// so theme actions cannot overwrite the user's preferences.
template<class Window> void runAppearanceThemeChecks(Window &window,QApplication &app,const QStringList &args){
    QTimer::singleShot(400,&window,[&window,&app,args]{
        QStringList failures;auto require=[&](bool ok,const QString &label){if(!ok)failures<<label;};
        auto *browser=window.template findChild<QDialog*>("animationBrowserDialog");
        auto *browse=window.template findChild<QPushButton*>("browseAnimations");
        auto *connections=window.template findChild<QWidget*>("connectionsSection");
        auto *picker=window.template findChild<QComboBox*>("animationPicker");
        auto *category=window.template findChild<QComboBox*>("animationCategory");
        auto *search=window.template findChild<QLineEdit*>("animationSearch");
        auto *library=window.template findChild<QListWidget*>("animationLibrary");
        auto *apply=window.template findChild<QPushButton*>("applyAnimation");
        if(!browser||!browse||!connections||!picker||!category||!search||!library||!apply){qInfo()<<"THEMES FAIL missing controls";app.exit(2);return;}
        auto &scene=window.scene;scene.setPattern(scene.selected,17);scene.dirty=false;
        const auto saved=scene.json();const auto pattern=scene.current()->pattern;
        QStringList styles;
        for(const auto &id:QStringList{"classic1","halloween","autumn","christmas"}){
            auto *action=window.template findChild<QAction*>("theme_"+id);require(action,"theme action "+id);
            if(action){action->trigger();styles<<app.styleSheet();require(app.property("potatoTheme").toString()==id,"theme applied "+id);}
            require(scene.json()==saved&&!scene.dirty,"theme keeps mapping and clean state "+id);
        }
        styles.removeDuplicates();require(styles.size()==4,"four distinct editor palettes");
        PotatoMaterialUiCheck::reveal(browse,app);browse->click();app.processEvents();require(browser->isVisible()&&library->isVisible(),"animation browser reachable from sidebar");
        category->setCurrentIndex(category->findData("christmas"));search->setText("Snowfall");
        require(scene.current()->pattern==pattern&&scene.json()==saved&&!scene.dirty,"category and search preserve assigned Halloween content");
        require(picker->findData(17)>=0,"assigned content remains editable across categories");
        for(int row=0;row<library->count();++row)require(library->item(row)->data(Qt::UserRole).toInt()>=21,"Christmas category contains Christmas IDs");
        category->setCurrentIndex(category->findData("halloween"));search->setText("Ghost Parade");
        require(library->count()==1&&library->item(0)->data(Qt::UserRole).toInt()==17,"filtered Ghost Parade keeps stable ID 17");
        scene.setPattern(scene.selected,1);scene.dirty=false;apply->click();
        require(scene.current()->pattern==17,"Apply resolves filtered item data, not row zero");
        scene.undo();require(scene.current()->pattern==1,"Apply supports Undo");
        browser->close();auto *connectionHeading=connections->template findChild<QToolButton*>("sectionHeading");require(connectionHeading,"direct Connections section navigation");
        if(connectionHeading){connectionHeading->setChecked(true);scene.select(0);require(connectionHeading->isChecked(),"selection preserves expanded Connections section");}
        require(!window.template findChild<QWidget*>("taskTabs"),"restored sidebar does not require Studio task tabs");
        window.resize(1024,640);PotatoMaterialUiCheck::reveal(picker,app);app.processEvents();
        const int snapshot=args.indexOf("--snapshot");if(snapshot>=0&&snapshot+1<args.size())require(window.grab().save(args[snapshot+1]),"save actual task UI snapshot");
        qInfo().noquote()<<"THEMES"<<(failures.isEmpty()?"PASS":"FAIL")<<failures.join(", ");app.exit(failures.isEmpty()?0:2);
    });
}
