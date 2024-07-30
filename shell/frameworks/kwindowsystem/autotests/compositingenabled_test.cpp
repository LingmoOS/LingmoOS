/*
    SPDX-FileCopyrightText: 2016 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/
#include <KWindowSystem>
#include <QSignalSpy>
#include <QTest>

#include "kselectionowner.h"
#include "kx11extras.h"

class CompositingEnabledTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testRecreatingNetEventFilter();
};

void CompositingEnabledTest::testRecreatingNetEventFilter()
{
    // this test simulates the condition that the compositor gets enabled while the NetEventFilter gets recreated
    QVERIFY(!KX11Extras::compositingActive());

    // fake the compositor
    QSignalSpy compositingChangedSpy(KX11Extras::self(), &KX11Extras::compositingChanged);
    QVERIFY(compositingChangedSpy.isValid());
    KSelectionOwner compositorSelection("_NET_WM_CM_S0");
    QSignalSpy claimedSpy(&compositorSelection, &KSelectionOwner::claimedOwnership);
    QVERIFY(claimedSpy.isValid());
    compositorSelection.claim(true);
    connect(&compositorSelection, &KSelectionOwner::claimedOwnership, [] {
        // let's connect to a signal which will cause a re-creation of NetEventFilter
        QSignalSpy workAreaChangedSpy(KX11Extras::self(), &KX11Extras::workAreaChanged);
        QVERIFY(workAreaChangedSpy.isValid());
    });

    QVERIFY(claimedSpy.wait());
    QTRY_VERIFY(KX11Extras::compositingActive());
    QCOMPARE(compositingChangedSpy.count(), 1);
}

QTEST_MAIN(CompositingEnabledTest)
#include "compositingenabled_test.moc"
