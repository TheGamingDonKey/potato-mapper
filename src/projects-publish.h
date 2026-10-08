#pragma once
#include <QDir>
#include <QString>
#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace PotatoProjectsDetail {
// Publish only a newly prepared directory. Never replace an existing destination.
inline bool publishDirectory(const QString &source, const QString &destination, QString &error) {
    error.clear();
#ifdef Q_OS_WIN
    auto nativePath = [](const QString &path) {
        QString native = QDir::toNativeSeparators(QDir::cleanPath(QDir(path).absolutePath()));
        if (!native.startsWith("\\\\?\\"))
            native = native.startsWith("\\\\") ? "\\\\?\\UNC\\" + native.mid(2) : "\\\\?\\" + native;
        return native.toStdWString();
    };
    const auto from = nativePath(source);
    const auto to = nativePath(destination);
    DWORD nativeError = 0;
    // Newly written files can still be held briefly by a scanner or another process.
    // Give only locking/access failures a short retry window; never replace originals.
    for (int attempt = 0; attempt < 9; ++attempt) {
        if (MoveFileExW(from.c_str(), to.c_str(), 0)) return true;
        nativeError = GetLastError();
        if (nativeError != ERROR_ACCESS_DENIED && nativeError != ERROR_SHARING_VIOLATION
            && nativeError != ERROR_LOCK_VIOLATION) break;
        if (attempt < 8) Sleep(50);
    }
    error = "Cannot finish collected project: " + destination
        + " (Windows error " + QString::number(nativeError) + "). Original projects and media are retained.";
    return false;
#else
    if (QDir().rename(source, destination)) return true;
    error = "Cannot finish collected project: " + destination + ". Original projects and media are retained.";
    return false;
#endif
}
}
