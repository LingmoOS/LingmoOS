/*
    SPDX-FileCopyrightText: 2018 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef POSTSCRIPTDSCEXTRACTORTEST_H
#define POSTSCRIPTDSCEXTRACTORTEST_H

#include <QObject>
#include <QMimeDatabase>

class PostscriptDscExtractorTest : public QObject
{
    Q_OBJECT
private:
    QString testFilePath(const QString& fileName) const;

private Q_SLOTS:
    void testNoExtraction();
    void testPS();
    void testEPS();
private:
    QMimeDatabase mimeDb;
};

#endif // POSTSCRIPTDSCEXTRACTORTEST_H
