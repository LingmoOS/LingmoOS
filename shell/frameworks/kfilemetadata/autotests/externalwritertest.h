/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2016 Varun Joshi <varunj.1011@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef EXTERNALWRITERTEST_H
#define EXTERNALWRITERTEST_H

#include <QObject>

class ExternalWriterTest : public QObject
{
    Q_OBJECT
private:
    QString testFilePath(const QString& fileName) const;

private Q_SLOTS:
    void test();
    void testProperties();
};

#endif // EXTERNALWRITERTEST_H
