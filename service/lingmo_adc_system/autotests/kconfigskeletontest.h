/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006 Olivier Goffart <ogoffart at kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KCONFIGSKELETONTEST_H
#define KCONFIGSKELETONTEST_H

#include <kconfigskeleton.h>

class KConfigSkeletonTest : public QObject
{
    Q_OBJECT
public:
private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();
    void testSimple();
    void testDefaults();
    void testRemoveItem();
    void testClear();
    void testKConfigDirty();
    void testSaveRead();

private:
    KConfigSkeleton *s;
    KConfigSkeleton::ItemBool *itemBool;
    bool mMyBool;
    QColor mMyColor;
    QFont mMyFont;
    QString mMyString;
};

#endif
