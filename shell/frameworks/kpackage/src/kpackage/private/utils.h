#pragma once

#include "kpackage_debug.h"
#include <KPluginMetaData>
#include <QString>
#include <QVersionNumber>

inline QString readKPackageType(const KPluginMetaData &metaData)
{
    return metaData.value(QStringLiteral("KPackageStructure"));
}

inline KPluginMetaData structureForKPackageType(const QString &packageFormat)
{
    const QString guessedPath = QStringLiteral("kf6/packagestructure/") + QString(packageFormat).toLower().replace(QLatin1Char('/'), QLatin1Char('_'));
    KPluginMetaData guessedData(guessedPath);
    if (guessedData.isValid() && readKPackageType(guessedData) == packageFormat) {
        return guessedData;
    }
    qCDebug(KPACKAGE_LOG) << "Could not find package structure for" << packageFormat << "by plugin path. The guessed path was" << guessedPath;

    const QList<KPluginMetaData> plugins =
        KPluginMetaData::findPlugins(QStringLiteral("kf6/packagestructure"), [packageFormat](const KPluginMetaData &metaData) {
            return readKPackageType(metaData) == packageFormat;
        });
    return plugins.isEmpty() ? KPluginMetaData() : plugins.first();
}

inline bool isVersionNewer(const QString &version1, const QString &version2)
{
    const auto v1 = QVersionNumber::fromString(version1);
    const auto v2 = QVersionNumber::fromString(version2);
    return v2 > v1;
}
