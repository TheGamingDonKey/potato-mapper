#pragma once
#include <QObject>
#include <QImage>
#include <QMediaPlayer>
#include <QVideoSink>
#include <QAudioOutput>
#include <QPolygonF>
#include <QTransform>
#include <QJsonObject>
#include <QHash>
#include <QUuid>
#include <QElapsedTimer>

class MediaSource : public QObject {
    Q_OBJECT
public:
    explicit MediaSource(const QString &path, QObject *parent = nullptr);
    QImage image;
    quint64 revision = 1;
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
    int pattern = 0; // 0 media/alignment grid, 1 animated dots
    int patternSpeed = 100;
    int patternSize = 18;
    bool patternPlaying = true;
    double patternPhase = 0;
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
    bool dirty = false;
    QString projectPath;
    Surface *current();
    MediaSource *source(const QString &path);
    void add();
    void newProject();
    void assignMedia(int index, const QString &path);
    void select(int index);
    void touch(bool structure = false);
    void checkpoint();
    void undo();
    void redo();
    QJsonObject json(const QString &base = {}) const;
    bool restore(const QJsonObject &, const QString &base, QString &error);
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
};
