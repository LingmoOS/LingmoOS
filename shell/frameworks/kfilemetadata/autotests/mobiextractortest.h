/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef MOBIEXTRACTORTEST_H
#define MOBIEXTRACTORTEST_H

#include <QObject>

class MobiExtractorTest : public QObject
{
    Q_OBJECT
private:
    QString testFilePath(const QString& fileName) const;

private Q_SLOTS:
    void test();
};

#endif // MOBIEXTRACTORTEST_H
