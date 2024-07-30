/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2005 Thomas Braxton <brax108@cox.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCOMPLETIONCORETEST_H
#define KCOMPLETIONCORETEST_H

#include "kcompletion.h"
#include <QStringList>

class Test_KCompletion : public QObject
{
    Q_OBJECT
    QStringList strings;
    QStringList wstrings;

private Q_SLOTS:
    void initTestCase();
    void isEmpty();
    void insertionOrder();
    void sortedOrder();
    void weightedOrder();
    void substringCompletion_Insertion();
    void substringCompletion_Sorted();
    void substringCompletion_Weighted();
    void allMatches_Insertion();
    void allMatches_Sorted();
    void allMatches_Weighted();
    void cycleMatches_Insertion();
    void cycleMatches_Sorted();
    void cycleMatches_Weighted();
};

#endif
