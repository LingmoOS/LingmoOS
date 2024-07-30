/*
    SPDX-FileCopyrightText: 2022 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef PNGEXTRACTORTEST_H
#define PNGEXTRACTORTEST_H

#include <QObject>

class PngExtractorTest : public QObject
{
    Q_OBJECT
private:
    QString testFilePath(const QString &fileName) const;

private Q_SLOTS:
    void testNoExtraction();
    void test();
};

#endif // PNGEXTRACTORTEST_H
