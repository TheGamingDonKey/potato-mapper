#include "projects.h"
#include "projects-publish.h"
#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QLockFile>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSet>
#include <QTemporaryDir>
#include <cmath>

namespace {
QString absolute(const QString &path) {
    const QFileInfo info(path);
    const QString canonical = info.canonicalFilePath();
    return QDir::cleanPath(canonical.isEmpty() ? info.absoluteFilePath() : canonical);
}
QString identity(const QString &path) {
    QString result = QDir::fromNativeSeparators(absolute(path));
#ifdef Q_OS_WIN
    result = result.toCaseFolded();
#endif
    return result;
}
bool inside(const QString &path, const QString &root) {
    return identity(path).startsWith(identity(root) + '/');
}
QString safeName(QString name) {
    name.replace(QRegularExpression("[<>:\"/\\\\|?*\\x00-\\x1f]"), "_");
    name = name.trimmed().left(100);
    while (name.endsWith('.') || name.endsWith(' ')) name.chop(1);
    if (name.isEmpty() || name == "." || name == "..") name = "Project";
    const QString device = name.section('.', 0, 0).toUpper();
    if (device == "CON" || device == "PRN" || device == "AUX" || device == "NUL"
        || QRegularExpression("^(COM|LPT)[1-9]$").match(device).hasMatch()) name.prepend('_');
    return name;
}
QString indexPath(const QString &projects) { return QDir(projects).filePath(".projects-index.json"); }
bool prepare(const QString &projects, QString &error) {
    if (QDir().mkpath(projects)) return true;
    error = "Cannot create project directory: " + projects;
    return false;
}
bool readIndex(const QString &projects, QJsonObject &index, QString &error) {
    const QString path = indexPath(projects);
    if (!QFileInfo::exists(path)) {
        index = {{"format", "PotatoProjects"}, {"version", 1}, {"imports", QJsonArray{}}, {"recent", QJsonArray{}}};
        return true;
    }
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) { error = "Cannot read project index " + path + ": " + file.errorString(); return false; }
    QJsonParseError parse;
    const auto doc = QJsonDocument::fromJson(file.readAll(), &parse);
    index = doc.object();
    if (parse.error != QJsonParseError::NoError || !doc.isObject() || index["format"] != "PotatoProjects"
        || index["version"].toDouble() != 1 || !index["imports"].isArray() || !index["recent"].isArray()) {
        error = "Invalid project index: " + path + ". Keep this file for recovery before repairing it.";
        return false;
    }
    return true;
}
bool writeAtomic(const QString &path, const QByteArray &bytes, QString &error) {
    QSaveFile file(path);
    file.setDirectWriteFallback(false);
    if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() || !file.commit()) {
        error = "Cannot save " + path + ": " + file.errorString(); return false;
    }
    return true;
}
bool writeIndex(const QString &projects, const QJsonObject &index, QString &error) {
    return writeAtomic(indexPath(projects), QJsonDocument(index).toJson(), error);
}
QString storedPath(const QString &path, const QString &projects) {
    return inside(path, projects) ? QDir(projects).relativeFilePath(absolute(path)) : absolute(path);
}
QString resolvedPath(const QString &path, const QString &projects) {
    return absolute(QDir::isAbsolutePath(path) ? path : QDir(projects).filePath(path));
}
void rememberInIndex(QJsonObject &index, const QString &path, const QString &projects) {
    QJsonArray paths;
    paths.append(storedPath(path, projects));
    for (const auto &old : index["recent"].toArray()) {
        if (!old.isString() || identity(resolvedPath(old.toString(), projects)) == identity(path)) continue;
        paths.append(old);
        if (paths.size() >= 12) break;
    }
    index["recent"] = paths;
}
bool pointsValid(const QJsonValue &value, int count) {
    if (!value.isArray() || value.toArray().size() != count) return false;
    for (const auto &point : value.toArray()) {
        const auto array = point.toArray();
        if (!point.isArray() || array.size() != 2 || !array[0].isDouble() || !array[1].isDouble()
            || !std::isfinite(array[0].toDouble()) || !std::isfinite(array[1].toDouble())) return false;
    }
    return true;
}
bool readProject(const QString &path, QByteArray &bytes, QJsonObject &project, QString &error) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) { error = "Cannot open project " + path + ": " + file.errorString(); return false; }
    bytes = file.readAll();
    if (file.error() != QFileDevice::NoError) { error = "Cannot read project " + path + ": " + file.errorString(); return false; }
    QJsonParseError parse;
    const auto doc = QJsonDocument::fromJson(bytes, &parse);
    if (parse.error != QJsonParseError::NoError || !doc.isObject()) {
        error = "Invalid project JSON in " + path + ": " + parse.errorString(); return false;
    }
    project = doc.object();
    if (!PotatoProjects::validateDocument(project, error)) { error += " Project: " + path; return false; }
    return true;
}
struct MediaFile {
    QString source, destination;
    qint64 size = 0, modified = 0;
};
bool findMedia(const QJsonObject &project, const QString &path, QList<MediaFile> &media, QString &error) {
    QSet<QString> seen;
    int number = 0;
    for (const auto &value : project["surfaces"].toArray()) {
        ++number;
        const QString reference = value.toObject()["media"].toString();
        if (reference.isEmpty()) continue;
        const QString resolved = absolute(QDir::isAbsolutePath(reference) ? reference : QDir(QFileInfo(path).absolutePath()).filePath(reference));
        QFile file(resolved);
        const QFileInfo info(resolved);
        if (!info.isFile() || !file.open(QIODevice::ReadOnly)) {
            error = "Cannot collect media for surface " + QString::number(number) + " in " + path + ": " + resolved
                + ". Restore the file or use Load media to locate it, then save the mapping.";
            return false;
        }
        if (seen.contains(identity(resolved))) continue;
        seen.insert(identity(resolved));
        media.append({resolved, {}, info.size(), info.lastModified().toMSecsSinceEpoch()});
    }
    return true;
}
QString sourceKey(const QString &path, const QString &installRoot) {
    return inside(path, installRoot) ? "install:" + QDir(absolute(installRoot)).relativeFilePath(identity(path)) : "external:" + identity(path);
}
bool selfContained(const QJsonObject &project, const QString &path, const QList<MediaFile> &media, const QString &projects) {
    const QString folder = QFileInfo(path).absolutePath();
    if (!inside(folder, projects)) return false;
    for (const auto &file : media) if (!inside(file.source, folder)) return false;
    for (const auto &value : project["surfaces"].toArray())
        if (QDir::isAbsolutePath(value.toObject()["media"].toString())) return false;
    return true;
}
QString fingerprint(const QByteArray &bytes, const QList<MediaFile> &media, const QString &source) {
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(bytes);
    for (const auto &file : media) {
        // Relative media identity stays stable when an entire portable installation moves.
        hash.addData(QByteArray(1, '\0') + QDir(QFileInfo(source).absolutePath()).relativeFilePath(identity(file.source)).toUtf8() + QByteArray(1, '\0')
            + QByteArray::number(file.size) + ':' + QByteArray::number(file.modified));
    }
    return QString::fromLatin1(hash.result().toHex());
}
bool lockIndex(QLockFile &lock, const QString &projects, QString &error) {
    if (lock.tryLock(2000)) return true;
    error = "Project storage is busy or unavailable: " + projects + ". Close another mapper using this folder and try again.";
    return false;
}
}

namespace PotatoProjects {
QString directory(const QString &installRoot) { return QDir(absolute(installRoot)).filePath("Projects"); }

bool validateDocument(const QJsonObject &project, QString &error) {
    error.clear();
    const QString format = project["format"].toString();
    if ((format != "PotatoMapper" && format != "HomeMapper") || project["version"].toDouble() != 1
        || !project["surfaces"].isArray()) {
        error = "Unsupported project format, version or surfaces."; return false;
    }
    const double width = project["width"].toDouble(), height = project["height"].toDouble();
    if (width < 1 || width > 16384 || height < 1 || height > 16384 || std::floor(width) != width || std::floor(height) != height) {
        error = "Invalid output size."; return false;
    }
    int number = 0;
    for (const auto &value : project["surfaces"].toArray()) {
        ++number;
        const auto surface = value.toObject();
        const double cellValue = surface["cells"].toDouble(2);
        const int cells = cellValue >= 1 && cellValue <= 16 ? int(cellValue) : 0;
        if (!value.isObject() || !cells || cellValue != cells || !pointsValid(surface["corners"], 4)
            || !pointsValid(surface["mesh"], (cells + 1) * (cells + 1))
            || (surface.contains("media") && !surface["media"].isString())) {
            error = "Invalid Surface " + QString::number(number) + "."; return false;
        }
    }
    return true;
}

QString collect(const QString &source, const QString &installRoot, QString &error) {
    error.clear();
    const QString original = absolute(source), projects = directory(installRoot);
    QByteArray bytes;
    QJsonObject project;
    QList<MediaFile> media;
    if (!readProject(original, bytes, project, error) || !findMedia(project, original, media, error)) return {};
    if (!prepare(projects, error)) return {};
    QLockFile lock(QDir(projects).filePath(".projects-index.lock"));
    if (!lockIndex(lock, projects, error)) return {};
    QJsonObject index;
    if (!readIndex(projects, index, error)) return {};
    // A project already contained with all its media can be opened without recollection.
    QString originalFolder = QFileInfo(original).absolutePath();
    if (selfContained(project, original, media, projects)) {
        rememberInIndex(index, original, projects);
        if (!writeIndex(projects, index, error)) return {};
        return original;
    }
    const QString key = sourceKey(original, installRoot), digest = fingerprint(bytes, media, original);
    auto imports = index["imports"].toArray();
    qsizetype replacementRecord = -1;
    for (qsizetype i = 0; i < imports.size(); ++i) {
        const auto item = imports[i].toObject();
        if (item["source"].toString() != key || item["fingerprint"].toString() != digest) continue;
        const QString existing = resolvedPath(item["project"].toString(), projects);
        if (!inside(existing, projects)) { error = "Imported project points outside Projects in " + indexPath(projects); return {}; }
        if (!QFileInfo::exists(existing)) continue;
        QByteArray existingBytes; QJsonObject existingProject; QList<MediaFile> existingMedia;
        if (!readProject(existing, existingBytes, existingProject, error) || !findMedia(existingProject, existing, existingMedia, error)) return {};
        // Retain edits and collect any media the user added outside the managed folder.
        if (selfContained(existingProject, existing, existingMedia, projects)) {
            rememberInIndex(index, existing, projects);
            if (!writeIndex(projects, index, error)) return {};
            return existing;
        }
        project = existingProject;
        media = existingMedia;
        originalFolder = QFileInfo(existing).absolutePath();
        replacementRecord = i;
        break;
    }
    QTemporaryDir staging(QDir(projects).filePath(".collect-XXXXXX"));
    if (!staging.isValid()) { error = "Cannot stage project in " + projects; return {}; }
    const QString baseName = safeName(QFileInfo(original).completeBaseName());
    QString name = baseName;
    for (int suffix = 2; QFileInfo::exists(QDir(projects).filePath(name)); ++suffix) name = baseName + '-' + QString::number(suffix);
    const QString folder = QDir(projects).filePath(name), destination = QDir(folder).filePath(name + ".pmap");
    if (!media.isEmpty() && !QDir(staging.path()).mkdir("Media")) { error = "Cannot create media folder in " + staging.path(); return {}; }
    QSet<QString> names;
    QHash<QString, QString> references;
    for (auto &file : media) {
        const QString fileName = safeName(QFileInfo(file.source).fileName());
        QString unique = fileName;
        for (int suffix = 2; names.contains(unique.toCaseFolded()); ++suffix) {
            const QFileInfo info(fileName);
            unique = info.completeBaseName() + '-' + QString::number(suffix)
                + (info.suffix().isEmpty() ? QString{} : '.' + info.suffix());
        }
        names.insert(unique.toCaseFolded());
        file.destination = "Media/" + unique;
        references.insert(identity(file.source), file.destination);
        QFile input(file.source);
        if (!input.copy(QDir(staging.path()).filePath(file.destination))) {
            error = "Cannot copy media " + file.source + " into " + folder + ": " + input.errorString(); return {};
        }
        const QFileInfo current(file.source);
        if (current.size() != file.size || current.lastModified().toMSecsSinceEpoch() != file.modified) {
            error = "Media changed during collection: " + file.source + ". Try opening the mapping again."; return {};
        }
    }
    auto surfaces = project["surfaces"].toArray();
    for (qsizetype i = 0; i < surfaces.size(); ++i) {
        auto surface = surfaces[i].toObject();
        const QString reference = surface["media"].toString();
        if (!reference.isEmpty()) {
            const QString path = QDir::isAbsolutePath(reference) ? reference : QDir(originalFolder).filePath(reference);
            surface["media"] = references.value(identity(path));
            surfaces[i] = surface;
        }
    }
    project["surfaces"] = surfaces;
    if (!writeAtomic(QDir(staging.path()).filePath(name + ".pmap"), QJsonDocument(project).toJson(), error)) return {};
    // Only the new staging folder is renamed; original projects/media remain untouched.
    if (!PotatoProjectsDetail::publishDirectory(staging.path(), folder, error)) return {};
    staging.setAutoRemove(false);
    const QJsonObject record{{"source", key}, {"fingerprint", digest}, {"project", QDir(projects).relativeFilePath(destination)}};
    if (replacementRecord >= 0) imports[replacementRecord] = record;
    else imports.append(record);
    index["imports"] = imports;
    rememberInIndex(index, destination, projects);
    if (!writeIndex(projects, index, error)) {
        // Roll back only the folder just created by this call, before it is exposed to a user.
        if (!QDir(folder).removeRecursively()) error += ". Unindexed collected copy remains at " + destination;
        return {};
    }
    return destination;
}

QStringList organizeLegacy(const QString &installRoot, QString &error) {
    error.clear();
    QStringList candidates, collected, failures;
    const QDir root(absolute(installRoot));
    auto mappings = [&candidates](const QDir &dir) {
        for (const auto &info : dir.entryInfoList(QDir::Files | QDir::NoSymLinks, QDir::Name)) {
            const QString suffix = info.suffix().toLower();
            if (suffix == "pmap" || suffix == "hmap") candidates.append(info.absoluteFilePath());
        }
    };
    mappings(root);
    const QDir versions(root.filePath("versions"));
    for (const auto &version : versions.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDir::Name))
        mappings(QDir(version.absoluteFilePath()));
    for (const auto &source : candidates) {
        QString failure;
        const QString path = collect(source, installRoot, failure);
        if (path.isEmpty()) failures.append(failure);
        else if (!collected.contains(path)) collected.append(path);
    }
    error = failures.join('\n');
    return collected;
}

bool backup(const QString &path, QString &error) {
    error.clear();
    if (!QFileInfo::exists(path)) return true;
    QFile source(path);
    if (!source.open(QIODevice::ReadOnly)) { error = "Cannot back up " + path + ": " + source.errorString(); return false; }
    QSaveFile copy(path + ".bak");
    copy.setDirectWriteFallback(false);
    if (!copy.open(QIODevice::WriteOnly)) { error = "Cannot create previous-save backup " + path + ".bak: " + copy.errorString(); return false; }
    while (!source.atEnd()) {
        const QByteArray chunk = source.read(1024 * 1024);
        if (source.error() != QFileDevice::NoError || copy.write(chunk) != chunk.size()) {
            error = "Cannot retain previous save for " + path + ": " + (source.error() != QFileDevice::NoError ? source.errorString() : copy.errorString()); return false;
        }
    }
    if (!copy.commit()) { error = "Cannot finish previous-save backup " + path + ".bak: " + copy.errorString(); return false; }
    return true;
}

QStringList recent(const QString &installRoot) {
    const QString projects = directory(installRoot);
    QString error;
    QJsonObject index;
    if (!readIndex(projects, index, error)) { qWarning().noquote() << error; return {}; }
    QStringList paths;
    for (const auto &value : index["recent"].toArray()) {
        if (!value.isString()) continue;
        const QString path = resolvedPath(value.toString(), projects);
        if (QFileInfo(path).isFile() && !paths.contains(path)) paths.append(path);
    }
    return paths;
}

void remember(const QString &installRoot, const QString &path) {
    const QString projects = directory(installRoot);
    QString error;
    if (!QFileInfo(path).isFile()) return;
    if (!prepare(projects, error)) { qWarning().noquote() << error; return; }
    QLockFile lock(QDir(projects).filePath(".projects-index.lock"));
    if (!lockIndex(lock, projects, error)) { qWarning().noquote() << error; return; }
    QJsonObject index;
    if (!readIndex(projects, index, error)) { qWarning().noquote() << error; return; }
    rememberInIndex(index, absolute(path), projects);
    if (!writeIndex(projects, index, error)) qWarning().noquote() << error;
}
}
