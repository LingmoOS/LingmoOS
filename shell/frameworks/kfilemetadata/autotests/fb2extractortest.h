/*
    SPDX-FileCopyrightText: 2022 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef FB2EXTRACTORTEST_H
#define FB2EXTRACTORTEST_H

#include <QObject>

class Fb2ExtractorTest : public QObject
{
    Q_OBJECT
private:
    QString testFilePath(const QString &fileName) const;

private Q_SLOTS:
    void testNoExtraction_data();
    void testNoExtraction();

    void test_data();
    void test();

    void testMetaDataOnly_data();
    void testMetaDataOnly();
};

#endif // FB2EXTRACTORTEST_H
