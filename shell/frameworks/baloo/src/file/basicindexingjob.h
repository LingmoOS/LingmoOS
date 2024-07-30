/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef BASICINDEXINGJOB_H
#define BASICINDEXINGJOB_H

#include "document.h"

namespace Baloo {

class BasicIndexingJob
{
public:
    enum IndexingLevel {
        NoLevel,
        MarkForContentIndexing,
    };

    BasicIndexingJob(const QString& filePath, const QString& mimetype,
                     IndexingLevel level = MarkForContentIndexing);
    ~BasicIndexingJob();

    bool index();

    Document document() { return m_doc; }

private:
    QString m_filePath;
    QString m_mimetype;
    IndexingLevel m_indexingLevel;

    Document m_doc;

    friend class BasicIndexingJobTest;
};

}

#endif // BASICINDEXINGJOB_H
