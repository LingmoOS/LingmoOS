/*
 *   Copyright 2013 Martin Gräßlin <mgraesslin@kde.org>
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
#include "nettesthelper.h"
#include "kwindowsystem.h"
#include "netwm.h"

#include <qtest_widgets.h>
#include <QSignalSpy>
#include <QWidget>
#include <QX11Info>
Q_DECLARE_METATYPE(WId)
Q_DECLARE_METATYPE(NET::Properties)
Q_DECLARE_METATYPE(NET::Properties2)
Q_DECLARE_METATYPE(const unsigned long*)

class KWindowSystemX11Test : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    // needs to be first test, would fail if run after others (X11)
    void testActiveWindowChanged();
    void testWindowAdded();
    void testWindowRemoved();
    void testDesktopChanged();
    void testNumberOfDesktopsChanged();
    void testDesktopNamesChanged();
    void testShowingDesktopChanged();
    void testSetShowingDesktop();
    void testWorkAreaChanged();
    void testWindowTitleChanged();
    void testMinimizeWindow();
    void testPlatformX11();
};

void KWindowSystemX11Test::initTestCase()
{
    QCoreApplication::setAttribute(Qt::AA_ForceRasterWidgets);
}

void KWindowSystemX11Test::testActiveWindowChanged()
{
    qRegisterMetaType<WId>("WId");
    QSignalSpy spy(KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)));

    QScopedPointer<QWidget> widget(new QWidget);
    widget->show();

    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).toULongLong(), widget->winId());
    QCOMPARE(KWindowSystem::activeWindow(), widget->winId());
}

void KWindowSystemX11Test::testWindowAdded()
{
    qRegisterMetaType<WId>("WId");
    QSignalSpy spy(KWindowSystem::self(), SIGNAL(windowAdded(WId)));
    QSignalSpy stackingOrderSpy(KWindowSystem::self(), SIGNAL(stackingOrderChanged()));
    QScopedPointer<QWidget> widget(new QWidget);
    widget->show();
    QVERIFY(QTest::qWaitForWindowExposed(widget.data()));
    QVERIFY(spy.count() > 0);
    bool hasWId = false;
    for (auto it = spy.constBegin(); it != spy.constEnd(); ++it) {
        if ((*it).isEmpty()) {
            continue;
        }
        QCOMPARE((*it).count(), 1);
        hasWId = (*it).at(0).toULongLong() == widget->winId();
        if (hasWId) {
            break;
        }
    }
    QVERIFY(hasWId);
    QVERIFY(KWindowSystem::hasWId(widget->winId()));
    QVERIFY(!stackingOrderSpy.isEmpty());
}

void KWindowSystemX11Test::testWindowRemoved()
{
    qRegisterMetaType<WId>("WId");
    QScopedPointer<QWidget> widget(new QWidget);
    widget->show();
    QVERIFY(QTest::qWaitForWindowExposed(widget.data()));
    QVERIFY(KWindowSystem::hasWId(widget->winId()));

    QSignalSpy spy(KWindowSystem::self(), SIGNAL(windowRemoved(WId)));
    widget->hide();
    spy.wait(1000);
    QCOMPARE(spy.first().at(0).toULongLong(), widget->winId());
    QVERIFY(!KWindowSystem::hasWId(widget->winId()));
}

void KWindowSystemX11Test::testDesktopChanged()
{
    // This test requires a running NETWM-compliant window manager
    if (KWindowSystem::numberOfDesktops() == 1) {
        QSKIP("At least two virtual desktops are required to test desktop changed");
    }
    const int current = KWindowSystem::currentDesktop();

    QSignalSpy spy(KWindowSystem::self(), SIGNAL(currentDesktopChanged(int)));
    int newDesktop = current + 1;
    if (newDesktop > KWindowSystem::numberOfDesktops()) {
        newDesktop = 1;
    }
    KWindowSystem::setCurrentDesktop(newDesktop);
    QVERIFY(spy.wait());
    QCOMPARE(KWindowSystem::currentDesktop(), newDesktop);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).toInt(), newDesktop);
    spy.clear();

    // setting to current desktop should not change anything
    KWindowSystem::setCurrentDesktop(newDesktop);

    // set back for clean state
    KWindowSystem::setCurrentDesktop(current);
    QVERIFY(spy.wait());
    QCOMPARE(KWindowSystem::currentDesktop(), current);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).toInt(), current);
}

void KWindowSystemX11Test::testNumberOfDesktopsChanged()
{
    // This test requires a running NETWM-compliant window manager
    const int oldNumber = KWindowSystem::numberOfDesktops();
    QSignalSpy spy(KWindowSystem::self(), SIGNAL(numberOfDesktopsChanged(int)));

    // KWin has arbitrary max number of 20 desktops, so don't fail the test if we use +1
    const int newNumber = oldNumber < 20 ? oldNumber + 1 : oldNumber - 1;

    NETRootInfo info(QX11Info::connection(), NET::NumberOfDesktops, NET::Properties2());
    info.setNumberOfDesktops(newNumber);

    QVERIFY(spy.wait());
    QCOMPARE(KWindowSystem::numberOfDesktops(), newNumber);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).toInt(), newNumber);
    spy.clear();

    // setting to same number should not change
    info.setNumberOfDesktops(newNumber);

    // set back for clean state
    info.setNumberOfDesktops(oldNumber);
    QVERIFY(spy.wait());
    QCOMPARE(KWindowSystem::numberOfDesktops(), oldNumber);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).toInt(), oldNumber);
}

void KWindowSystemX11Test::testDesktopNamesChanged()
{
    // This test requires a running NETWM-compliant window manager
    const QString origName = KWindowSystem::desktopName(KWindowSystem::currentDesktop());
    QSignalSpy spy(KWindowSystem::self(), SIGNAL(desktopNamesChanged()));

    const QString testName = QStringLiteral("testFooBar");

    KWindowSystem::setDesktopName(KWindowSystem::currentDesktop(), testName);
    QVERIFY(spy.wait());
    QCOMPARE(KWindowSystem::desktopName(KWindowSystem::currentDesktop()), testName);
    QCOMPARE(spy.count(), 1);
    spy.clear();

    QX11Info::setAppTime(QX11Info::getTimestamp());

    // setting back to clean state
    KWindowSystem::setDesktopName(KWindowSystem::currentDesktop(), origName);
    QVERIFY(spy.wait());
    QCOMPARE(KWindowSystem::desktopName(KWindowSystem::currentDesktop()), origName);
    QCOMPARE(spy.count(), 1);
}

void KWindowSystemX11Test::testShowingDesktopChanged()
{
    QX11Info::setAppTime(QX11Info::getTimestamp());
    const bool showingDesktop = KWindowSystem::showingDesktop();

    QSignalSpy spy(KWindowSystem::self(), SIGNAL(showingDesktopChanged(bool)));

    NETRootInfo info(QX11Info::connection(), NET::Properties(), NET::WM2ShowingDesktop);
    info.setShowingDesktop(!showingDesktop);

    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).toBool(), !showingDesktop);
    QCOMPARE(KWindowSystem::showingDesktop(), !showingDesktop);
    spy.clear();

    QX11Info::setAppTime(QX11Info::getTimestamp());

    // setting again should not change
    info.setShowingDesktop(!showingDesktop);

    // setting back to clean state
    info.setShowingDesktop(showingDesktop);
    QVERIFY(spy.wait(100));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).toBool(), showingDesktop);
    QCOMPARE(KWindowSystem::showingDesktop(), showingDesktop);
}

void KWindowSystemX11Test::testSetShowingDesktop()
{
    QSignalSpy spy(KWindowSystem::self(), SIGNAL(showingDesktopChanged(bool)));
    const bool showingDesktop = KWindowSystem::showingDesktop();

    // setting the same state shouldn't change it
    QX11Info::setAppTime(QX11Info::getTimestamp());
    KWindowSystem::setShowingDesktop(showingDesktop);
    QCOMPARE(spy.wait(), false); // spy.wait() waits for 5s
    QCOMPARE(KWindowSystem::showingDesktop(), showingDesktop);
    spy.clear();

    // set opposite state
    QX11Info::setAppTime(QX11Info::getTimestamp());
    KWindowSystem::setShowingDesktop(!showingDesktop);
    QVERIFY(spy.wait());
    QCOMPARE(KWindowSystem::showingDesktop(), !showingDesktop);
    spy.clear();

    // setting back to clean state
    QX11Info::setAppTime(QX11Info::getTimestamp());
    KWindowSystem::setShowingDesktop(showingDesktop);
    QVERIFY(spy.wait());
    QCOMPARE(KWindowSystem::showingDesktop(), showingDesktop);
    spy.clear();
}

void KWindowSystemX11Test::testWorkAreaChanged()
{
    // if there are multiple screens this test can fail as workarea is not multi screen aware
    QSignalSpy spy(KWindowSystem::self(), SIGNAL(workAreaChanged()));
    QSignalSpy strutSpy(KWindowSystem::self(), SIGNAL(strutChanged()));

    QWidget widget;
    widget.setGeometry(0, 0, 100, 10);
    widget.show();

    KWindowSystem::setExtendedStrut(widget.winId(), 10, 0, 10, 0, 0, 0, 100, 0, 100, 0, 0, 0);
    QVERIFY(spy.wait());
    QVERIFY(!spy.isEmpty());
    QVERIFY(!strutSpy.isEmpty());
}

void KWindowSystemX11Test::testWindowTitleChanged()
{
    qRegisterMetaType<WId>("WId");
    qRegisterMetaType<NET::Properties>("NET::Properties");
    qRegisterMetaType<NET::Properties2>("NET::Properties2");
    qRegisterMetaType<const unsigned long*>("const ulong*");
    QWidget widget;
    widget.setWindowTitle(QStringLiteral("foo"));
    widget.show();
    QVERIFY(QTest::qWaitForWindowExposed(&widget));

    // wait till the window is mapped, etc.
    QTest::qWait(200);

    QSignalSpy propertiesChangedSpy(KWindowSystem::self(), SIGNAL(windowChanged(WId,NET::Properties,NET::Properties2)));
    QSignalSpy propertyChangedSpy(KWindowSystem::self(), SIGNAL(windowChanged(WId,unsigned int)));
    QSignalSpy windowChangedSpy(KWindowSystem::self(), SIGNAL(windowChanged(WId)));

    QVERIFY(propertiesChangedSpy.isValid());
    QVERIFY(propertyChangedSpy.isValid());
    QVERIFY(windowChangedSpy.isValid());

    widget.setWindowTitle(QStringLiteral("bar"));
    QX11Info::setAppTime(QX11Info::getTimestamp());

    int counter = 0;
    bool gotWMName = false;
    while (propertiesChangedSpy.wait() && counter < 10) {
        for (auto it = propertiesChangedSpy.constBegin(); it != propertiesChangedSpy.constEnd(); ++it) {
            if ((*it).isEmpty()) {
                continue;
            }
            if ((*it).at(0).toULongLong() == widget.winId()) {
                NET::Properties props = (*it).at(1).value<NET::Properties>();
                if (props.testFlag(NET::WMName)) {
                    gotWMName = true;
                }
            }
        }
        if (gotWMName) {
            break;
        }
        propertiesChangedSpy.clear();
        counter++;
    }
    QVERIFY(gotWMName);

    gotWMName = false;
    QCOMPARE(propertyChangedSpy.isEmpty(), false);
    for (auto it = propertyChangedSpy.constBegin(); it != propertyChangedSpy.constEnd(); ++it) {
        if ((*it).isEmpty()) {
            continue;
        }
        if ((*it).at(0).toULongLong() == widget.winId()) {
            unsigned int props = (*it).at(1).value<unsigned int>();
            if (props & NET::WMName) {
                gotWMName = true;
            }
        }
        if (gotWMName) {
            break;
        }
    }
    QVERIFY(gotWMName);

    QCOMPARE(windowChangedSpy.isEmpty(), false);
    bool gotWindow = false;
    for (auto it = windowChangedSpy.constBegin(); it != windowChangedSpy.constEnd(); ++it) {
        if ((*it).isEmpty()) {
            continue;
        }
        if ((*it).at(0).toULongLong() == widget.winId()) {
            gotWindow = true;
        }
        if (gotWindow) {
            break;
        }
    }
    QVERIFY(gotWindow);

    // now let's verify the info in KWindowInfo
    // we wait a little bit more as openbox is updating the visible name
    QTest::qWait(500);
    KWindowInfo info(widget.winId(), NET::WMName | NET::WMVisibleName | NET::WMVisibleIconName | NET::WMIconName, NET::Properties2());
    QVERIFY(info.valid());
    const QString expectedName = QStringLiteral("bar");
    QCOMPARE(info.name(), expectedName);
    QCOMPARE(info.visibleName(), expectedName);
    QCOMPARE(info.visibleIconName(), expectedName);
    QCOMPARE(info.iconName(), expectedName);
}

void KWindowSystemX11Test::testMinimizeWindow()
{
    NETRootInfo rootInfo(QX11Info::connection(), NET::Supported | NET::SupportingWMCheck);
    if (qstrcmp(rootInfo.wmName(), "Openbox") != 0 &&
        qstrcmp(rootInfo.wmName(), "KWin") != 0) {
        QSKIP("Test minimize window might not be supported on the used window manager.");
    }
    QWidget widget;
    widget.show();
    QVERIFY(QTest::qWaitForWindowExposed(&widget));

    KWindowInfo info(widget.winId(), NET::WMState | NET::XAWMState);
    QVERIFY(!info.isMinimized());

    KWindowSystem::minimizeWindow(widget.winId());
    // create a roundtrip, updating minimized state is done by the window manager and wait a short time
    QX11Info::setAppTime(QX11Info::getTimestamp());
    QTest::qWait(200);

    KWindowInfo info2(widget.winId(), NET::WMState | NET::XAWMState);
    QVERIFY(info2.isMinimized());

    KWindowSystem::unminimizeWindow(widget.winId());
    // create a roundtrip, updating minimized state is done by the window manager and wait a short time
    QX11Info::setAppTime(QX11Info::getTimestamp());
    QTest::qWait(200);

    KWindowInfo info3(widget.winId(), NET::WMState | NET::XAWMState);
    QVERIFY(!info3.isMinimized());
}

void KWindowSystemX11Test::testPlatformX11()
{
    QCOMPARE(KWindowSystem::platform(), KWindowSystem::Platform::X11);
    QCOMPARE(KWindowSystem::isPlatformX11(), true);
    QCOMPARE(KWindowSystem::isPlatformWayland(), false);
}

QTEST_MAIN(KWindowSystemX11Test)

#include "kwindowsystemx11test.moc"
