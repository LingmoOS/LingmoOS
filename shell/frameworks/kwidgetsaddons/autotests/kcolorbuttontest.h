/*
    SPDX-FileCopyrightText: 2013 Albert Astals Cid <aacid@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCOLORBUTTONTEST_H
#define KCOLORBUTTONTEST_H

#include <QObject>

class KColorButtonTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testOpenDialog();
};

#endif
