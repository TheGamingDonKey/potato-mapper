#pragma once

#include <QString>
#include <QStringList>
#include <QJsonObject>

namespace PotatoProjects {
QString directory(const QString &installRoot);
// Validates document structure without checking media availability or mutating a scene.
bool validateDocument(const QJsonObject &project, QString &error);
// Returns an independently portable copy, or an empty path with an actionable error.
QString collect(const QString &source, const QString &installRoot, QString &error);
// Copies only mappings directly in the installation root and immediate version folders.
QStringList organizeLegacy(const QString &installRoot, QString &error);
// Atomically retains the existing file at path + ".bak"; a new file needs no backup.
bool backup(const QString &path, QString &error);
QStringList recent(const QString &installRoot);
void remember(const QString &installRoot, const QString &path);
}
