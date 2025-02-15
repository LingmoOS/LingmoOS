/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2005 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KSTDACCELTEST_H
#define KSTDACCELTEST_H

#include <QObject>

class KStandardShortcutTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    // KStandardShortcut tests
    void testShortcutDefault();
    void testName();
    void testLabel();
    void testShortcut();
    void testFindStdAccel();
    void testFindByName();
};

#endif
