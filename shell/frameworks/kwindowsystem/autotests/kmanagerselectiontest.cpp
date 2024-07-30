/*
    This file is part of the KDE Libraries

    SPDX-FileCopyrightText: 2009 Lubos Lunak <l.lunak@kde.org>
    SPDX-FileCopyrightText: 2013 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "kmanagerselectiontest.h"
#include "cptr_p.h"

#include <QSignalSpy>
#include <kselectionowner.h>
#include <kselectionwatcher.h>

#include <private/qtx11extras_p.h>

#define SNAME "_KDE_KMANAGERSELECTIONTEST"

using namespace QTest;

void KManagerSelectionTest::xSync()
{
    xcb_connection_t *c = QX11Info::connection();
    const xcb_get_input_focus_cookie_t cookie = xcb_get_input_focus(c);
    xcb_generic_error_t *error = nullptr;
    UniqueCPointer<xcb_get_input_focus_reply_t> sync(xcb_get_input_focus_reply(c, cookie, &error));
    if (error) {
        free(error);
    }
}

void KManagerSelectionTest::claim(KSelectionOwner *owner, bool force, bool forceKill)
{
    QSignalSpy claimSpy(owner, &KSelectionOwner::claimedOwnership);
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
    QSignalSpy newOwnerSpy(&watcher, &KSelectionWatcher::newOwner);
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
    QSignalSpy lostOwnerSpy(&watcher, &KSelectionWatcher::lostOwner);
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

    QSignalSpy claimSpy(&owner2, &KSelectionOwner::failedToClaimOwnership);
    owner2.claim(false);
    claimSpy.wait();
    QCOMPARE(claimSpy.count(), 1);
    claim(&owner2, true, false);

    QEXPECT_FAIL("", "selectionClear event is not sent to the same X client", Abort);
    QSignalSpy lostOwnershipSpy(&owner1, &KSelectionOwner::lostOwnership);
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
    QSignalSpy newOwnerSpy(&watcher, &KSelectionWatcher::newOwner);
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
    QSignalSpy lostOwnerSpy(&watcher, &KSelectionWatcher::lostOwner);
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
    connect(&owner, &KSelectionOwner::lostOwnership, this, &SigCheckOwner::lostOwnership);
}

void SigCheckOwner::lostOwnership()
{
    lostownership = true;
}

SigCheckWatcher::SigCheckWatcher(const KSelectionWatcher &watcher)
    : newowner(false)
    , lostowner(false)
{
    connect(&watcher, &KSelectionWatcher::newOwner, this, &SigCheckWatcher::newOwner);
    connect(&watcher, &KSelectionWatcher::lostOwner, this, &SigCheckWatcher::lostOwner);
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

#include "moc_kmanagerselectiontest.cpp"
