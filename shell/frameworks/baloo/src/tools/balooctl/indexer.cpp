/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2015 Pinak Ahuja <pinak.ahuja@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "indexer.h"
#include "basicindexingjob.h"
#include "database.h"
#include "./extractor/result.h"

#include <KFileMetaData/Extractor>
#include <KFileMetaData/PropertyInfo>

using namespace Baloo;

Indexer::Indexer(const QString& url, Transaction* tr)
    : m_url(url)
    , m_tr(tr)
{
}

void Indexer::index()
{
    const QString mimetype = m_mimeDB.mimeTypeForFile(m_url).name();
    BasicIndexingJob basicIJ(m_url, mimetype, BasicIndexingJob::NoLevel);
    basicIJ.index();
    Baloo::Document doc = basicIJ.document();

    Result result(m_url, mimetype, KFileMetaData::ExtractionResult::ExtractMetaData | KFileMetaData::ExtractionResult::ExtractPlainText);
    result.setDocument(doc);

    const QList<KFileMetaData::Extractor*> exList = m_extractorCollection.fetchExtractors(mimetype);

    for (KFileMetaData::Extractor* ex : exList) {
        ex->extract(&result);
    }

    result.finish();
    if (m_tr->hasDocument(doc.id())) {
        m_tr->replaceDocument(doc, Everything);
    } else {
        m_tr->addDocument(result.document());
    }
}
