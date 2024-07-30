/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KDED_BALOO_SEARCH_MODULE_H_
#define KDED_BALOO_SEARCH_MODULE_H_

#include <KDEDModule>
#include <KDirNotify>
#include <QUrl>

namespace Baloo {

class SearchModule : public KDEDModule
{
    Q_OBJECT

public:
    SearchModule(QObject* parent, const QList<QVariant>&);

private Q_SLOTS:
    void init();
    void registerSearchUrl(const QString& url);
    void unregisterSearchUrl(const QString& url);

    void slotBalooFileDbChanged();
    void slotFileMetaDataChanged(const QStringList& list);
private:
    QList<QUrl> m_searchUrls;
    org::kde::KDirNotify* m_dirNotify;
};

}

#endif
