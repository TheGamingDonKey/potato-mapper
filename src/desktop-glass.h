#pragma once
class QWidget;
namespace PotatoDesktopGlass {
// Call before the editor gets a native handle; never change its window flags.
void prepare(QWidget *editor);
void sync(QWidget *editor);
bool active(const QWidget *window);
}
