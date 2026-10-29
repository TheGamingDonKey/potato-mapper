#pragma once
#include "model.h"
#include "entrance.h"
#include <QPainter>
#include <QPainterPath>
#include <QLineF>
#include <QVector>
#include <algorithm>
#include <cmath>

// Original vector actors. One 32 s Scene phrase contains two unequal casts;
// output pixels, mapped target positions and the same clock serve both views.
inline void potatoDrawWizardDuel(QPainter &p,const Scene &scene,const QRectF &rect){
    if(scene.outputSize.isEmpty()||rect.isEmpty())return;
    const double tau=6.283185307179586;
    auto smooth=[](double a,double b,double x){double u=std::clamp((x-a)/(b-a),0.0,1.0);return u*u*(3-2*u);};
    auto beat=[&](double t,double a,double b,double c,double d){return smooth(a,b,t)*(1-smooth(c,d,t));};
    auto pixel=[&](QPointF q){return QPointF(q.x()*scene.outputSize.width(),q.y()*scene.outputSize.height());};
    auto mapped=[&](const Surface &s,double u,double v){return pixel(s.transform().map(s.sample(u,v)));};
    QVector<const Surface*> panels;QVector<QPainterPath> clips;
    for(const auto &s:scene.surfaces)if(s.visible&&s.opacity>0&&s.valid()){
        QPolygonF outline;
        for(int side=0;side<4;++side)for(int k=0;k<s.cells;++k){
            const double f=double(k)/s.cells;const QPointF uv=side==0?QPointF(f,0):side==1?QPointF(1,f):side==2?QPointF(1-f,1):QPointF(0,1-f);
            outline<<mapped(s,uv.x(),uv.y());
        }
        QPainterPath clip;clip.addPolygon(outline);clip.closeSubpath();panels<<&s;clips<<clip;
    }
    if(panels.isEmpty())return;
    const Surface *owners[2]{panels[0],panels.size()>1?panels[1]:panels[0]};
    QPointF base[2]{mapped(*owners[0],panels.size()>1?.5:.30,.56),mapped(*owners[1],panels.size()>1?.5:.70,.56)};
    double radius[2];
    for(int i=0;i<2;++i){
        const double width=QLineF(mapped(*owners[i],.1,.5),mapped(*owners[i],.9,.5)).length();
        const double height=QLineF(mapped(*owners[i],.5,.1),mapped(*owners[i],.5,.9)).length();
        radius[i]=std::clamp(std::min(width,height)*.19,9.0,44.0);
    }
    if(QLineF(base[0],base[1]).length()<2.6*std::max(radius[0],radius[1])){
        base[0]=mapped(*owners[0],.29,.56);base[1]=mapped(*owners[1],.71,.56);
    }
    const double phase=std::fmod(std::fmod(scene.actorElapsed,32.0)+32.0,32.0);
    const int caster=phase<16?0:1,target=1-caster;const double t=std::fmod(phase,16.0);
    const QPointF delta=base[target]-base[caster];const double distance=std::max(1.0,std::hypot(delta.x(),delta.y()));
    const QPointF direction=delta/distance,normal(-direction.y(),direction.x());
    const double travel=std::clamp(distance/260.0,1.15,3.2),launch=caster==0?5.1:5.6,arrival=launch+travel;
    const double prepare=beat(t,2.1,3.8,launch-.4,launch+.12);
    const double cast=beat(t,launch-.45,launch,launch+.28,launch+1.25);
    const double dodge=beat(t,arrival-.60,arrival-.10,arrival+.40,arrival+1.55);
    const double surprise=beat(t,arrival-.85,arrival-.35,arrival+.25,arrival+1.2);
    const QColor robes[2]{QColor("#7661aa"),QColor("#378b7c")};
    const QColor magic[2]{QColor("#d4b8ff"),QColor("#80f0cd")};
    auto star=[&](QPointF centre,double r,QColor color,double rotation){
        QPolygonF points;for(int k=0;k<10;++k){double a=rotation+k*tau/10,rr=r*(k%2?.43:1);points<<centre+QPointF(std::cos(a)*rr,std::sin(a)*rr);}
        p.setPen(Qt::NoPen);p.setBrush(color);p.drawPolygon(points);
    };
    // Draw through each real panel separately so an intermediate mapped panel
    // receives the passing spell at ITS brightness/opacity, never in the gaps.
    p.save();p.setClipRect(rect);p.translate(rect.topLeft());
    p.scale(rect.width()/scene.outputSize.width(),rect.height()/scene.outputSize.height());p.setRenderHint(QPainter::Antialiasing);
    const double reveal=scene.entranceActive?PotatoEntrance::reveal(scene.entranceElapsed):1.0;
    for(int panel=0;panel<panels.size();++panel){
        p.save();p.setClipPath(clips[panel],Qt::IntersectClip);
        p.setOpacity(reveal*panels[panel]->opacity/100.0*panels[panel]->brightness/100.0);
        for(int i=0;i<2;++i){
            const bool active=i==caster;const double r=radius[i],facing=base[1-i].x()<base[i].x()?-1.0:1.0;
            const double a=active?prepare:0,c=active?cast:0,d=active?0:dodge;
            const double breath=std::sin(phase*tau/8+i*1.7)*.017;
            const QPointF body=base[i]+(active?QPointF(0,0):normal*(r*1.45*d));
            const QPointF aim=active?direction:direction*-1;
            const QPointF localAim(aim.x()*facing,aim.y());
            p.save();p.translate(body);p.scale(facing*r,r);
            p.setPen(Qt::NoPen);p.setBrush(QColor(7,14,24,85));p.drawEllipse(QRectF(-.64,1.04,1.28,.18));
            p.setBrush(QColor("#26313f"));p.drawEllipse(QRectF(-.43-.08*d,.91,.39,.20));p.drawEllipse(QRectF(.12+.08*d,.91,.42,.20));
            QPainterPath cape;cape.moveTo(-.30,-.03);cape.cubicTo(-.54,.23,-.50,.66,-.66,.94);
            cape.quadTo(-.10,1.10,.52,.94);cape.quadTo(.39,.42,.27,-.03);cape.closeSubpath();
            p.setBrush(robes[i].darker(130));p.drawPath(cape);p.setBrush(robes[i]);
            QPainterPath coat;coat.moveTo(-.24,-.02);coat.lineTo(.25,-.02);coat.quadTo(.38,.45,.40,.91);coat.quadTo(0,1.02,-.39,.91);coat.closeSubpath();p.drawPath(coat);
            p.setPen(QPen(robes[i].lighter(128),.025));p.drawLine(QPointF(-.17,.20),QPointF(-.23,.81));
            p.setPen(QPen(QColor("#cfb681"),.075,Qt::SolidLine,Qt::RoundCap));p.drawLine(QPointF(-.29,.53),QPointF(.30,.53));
            const QPointF head(.02-.035*a,-.25+breath+.04*a);
            p.setPen(Qt::NoPen);p.setBrush(QColor("#eac493"));p.drawEllipse(head,.33,.32);
            p.setBrush(QColor("#b7806d"));p.drawEllipse(head+QPointF(.20,.09),.07,.035);
            const double blink=beat(phase,1.8+i*9,1.89+i*9,1.97+i*9,2.14+i*9)+beat(phase,25+i*1.7,25.09+i*1.7,25.17+i*1.7,25.34+i*1.7);
            p.setPen(QPen(QColor("#24223a"),.035,Qt::SolidLine,Qt::RoundCap));
            for(double x:{-.095,.14}){QPointF eye=head+QPointF(x+.025*localAim.x(),-.005+.020*localAim.y());if(blink>.55)p.drawLine(eye-QPointF(.036,0),eye+QPointF(.036,0));else p.drawEllipse(eye,.032,.043+.016*(active?0:surprise));}
            if(!active&&surprise>.25)p.drawEllipse(head+QPointF(.05,.15),.042,.055);
            else{QPainterPath smile;smile.moveTo(head+QPointF(-.045,.13));smile.quadTo(head+QPointF(.04,.19),head+QPointF(.12,.12));p.drawPath(smile);}
            QPainterPath hat;hat.moveTo(-.40,-.47+breath);hat.quadTo(-.25,-.90,-.11,-1.21+breath-.08*a);
            hat.quadTo(.31,-1.06,.25+.09*std::sin(phase*tau/16+i),-.85);hat.lineTo(.34,-.47+breath);hat.closeSubpath();
            p.setPen(QPen(robes[i].darker(150),.025));p.setBrush(robes[i].darker(112));p.drawPath(hat);
            p.setPen(QPen(QColor("#d2ba84"),.067,Qt::SolidLine,Qt::RoundCap));p.drawLine(QPointF(-.38,-.48+breath),QPointF(.37,-.48+breath));
            star(QPointF(.06,-.76+breath),.095,QColor("#e4c57a"),-.35);
            const QPointF shoulder(.25,.03),neutral(.54,.27),raised(.46,-.42);
            const QPointF hand=(neutral*(1-a)+raised*a)*(1-c)+(shoulder+localAim*.70)*c;
            p.setPen(QPen(robes[i].lighter(115),.14,Qt::SolidLine,Qt::RoundCap));p.drawLine(shoulder,hand);
            p.setPen(Qt::NoPen);p.setBrush(QColor("#eac493"));p.drawEllipse(hand,.087,.087);
            const QPointF wandDirection=QPointF(.35,-.94)*(1-c)+localAim*c,tip=hand+wandDirection*.45;
            p.setPen(QPen(QColor("#dbc29c"),.038,Qt::SolidLine,Qt::RoundCap));p.drawLine(hand-wandDirection*.13,tip);
            if(active&&(a+c)>.01)star(tip,.07+.06*a,magic[i],phase*tau/4);
            p.restore();
        }
        if(t>=launch&&t<=arrival+.55){
            const double u=std::clamp((t-launch)/travel,0.0,1.0),alive=1-smooth(arrival,arrival+.55,t);
            const double facing=delta.x()<0?-1.0:1.0;
            const QPointF start=base[caster]+QPointF(facing*radius[caster]*.25,radius[caster]*.03)+direction*(radius[caster]*1.15);
            const QPointF finish=base[target]+QPointF(0,-radius[target]*.15);
            auto point=[&](double v){return start*(1-v)+finish*v+normal*(std::sin(v*3.141592653589793)*std::min(42.0,distance*.10));};
            for(int k=8;k>=1;--k){const double v=std::max(0.0,u-k*.017);QColor trail=magic[caster];trail.setAlphaF(alive*(1-k/9.0)*.45);p.setPen(QPen(trail,2.0+(9-k)*.45,Qt::SolidLine,Qt::RoundCap));p.drawLine(point(v),point(std::min(u,v+.017)));}
            QColor core=magic[caster];core.setAlphaF(alive);star(point(u),6.5+1.5*std::sin(t*tau),core,t*tau);
            if(t>arrival)for(int k=0;k<5;++k){double a=k*tau/5;QColor spark=magic[caster];spark.setAlphaF(alive*.8);star(finish+QPointF(std::cos(a),std::sin(a))*(t-arrival)*42,2.2,spark,a);}
        }
        p.restore();
    }
    p.restore();
}
