#pragma once
class QWidget;
class QOpenGLWidget;
namespace PotatoDesktopGlass {
// Call after parenting the canvas and before showing the editor.
void prepare(QWidget *editor,QOpenGLWidget *canvas);
void sync(QWidget *editor);
bool active(const QWidget *window);
}
