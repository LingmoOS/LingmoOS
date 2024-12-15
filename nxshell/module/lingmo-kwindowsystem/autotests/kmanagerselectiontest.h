/*
    This file is part of the KDE Libraries

    Copyright (C) 2009 Lubos Lunak <l.lunak@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef KMANAGERSELECTIONTESTTEST_H
#define KMANAGERSELECTIONTESTTEST_H

#include <QtCore/QObject>
#include <QtTest/QtTest>

class KSelectionOwner;

class KManagerSelectionTest
    : public QObject
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
class SigCheckOwner
    : public QObject
{
    Q_OBJECT
public:
    SigCheckOwner(const KSelectionOwner &owner);
private Q_SLOTS:
    void lostOwnership();
public:
    bool lostownership;
};

class SigCheckWatcher
    : public QObject
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
