/*
    SPDX-FileCopyrightText: 2006 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDESKTOPFILETEST_H
#define KDESKTOPFILETEST_H

#include <QObject>
#include <kconfigcore_export.h>

class KDesktopFileTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testRead();
    void testReadLocalized_data();
    void testReadLocalized();
    void testUnsuccessfulTryExec();
    void testSuccessfulTryExec();
    void testActionGroup();
    void testIsAuthorizedDesktopFile();
    void testTryExecWithAuthorizeAction();
    void testLocateLocal_data();
    void testLocateLocal();
};

#endif /* KDESKTOPFILETEST_H */
