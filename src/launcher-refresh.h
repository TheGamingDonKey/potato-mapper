#pragma once
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>

// Run once at editor startup. The independent native helper can wait for the
// launcher that opened this editor to release its root executable.
inline bool requestPotatoLauncherRefresh(const QString &installRoot,QString *error=nullptr) {
    if(error)error->clear();
#ifdef Q_OS_WIN
    auto fail=[&](const QString &message){if(error)*error=message;return false;};
    const QDir root(installRoot),runtime(QCoreApplication::applicationDirPath());
    QFile marker(root.filePath("installation.txt"));
    if(!marker.open(QIODevice::ReadOnly)||marker.readAll()!="PotatoMapper/1\n")return true;
    QFile pointer(root.filePath("current.txt"));
    if(!pointer.open(QIODevice::ReadOnly))return true;
    const auto version=QString::fromUtf8(pointer.readAll()).trimmed();
    if(!QRegularExpression("^[0-9]{1,5}\\.[0-9]{1,5}\\.[0-9]{1,5}$").match(version).hasMatch())return true;
    // A directly opened retained editor must not replace the selected release's launcher.
    if(runtime.canonicalPath().compare(QDir(root.filePath("versions/"+version)).canonicalPath(),Qt::CaseInsensitive)!=0)return true;
    const auto source=runtime.filePath("launcher/PotatoMapper.exe"),helper=runtime.filePath("PotatoLauncherRefresh.exe");
    const auto destination=root.filePath("PotatoMapper.exe");
    if(!QFileInfo::exists(source)&&!QFileInfo::exists(helper))return true; // old payload
    auto hash=[](const QString &path){QFile file(path);QCryptographicHash digest(QCryptographicHash::Sha256);if(!file.open(QIODevice::ReadOnly)||!digest.addData(&file))return QByteArray();return digest.result().toHex();};
    QFile expected(runtime.filePath("launcher/launcher-sha256.txt"));
    const auto sourceHash=hash(source);
    if(sourceHash.isEmpty()||!expected.open(QIODevice::ReadOnly)||expected.readAll().trimmed()!=sourceHash)
        return fail("Launcher maintenance could not verify the packaged launcher. Extract a complete release ZIP.");
    if(sourceHash==hash(destination))return true;
    if(!QFileInfo(helper).isFile()||QFileInfo(helper).isSymLink())return fail("The launcher refresh helper is missing. Extract a complete release ZIP.");
    if(!QProcess::startDetached(helper,{"--install-root",root.absolutePath(),"--version",version},root.absolutePath()))
        return fail("Windows could not start launcher maintenance. Check folder permissions. Your editor remains open.");
#else
    Q_UNUSED(installRoot);
#endif
    return true;
}
