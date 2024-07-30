/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 1998 Sven Radej <sven@lisa.exp.univie.ac.at>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef _KDIRWATCHTEST_H_
#define _KDIRWATCHTEST_H_

#include <QObject>
#include <stdio.h>
#include <stdlib.h>

#include "kdirwatch.h"

class myTest : public QObject
{
    Q_OBJECT
public:
    myTest()
    {
    }
public Q_SLOTS:
    void dirty(const QString &a)
    {
        printf("Dirty: %s\n", a.toLocal8Bit().constData());
    }
    void created(const QString &f)
    {
        printf("Created: %s\n", f.toLocal8Bit().constData());
    }
    void deleted(const QString &f)
    {
        printf("Deleted: %s\n", f.toLocal8Bit().constData());
    }
};

#endif
