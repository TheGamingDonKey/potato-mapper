#pragma once
#include <QColor>
#include <QJsonObject>
#include <QPolygonF>
#include <QStringList>
#include <QTransform>
#include <QVector>

namespace PotatoDynamic {
// Scene advances one shared clock by speed/100 and freezes it when paused.
// Effect seconds below are already scaled; generators never read wall time.
struct Settings {
    int effect = 0; // 0 off, 1 Hot-Ass Potato, 2 Potato Focus
    int speed = 100;
    int density = 36;
    int palette = 0; // Holographic, White, Cyan, Violet, Amber, Mint (0..5).
    int glitch = 35;
    int snakeWidth = 40; // Projector-output pixels, independent of panel size.
    int cellSize = 56; // Cell side in output pixels; spacing is 1.8 times this.
    bool playing = true;
    quint32 seed = 0x504f5441u;
    QStringList members;
    bool operator==(const Settings &) const;
    bool operator!=(const Settings &other) const { return !(*this == other); }
    QJsonObject json() const;
    static Settings fromJson(const QJsonObject &);
};
struct Surface {
    QString id;
    QPolygonF boundary; // Mapped output positions, used only to choose the tour.
    double aspect = 1; // Mapped physical width/height, including output aspect.
    QTransform projection; // Deformed local coordinates to output pixels.
    int cells = 1;
    QVector<QPointF> mesh; // Empty means a regular mesh derived from boundary.
};
struct Primitive {
    QString surfaceId;
    QPolygonF points; // Local unit-square UV, forwarded through the mapped mesh.
    QColor color;
    bool filled = false;
    double width = .002; // Local surface units; renderer may draw a soft glow.
    bool closed = true; // Two-point strokes are open; perimeter arcs set false.
    double outputWidth = 0; // Optional output-pixel stroke; zero retains legacy UV width.
};
struct Frame {
    QVector<Primitive> shapes;
    QString stage;
    double titleOpacity = 0;
};
struct Timeline {
    double exploreEnd = 0;
    double overloadEnd = 0;
    double titleEnd = 0;
};
Timeline timeline(const Settings &, int surfaceCount);
double ambientStart(const Settings &, int surfaceCount);
// Caller supplies participating visible surfaces, and caches this once per tick.
// At most 500 primitives are produced; no state accumulates between frames.
Frame frame(const Settings &, const QVector<Surface> &, double elapsedSeconds);
}
