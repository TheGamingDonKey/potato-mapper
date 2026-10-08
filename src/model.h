#pragma once
#include <QObject>
#include <QImage>
#include <QMediaPlayer>
#include <QVideoSink>
#include <QVideoFrame>
#include <QAudioOutput>
#include <QPolygonF>
#include <QTransform>
#include <QJsonObject>
#include <QHash>
#include <QUuid>
#include <QElapsedTimer>
#include "patterns.h"
#include "dynamic.h"

class MediaSource : public QObject {
    Q_OBJECT
public:
    explicit MediaSource(const QString &path, QObject *parent = nullptr);
    ~MediaSource() override;
    void presentFrame(const QVideoFrame &frame);
    QVideoFrame mappedVideo;
    QSize frameSize() const { return mappedVideo.isValid()?mappedVideo.size():image.size(); }
    QImage image;
    quint64 revision = 1;
    qint64 conversionNs = 0;
    QMediaPlayer *player = nullptr;
    QAudioOutput *audio = nullptr;
signals:
    void frameChanged();
    void error(const QString &message);
};

struct Surface {
    QString id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QString name;
    QString media;
    int pattern = 0; // Stable IDs in patterns.h, also used by the shared shader.
    int patternSpeed = 100;
    int patternSize = 18;
    bool patternPlaying = true;
    double patternPhase = 0;
    FxLook fx;
    int brightness = 100;
    int opacity = 100;
    int blend = 0; // 0 Normal, 1 Screen, 2 Add
    QPolygonF corners{QPointF(.15,.15), QPointF(.65,.15), QPointF(.65,.65), QPointF(.15,.65)};
    int cells = 2;
    QVector<QPointF> mesh;
    bool visible = true;
    bool locked = false;
    int fit = 0; // 0 stretch, 1 fit, 2 fill
    Surface();
    QTransform transform() const;
    bool valid() const;
    QPointF sample(double u, double v) const;
    void subdivide(int count);
    void resetMesh();
    QJsonObject json(const QString &baseDir = {}) const;
    static bool fromJson(const QJsonObject &, const QString &baseDir, Surface &out);
};

class Scene : public QObject {
    Q_OBJECT
public:
    explicit Scene(QObject *parent = nullptr);
    QVector<Surface> surfaces;
    int selected = -1;
    QSize outputSize{1920,1080};
    bool blackout = false;
    // One transient clock is shared by preview/output. Never saved or added to Undo.
    bool entranceActive=false,entrancePlaying=true;
    double entranceElapsed=0;
    void playEntrance();
    void skipEntrance();
    void setEntranceTime(double seconds);
    bool dirty = false;
    QString projectPath;
    PotatoDynamic::Settings dynamic;
    PotatoDynamic::Frame dynamicFrame;
    double dynamicElapsed = 0;
    QVector<PotatoDynamic::Surface> dynamicSurfaces;
    struct DynamicGeometry {
        QVector<float> vertices;
        QHash<int,QPair<int,int>> ranges;
        quint64 revision=0;
    };
    const DynamicGeometry &entranceGeometry(); // Cached mesh, four floats per vertex.
    const DynamicGeometry &dynamicGeometry();
    quint64 dynamicGeometryBuilds=0;
    void setDynamic(PotatoDynamic::Settings settings);
    void setDynamicTime(double seconds);
    void rebuildDynamic();
    bool dynamicSurface(const QString &id) const;
    Surface *current();
    MediaSource *source(const QString &path);
    void add();
    void newProject();
    void assignMedia(int index, const QString &path);
    void clearMedia(int index);
    void renameSurface(int index, const QString &name);
    void setAppearance(int index, int brightness, int opacity, bool recordUndo=true);
    void setBlend(int index, int blend);
    void resetAppearance(int index);
    void setPattern(int index, int pattern);
    void setFxLook(int index, FxLook look);
    void select(int index);
    void touch(bool structure = false);
    void checkpoint();
    void undo();
    void redo();
    QJsonObject json(const QString &base = {}) const;
    bool restore(const QJsonObject &, const QString &base, QString &error, bool restartDynamic=false);
    bool save(const QString &path, QString &error);
    bool load(const QString &path, QString &error);
signals:
    void changed();
    void structureChanged();
    void message(const QString &text);
private:
    QElapsedTimer animationClock;
    QHash<QString, MediaSource*> mediaSources;
    QVector<QJsonObject> history, future;
    DynamicGeometry cachedDynamicGeometry;
    DynamicGeometry cachedEntranceGeometry;
    quint64 mappingRevision=1;
    quint64 dynamicRevision=1;
    void updateDynamicFrame();
};
