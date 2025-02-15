/*
    SPDX-FileCopyrightText: 2007 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 2007 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KSTANDARDACTIONTEST_H
#define KSTANDARDACTIONTEST_H

#include <QObject>

class tst_KStandardAction : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void shortcutForActionId();
    void changingShortcut();
    void testCreateOldStyle();
    void testCreateNewStyle();
};

#endif // KSTANDARDACTIONTEST_H
