/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2022 kirito <baijunjie@kylinos.cn>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef PDFEXTRACTORTEST_H
#define PDFEXTRACTORTEST_H

#include <QObject>

namespace LingmoUIFileMetadata {

class PdfExtractorTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testNoExtraction();
    void testAllData();
    void testMetaDataOnly();
    void testThumbnail();
};

}



#endif // PDFEXTRACTORTEST_H
