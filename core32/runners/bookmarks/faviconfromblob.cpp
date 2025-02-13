/*
    SPDX-FileCopyrightText: 2007 Glenn Ergeerts <glenn.ergeerts@telenet.be>
    SPDX-FileCopyrightText: 2012 Marco Gulino <marco.gulino@xpeppers.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "faviconfromblob.h"

#include "bookmarksrunner_defs.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QPainter>
#include <QPixmap>
#include <QStandardPaths>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

FaviconFromBlob *FaviconFromBlob::chrome(const QString &profileDirectory, QObject *parent)
{
    QString profileName = QFileInfo(profileDirectory).fileName();
    QString faviconCache = QStringLiteral("%1/bookmarksrunner/KRunner-Chrome-Favicons-%2.sqlite")
                               .arg(QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation), profileName);
    FetchSqlite *fetchSqlite = new FetchSqlite(faviconCache, parent);

    QString faviconQuery;
    if (fetchSqlite->tables().contains(QLatin1String("favicon_bitmaps"))) {
        faviconQuery = QLatin1String(
            "SELECT * FROM favicons "
            "inner join icon_mapping on icon_mapping.icon_id = favicons.id "
            "inner join favicon_bitmaps on icon_mapping.icon_id = favicon_bitmaps.icon_id "
            "WHERE page_url = :url ORDER BY height desc LIMIT 1;");
    } else {
        faviconQuery = QLatin1String(
            "SELECT * FROM favicons "
            "inner join icon_mapping on icon_mapping.icon_id = favicons.id "
            "WHERE page_url = :url LIMIT 1;");
    }

    return new FaviconFromBlob(profileName, faviconQuery, QStringLiteral("image_data"), fetchSqlite, parent);
}

FaviconFromBlob *FaviconFromBlob::firefox(FetchSqlite *fetchSqlite, QObject *parent)
{
    QString faviconQuery = QStringLiteral(
        "SELECT moz_icons.data FROM moz_icons"
        " INNER JOIN moz_icons_to_pages ON moz_icons.id = moz_icons_to_pages.icon_id"
        " INNER JOIN moz_pages_w_icons ON moz_icons_to_pages.page_id = moz_pages_w_icons.id"
        " WHERE moz_pages_w_icons.page_url = :url LIMIT 1;");
    return new FaviconFromBlob(QStringLiteral("firefox-default"), faviconQuery, QStringLiteral("data"), fetchSqlite, parent);
}

FaviconFromBlob *FaviconFromBlob::falkon(const QString &profileDirectory, QObject *parent)
{
    const QString dbPath = profileDirectory + QStringLiteral("/browsedata.db");
    FetchSqlite *fetchSqlite = new FetchSqlite(dbPath, parent);
    const QString faviconQuery = QStringLiteral("SELECT icon FROM icons WHERE url = :url LIMIT 1;");
    return new FaviconFromBlob(QStringLiteral("falkon-default"), faviconQuery, QStringLiteral("icon"), fetchSqlite, parent);
}

FaviconFromBlob::FaviconFromBlob(const QString &profileName, const QString &query, const QString &blobColumn, FetchSqlite *fetchSqlite, QObject *parent)
    : Favicon(parent)
    , m_query(query)
    , m_blobcolumn(blobColumn)
    , m_fetchsqlite(fetchSqlite)
{
    m_profileCacheDirectory =
        QStringLiteral("%1/bookmarksrunner/KRunner-Favicons-%2").arg(QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation), profileName);
    // qDebug() << "got cache directory: " << m_profileCacheDirectory;
    cleanCacheDirectory();
    QDir().mkpath(m_profileCacheDirectory);
}

FaviconFromBlob::~FaviconFromBlob()
{
    cleanCacheDirectory();
}

void FaviconFromBlob::prepare()
{
    m_fetchsqlite->prepare();
}

void FaviconFromBlob::teardown()
{
    m_fetchsqlite->teardown();
}

void FaviconFromBlob::cleanCacheDirectory()
{
    QDir(m_profileCacheDirectory).removeRecursively();
}

QIcon FaviconFromBlob::iconFor(const QString &url)
{
    // qDebug() << "got url: " << url;
    QString fileChecksum = QString::number(qChecksum(url.toLatin1(), url.toLatin1().size()));
    QFile iconFile(m_profileCacheDirectory + QDir::separator() + fileChecksum + QStringLiteral("_favicon"));
    if (iconFile.size() == 0)
        iconFile.remove();
    if (!iconFile.exists()) {
        QMap<QString, QVariant> bindVariables;
        bindVariables.insert(QStringLiteral(":url"), url);
        QList<QVariantMap> faviconFound = m_fetchsqlite->query(m_query, bindVariables);
        if (faviconFound.isEmpty())
            return defaultIcon();

        QByteArray iconData = faviconFound.first().value(m_blobcolumn).toByteArray();
        // qDebug() << "Favicon found: " << iconData.size() << " bytes";
        if (iconData.size() <= 0)
            return defaultIcon();

        iconFile.open(QFile::WriteOnly);
        iconFile.write(iconData);
        iconFile.close();
    }
    return QIcon(iconFile.fileName());
}
