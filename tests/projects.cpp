#include "projects.h"
#include "projects-publish.h"
#include <QCoreApplication>
#include <QDir>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <chrono>

static void check(bool pass, const char *label) {
    std::cout << (pass ? "PASS " : "FAIL ") << label << std::endl;
    if (!pass) throw std::runtime_error(label);
}
static void write(const QString &path, const QByteArray &data) {
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly) || file.write(data) != data.size())
        throw std::runtime_error("Could not write temporary fixture");
}
static QByteArray bytes(const QString &path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return {};
    return file.readAll();
}
static QJsonObject mapping(const QStringList &media = {}) {
    QJsonArray surfaces;
    for (const auto &path : media) {
        surfaces.append(QJsonObject{{"name", "Fixture"}, {"media", path}, {"cells", 1},
            {"corners", QJsonArray{QJsonArray{0,0}, QJsonArray{1,0}, QJsonArray{1,1}, QJsonArray{0,1}}},
            {"mesh", QJsonArray{QJsonArray{0,0}, QJsonArray{1,0}, QJsonArray{0,1}, QJsonArray{1,1}}}});
    }
    return {{"format", "PotatoMapper"}, {"version", 1}, {"width", 1920}, {"height", 1080}, {"surfaces", surfaces}};
}
static void writeMapping(const QString &path, const QJsonObject &object) {
    write(path, QJsonDocument(object).toJson());
}
int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    try {
        QTemporaryDir fixture;
        check(fixture.isValid(), "temporary fixture root");
#ifdef Q_OS_WIN
        if (!app.arguments().contains("--recollection-only")) {
            const auto staged = fixture.filePath("locked-staging");
            const auto published = fixture.filePath("published");
            write(QDir(staged).filePath("saved.pmap"), "retained bytes");
            const auto native = QDir::toNativeSeparators(staged).toStdWString();
            HANDLE handle = CreateFileW(native.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
            check(handle != INVALID_HANDLE_VALUE, "temporary directory lock acquired");
            std::thread unlock([handle] { std::this_thread::sleep_for(std::chrono::milliseconds(120)); CloseHandle(handle); });
            QString publishError;
            const bool publishedOk = PotatoProjectsDetail::publishDirectory(staged, published, publishError);
            unlock.join();
            check(publishedOk && bytes(QDir(published).filePath("saved.pmap")) == "retained bytes",
                "temporary Windows directory lock is retried without losing saved bytes");
            QDir().mkpath(staged);
            write(QDir(staged).filePath("saved.pmap"), "new staged bytes");
            check(!PotatoProjectsDetail::publishDirectory(staged, published, publishError)
                && bytes(QDir(staged).filePath("saved.pmap")) == "new staged bytes"
                && bytes(QDir(published).filePath("saved.pmap")) == "retained bytes",
                "publication never replaces an existing project directory");
            handle = CreateFileW(native.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
            check(handle != INVALID_HANDLE_VALUE, "persistent directory lock acquired");
            const bool blocked = !PotatoProjectsDetail::publishDirectory(staged, fixture.filePath("blocked-publication"), publishError);
            CloseHandle(handle);
            check(blocked && publishError.contains("Windows error")
                && bytes(QDir(staged).filePath("saved.pmap")) == "new staged bytes"
                && !QFileInfo::exists(fixture.filePath("blocked-publication")),
                "persistent lock fails visibly with staged and original bytes intact");
            QString deep = fixture.path();
            for (int i = 0; i < 4; ++i) deep += '/' + QString(70, QChar('a' + i));
            const auto deepSource = QDir(deep).filePath("staged");
            const auto deepDestination = QDir(deep).filePath("published");
            write(QDir(deepSource).filePath("saved.pmap"), "long path bytes");
            check(deepSource.size() > 260 && PotatoProjectsDetail::publishDirectory(deepSource, deepDestination, publishError)
                && bytes(QDir(deepDestination).filePath("saved.pmap")) == "long path bytes",
                "publication preserves Qt long-path support beyond MAX_PATH");
        }
#endif
        const QString root = fixture.filePath("install"), source = fixture.filePath("source/show.pmap");
        const QString firstMedia = fixture.filePath("source/a/clip.bin"), secondMedia = fixture.filePath("source/b/clip.bin");
        write(firstMedia, "first clip"); write(secondMedia, "second clip");
        const auto original = mapping({"a/clip.bin", "b/clip.bin", "a/clip.bin"});
        writeMapping(source, original);
        const auto originalBytes = bytes(source);
        QString error;
        const QString collected = PotatoProjects::collect(source, root, error);
        check(!collected.isEmpty() && error.isEmpty(), "collect succeeds");
        check(bytes(source) == originalBytes && bytes(firstMedia) == "first clip", "original mapping and media retained");
        const auto surfaces = QJsonDocument::fromJson(bytes(collected)).object()["surfaces"].toArray();
        const QString pathA = surfaces[0].toObject()["media"].toString(), pathB = surfaces[1].toObject()["media"].toString();
        check(!QDir::isAbsolutePath(pathA) && pathA != pathB && pathA == surfaces[2].toObject()["media"].toString(), "relative media paths disambiguate equal basenames and share duplicate references");
        const QString projectDir = QFileInfo(collected).absolutePath();
        check(bytes(QDir(projectDir).filePath(pathA)) == "first clip" && bytes(QDir(projectDir).filePath(pathB)) == "second clip", "copied media bytes retained");
        check(PotatoProjects::collect(source, root, error) == collected, "repeated source uses existing import");
        auto edited = QJsonDocument::fromJson(bytes(collected)).object(); edited["width"] = 1280;
        writeMapping(collected, edited); const auto editedBytes = bytes(collected);
        check(PotatoProjects::collect(source, root, error) == collected && bytes(collected) == editedBytes, "repeated import preserves edits in collected copy");
        auto externalEdit = edited;
        auto externalSurfaces = externalEdit["surfaces"].toArray();
        auto externalSurface = externalSurfaces[0].toObject(); externalSurface["media"] = firstMedia;
        externalSurfaces[0] = externalSurface; externalEdit["surfaces"] = externalSurfaces;
        writeMapping(collected, externalEdit); const auto externalBytes = bytes(collected);
        const auto indexBefore = bytes(QDir(root).filePath("Projects/.projects-index.json"));
        const QString recollected = PotatoProjects::collect(source, root, error);
        const auto returnedSurfaces = QJsonDocument::fromJson(bytes(recollected)).object()["surfaces"].toArray();
        const QString returnedMedia = returnedSurfaces.isEmpty() ? QString() : returnedSurfaces[0].toObject()["media"].toString();
        const bool returned=!recollected.isEmpty(),different=recollected!=collected,retained=bytes(collected)==externalBytes;
        const bool relative=!returnedMedia.isEmpty()&&!QDir::isAbsolutePath(returnedMedia);
        const bool good=returned&&different&&retained&&relative;
        if(!good||app.arguments().contains("--recollection-only")){
            std::cout<<"RECOLLECT returned="<<returned<<" different="<<different<<" original-retained="<<retained<<" relative-media="<<relative
                <<" error="<<error.toStdString()<<" source="<<source.toStdString()<<" edited="<<collected.toStdString()<<" result="<<recollected.toStdString()<<" reference="<<returnedMedia.toStdString()<<std::endl;
        }
        if(!good){
            fixture.setAutoRemove(false);
            std::cout<<"RECOLLECT FIXTURE RETAINED "<<fixture.path().toStdString()<<std::endl;
            for(const auto &path:QStringList{source,collected,recollected,firstMedia,secondMedia}){const QFileInfo info(path);
                std::cout<<"META "<<path.toStdString()<<" exists="<<info.exists()<<" size="<<info.size()<<" modified-ms="<<info.lastModified().toMSecsSinceEpoch()<<std::endl;}
            std::cout<<"INDEX BEFORE "<<indexBefore.toStdString()<<"\nINDEX AFTER "<<bytes(QDir(root).filePath("Projects/.projects-index.json")).toStdString()<<std::endl;
            if(returned)std::cout<<"RETURNED DOCUMENT "<<bytes(recollected).toStdString()<<std::endl;
        }
        check(good,
            "new external media in an edited import is collected without overwriting edits");
        check(PotatoProjects::collect(source, root, error) == recollected, "recollected edits are also deduplicated");
        if(app.arguments().contains("--recollection-only"))return 0;
        write(collected, editedBytes);
        auto changed = original; changed["width"] = 640; writeMapping(source, changed);
        const QString updated = PotatoProjects::collect(source, root, error);
        check(!updated.isEmpty() && updated != collected && bytes(collected) == editedBytes, "changed source creates new copy without overwriting edits");
        check(PotatoProjects::collect(source, root, error) == updated, "changed source is also deduplicated");
        // Renaming only fixture-owned originals proves imports no longer depend on them.
        check(QDir().rename(fixture.filePath("source"), fixture.filePath("source-unavailable")), "hide fixture original directory");
        check(PotatoProjects::collect(collected, root, error) == collected && bytes(QDir(projectDir).filePath(pathB)) == "second clip", "managed copy reopens independently");
        const QString movedRoot = fixture.filePath("moved-install");
        check(QDir().rename(root, movedRoot), "move fixture portable installation");
        const QString movedProject = QDir(movedRoot).filePath(QDir(root).relativeFilePath(collected));
        check(PotatoProjects::recent(movedRoot).contains(movedProject), "recent projects follow portable installation relocation");

        const QString invalid = fixture.filePath("invalid.pmap");
        auto malformed = mapping(); malformed["surfaces"] = "bad"; writeMapping(invalid, malformed);
        check(PotatoProjects::collect(invalid, movedRoot, error).isEmpty() && error.contains(invalid), "invalid project is rejected with source path");
        writeMapping(invalid, mapping({"missing.bin"}));
        check(PotatoProjects::collect(invalid, movedRoot, error).isEmpty() && error.contains("missing.bin"), "missing media fails visibly");
        auto badSurface = mapping({}); badSurface["surfaces"] = QJsonArray{QJsonObject{{"corners", QJsonArray{}}}};
        writeMapping(invalid, badSurface);
        check(PotatoProjects::collect(invalid, movedRoot, error).isEmpty() && error.contains("surface", Qt::CaseInsensitive), "malformed surface is rejected");

        const QString legacyRoot = fixture.filePath("legacy");
        write(QDir(legacyRoot).filePath("media/legacy.bin"), "legacy media");
        auto older = mapping({"media/legacy.bin"}); older["format"] = "HomeMapper";
        writeMapping(QDir(legacyRoot).filePath("old.hmap"), older);
        writeMapping(QDir(legacyRoot).filePath("versions/0.7.0/version.pmap"), mapping());
        writeMapping(QDir(legacyRoot).filePath("versions/0.7.0/tests/ignored.pmap"), mapping());
        writeMapping(QDir(legacyRoot).filePath("developer/ignored.pmap"), mapping());
        const auto imported = PotatoProjects::organizeLegacy(legacyRoot, error);
        check(imported.size() == 2 && error.isEmpty(), "legacy scan imports only root and immediate runtime mappings");
        check(PotatoProjects::organizeLegacy(legacyRoot, error) == imported, "legacy organization does not repeat imports");
        const QString saved = imported.first(); const auto before = bytes(saved);
        check(PotatoProjects::backup(saved, error) && bytes(saved + ".bak") == before, "backup retains exact previous saved bytes");
        write(saved, "later save");
        check(PotatoProjects::backup(saved, error) && bytes(saved + ".bak") == "later save", "backup atomically refreshes previous-save copy");
        check(PotatoProjects::backup(QDir(legacyRoot).filePath("new.pmap"), error), "first save requires no backup");
        PotatoProjects::remember(legacyRoot, imported.last()); PotatoProjects::remember(legacyRoot, imported.last());
        const auto recent = PotatoProjects::recent(legacyRoot);
        check(recent.first() == imported.last() && recent.count(imported.last()) == 1, "recent order is newest first without duplicates");
        write(saved, before);
        const QString relocatedLegacy = fixture.filePath("relocated-legacy");
        check(QDir().rename(legacyRoot, relocatedLegacy), "relocate fixture legacy installation");
        const auto reorganized = PotatoProjects::organizeLegacy(relocatedLegacy, error);
        check(reorganized.size() == 2 && error.isEmpty()
            && reorganized.first() == QDir(relocatedLegacy).filePath(QDir(legacyRoot).relativeFilePath(imported.first())),
            "relocated legacy installation reuses previous import with relative media");
        std::cout << "Project storage checks passed" << std::endl;
        return 0;
    } catch (const std::exception &exception) {
        std::cerr << exception.what() << std::endl; return 1;
    }
}
