#pragma once
#include "../src/canvas.h"
#include "harvest-check.h"
#include <QAction>
#include <QCheckBox>
#include <QEventLoop>
#include <QFile>
#include <QJsonDocument>

namespace PotatoCutoutCheck {
inline Surface panel(int pattern, double left = 0, double right = 1) {
    Surface surface;
    surface.corners = {{left, 0}, {right, 0}, {right, 1}, {left, 1}};
    surface.subdivide(1);
    surface.pattern = pattern;
    surface.patternPlaying = false;
    surface.patternSize = potatoPatternDefaultSize(pattern);
    surface.fx.density = potatoPatternDensity(pattern);
    surface.fx.flow = potatoPatternDefaultFlow(pattern);
    surface.fx.palette = potatoPatternPalette(pattern);
    surface.subjectOnly = pattern >= 23 && pattern <= 26;
    return surface;
}

inline QImage projection(Canvas &output, const QSize &size = {320, 180}) {
    const QImage raw = output.grabFramebuffer();
    if (raw.isNull()) return {};
    const QRectF stage = output.canvasRect();
    const double sx = double(raw.width()) / output.width();
    const double sy = double(raw.height()) / output.height();
    const QRect pixels(qRound(stage.x() * sx), qRound(stage.y() * sy),
                       qRound(stage.width() * sx), qRound(stage.height() * sy));
    const auto image = pixels == raw.rect() ? raw : raw.copy(pixels);
    return image.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation).convertToFormat(QImage::Format_RGB32);
}

inline int brightest(QRgb pixel) { return std::max({qRed(pixel), qGreen(pixel), qBlue(pixel)}); }
inline int distance(QRgb a, QRgb b) {
    return std::max({std::abs(qRed(a) - qRed(b)), std::abs(qGreen(a) - qGreen(b)), std::abs(qBlue(a) - qBlue(b))});
}
struct Coverage { int dark = 0, black = 0, lit = 0; double centerX = 0; };
inline Coverage coverage(const QImage &image) {
    Coverage result;
    for (int y = 0; y < image.height(); ++y) {
        const auto *row = reinterpret_cast<const QRgb *>(image.constScanLine(y));
        for (int x = 0; x < image.width(); ++x) {
            const int value = brightest(row[x]);
            if (value == 0) ++result.black;
            if (value <= 3) ++result.dark;
            if (value > 30) { ++result.lit; result.centerX += x + .5; }
        }
    }
    if (result.lit) result.centerX /= double(result.lit) * image.width();
    return result;
}
// Score on the illuminated artwork so a small duplicated or missing subject
// cannot disappear into a comparison dominated by black background pixels.
inline double foregroundMismatch(const QImage &a, const QImage &b) {
    if (a.isNull() || a.size() != b.size()) return 1;
    int lit = 0, mismatched = 0;
    for (int y = 0; y < a.height(); ++y) {
        const auto *pa = reinterpret_cast<const QRgb *>(a.constScanLine(y));
        const auto *pb = reinterpret_cast<const QRgb *>(b.constScanLine(y));
        for (int x = 0; x < a.width(); ++x) {
            if (std::max(brightest(pa[x]), brightest(pb[x])) <= 30) continue;
            ++lit;
            if (distance(pa[x], pb[x]) > 18) ++mismatched;
        }
    }
    return lit ? double(mismatched) / lit : 1;
}
inline bool retainsUnderlayer(const QImage &solo, const QImage &layered, QRgb background) {
    if (solo.isNull() || solo.size() != layered.size()) return false;
    int dark = 0, restored = 0, lit = 0, painted = 0;
    for (int y = 0; y < solo.height(); ++y) {
        const auto *a = reinterpret_cast<const QRgb *>(solo.constScanLine(y));
        const auto *b = reinterpret_cast<const QRgb *>(layered.constScanLine(y));
        for (int x = 0; x < solo.width(); ++x) {
            if (brightest(a[x]) <= 3) { ++dark; if (distance(b[x], background) <= 5) ++restored; }
            if (brightest(a[x]) > 40) { ++lit; if (distance(b[x], background) > 16) ++painted; }
        }
    }
    return dark > 0 && lit > 0 && restored > .91 * dark && painted > .6 * lit;
}
}

template<class Window> void runCutoutChecks(Window &window, QApplication &app, const QStringList &args) {
    using namespace PotatoCutoutCheck;
    window.output->resize(args.contains("--cutout-frames") ? QSize(1280,720) : QSize(960,540));
    window.output->show();
    QTimer::singleShot(650, &window, [&window, &app, args] {
        auto &scene = window.scene;
        int checks = 0;
        QStringList failures;
        auto require = [&](bool pass, const QString &message) { ++checks; if (!pass) failures << message; };
        auto finish = [&] {
            qInfo().noquote() << "CUTOUT CHECK" << (failures.isEmpty() ? "PASS" : "FAIL") << checks << "checks" << failures.join("; ");
            app.exit(failures.isEmpty() ? 0 : 2);
        };
        auto option = [&](const QString &key) {
            const int i = args.indexOf(key);
            if (i < 0) return QString();
            if (i + 1 < args.size() && !args[i + 1].startsWith("--")) return args[i + 1];
            require(false, key + " requires a path"); return QString();
        };
        const QString frames = option("--cutout-frames"), still = option("--cutout-still");
        QTemporaryDir temp;
        require(temp.isValid(), "temporary fixture directory");
        require(window.output->graphicsReady && window.output->graphicsError.isEmpty(), "output OpenGL: " + window.output->graphicsError);
        require(window.canvas->graphicsReady && window.canvas->graphicsError.isEmpty(), "editor OpenGL: " + window.canvas->graphicsError);
        if (!failures.isEmpty()) { finish(); return; }
        scene.newProject();
        scene.outputSize = {960, 540};
        auto install = [&](const QVector<Surface> &surfaces) {
            scene.surfaces = surfaces;
            scene.selected = -1;
            scene.touch(true);
        };
        auto frame = [&] { return projection(*window.output); };
        auto setTime = [&](double phase) { for (auto &s : scene.surfaces) if (s.pattern) s.patternPhase = phase; };
        Surface backing = panel(0);
        backing.media = temp.filePath("underlayer.png");
        QImage solid(16, 16, QImage::Format_RGB32);
        solid.fill(QColor(31, 151, 209));
        require(solid.save(backing.media), "create synthetic opaque underlayer");
        install({backing});
        const auto baseline = frame();
        if (baseline.isNull()) { require(false, "projector framebuffer is empty"); finish(); return; }
        const QRgb base = baseline.pixel(baseline.width() / 2, baseline.height() / 2);
        require(distance(base, solid.pixel(0, 0)) <= 5, "underlayer really renders its test colour");
        const double cover[]{8., 13.25, 8., 3., 12.};
        QVector<Surface> presets;
        for (int id = 23; id <= 27; ++id) {
            auto subject = panel(id);
            subject.patternPhase = cover[id - 23];
            presets.append(subject);
            const QString label = QString("pattern %1: ").arg(id);
            install({subject});
            const auto solo = frame();
            const auto stats = coverage(solo);
            const int count = solo.width() * solo.height();
            require(stats.black > count * .27 && stats.lit > count * .003, label + "nonempty artwork surrounded by exact RGB black");
            scene.surfaces[0].patternSize = 34;
            scene.touch();
            require(foregroundMismatch(solo, frame()) > .025, label + "scene size changes the projected artwork");
            if (id < 27) {
                scene.surfaces[0] = subject;
                scene.surfaces[0].subjectOnly = false;
                require(stats.dark - coverage(frame()).dark > .15 * count, label + "full scene retains its background");
            }
            install({backing, subject});
            require(retainsUnderlayer(solo, frame(), base), label + "Normal blend preserves the coloured layer around the subject");
            const auto editor = PotatoHarvestCheck::samples(window.canvas->grabFramebuffer(), window.canvas->canvasRect(), window.canvas->size(), subject);
            const auto output = PotatoHarvestCheck::samples(window.output->grabFramebuffer(), window.output->canvasRect(), window.output->size(), subject);
            require(PotatoHarvestCheck::difference(editor, output) < 8, label + "editor and projector agree inside the surface");
        }

        // Joining a flight midway must use its existing time, including when
        // the new surface is selected in the inspector when the preset is applied.
        Surface left = panel(27, 0, .5), right = panel(27, .5, 1);
        left.patternPhase = 9.25;
        install({left, panel(0, .5, 1)});
        scene.select(1);
        scene.setPattern(1, 27);
        require(std::abs(scene.surfaces[0].patternPhase - 9.25) < .001 &&
                std::abs(scene.surfaces[1].patternPhase - 9.25) < .001, "a later Santa joins the existing flight without rewinding it");
        const auto joined = frame();
        install({panel(27)});
        setTime(9.25);
        require(foregroundMismatch(joined, frame()) < .055, "adding Santa through the picker gives one shared subject");

        QImage middle;
        double firstX = 0, lastX = 0;
        for (double t : {8., 12., 16.}) {
            install({panel(27)}); setTime(t);
            const auto single = frame();
            const auto stats = coverage(single);
            require(stats.lit > 100, QString("Santa visible at %1 seconds").arg(t));
            if (t == 8.) firstX = stats.centerX;
            if (t == 16.) lastX = stats.centerX;
            if (t == 12.) middle = single;
            setTime(t - 24.);
            require(foregroundMismatch(single, frame()) < .02, "24-second Santa timeline repeats in negative time");
            install({left, right}); setTime(t);
            require(foregroundMismatch(single, frame()) < .055, QString("one continuous subject across touching panels at %1 seconds").arg(t));
        }
        require(std::abs(lastX - firstX) > .15, "Santa crosses the projection horizontally");
        const int mid = middle.width() / 2;
        require(coverage(middle.copy(0, 0, mid, middle.height())).lit > 35 &&
                coverage(middle.copy(mid, 0, mid, middle.height())).lit > 35, "the subject crosses the shared edge at mid-flight");
        const QRect gap(qRound(middle.width() * .46), 0, qRound(middle.width() * .08), middle.height());
        require(coverage(middle.copy(gap)).lit > 15, "the uninterrupted subject illuminates the test gap");
        install({panel(27, 0, .45), panel(27, .55, 1)}); setTime(12.);
        const auto throughGap = frame();
        require(coverage(throughGap.copy(gap)).black == gap.width() * gap.height(), "a physical gap between the panels stays exactly black");
        require(coverage(throughGap).lit > 75, "the portions on either side of the gap remain visible");

        auto upper=panel(27,.05,.45),lower=panel(27,.55,.95);
        auto setHeight=[](Surface &s,double top,double bottom){
            s.corners[0].setY(top);s.corners[1].setY(top);
            s.corners[2].setY(bottom);s.corners[3].setY(bottom);
        };
        setHeight(upper,.03,.40);setHeight(lower,.60,.97);
        install({upper,lower});setTime(9.);
        auto staggered=frame();
        const QRect topHalf(0,0,staggered.width(),staggered.height()/2);
        const QRect bottomHalf=topHalf.translated(0,topHalf.height());
        require(coverage(staggered.copy(topHalf)).lit>120,"flight visits the raised window");
        setTime(15.);staggered=frame();
        require(coverage(staggered.copy(bottomHalf)).lit>120,"same flight descends into the lower window");
        upper=panel(27,.25,.75);lower=upper;
        setHeight(upper,.02,.44);setHeight(lower,.56,.98);
        install({upper,lower});setTime(9.);
        require(coverage(frame().copy(topHalf)).lit>120,"stacked route visits the upper panel");
        setTime(15.);
        require(coverage(frame().copy(bottomHalf)).lit>120,"stacked route visits the lower panel");

        // Moving a narrow mapping must move its own flight bounds. A hidden
        // far-away surface must not widen those bounds or shift the subject.
        auto near = panel(27, .0625, .4375), far = panel(27, .5625, .9375);
        install({near}); setTime(12.); const auto narrow = frame();
        far.visible = false;
        install({near, far}); setTime(12.);
        require(foregroundMismatch(narrow, frame()) < .035, "hidden Santa does not change the visible route");
        far.visible = true;
        install({far}); setTime(12.);
        require(foregroundMismatch(narrow.copy(0, 0, mid, narrow.height()), frame().copy(mid, 0, mid, narrow.height())) < .055,
                "the same narrow flight translates with its mapped region");

        install(presets);
        for (auto &s : scene.surfaces) { s.patternPlaying = true; s.fx.reverse = true; }
        setTime(12.);
        scene.dirty = false;
        const auto playing = scene.json();
        auto tick = [] { QEventLoop wait; QTimer::singleShot(140, &wait, &QEventLoop::quit); wait.exec(); };
        tick();
        for (const auto &s : scene.surfaces) require(s.patternPhase < 12., QString("pattern %1 reverses").arg(s.pattern));
        require(!scene.dirty && scene.json() == playing, "playback leaves saved controls and dirty state alone");
        for (auto &s : scene.surfaces) s.patternPlaying = false;
        const auto stopped = frame();
        QVector<double> phases;
        for (const auto &s : scene.surfaces) phases.append(s.patternPhase);
        tick();
        require(stopped == frame(), "paused cutouts keep their projected pixels");
        for (int i = 0; i < phases.size(); ++i) require(scene.surfaces[i].patternPhase == phases[i], "paused phase remains fixed");

        QString error;
        const auto saved = scene.json();
        const auto project = temp.filePath("cutouts.pmap");
        const bool reopened = scene.save(project, error) && scene.load(project, error);
        require(reopened && scene.json() == saved, "cutout presets and controls save and reopen: " + error);
        if (reopened && scene.surfaces.size() == 5) {
            for (int i = 0; i < 4; ++i) require(scene.surfaces[i].subjectOnly, "subject-only choice survives reopen");
            require(scene.surfaces.last().pattern == 27, "Santa keeps its stable ID");
        }
        auto legacy = saved;
        legacy.insert("version", 2);
        QJsonArray surfaces;
        for (const auto &entry : saved.value("surfaces").toArray()) {
            auto item = entry.toObject();
            if (item.value("pattern").toInt() == 27) continue;
            item.remove("subjectOnly"); surfaces.append(item);
        }
        legacy.insert("surfaces", surfaces);
        QFile old(temp.filePath("earlier-project.pmap"));
        const auto bytes = QJsonDocument(legacy).toJson();
        const bool written = old.open(QIODevice::WriteOnly) && old.write(bytes) == bytes.size();
        old.close();
        const bool oldLoaded = written && scene.load(old.fileName(), error);
        require(oldLoaded && scene.surfaces.size() == 4, "open an earlier Christmas mapping without cutout fields: " + error);
        if (oldLoaded) for (const auto &s : scene.surfaces) require(!s.subjectOnly, "earlier scene retains its original full background");
        if (oldLoaded) {
            scene.select(0);
            auto *toggle=window.template findChild<QCheckBox*>("subjectOnly");
            require(toggle && !toggle->isHidden() && !toggle->isChecked(),"Subject only is available on a full-scene Christmas surface");
            if(toggle){
                toggle->click();
                require(scene.current() && scene.current()->subjectOnly && scene.json()["version"].toInt()==4,"the actual inspector switch enables the cutout");
                toggle->click();
                require(scene.current() && !scene.current()->subjectOnly,"the inspector restores the scenery");
            }
        }
        const auto actions=window.template findChildren<QAction*>();
        require(std::any_of(actions.cbegin(),actions.cend(),[](QAction *a){return a->text()=="Santa's Sleigh Ride";}),"Santa is reachable from File > Examples");
        require(QFile::exists(":/assets/thumbnails/pattern-27.png"),"native Santa thumbnail is built in");

        if (failures.isEmpty() && (!frames.isEmpty() || !still.isEmpty())) {
            install({left, right}); setTime(12.);
            if (!still.isEmpty()) {
                require(QDir().mkpath(QFileInfo(still).absolutePath()) && projection(*window.output, {960, 540}).save(still), "save native sleigh still " + still);
            }
            if (!frames.isEmpty()) {
                bool captured = QDir().mkpath(frames);
                constexpr int frameRate=24;
                for (int i = 0; i < 24*frameRate && captured; ++i) {
                    setTime(double(i) / frameRate);
                    const auto file = QDir(frames).filePath(QString("%1.png").arg(i, 3, 10, QChar('0')));
                    captured = projection(*window.output, {1280, 720}).save(file);
                }
                require(captured, "write the 24-second 720p sleigh flight at 24 fps to " + frames);
            }
        }
        finish();
    });
}
