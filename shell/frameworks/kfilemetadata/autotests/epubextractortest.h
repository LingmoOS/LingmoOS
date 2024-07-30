/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef EPUBEXTRACTORTEST_H
#define EPUBEXTRACTORTEST_H

#include <QObject>

class EPubExtractorTest : public QObject
{
    Q_OBJECT
private:
    QString testFilePath(const QString& fileName) const;

private Q_SLOTS:
    void testNoExtraction();
    void test();
    void testRepeated();
    void testMetaDataOnly();
};

#endif // EPUBEXTRACTORTEST_H
