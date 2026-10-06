#include "canvas.h"
#include <QPainter>
#include <QFile>
#include <QMimeData>
#include <QMatrix3x3>
#include <QLineF>
#include <QTimer>
#include <QContextMenuEvent>
#include <algorithm>
#include <cmath>

Canvas::Canvas(Scene *s,bool editing,QWidget *parent):QOpenGLWidget(parent),scene(s),editor(editing){
    setMinimumSize(320,180);setFocusPolicy(Qt::StrongFocus);setAcceptDrops(editor);
    connect(scene,&Scene::changed,this,qOverload<>(&Canvas::update));
    if(editor)setToolTip("Drag handles to map. Drag inside a surface to move it. Middle-drag to pan; wheel to zoom.");
    else setCursor(Qt::BlankCursor);
}
Canvas::~Canvas(){cleanup();}
void Canvas::cleanup(){
    if(!context())return;makeCurrent();
    for(auto &t:textures){if(t.id)glDeleteTextures(1,&t.id);if(t.chroma)glDeleteTextures(1,&t.chroma);}
    if(profileQueries[0])glDeleteQueries(4,profileQueries);
    std::fill(std::begin(profileQueries),std::end(profileQueries),0u);std::fill(std::begin(profilePending),std::end(profilePending),false);activeProfileQuery=-1;
    textures.clear();buffer.destroy();vao.destroy();program.removeAllShaders();dynamicBuffer.destroy();dynamicVao.destroy();dynamicProgram.removeAllShaders();dynamicUploadedRevision=0;doneCurrent();graphicsReady=false;
}
void Canvas::initializeGL(){
    if(!initializeOpenGLFunctions()){graphicsError="OpenGL 3.3 could not be initialized.";emit graphicsInitialized(graphicsError);return;}
    connect(context(),&QOpenGLContext::aboutToBeDestroyed,this,&Canvas::cleanup,Qt::DirectConnection);
    graphicsDescription=QString::fromLatin1(reinterpret_cast<const char*>(glGetString(GL_RENDERER)))+" / "+QString::fromLatin1(reinterpret_cast<const char*>(glGetString(GL_VERSION)));
    const char *vertex=R"GLSL(#version 330 core
layout(location=0) in vec2 position;
layout(location=1) in vec2 texturePosition;
uniform mat3 mapping;
out vec2 uv;
void main(){vec3 p=mapping*vec3(position,1.0);gl_Position=vec4(2.0*p.x-p.z,p.z-2.0*p.y,0.0,p.z);uv=texturePosition;}
)GLSL";
    QFile shaderFile(":/assets/patterns.frag");
    if(!shaderFile.open(QIODevice::ReadOnly)){graphicsError="The built-in pattern shader could not be loaded.";emit graphicsInitialized(graphicsError);return;}
    const auto fragment=shaderFile.readAll();
    if(!program.addShaderFromSourceCode(QOpenGLShader::Vertex,vertex)||!program.addShaderFromSourceCode(QOpenGLShader::Fragment,fragment.constData())||!program.link()){
        graphicsError=program.log();emit graphicsInitialized(graphicsError);return;
    }
    const char *dynamicVertex=R"GLSL(#version 330 core
layout(location=0) in vec2 position;
layout(location=1) in vec4 colour;
uniform mat3 mapping;
out vec4 ink;
void main(){vec3 p=mapping*vec3(position,1.0);gl_Position=vec4(2.0*p.x-p.z,p.z-2.0*p.y,0.0,p.z);ink=colour;}
)GLSL";
    const char *dynamicFragment=R"GLSL(#version 330 core
in vec4 ink;uniform float brightness;uniform float opacity;uniform int blendMode;out vec4 outputColour;
void main(){vec4 c=vec4(ink.rgb*brightness,ink.a*opacity);if(blendMode==1)c.rgb*=c.a;outputColour=c;}
)GLSL";
    if(!dynamicProgram.addShaderFromSourceCode(QOpenGLShader::Vertex,dynamicVertex)||!dynamicProgram.addShaderFromSourceCode(QOpenGLShader::Fragment,dynamicFragment)||!dynamicProgram.link()){
        graphicsError=dynamicProgram.log();emit graphicsInitialized(graphicsError);return;
    }
    vao.create();buffer.create();dynamicVao.create();dynamicBuffer.create();graphicsReady=true;emit graphicsInitialized(graphicsDescription);
}
void Canvas::drawDynamic(const Surface &s,int surfaceIndex){
    const auto &geometry=scene->dynamicGeometry();
    program.release();buffer.release();vao.release();dynamicProgram.bind();dynamicVao.bind();dynamicBuffer.bind();
    dynamicProgram.enableAttributeArray(0);dynamicProgram.enableAttributeArray(1);
    dynamicProgram.setAttributeBuffer(0,GL_FLOAT,0,2,6*sizeof(float));dynamicProgram.setAttributeBuffer(1,GL_FLOAT,2*sizeof(float),4,6*sizeof(float));
    const auto t=s.transform();QMatrix3x3 m;
    m(0,0)=float(t.m11());m(0,1)=float(t.m21());m(0,2)=float(t.m31());m(1,0)=float(t.m12());m(1,1)=float(t.m22());m(1,2)=float(t.m32());m(2,0)=float(t.m13());m(2,1)=float(t.m23());m(2,2)=float(t.m33());
    dynamicProgram.setUniformValue("mapping",m);dynamicProgram.setUniformValue("brightness",s.brightness/100.f);dynamicProgram.setUniformValue("opacity",s.opacity/100.f);dynamicProgram.setUniformValue("blendMode",s.blend);
    if(dynamicUploadedRevision!=geometry.revision){
        dynamicBuffer.allocate(geometry.vertices.constData(),int(geometry.vertices.size()*sizeof(float)));dynamicUploadedRevision=geometry.revision;if(profileRendering)++renderStats.uploads;
    }
    const auto range=geometry.ranges.value(surfaceIndex);glDrawArrays(GL_TRIANGLES,range.first,range.second);
    dynamicBuffer.release();dynamicVao.release();dynamicProgram.release();program.bind();vao.bind();buffer.bind();
}
QRectF Canvas::canvasRect() const {
    const double margin=editor?28:0;
    const double scale=std::min(std::max(1.0,width()-2*margin)/scene->outputSize.width(),std::max(1.0,height()-2*margin)/scene->outputSize.height())*(editor?zoom:1);
    QSizeF size(scene->outputSize.width()*scale,scene->outputSize.height()*scale);
    return QRectF(QPointF((width()-size.width())/2,(height()-size.height())/2)+(editor?pan:QPointF()),size);
}
QPointF Canvas::normalized(const QPointF &p)const{auto r=canvasRect();return {(p.x()-r.x())/r.width(),(p.y()-r.y())/r.height()};}
QPointF Canvas::screenPoint(const QPointF &p)const{auto r=canvasRect();return {r.x()+p.x()*r.width(),r.y()+p.y()*r.height()};}
void Canvas::resetView(){zoom=1;pan={};update();}
QVector<QPointF> Canvas::handles(const Surface &s)const{
    if(!meshMode)return s.corners;
    QVector<QPointF> result;const auto t=s.transform();for(auto p:s.mesh)result.append(t.map(p));return result;
}
int Canvas::hitSurface(const QPointF &pos)const{
    const auto p=normalized(pos);
    for(int i=int(scene->surfaces.size())-1;i>=0;i--){
        const auto &s=scene->surfaces[i];if(!s.visible)continue;const auto t=s.transform();
        for(int y=0;y<s.cells;y++)for(int x=0;x<s.cells;x++){
            int a=y*(s.cells+1)+x;QPolygonF cell{t.map(s.mesh[a]),t.map(s.mesh[a+1]),t.map(s.mesh[a+s.cells+2]),t.map(s.mesh[a+s.cells+1])};
            if(cell.containsPoint(p,Qt::OddEvenFill))return i;
        }
    }
    return -1;
}
void Canvas::paintGL(){
    ++paintedFrames;
    if(!graphicsReady){QPainter p(this);p.fillRect(rect(),Qt::black);p.setPen(Qt::white);if(editor)p.drawText(rect(),Qt::AlignCenter,graphicsError);return;}
    if((editor||!scene->blackout)&&scene->dynamic.effect&&!scene->dynamicSurfaces.isEmpty()){
        // Prepare once before the native pass so profiling excludes CPU meshing.
        QElapsedTimer meshTimer;if(profileRendering)meshTimer.start();const auto builds=scene->dynamicGeometryBuilds;
        scene->dynamicGeometry();
        if(profileRendering){renderStats.meshNs+=meshTimer.nsecsElapsed();renderStats.meshBuilds+=scene->dynamicGeometryBuilds-builds;}
    }
    QPainter p(this);
    p.beginNativePainting();
    if(profileRendering){
        if(!profileQueries[0])glGenQueries(4,profileQueries);
        for(int i=0;i<4;++i)if(profilePending[i]){
            GLint ready=0;glGetQueryObjectiv(profileQueries[i],GL_QUERY_RESULT_AVAILABLE,&ready);
            if(ready){GLuint64 ns=0;glGetQueryObjectui64v(profileQueries[i],GL_QUERY_RESULT,&ns);renderStats.gpuNs+=ns;++renderStats.gpuSamples;profilePending[i]=false;}
        }
        for(int i=0;i<4;++i)if(!profilePending[i]){activeProfileQuery=i;glBeginQuery(GL_TIME_ELAPSED,profileQueries[i]);break;}
    }
    glDisable(GL_SCISSOR_TEST);glDisable(GL_DEPTH_TEST);glDisable(GL_CULL_FACE);
    glClearColor(editor?.065f:0,editor?.078f:0,editor?.095f:0,1);glClear(GL_COLOR_BUFFER_BIT);
    const auto r=canvasRect();const double dpi=devicePixelRatioF();
    const int vx=qRound(r.x()*dpi),vy=qRound((height()-r.bottom())*dpi),vw=qRound(r.width()*dpi),vh=qRound(r.height()*dpi);
    glViewport(vx,vy,vw,vh);glEnable(GL_SCISSOR_TEST);glScissor(vx,vy,vw,vh);glClearColor(0,0,0,1);glClear(GL_COLOR_BUFFER_BIT);
    // The canvas is opaque; retain that alpha while compositing surface layers.
    // Otherwise Qt's window composition applies their transparency a second time.
    glEnable(GL_BLEND);glBlendFuncSeparate(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
    program.bind();vao.bind();buffer.bind();
    program.enableAttributeArray(0);program.enableAttributeArray(1);
    program.setAttributeBuffer(0,GL_FLOAT,0,2,4*sizeof(float));program.setAttributeBuffer(1,GL_FLOAT,2*sizeof(float),2,4*sizeof(float));
    program.setUniformValue("picture",0);
    program.setUniformValue("chromaPicture",1);
    if(editor||!scene->blackout)for(int surfaceIndex=0;surfaceIndex<scene->surfaces.size();++surfaceIndex){
        const auto &s=scene->surfaces[surfaceIndex];
        if(!s.visible)continue;
        if(s.blend==1)glBlendFuncSeparate(GL_ONE,GL_ONE_MINUS_SRC_COLOR,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
        else glBlendFuncSeparate(GL_SRC_ALPHA,s.blend==2?GL_ONE:GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
        program.setUniformValue("blendMode",s.blend);
        if(scene->dynamicSurface(s.id)){drawDynamic(s,surfaceIndex);continue;}
        auto *media=scene->source(s.media);const auto &img=media->image;const auto frameSize=media->frameSize();if(frameSize.isEmpty())continue;
        const bool yuv=media->mappedVideo.isValid();program.setUniformValue("yuvVideo",yuv);
        auto &tex=textures[s.media];if(!tex.id){glGenTextures(1,&tex.id);}
        glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,tex.id);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,s.media.isEmpty()?GL_LINEAR_MIPMAP_LINEAR:GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
        if(yuv){
            const auto &frame=media->mappedVideo;const auto format=frame.surfaceFormat();
            if(!tex.chroma)glGenTextures(1,&tex.chroma);
            const bool allocate=tex.size!=frameSize||!tex.yuv;
            for(int plane=0;plane<2;++plane){
                glActiveTexture(GL_TEXTURE0+plane);glBindTexture(GL_TEXTURE_2D,plane?tex.chroma:tex.id);
                glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
                if(tex.revision!=media->revision){
                    const int w=plane?(frame.width()+1)/2:frame.width(),h=plane?(frame.height()+1)/2:frame.height();
                    glPixelStorei(GL_UNPACK_ALIGNMENT,1);glPixelStorei(GL_UNPACK_ROW_LENGTH,frame.bytesPerLine(plane)/(plane?2:1));
                    if(allocate)glTexImage2D(GL_TEXTURE_2D,0,plane?GL_RG8:GL_R8,w,h,0,plane?GL_RG:GL_RED,GL_UNSIGNED_BYTE,frame.bits(plane));
                    else glTexSubImage2D(GL_TEXTURE_2D,0,0,0,w,h,plane?GL_RG:GL_RED,GL_UNSIGNED_BYTE,frame.bits(plane));
                }
            }
            glPixelStorei(GL_UNPACK_ROW_LENGTH,0);glPixelStorei(GL_UNPACK_ALIGNMENT,4);glActiveTexture(GL_TEXTURE0);
            tex.size=frameSize;tex.yuv=true;tex.revision=media->revision;
            double kr=.2126,kb=.0722;
            if(format.colorSpace()==QVideoFrameFormat::ColorSpace_BT601){kr=.299;kb=.114;}
            const double kg=1-kr-kb;const bool full=format.colorRange()==QVideoFrameFormat::ColorRange_Full;
            const float ys=full?1.f:255.f/219.f,cs=full?1.f:255.f/224.f;
            QMatrix3x3 colorMatrix;colorMatrix(0,0)=ys;colorMatrix(0,1)=0;colorMatrix(0,2)=float(2*(1-kr))*cs;
            colorMatrix(1,0)=ys;colorMatrix(1,1)=float(-2*kb*(1-kb)/kg)*cs;colorMatrix(1,2)=float(-2*kr*(1-kr)/kg)*cs;
            colorMatrix(2,0)=ys;colorMatrix(2,1)=float(2*(1-kb))*cs;colorMatrix(2,2)=0;
            program.setUniformValue("yuvToRgb",colorMatrix);program.setUniformValue("yuvOffset",QVector3D(full?0.f:16.f/255.f,128.f/255.f,128.f/255.f));
        }else if(tex.revision!=media->revision){
            glPixelStorei(GL_UNPACK_ALIGNMENT,4);glPixelStorei(GL_UNPACK_ROW_LENGTH,img.bytesPerLine()/4);
            if(tex.size!=img.size()||tex.yuv){glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,img.width(),img.height(),0,GL_RGBA,GL_UNSIGNED_BYTE,img.constBits());tex.size=img.size();}
            else glTexSubImage2D(GL_TEXTURE_2D,0,0,0,img.width(),img.height(),GL_RGBA,GL_UNSIGNED_BYTE,img.constBits());
            if(s.media.isEmpty())glGenerateMipmap(GL_TEXTURE_2D);
            glPixelStorei(GL_UNPACK_ROW_LENGTH,0);tex.revision=media->revision;tex.yuv=false;
        }
        const auto t=s.transform();QMatrix3x3 m;
        m(0,0)=float(t.m11());m(0,1)=float(t.m21());m(0,2)=float(t.m31());
        m(1,0)=float(t.m12());m(1,1)=float(t.m22());m(1,2)=float(t.m32());
        m(2,0)=float(t.m13());m(2,1)=float(t.m23());m(2,2)=float(t.m33());program.setUniformValue("mapping",m);
        QVector2D uvScale(1,1),uvOffset(0,0);
        program.setUniformValue("pattern",s.pattern);program.setUniformValue("phase",float(std::fmod(s.patternPhase,10000.0)));program.setUniformValue("dotRadius",s.patternSize/100.0f);
        program.setUniformValue("density",float(s.fx.density));program.setUniformValue("flow",s.fx.flow/100.0f);program.setUniformValue("angle",s.fx.angle*float(3.141592653589793/180.0));program.setUniformValue("palette",s.fx.palette);program.setUniformValue("edgeFade",s.fx.edge/100.0f);
        program.setUniformValue("brightness",s.brightness/100.0f);program.setUniformValue("opacity",s.opacity/100.0f);
        {
            const auto pixel=[this](QPointF p){return QPointF(p.x()*scene->outputSize.width(),p.y()*scene->outputSize.height());};
            double sw=(QLineF(pixel(s.corners[0]),pixel(s.corners[1])).length()+QLineF(pixel(s.corners[3]),pixel(s.corners[2])).length())/2;
            double sh=(QLineF(pixel(s.corners[0]),pixel(s.corners[3])).length()+QLineF(pixel(s.corners[1]),pixel(s.corners[2])).length())/2;
            program.setUniformValue("surfaceAspect",float(std::clamp(sw/std::max(sh,.0001),.05,20.0)));
            if(s.fit&&!s.pattern){
            double ratio=(double(frameSize.width())/frameSize.height())/(sw/std::max(sh,.0001));
            if(s.fit==1){if(ratio>1)uvScale.setY(float(ratio));else uvScale.setX(float(1/ratio));}
            else {if(ratio>1)uvScale.setX(float(1/ratio));else uvScale.setY(float(ratio));}
            uvOffset=(QVector2D(1,1)-uvScale)*.5f;
            }
        }
        program.setUniformValue("uvScale",uvScale);program.setUniformValue("uvOffset",uvOffset);
        QVector<float> vertices;vertices.reserve(s.cells*s.cells*24);
        auto add=[&](int index){auto p=s.mesh[index];vertices<<float(p.x())<<float(p.y())<<float(index%(s.cells+1))/s.cells<<float(index/(s.cells+1))/s.cells;};
        for(int y=0;y<s.cells;y++)for(int x=0;x<s.cells;x++){int a=y*(s.cells+1)+x,b=a+1,c=a+s.cells+1,d=c+1;add(a);add(b);add(d);add(a);add(d);add(c);}
        buffer.allocate(vertices.constData(),int(vertices.size()*sizeof(float)));glDrawArrays(GL_TRIANGLES,0,int(vertices.size()/4));
    }
    buffer.release();vao.release();program.release();glActiveTexture(GL_TEXTURE1);glBindTexture(GL_TEXTURE_2D,0);glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,0);glDisable(GL_SCISSOR_TEST);
    glViewport(0,0,qRound(width()*dpi),qRound(height()*dpi));
    if(activeProfileQuery>=0){glEndQuery(GL_TIME_ELAPSED);profilePending[activeProfileQuery]=true;activeProfileQuery=-1;}
    p.endNativePainting();
    if((editor||!scene->blackout)&&scene->dynamicFrame.titleOpacity>0){
        p.save();p.setClipRect(r);p.setRenderHint(QPainter::Antialiasing);p.setOpacity(scene->dynamicFrame.titleOpacity);
        QFont font("Segoe UI");font.setBold(true);font.setPixelSize(qMax(12,int(r.height()*.065)));font.setLetterSpacing(QFont::AbsoluteSpacing,r.height()*.004);p.setFont(font);p.setPen(QColor("#bcfaff"));p.drawText(r,Qt::AlignCenter,"POTATO MAPPER");p.restore();
    }
    if(!editor)return;
    p.setRenderHint(QPainter::Antialiasing);p.setPen(QPen(QColor("#566171"),1));p.drawRect(r);
    for(int i=0;i<scene->surfaces.size();i++){
        const auto &s=scene->surfaces[i];if(!s.visible)continue;const auto t=s.transform();bool selected=i==scene->selected;
        p.setPen(QPen(selected?QColor("#78efce"):QColor("#718090"),selected?1.5:1));
        if(selected&&meshMode){for(int y=0;y<=s.cells;y++)for(int x=0;x<=s.cells;x++){int a=y*(s.cells+1)+x;if(x<s.cells)p.drawLine(screenPoint(t.map(s.mesh[a])),screenPoint(t.map(s.mesh[a+1])));if(y<s.cells)p.drawLine(screenPoint(t.map(s.mesh[a])),screenPoint(t.map(s.mesh[a+s.cells+1])));}}
        QPolygonF boundary;for(auto c:s.corners)boundary<<screenPoint(c);p.setBrush(Qt::NoBrush);p.drawPolygon(boundary);
        p.setPen(selected?QColor("#bbffeb"):QColor("#c1cad4"));p.drawText(screenPoint(s.corners[0])+QPointF(0,-12),s.name+(s.locked?"  [locked]":""));
        if(selected&&!s.locked){auto points=handles(s);for(int j=0;j<points.size();j++){p.setPen(QPen(QColor("#11201d"),2));p.setBrush(j==activeHandle?QColor("#ffffff"):QColor("#78efce"));p.drawEllipse(screenPoint(points[j]),5.5,5.5);}}
    }
    if(scene->blackout){p.setPen(QColor("#ffcc86"));p.drawText(12,21,"PROJECTOR BLACKOUT");}
}
void Canvas::mousePressEvent(QMouseEvent *e){
    if(!editor)return;setFocus();lastMouse=e->position();
    if(e->button()==Qt::MiddleButton){panning=true;return;}if(e->button()!=Qt::LeftButton)return;
    activeHandle=-1;
    if(auto *s=scene->current();s&&s->visible&&!s->locked){auto points=handles(*s);for(int i=0;i<points.size();i++)if(QLineF(screenPoint(points[i]),e->position()).length()<12){activeHandle=i;break;}}
    if(activeHandle<0)scene->select(hitSurface(e->position()));
    if(auto *s=scene->current();s&&!s->locked){scene->checkpoint();dragOriginal=*s;dragStart=normalized(e->position());dragging=true;}
    update();
}
void Canvas::contextMenuEvent(QContextMenuEvent *e){
    if(!editor){e->ignore();return;}
    const int index=hitSurface(e->pos());
    if(index<0){e->ignore();return;}
    scene->select(index);emit surfaceContextMenuRequested(index,e->globalPos());e->accept();
}
void Canvas::mouseMoveEvent(QMouseEvent *e){
    if(panning){pan+=e->position()-lastMouse;lastMouse=e->position();update();return;}
    if(!dragging)return;auto *s=scene->current();if(!s)return;
    Surface candidate=dragOriginal;auto pos=normalized(e->position());
    if(activeHandle>=0){if(meshMode)candidate.mesh[activeHandle]=candidate.transform().inverted().map(pos);else candidate.corners[activeHandle]=pos;}
    else for(auto &point:candidate.corners)point+=pos-dragStart;
    if(candidate.valid()){*s=candidate;scene->touch();}
}
void Canvas::mouseReleaseEvent(QMouseEvent *){dragging=false;panning=false;}
void Canvas::wheelEvent(QWheelEvent *e){
    if(!editor)return;auto anchor=normalized(e->position());zoom=std::clamp(zoom*std::pow(1.0015,e->angleDelta().y()),.25,8.0);pan+=e->position()-screenPoint(anchor);update();e->accept();
}
void Canvas::keyPressEvent(QKeyEvent *e){
    if(!editor){if(e->key()==Qt::Key_Escape)hide();return;}
    if(e->key()==Qt::Key_F){resetView();return;}
    auto *s=scene->current();if(!s||s->locked)return;
    QPointF delta;double step=e->modifiers().testFlag(Qt::ShiftModifier)?10:1;
    switch(e->key()){case Qt::Key_Left:delta.setX(-step/scene->outputSize.width());break;case Qt::Key_Right:delta.setX(step/scene->outputSize.width());break;case Qt::Key_Up:delta.setY(-step/scene->outputSize.height());break;case Qt::Key_Down:delta.setY(step/scene->outputSize.height());break;default:QOpenGLWidget::keyPressEvent(e);return;}
    Surface candidate=*s;
    if(activeHandle>=0&&activeHandle<handles(candidate).size()){
        if(meshMode){auto t=candidate.transform();candidate.mesh[activeHandle]=t.inverted().map(t.map(candidate.mesh[activeHandle])+delta);}else candidate.corners[activeHandle]+=delta;
    }else for(auto &p:candidate.corners)p+=delta;
    if(candidate.valid()){scene->checkpoint();*s=candidate;scene->touch();}e->accept();
}
void Canvas::dragEnterEvent(QDragEnterEvent *e){if(editor&&e->mimeData()->hasUrls())e->acceptProposedAction();}
void Canvas::dropEvent(QDropEvent *e){
    if(!editor||!e->mimeData()->hasUrls())return;int target=hitSurface(e->position());
    for(const auto &url:e->mimeData()->urls()){if(!url.isLocalFile())continue;if(target<0){scene->add();target=scene->selected;}scene->assignMedia(target,url.toLocalFile());scene->select(target);target=-1;}
    e->acceptProposedAction();
}
