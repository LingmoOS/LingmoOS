/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "baloosearchmodule.h"

#include <QDBusConnection>
#include <QUrl>
#include <QTimer>

#include <kpluginfactory.h>

namespace
{
    inline bool isSearchUrl(const QUrl& url)
    {
        return url.scheme() == QLatin1String("baloosearch") ||
               url.scheme() == QLatin1String("timeline") ||
               url.scheme() == QLatin1String("tags");
    }
}

using namespace Baloo;

SearchModule::SearchModule(QObject* parent, const QList<QVariant>&)
    : KDEDModule(parent)
    , m_dirNotify(nullptr)
{
    QTimer::singleShot(0, this, SLOT(init()));
}

void SearchModule::init()
{
    m_dirNotify = new org::kde::KDirNotify(QString(), QString(),
                                           QDBusConnection::sessionBus(), this);
    connect(m_dirNotify, &OrgKdeKDirNotifyInterface::enteredDirectory,
            this, &SearchModule::registerSearchUrl);
    connect(m_dirNotify, &OrgKdeKDirNotifyInterface::leftDirectory,
            this, &SearchModule::unregisterSearchUrl);

    // FIXME: Listen to changes from Baloo!!
    // Listen to dbChanged
    QDBusConnection con = QDBusConnection::sessionBus();
    con.connect(QString(), QStringLiteral("/files"), QStringLiteral("org.kde.baloo"),
                QStringLiteral("updated"), this, SLOT(slotBalooFileDbChanged()));
    con.connect(QString(), QStringLiteral("/files"), QStringLiteral("org.kde"),
                QStringLiteral("changed"), this, SLOT(slotFileMetaDataChanged(QStringList)));
}

void SearchModule::registerSearchUrl(const QString& urlString)
{
    QUrl url(urlString);
    if (isSearchUrl(url)) {
        m_searchUrls << url;
    }
}

void SearchModule::unregisterSearchUrl(const QString& urlString)
{
    QUrl url(urlString);
    m_searchUrls.removeAll(url);
}

void SearchModule::slotBalooFileDbChanged()
{
    for (const QUrl& dirUrl : std::as_const(m_searchUrls)) {
        org::kde::KDirNotify::emitFilesAdded(dirUrl);
    }
}

void SearchModule::slotFileMetaDataChanged(const QStringList& list)
{
    QList<QUrl> localFileUrls;
    localFileUrls.reserve(list.size());
    for (const QString& path : list) {
        localFileUrls << QUrl::fromLocalFile(path);
    }
    org::kde::KDirNotify::emitFilesChanged(localFileUrls);
    slotBalooFileDbChanged();
}

K_PLUGIN_CLASS_WITH_JSON(SearchModule, "baloosearchmodule.json")

#include "baloosearchmodule.moc"
#include "moc_baloosearchmodule.cpp"
