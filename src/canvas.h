#pragma once
#include "model.h"
#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>

class Canvas : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core {
    Q_OBJECT
public:
    explicit Canvas(Scene *scene,bool editor,QWidget *parent=nullptr);
    ~Canvas() override;
    bool meshMode=false;
    bool graphicsReady=false;
    QString graphicsError;
    QString graphicsDescription;
    QRectF canvasRect() const;
    void resetView();
signals:
    void graphicsInitialized(const QString &description);
protected:
    void initializeGL() override;
    void paintGL() override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void wheelEvent(QWheelEvent *) override;
    void keyPressEvent(QKeyEvent *) override;
    void dragEnterEvent(QDragEnterEvent *) override;
    void dropEvent(QDropEvent *) override;
private:
    Scene *scene;
    bool editor;
    QOpenGLShaderProgram program;
    QOpenGLBuffer buffer{QOpenGLBuffer::VertexBuffer};
    QOpenGLVertexArrayObject vao;
    struct Texture {GLuint id=0;quint64 revision=0;QSize size;};
    QHash<QString,Texture> textures;
    double zoom=1;
    QPointF pan,lastMouse,dragStart;
    int activeHandle=-1;
    bool dragging=false,panning=false;
    Surface dragOriginal;
    QPointF normalized(const QPointF &) const;
    QPointF screenPoint(const QPointF &) const;
    int hitSurface(const QPointF &) const;
    QVector<QPointF> handles(const Surface &) const;
    void cleanup();
};
