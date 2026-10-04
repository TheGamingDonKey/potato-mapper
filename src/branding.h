#pragma once
#include <QSplashScreen>
#include <QPainter>
#include <QScreen>
#include <QGuiApplication>

inline QSplashScreen *potatoSplash(){
    auto *screen=QGuiApplication::primaryScreen();
    const qreal scale=screen?screen->devicePixelRatio():1;
    QPixmap image(QSize(620,340)*scale);image.setDevicePixelRatio(scale);image.fill(QColor("#101720"));
    QPainter p(&image);p.setRenderHint(QPainter::Antialiasing);
    QRadialGradient glow(QPointF(145,160),220);glow.setColorAt(0,QColor(70,135,130,95));glow.setColorAt(1,QColor(16,23,32,0));
    p.fillRect(QRect(0,0,620,340),glow);
    p.setPen(QPen(QColor("#334b54"),1));p.drawRect(0,0,619,339);
    p.drawPixmap(QRect(24,42,260,260),QPixmap(":/assets/potato-mapper.png"));
    p.setPen(QColor("#83e5cc"));p.setFont(QFont("Segoe UI",10,QFont::DemiBold));p.drawText(310,106,"PROJECTION, YOUR WAY");
    p.setPen(QColor("#f4e0b4"));p.setFont(QFont("Segoe UI",30,QFont::Bold));p.drawText(307,157,"POTATO");p.drawText(307,202,"MAPPER");
    p.setPen(QColor("#bac9d4"));p.setFont(QFont("Segoe UI",11));p.drawText(310,240,"Version "+QCoreApplication::applicationVersion());
    p.setPen(QColor("#718895"));p.setFont(QFont("Segoe UI",9));p.drawText(310,294,"Click to enter your workspace");
    p.end();
    auto *splash=new QSplashScreen(screen,image);splash->setWindowFlag(Qt::WindowStaysOnTopHint);
    splash->setWindowTitle("Potato Mapper — Starting");
    if(screen)splash->move(screen->availableGeometry().center()-QPoint(310,170));
    return splash;
}
