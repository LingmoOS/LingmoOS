/*
 *   Copyright 2016 Martin Gräßlin <mgraesslin@kde.org>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <KWindowSystem>
#include <kmanagerselection.h>
#include <QtTest/QtTest>

class CompositingEnabledTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testRecreatingNetEventFilter();
};

void CompositingEnabledTest::testRecreatingNetEventFilter()
{
    // this test simulates the condition that the compositor gets enabled while the NetEventFilter gets recreated
    QVERIFY(!KWindowSystem::compositingActive());

    // fake the compositor
    QSignalSpy compositingChangedSpy(KWindowSystem::self(), &KWindowSystem::compositingChanged);
    QVERIFY(compositingChangedSpy.isValid());
    KSelectionOwner compositorSelection("_NET_WM_CM_S0");
    QSignalSpy claimedSpy(&compositorSelection, &KSelectionOwner::claimedOwnership);
    QVERIFY(claimedSpy.isValid());
    compositorSelection.claim(true);
    connect(&compositorSelection, &KSelectionOwner::claimedOwnership,
        [] {
            // let's connect to a signal which will cause a re-creation of NetEventFilter
            QSignalSpy workAreaChangedSpy(KWindowSystem::self(), &KWindowSystem::workAreaChanged);
            QVERIFY(workAreaChangedSpy.isValid());
        }
    );

    QVERIFY(claimedSpy.wait());
    QTRY_VERIFY(KWindowSystem::compositingActive());
    QCOMPARE(compositingChangedSpy.count(), 1);
}

QTEST_MAIN(CompositingEnabledTest)
#include "compositingenabled_test.moc"
