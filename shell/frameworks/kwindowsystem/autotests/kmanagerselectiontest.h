/*
    This file is part of the KDE Libraries

    SPDX-FileCopyrightText: 2009 Lubos Lunak <l.lunak@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KMANAGERSELECTIONTESTTEST_H
#define KMANAGERSELECTIONTESTTEST_H

#include <QObject>
#include <QTest>

class KSelectionOwner;

class KManagerSelectionTest : public QObject
{
    Q_OBJECT
public:
private Q_SLOTS:
    void testAcquireRelease();
    void testInitiallyOwned();
    void testLostOwnership();
    void testWatching();

private:
    void claim(KSelectionOwner *owner, bool force = false, bool forceKill = true);
    void xSync();
};

class KSelectionWatcher;

// For checking whether several signal have or have not been emitted,
// QSignalSpy::wait() is not powerful enough for that (it is still
// needed to do the event processing though). TODO: check if this is still true.
class SigCheckOwner : public QObject
{
    Q_OBJECT
public:
    SigCheckOwner(const KSelectionOwner &owner);
private Q_SLOTS:
    void lostOwnership();

public:
    bool lostownership;
};

class SigCheckWatcher : public QObject
{
    Q_OBJECT
public:
    SigCheckWatcher(const KSelectionWatcher &watcher);
private Q_SLOTS:
    void newOwner();
    void lostOwner();

public:
    bool newowner;
    bool lostowner;
};

#endif
