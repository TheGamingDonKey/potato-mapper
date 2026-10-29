#pragma once

#include <QDir>
#include <QStandardPaths>
#include <QString>

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <appmodel.h>
#ifdef small
#undef small
#endif
#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif
#endif

// Shared by the same editor in both distributions. Do not use an executable
// location or a writable package directory as a substitute for package identity.
namespace PotatoDistribution {
inline bool isStorePackage() {
#if defined(POTATO_STORE_BUILD) && POTATO_STORE_BUILD
    return true;
#elif defined(Q_OS_WIN)
    UINT32 length = 0;
    return GetCurrentPackageFullName(&length, nullptr) == ERROR_INSUFFICIENT_BUFFER;
#else
    return false;
#endif
}

inline QString projectLibraryRoot() {
    // An empty result deliberately leaves the portable install-root/Projects
    // policy with the existing caller, including --install-root handling.
    if (!isStorePackage()) return {};
    QString documents = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if (documents.isEmpty()) documents = QDir::home().filePath("Documents");
    return QDir(documents).filePath("Potato Mapper Projects");
}

inline QString channelName() {
    return isStorePackage() ? QStringLiteral("Microsoft Store") : QStringLiteral("Direct download");
}
}
