/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KFUZZYMATCHERTEST_H
#define KFUZZYMATCHERTEST_H

#include <QObject>

class KFuzzyMatcherTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testMatchSimple_data();
    void testMatchSimple();
    void testMatch_data();
    void testMatch();
    void testMatchedRanges_data();
    void testMatchedRanges();
};

#endif // KFUZZYMATCHERTEST_H
