/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KRITAEXTRACTORTEST_H
#define KRITAEXTRACTORTEST_H

#include <QObject>

class KritaExtractorTest : public QObject
{
    Q_OBJECT
private:
    QString testFilePath(const QString &fileName) const;

private Q_SLOTS:
    void testNoExtraction();
    void test();
};

#endif // KRITAEXTRACTORTEST_H
