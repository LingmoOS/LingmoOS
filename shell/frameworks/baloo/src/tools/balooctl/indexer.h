/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2015 Pinak Ahuja <pinak.ahuja@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef BALOO_INDEXER_H
#define BALOO_INDEXER_H

#include <QString>
#include <QMimeDatabase>
#include <KFileMetaData/ExtractorCollection>

#include "transaction.h"

namespace Baloo {
class Indexer
{
public:
    Indexer(const QString& url, Transaction* tr);

    void index();

private:
    QString m_url;
    QMimeDatabase m_mimeDB;
    KFileMetaData::ExtractorCollection m_extractorCollection;

    Transaction* m_tr;
};
}

#endif //BALOO_INDEXER_H
