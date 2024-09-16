/*
    This file is part of the KDE Libraries

    Copyright (C) 2009 Lubos Lunak <l.lunak@kde.org>
    Copyright (C) 2013 Martin Gräßlin <mgraesslin@kde.org>

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
#include "kmanagerselectiontest.h"

#include <QtTest>

#include <kselectionwatcher.h>
#include <kselectionowner.h>
#include <qx11info_x11.h>

#define SNAME "_KDE_KMANAGERSELECTIONTEST"

using namespace QTest;

void KManagerSelectionTest::xSync()
{
    xcb_connection_t *c = QX11Info::connection();
    const xcb_get_input_focus_cookie_t cookie = xcb_get_input_focus(c);
    xcb_generic_error_t *error = nullptr;
    QScopedPointer<xcb_get_input_focus_reply_t, QScopedPointerPodDeleter> sync(xcb_get_input_focus_reply(c, cookie, &error));
    if (error) {
        free(error);
    }
}

void KManagerSelectionTest::claim(KSelectionOwner *owner, bool force, bool forceKill)
{
    QSignalSpy claimSpy(owner, SIGNAL(claimedOwnership()));
    owner->claim(force, forceKill);
    xSync();
    QVERIFY(claimSpy.wait());
    QCOMPARE(claimSpy.count(), 1);
}

void KManagerSelectionTest::testAcquireRelease()
{
    // test that newOwner() is emitted when there is a new selection owner
    KSelectionWatcher watcher(SNAME);
    KSelectionOwner owner(SNAME);
    QVERIFY(owner.ownerWindow() == XCB_WINDOW_NONE);
    QVERIFY(watcher.owner() == XCB_WINDOW_NONE);
    SigCheckWatcher sw(watcher);
    SigCheckOwner so(owner);
    claim(&owner);
    QSignalSpy newOwnerSpy(&watcher, SIGNAL(newOwner(xcb_window_t)));
    QVERIFY(newOwnerSpy.wait());
    QVERIFY(sw.newowner == true);
    QVERIFY(sw.lostowner == false);
    QVERIFY(so.lostownership == false);
}

void KManagerSelectionTest::testInitiallyOwned()
{
    // test that lostOwner() is emitted when the selection is disowned
    KSelectionOwner owner(SNAME);
    SigCheckOwner so(owner);
    claim(&owner);
    KSelectionWatcher watcher(SNAME);
    SigCheckWatcher sw(watcher);
    owner.release();
    QSignalSpy lostOwnerSpy(&watcher, SIGNAL(lostOwner()));
    QVERIFY(lostOwnerSpy.wait(2000));
    QVERIFY(sw.newowner == false);
    QVERIFY(sw.lostowner == true);
    QVERIFY(so.lostownership == false);
}

void KManagerSelectionTest::testLostOwnership()
{
    // test that lostOwnership() is emitted when something else forces taking the ownership
    KSelectionOwner owner1(SNAME);
    KSelectionOwner owner2(SNAME);
    claim(&owner1);

    QSignalSpy claimSpy(&owner2, SIGNAL(failedToClaimOwnership()));
    owner2.claim(false);
    claimSpy.wait();
    QCOMPARE(claimSpy.count(), 1);
    claim(&owner2, true, false);

    QEXPECT_FAIL("", "selectionClear event is not sent to the same X client", Abort);
    QSignalSpy lostOwnershipSpy(&owner1, SIGNAL(lostOwnership()));
    QVERIFY(lostOwnershipSpy.wait());
    QVERIFY(owner1.ownerWindow() == XCB_WINDOW_NONE);
    QVERIFY(owner2.ownerWindow() != XCB_WINDOW_NONE);
}

void KManagerSelectionTest::testWatching()
{
    // test that KSelectionWatcher reports changes properly
    KSelectionWatcher watcher(SNAME);
    KSelectionOwner owner1(SNAME);
    KSelectionOwner owner2(SNAME);
    SigCheckWatcher sw(watcher);
    QSignalSpy newOwnerSpy(&watcher, SIGNAL(newOwner(xcb_window_t)));
    QVERIFY(newOwnerSpy.isValid());
    claim(&owner1);
    if (newOwnerSpy.isEmpty()) {
        QVERIFY(newOwnerSpy.wait());
    }
    QCOMPARE(newOwnerSpy.count(), 1);
    QVERIFY(sw.newowner == true);
    QVERIFY(sw.lostowner == false);
    sw.newowner = sw.lostowner = false;
    newOwnerSpy.clear();
    QVERIFY(newOwnerSpy.isEmpty());
    claim(&owner2, true, false);
    xSync();
    if (newOwnerSpy.isEmpty()) {
        QVERIFY(newOwnerSpy.wait());
    }
    QCOMPARE(newOwnerSpy.count(), 1);
    QVERIFY(sw.newowner == true);
    QVERIFY(sw.lostowner == false);
    sw.newowner = sw.lostowner = false;
    QSignalSpy lostOwnerSpy(&watcher, SIGNAL(lostOwner()));
    owner2.release();
    xSync();
    QVERIFY(lostOwnerSpy.wait());
    QVERIFY(sw.newowner == false);
    QVERIFY(sw.lostowner == true);
    sw.newowner = sw.lostowner = false;
    claim(&owner2);
    QVERIFY(newOwnerSpy.wait(2000));
    QVERIFY(sw.newowner == true);
    QVERIFY(sw.lostowner == false);
}

SigCheckOwner::SigCheckOwner(const KSelectionOwner &owner)
    : lostownership(false)
{
    connect(&owner, SIGNAL(lostOwnership()), this, SLOT(lostOwnership()));
}

void SigCheckOwner::lostOwnership()
{
    lostownership = true;
}

SigCheckWatcher::SigCheckWatcher(const KSelectionWatcher &watcher)
    : newowner(false)
    , lostowner(false)
{
    connect(&watcher, SIGNAL(newOwner(xcb_window_t)), this, SLOT(newOwner()));
    connect(&watcher, SIGNAL(lostOwner()), this, SLOT(lostOwner()));
}

void SigCheckWatcher::newOwner()
{
    newowner = true;
}

void SigCheckWatcher::lostOwner()
{
    lostowner = true;
}

QTEST_MAIN(KManagerSelectionTest)
