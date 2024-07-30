/*
    SPDX-FileCopyrightText: 2015 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "helper.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QStandardPaths>

using namespace Purpose;

QJsonObject Purpose::readPluginType(const QString &pluginType)
{
    const QString lookup = QStringLiteral("purpose/types/") + pluginType + QStringLiteral("PluginType.json");

    QString path = QStringLiteral(":/") + lookup;
    if (!QFileInfo::exists(path)) {
        path = QStandardPaths::locate(QStandardPaths::GenericDataLocation, lookup);
        if (path.isEmpty()) {
            qWarning() << "Couldn't find" << lookup << QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
            return QJsonObject();
        }
    }
    QFile typeFile(path);
    if (!typeFile.open(QFile::ReadOnly)) {
        qWarning() << "Couldn't open" << lookup;
        return QJsonObject();
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(typeFile.readAll(), &error);
    if (error.error) {
        qWarning() << "JSON error in " << path << error.offset << ":" << error.errorString();
        return QJsonObject();
    }

    Q_ASSERT(doc.isObject());
    return doc.object();
}

KPluginMetaData Purpose::createMetaData(const QString &file)
{
    const QFileInfo fi(file);
    const QString fileName = fi.absoluteFilePath();
    QJsonObject metaData;

    QFile f(fileName);
    if (f.open(QIODevice::ReadOnly)) {
        const QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
        metaData = doc.object();
    }

    QDir dir = fi.dir().filePath(QStringLiteral("contents/code"));
    QStringList mainFile = dir.entryList({QStringLiteral("main.*")}, QDir::Files);
    if (mainFile.isEmpty()) {
        qWarning() << "no main file for" << file;
        return KPluginMetaData();
    }

    auto info = KPluginMetaData(metaData, dir.absoluteFilePath(mainFile.first()));
    Q_ASSERT(info.isValid() && !info.rawData().isEmpty());
    return info;
}
