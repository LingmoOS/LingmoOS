/*
    SPDX-FileCopyrightText: 2006 Michaël Larouche <michael.larouche@kdemail.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef FAKEHARDWARETEST_H
#define FAKEHARDWARETEST_H

#include <QObject>

class FakeHardwareTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testFakeBackend();
};

#endif
