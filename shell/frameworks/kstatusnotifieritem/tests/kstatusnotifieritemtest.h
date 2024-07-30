/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2009 Marco Martin <notmart@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KSTATUSNOTIFIERITEMTEST_H
#define KSTATUSNOTIFIERITEMTEST_H

#include <QObject>

class KStatusNotifierItem;

class KStatusNotifierItemTest : public QObject
{
    Q_OBJECT

public:
    KStatusNotifierItemTest(QObject *parent, KStatusNotifierItem *tray);
    //~KStatusNotifierItemTest();

public Q_SLOTS:
    void setNeedsAttention();
    void setActive();
    void setPassive();

private:
    KStatusNotifierItem *m_tray;
};

#endif
