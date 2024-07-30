/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2007 Oswald Buddenhagen <ossi@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef kptyprocesstest_h
#define kptyprocesstest_h

#include <kptyprocess.h>

#include <QTimer>

class KPtyProcessTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void test_pty_basic();
    void test_pty_signals();
    void test_ctty();
    void test_shared_pty();
    void test_suspend_pty();

    // for pty_signals
public Q_SLOTS:
    void slotReadyRead();
    void slotDoRead();
    void slotReadEof();
    void slotBytesWritten();
    void slotStep();

private:
    KPtyProcess sp;
    QTimer delay;
    QByteArray log;
    int phase;
};

#endif
