/*
    SPDX-FileCopyrightText: 2013 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "kwindowinfo.h"
#include "kwindowsystem.h"
#include "kx11extras.h"
#include "nettesthelper.h"
#include "netwm.h"

#include <QSignalSpy>
#include <QWidget>
#include <private/qtx11extras_p.h>

#include <qtest_widgets.h>
Q_DECLARE_METATYPE(WId)
Q_DECLARE_METATYPE(NET::Properties)
Q_DECLARE_METATYPE(NET::Properties2)
Q_DECLARE_METATYPE(const unsigned long *)

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
    QSignalSpy spy(KX11Extras::self(), &KX11Extras::activeWindowChanged);

    std::unique_ptr<QWidget> widget(new QWidget);
    widget->show();

    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).toULongLong(), widget->winId());
    QCOMPARE(KX11Extras::activeWindow(), widget->winId());
}

void KWindowSystemX11Test::testWindowAdded()
{
    qRegisterMetaType<WId>("WId");
    QSignalSpy spy(KX11Extras::self(), &KX11Extras::windowAdded);
    QSignalSpy stackingOrderSpy(KX11Extras::self(), &KX11Extras::stackingOrderChanged);
    std::unique_ptr<QWidget> widget(new QWidget);
    widget->show();
    QVERIFY(QTest::qWaitForWindowExposed(widget.get()));
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
    QVERIFY(KX11Extras::hasWId(widget->winId()));
    QVERIFY(!stackingOrderSpy.isEmpty());
}

void KWindowSystemX11Test::testWindowRemoved()
{
    qRegisterMetaType<WId>("WId");
    std::unique_ptr<QWidget> widget(new QWidget);
    widget->show();
    QVERIFY(QTest::qWaitForWindowExposed(widget.get()));
    QVERIFY(KX11Extras::hasWId(widget->winId()));

    QSignalSpy spy(KX11Extras::self(), &KX11Extras::windowRemoved);
    widget->hide();
    spy.wait(1000);
    QCOMPARE(spy.first().at(0).toULongLong(), widget->winId());
    QVERIFY(!KX11Extras::hasWId(widget->winId()));
}

void KWindowSystemX11Test::testDesktopChanged()
{
    // This test requires a running NETWM-compliant window manager
    if (KX11Extras::numberOfDesktops() == 1) {
        QSKIP("At least two virtual desktops are required to test desktop changed");
    }
    const int current = KX11Extras::currentDesktop();

    QSignalSpy spy(KX11Extras::self(), &KX11Extras::currentDesktopChanged);
    int newDesktop = current + 1;
    if (newDesktop > KX11Extras::numberOfDesktops()) {
        newDesktop = 1;
    }
    KX11Extras::setCurrentDesktop(newDesktop);
    QVERIFY(spy.wait());
    QCOMPARE(KX11Extras::currentDesktop(), newDesktop);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).toInt(), newDesktop);
    spy.clear();

    // setting to current desktop should not change anything
    KX11Extras::setCurrentDesktop(newDesktop);

    // set back for clean state
    KX11Extras::setCurrentDesktop(current);
    QVERIFY(spy.wait());
    QCOMPARE(KX11Extras::currentDesktop(), current);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).toInt(), current);
}

void KWindowSystemX11Test::testNumberOfDesktopsChanged()
{
    // This test requires a running NETWM-compliant window manager
    const int oldNumber = KX11Extras::numberOfDesktops();
    QSignalSpy spy(KX11Extras::self(), &KX11Extras::numberOfDesktopsChanged);

    // KWin has arbitrary max number of 20 desktops, so don't fail the test if we use +1
    const int newNumber = oldNumber < 20 ? oldNumber + 1 : oldNumber - 1;

    NETRootInfo info(QX11Info::connection(), NET::NumberOfDesktops, NET::Properties2());
    info.setNumberOfDesktops(newNumber);

    QVERIFY(spy.wait());
    QCOMPARE(KX11Extras::numberOfDesktops(), newNumber);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).toInt(), newNumber);
    spy.clear();

    // setting to same number should not change
    info.setNumberOfDesktops(newNumber);

    // set back for clean state
    info.setNumberOfDesktops(oldNumber);
    QVERIFY(spy.wait());
    QCOMPARE(KX11Extras::numberOfDesktops(), oldNumber);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).toInt(), oldNumber);
}

void KWindowSystemX11Test::testDesktopNamesChanged()
{
    // This test requires a running NETWM-compliant window manager
    const QString origName = KX11Extras::desktopName(KX11Extras::currentDesktop());
    QSignalSpy spy(KX11Extras::self(), &KX11Extras::desktopNamesChanged);

    const QString testName = QStringLiteral("testFooBar");

    KX11Extras::setDesktopName(KX11Extras::currentDesktop(), testName);
    QVERIFY(spy.wait());
    QCOMPARE(KX11Extras::desktopName(KX11Extras::currentDesktop()), testName);
    QCOMPARE(spy.count(), 1);
    spy.clear();

    QX11Info::setAppTime(QX11Info::getTimestamp());

    // setting back to clean state
    KX11Extras::setDesktopName(KX11Extras::currentDesktop(), origName);
    QVERIFY(spy.wait());
    QCOMPARE(KX11Extras::desktopName(KX11Extras::currentDesktop()), origName);
    QCOMPARE(spy.count(), 1);
}

void KWindowSystemX11Test::testShowingDesktopChanged()
{
    QX11Info::setAppTime(QX11Info::getTimestamp());
    const bool showingDesktop = KWindowSystem::showingDesktop();

    QSignalSpy spy(KWindowSystem::self(), &KWindowSystem::showingDesktopChanged);

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
    QSignalSpy spy(KWindowSystem::self(), &KWindowSystem::showingDesktopChanged);
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
    QSignalSpy spy(KX11Extras::self(), &KX11Extras::workAreaChanged);
    QSignalSpy strutSpy(KX11Extras::self(), &KX11Extras::strutChanged);

    QWidget widget;
    widget.setGeometry(0, 0, 100, 10);
    widget.show();

    KX11Extras::setExtendedStrut(widget.winId(), 10, 0, 10, 0, 0, 0, 100, 0, 100, 0, 0, 0);
    QVERIFY(spy.wait());
    QVERIFY(!spy.isEmpty());
    QVERIFY(!strutSpy.isEmpty());
}

void KWindowSystemX11Test::testWindowTitleChanged()
{
    qRegisterMetaType<WId>("WId");
    qRegisterMetaType<NET::Properties>("NET::Properties");
    qRegisterMetaType<NET::Properties2>("NET::Properties2");
    qRegisterMetaType<const unsigned long *>("const ulong*");
    QWidget widget;
    widget.setWindowTitle(QStringLiteral("foo"));
    widget.show();
    QVERIFY(QTest::qWaitForWindowExposed(&widget));

    // wait till the window is mapped, etc.
    QTest::qWait(200);

    QSignalSpy propertiesChangedSpy(KX11Extras::self(), &KX11Extras::windowChanged);
    QVERIFY(propertiesChangedSpy.isValid());

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
    if (qstrcmp(rootInfo.wmName(), "Openbox") != 0 && qstrcmp(rootInfo.wmName(), "KWin") != 0) {
        QSKIP("Test minimize window might not be supported on the used window manager.");
    }
    QWidget widget;
    widget.show();
    QVERIFY(QTest::qWaitForWindowExposed(&widget));

    KWindowInfo info(widget.winId(), NET::WMState | NET::XAWMState);
    QVERIFY(!info.isMinimized());

    KX11Extras::minimizeWindow(widget.winId());
    // create a roundtrip, updating minimized state is done by the window manager and wait a short time
    QX11Info::setAppTime(QX11Info::getTimestamp());
    QTest::qWait(200);

    KWindowInfo info2(widget.winId(), NET::WMState | NET::XAWMState);
    QVERIFY(info2.isMinimized());

    KX11Extras::unminimizeWindow(widget.winId());
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
