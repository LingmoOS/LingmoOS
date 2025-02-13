/*
    SPDX-FileCopyrightText: 2007 Glenn Ergeerts <glenn.ergeerts@telenet.be>
    SPDX-FileCopyrightText: 2012 Marco Gulino <marco.gulino@xpeppers.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "chromefindprofile.h"
#include "bookmarksrunner_defs.h"
#include "faviconfromblob.h"
#include <QDebug>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariantMap>

FindChromeProfile::FindChromeProfile(const QString &applicationName, const QString &homeDirectory, QObject *parent)
    : QObject(parent)
    , m_applicationName(applicationName)
    , m_homeDirectory(homeDirectory)
{
}

QList<Profile> FindChromeProfile::find()
{
    QString configDirectory = QStringLiteral("%1/.config/%2").arg(m_homeDirectory, m_applicationName);
    QString localStateFileName = QStringLiteral("%1/Local State").arg(configDirectory);

    QList<Profile> profiles;

    QFile localStateFile(localStateFileName);

    if (!localStateFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return profiles;
    }
    QJsonDocument jdoc = QJsonDocument::fromJson(localStateFile.readAll());

    if (jdoc.isNull()) {
        qDebug() << "error opening " << QFileInfo(localStateFile).absoluteFilePath();
        return profiles;
    }

    QVariantMap localState = jdoc.object().toVariantMap();
    QVariantMap profilesConfig = localState.value(QStringLiteral("profile")).toMap().value(QStringLiteral("info_cache")).toMap();

    for (const QString &profile : profilesConfig.keys()) {
        const QString profilePath = QStringLiteral("%1/%2").arg(configDirectory, profile);
        const QString profileBookmarksPath = QStringLiteral("%1/%2").arg(profilePath, QStringLiteral("Bookmarks"));
        profiles << Profile(profileBookmarksPath, profile, FaviconFromBlob::chrome(profilePath, this));
    }

    return profiles;
}
