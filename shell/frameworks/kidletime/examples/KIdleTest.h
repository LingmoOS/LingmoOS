/*
 * SPDX-FileCopyrightText: 2009 Dario Freddi <drf@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIDLETEST_H
#define KIDLETEST_H

#include <QObject>

class KIdleTest : public QObject
{
    Q_OBJECT

public:
    KIdleTest();
    ~KIdleTest() override;

public Q_SLOTS:
    void timeoutReached(int id, int timeout);
    void resumeEvent();
};

#endif /* KIDLETEST_H */
