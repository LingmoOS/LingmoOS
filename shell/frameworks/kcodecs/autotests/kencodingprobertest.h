/*
    SPDX-FileCopyrightText: 2012 Ni Hui <shuizhuyuanluo@126.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KENCODINGPROBERTEST_H
#define KENCODINGPROBERTEST_H

#include <QObject>

class KEncodingProberTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();
    void testReset();
    void testProbe();
};

#endif // KENCODINGPROBERTEST_H
