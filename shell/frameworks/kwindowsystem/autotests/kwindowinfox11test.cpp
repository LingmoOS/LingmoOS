/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kwindowinfo.h"
#include "kwindowsystem.h"
#include "kx11extras.h"
#include "nettesthelper.h"
#include "netwm.h"

#include <QScreen>
#include <QSignalSpy>
#include <QSysInfo>
#include <private/qtx11extras_p.h>
#include <qtest_widgets.h>

#include <xcb/xcb_icccm.h>

#include <unistd.h>

Q_DECLARE_METATYPE(WId)
Q_DECLARE_METATYPE(NET::State)
Q_DECLARE_METATYPE(NET::States)
Q_DECLARE_METATYPE(NET::WindowType)
Q_DECLARE_METATYPE(NET::WindowTypeMask)
Q_DECLARE_METATYPE(NET::WindowTypes)
Q_DECLARE_METATYPE(NET::Properties)
Q_DECLARE_METATYPE(NET::Properties2)

class KWindowInfoX11Test : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();

    void testState_data();
    void testState();
    void testMinimized();
    void testMappingState();
    void testWindowType_data();
    void testWindowType();
    void testDesktop();
    void testActivities();
    void testWindowClass();
    void testWindowRole();
    void testClientMachine();
    void testName();
    void testTransientFor();
    void testGroupLeader();
    void testExtendedStrut();
    void testGeometry();
    void testDesktopFileName();
    void testPid();

    // actionSupported is not tested as it's too window manager specific
    // we could write a test against KWin's behavior, but that would fail on
    // build.kde.org as we use OpenBox there.

private:
    void showWidget(QWidget *widget);
    bool waitForWindow(QSignalSpy &spy, WId winId, NET::Properties property, NET::Properties2 properties2 = NET::Properties2()) const;
    bool verifyMinimized(WId window) const;

    std::unique_ptr<QWidget> window;
};

void KWindowInfoX11Test::initTestCase()
{
    QCoreApplication::setAttribute(Qt::AA_ForceRasterWidgets);
    qRegisterMetaType<NET::Properties>();
    qRegisterMetaType<NET::Properties2>();
}

bool KWindowInfoX11Test::waitForWindow(QSignalSpy &spy, WId winId, NET::Properties property, NET::Properties2 property2) const
{
    // we need to wait, window manager has to react and update the property.
    bool foundOurWindow = false;
    for (int i = 0; i < 10; ++i) {
        spy.wait(50);
        if (spy.isEmpty()) {
            continue;
        }
        for (auto it = spy.constBegin(); it != spy.constEnd(); ++it) {
            if (it->first().value<WId>() != winId) {
                continue;
            }
            if (property != NET::Properties()) {
                if (it->at(1).value<NET::Properties>() != property) {
                    continue;
                }
            }
            if (property2 != NET::Properties2()) {
                if (it->at(2).value<NET::Properties2>() != property2) {
                    continue;
                }
            }
            foundOurWindow = true;
            break;
        }
        if (foundOurWindow) {
            break;
        }
        spy.clear();
    }
    return foundOurWindow;
}

bool KWindowInfoX11Test::verifyMinimized(WId window) const
{
    KWindowInfo info(window, NET::WMState | NET::XAWMState);
    return info.isMinimized();
}

void KWindowInfoX11Test::init()
{
    // create the window and ensure it has been managed
    window.reset(new QWidget());
    showWidget(window.get());
}

void KWindowInfoX11Test::showWidget(QWidget *window)
{
    qRegisterMetaType<WId>("WId");
    QSignalSpy spy(KX11Extras::self(), &KX11Extras::windowAdded);
    window->show();
    bool foundOurWindow = false;
    for (int i = 0; i < 50; ++i) {
        spy.wait(50);
        if (spy.isEmpty()) {
            continue;
        }
        for (auto it = spy.constBegin(); it != spy.constEnd(); ++it) {
            if (it->isEmpty()) {
                continue;
            }
            if (it->first().value<WId>() == window->winId()) {
                foundOurWindow = true;
                break;
            }
        }
        if (foundOurWindow) {
            break;
        }
        spy.clear();
    }
}

void KWindowInfoX11Test::cleanup()
{
    // we hide the window and wait till it is gone so that we have a clean state in next test
    if (window && window->isVisible()) {
        WId id = window->winId();
        QSignalSpy spy(KX11Extras::self(), &KX11Extras::windowRemoved);
        window->hide();
        bool foundOurWindow = false;
        for (int i = 0; i < 50; ++i) {
            spy.wait(50);
            if (spy.isEmpty()) {
                continue;
            }
            for (auto it = spy.constBegin(); it != spy.constEnd(); ++it) {
                if (it->first().value<WId>() == id) {
                    foundOurWindow = true;
                    break;
                }
            }
            if (foundOurWindow) {
                break;
            }
            spy.clear();
        }
    }
    window.reset();
}

void KWindowInfoX11Test::testState_data()
{
    QTest::addColumn<NET::States>("state");

    QTest::newRow("max") << NET::States(NET::Max);
    QTest::newRow("maxHoriz") << NET::States(NET::MaxHoriz);
    QTest::newRow("shaded") << NET::States(NET::Shaded);
    QTest::newRow("skipTaskbar") << NET::States(NET::SkipTaskbar);
    QTest::newRow("skipPager") << NET::States(NET::SkipPager);
    QTest::newRow("keep above") << NET::States(NET::KeepAbove);
    QTest::newRow("keep below") << NET::States(NET::KeepBelow);
    QTest::newRow("fullscreen") << NET::States(NET::FullScreen);

    NETRootInfo info(QX11Info::connection(), NET::Supported);
    if (info.isSupported(NET::SkipSwitcher)) {
        QTest::newRow("skipSwitcher") << NET::States(NET::SkipSwitcher);
    }

    // NOTE: modal, sticky and hidden cannot be tested with this variant
    // demands attention is not tested as that's already part of the first run adjustments
}

void KWindowInfoX11Test::testState()
{
    QFETCH(NET::States, state);
    QX11Info::getTimestamp();

    KWindowInfo info(window->winId(), NET::WMState);
    QVERIFY(info.valid());
    // all states except demands attention
    for (int i = 0; i < 12; ++i) {
        QVERIFY(!info.hasState(NET::States(1 << i)));
    }

    QSignalSpy spy(KX11Extras::self(), &KX11Extras::windowChanged);
    QVERIFY(spy.isValid());
    // now we have a clean window and can do fun stuff
    KX11Extras::setState(window->winId(), state);

    QVERIFY(waitForWindow(spy, window->winId(), NET::WMState));

    KWindowInfo info3(window->winId(), NET::WMState);
    QVERIFY(info3.valid());
    QCOMPARE(int(info3.state()), int(state));
    QVERIFY(info3.hasState(state));
}

// This struct is defined here to avoid a dependency on xcb-icccm
struct kde_wm_hints {
    uint32_t flags;
    uint32_t input;
    int32_t initial_state;
    xcb_pixmap_t icon_pixmap;
    xcb_window_t icon_window;
    int32_t icon_x;
    int32_t icon_y;
    xcb_pixmap_t icon_mask;
    xcb_window_t window_group;
};

void KWindowInfoX11Test::testMinimized()
{
    // should not be minimized, now
    QVERIFY(!verifyMinimized(window->winId()));

    window->showMinimized();
    // TODO: improve by using signalspy?
    QTest::qWait(100);

    // should be minimized, now
    QVERIFY(verifyMinimized(window->winId()));

    // back to normal
    window->showNormal();
    // TODO: improve by using signalspy?
    QTest::qWait(100);

    // should no longer be minimized
    QVERIFY(!verifyMinimized(window->winId()));
}

void KWindowInfoX11Test::testMappingState()
{
    KWindowInfo info(window->winId(), NET::XAWMState);
    QCOMPARE(info.mappingState(), NET::Visible);

    window->showMinimized();
    // TODO: improve by using signalspy?
    QTest::qWait(100);
    KWindowInfo info2(window->winId(), NET::XAWMState);
    QCOMPARE(info2.mappingState(), NET::Iconic);

    window->hide();
    // TODO: improve by using signalspy?
    QTest::qWait(100);
    KWindowInfo info3(window->winId(), NET::XAWMState);
    QCOMPARE(info3.mappingState(), NET::Withdrawn);
}

void KWindowInfoX11Test::testWindowType_data()
{
    QTest::addColumn<NET::WindowTypeMask>("mask");
    QTest::addColumn<NET::WindowType>("type");
    QTest::addColumn<NET::WindowType>("expectedType");

    // clang-format off
    QTest::newRow("desktop")            << NET::DesktopMask      << NET::Desktop      << NET::Desktop;
    QTest::newRow("dock")               << NET::DockMask         << NET::Dock         << NET::Dock;
    QTest::newRow("toolbar")            << NET::ToolbarMask      << NET::Toolbar      << NET::Toolbar;
    QTest::newRow("menu")               << NET::MenuMask         << NET::Menu         << NET::Menu;
    QTest::newRow("dialog")             << NET::DialogMask       << NET::Dialog       << NET::Dialog;
    QTest::newRow("override")           << NET::OverrideMask     << NET::Override     << NET::Override;
    QTest::newRow("override as normal") << NET::NormalMask       << NET::Override     << NET::Normal;
    QTest::newRow("topmenu")            << NET::TopMenuMask      << NET::TopMenu      << NET::TopMenu;
    QTest::newRow("topmenu as dock")    << NET::DockMask         << NET::TopMenu      << NET::Dock;
    QTest::newRow("utility")            << NET::UtilityMask      << NET::Utility      << NET::Utility;
    QTest::newRow("utility as dialog")  << NET::DialogMask       << NET::Utility      << NET::Dialog;
    QTest::newRow("splash")             << NET::SplashMask       << NET::Splash       << NET::Splash;
    QTest::newRow("splash as dock")     << NET::DockMask         << NET::Splash       << NET::Dock;
    QTest::newRow("dropdownmenu")       << NET::DropdownMenuMask << NET::DropdownMenu << NET::DropdownMenu;
    QTest::newRow("popupmenu")          << NET::PopupMenuMask    << NET::PopupMenu    << NET::PopupMenu;
    QTest::newRow("popupmenu as menu")  << NET::MenuMask         << NET::Menu         << NET::Menu;
    QTest::newRow("tooltip")            << NET::TooltipMask      << NET::Tooltip      << NET::Tooltip;
    QTest::newRow("notification")       << NET::NotificationMask << NET::Notification << NET::Notification;
    QTest::newRow("ComboBox")           << NET::ComboBoxMask     << NET::ComboBox     << NET::ComboBox;
    QTest::newRow("DNDIcon")            << NET::DNDIconMask      << NET::DNDIcon      << NET::DNDIcon;
    QTest::newRow("OnScreenDisplay")    << NET::OnScreenDisplayMask << NET::OnScreenDisplay << NET::OnScreenDisplay;
    QTest::newRow("CriticalNotification") << NET::CriticalNotificationMask << NET::CriticalNotification << NET::CriticalNotification;
    QTest::newRow("AppletPopup")        << NET::AppletPopupMask  << NET::AppletPopup  << NET::AppletPopup;

    // incorrect masks
    QTest::newRow("desktop-unknown")      << NET::NormalMask << NET::Desktop      << NET::Unknown;
    QTest::newRow("dock-unknown")         << NET::NormalMask << NET::Dock         << NET::Unknown;
    QTest::newRow("toolbar-unknown")      << NET::NormalMask << NET::Toolbar      << NET::Unknown;
    QTest::newRow("menu-unknown")         << NET::NormalMask << NET::Menu         << NET::Unknown;
    QTest::newRow("dialog-unknown")       << NET::NormalMask << NET::Dialog       << NET::Unknown;
    QTest::newRow("override-unknown")     << NET::DialogMask << NET::Override     << NET::Unknown;
    QTest::newRow("topmenu-unknown")      << NET::NormalMask << NET::TopMenu      << NET::Unknown;
    QTest::newRow("utility-unknown")      << NET::NormalMask << NET::Utility      << NET::Unknown;
    QTest::newRow("splash-unknown")       << NET::NormalMask << NET::Splash       << NET::Unknown;
    QTest::newRow("dropdownmenu-unknown") << NET::NormalMask << NET::DropdownMenu << NET::Unknown;
    QTest::newRow("popupmenu-unknown")    << NET::NormalMask << NET::PopupMenu    << NET::Unknown;
    QTest::newRow("tooltip-unknown")      << NET::NormalMask << NET::Tooltip      << NET::Unknown;
    QTest::newRow("notification-unknown") << NET::NormalMask << NET::Notification << NET::Unknown;
    QTest::newRow("ComboBox-unknown")     << NET::NormalMask << NET::ComboBox     << NET::Unknown;
    QTest::newRow("DNDIcon-unknown")      << NET::NormalMask << NET::DNDIcon      << NET::Unknown;
    QTest::newRow("OnScreenDisplay-unknown") << NET::NormalMask << NET::OnScreenDisplay << NET::Unknown;
    QTest::newRow("CriticalNotification-unknown") << NET::NormalMask << NET::CriticalNotification << NET::Unknown;
    QTest::newRow("AppletPopup-unknown")  << NET::NormalMask << NET::AppletPopup  << NET::Unknown;
    // clang-format on
}

void KWindowInfoX11Test::testWindowType()
{
    KWindowInfo info(window->winId(), NET::WMWindowType);
    QCOMPARE(info.windowType(NET::NormalMask), NET::Normal);

    QFETCH(NET::WindowTypeMask, mask);
    QFETCH(NET::WindowType, type);
    QFETCH(NET::WindowType, expectedType);

    KX11Extras::setType(window->winId(), type);
    // setWindowType just changes an xproperty, so a roundtrip waiting for another property ensures we are updated
    QX11Info::getTimestamp();
    KWindowInfo info2(window->winId(), NET::WMWindowType);
    QCOMPARE(info2.windowType(mask), expectedType);
}

void KWindowInfoX11Test::testDesktop()
{
    if (KX11Extras::numberOfDesktops() < 2) {
        QSKIP("We need at least two virtual desktops to perform proper virtual desktop testing");
    }
    KWindowInfo info(window->winId(), NET::WMDesktop);
    QVERIFY(info.isOnCurrentDesktop());
    QVERIFY(!info.onAllDesktops());
    QCOMPARE(info.desktop(), KX11Extras::currentDesktop());
    for (int i = 1; i < KX11Extras::numberOfDesktops(); i++) {
        if (i == KX11Extras::currentDesktop()) {
            QVERIFY(info.isOnDesktop(i));
        } else {
            QVERIFY(!info.isOnDesktop(i));
        }
    }

    // set on all desktop
    QSignalSpy spy(KX11Extras::self(), &KX11Extras::windowChanged);
    QVERIFY(spy.isValid());
    KX11Extras::setOnAllDesktops(window->winId(), true);
    QVERIFY(waitForWindow(spy, window->winId(), NET::WMDesktop));

    KWindowInfo info2(window->winId(), NET::WMDesktop);
    QVERIFY(info2.isOnCurrentDesktop());
    QVERIFY(info2.onAllDesktops());
    QCOMPARE(info2.desktop(), int(NET::OnAllDesktops));
    for (int i = 1; i < KX11Extras::numberOfDesktops(); i++) {
        QVERIFY(info2.isOnDesktop(i));
    }

    const int desktop = (KX11Extras::currentDesktop() % KX11Extras::numberOfDesktops()) + 1;
    spy.clear();
    KX11Extras::setOnDesktop(window->winId(), desktop);
    QX11Info::getTimestamp();
    QVERIFY(waitForWindow(spy, window->winId(), NET::WMDesktop));

    KWindowInfo info3(window->winId(), NET::WMDesktop);
    QVERIFY(!info3.isOnCurrentDesktop());
    QVERIFY(!info3.onAllDesktops());
    QCOMPARE(info3.desktop(), desktop);
    for (int i = 1; i < KX11Extras::numberOfDesktops(); i++) {
        if (i == desktop) {
            QVERIFY(info3.isOnDesktop(i));
        } else {
            QVERIFY(!info3.isOnDesktop(i));
        }
    }
}

void KWindowInfoX11Test::testActivities()
{
    NETRootInfo rootInfo(QX11Info::connection(), NET::Supported | NET::SupportingWMCheck);

    QSignalSpy spyReal(KX11Extras::self(), &KX11Extras::windowChanged);
    QVERIFY(spyReal.isValid());

    KWindowInfo info(window->winId(), NET::Properties(), NET::WM2Activities);
    QVERIFY(info.valid());

    QStringList startingActivities = info.activities();

    // The window is either on a specific activity when created,
    // or on all of them (aka startingActivities is empty or contains
    // just one element)
    QVERIFY(startingActivities.size() <= 1);

    // Window on all activities
    KX11Extras::self()->setOnActivities(window->winId(), QStringList());

    QVERIFY(waitForWindow(spyReal, window->winId(), NET::Properties(), NET::WM2Activities));

    KWindowInfo info2(window->winId(), NET::Properties(), NET::WM2Activities);

    QVERIFY(info2.activities().size() == 0);

    // Window on a specific activity
    KX11Extras::self()->setOnActivities(window->winId(), QStringList() << "test-activity");

    QVERIFY(waitForWindow(spyReal, window->winId(), NET::Properties(), NET::WM2Activities));

    KWindowInfo info3(window->winId(), NET::Properties(), NET::WM2Activities);

    QVERIFY(info3.activities().size() == 1);
    QVERIFY(info3.activities()[0] == "test-activity");

    // Window on two specific activities
    KX11Extras::self()->setOnActivities(window->winId(), QStringList{"test-activity", "test-activity2"});

    QVERIFY(waitForWindow(spyReal, window->winId(), NET::Properties(), NET::WM2Activities));

    KWindowInfo info4(window->winId(), NET::Properties(), NET::WM2Activities);

    QCOMPARE(info4.activities().size(), 2);
    QVERIFY(info4.activities()[0] == "test-activity");
    QVERIFY(info4.activities()[1] == "test-activity2");

    // Window on the starting activity
    KX11Extras::self()->setOnActivities(window->winId(), startingActivities);

    QVERIFY(waitForWindow(spyReal, window->winId(), NET::Properties(), NET::WM2Activities));

    KWindowInfo info5(window->winId(), NET::Properties(), NET::WM2Activities);

    QVERIFY(info5.activities() == startingActivities);
}

void KWindowInfoX11Test::testWindowClass()
{
    KWindowInfo info(window->winId(), NET::Properties(), NET::WM2WindowClass);
    QCOMPARE(info.windowClassName(), QByteArrayLiteral("kwindowinfox11test"));
    QCOMPARE(info.windowClassClass(), QByteArrayLiteral("kwindowinfox11test"));

    // window class needs to be changed using xcb
    xcb_change_property(QX11Info::connection(), XCB_PROP_MODE_REPLACE, window->winId(), XCB_ATOM_WM_CLASS, XCB_ATOM_STRING, 8, 7, "foo\0bar");
    xcb_flush(QX11Info::connection());

    // it's just a property change so we can easily refresh
    QX11Info::getTimestamp();

    KWindowInfo info2(window->winId(), NET::Properties(), NET::WM2WindowClass);
    QCOMPARE(info2.windowClassName(), QByteArrayLiteral("foo"));
    QCOMPARE(info2.windowClassClass(), QByteArrayLiteral("bar"));
}

void KWindowInfoX11Test::testWindowRole()
{
    KWindowInfo info(window->winId(), NET::Properties(), NET::WM2WindowRole);
    QVERIFY(info.windowRole().isNull());

    // window role needs to be changed using xcb
    KXUtils::Atom atom(QX11Info::connection(), QByteArrayLiteral("WM_WINDOW_ROLE"));
    xcb_change_property(QX11Info::connection(), XCB_PROP_MODE_REPLACE, window->winId(), atom, XCB_ATOM_STRING, 8, 3, "bar");
    xcb_flush(QX11Info::connection());

    // it's just a property change so we can easily refresh
    QX11Info::getTimestamp();

    KWindowInfo info2(window->winId(), NET::Properties(), NET::WM2WindowRole);
    QCOMPARE(info2.windowRole(), QByteArrayLiteral("bar"));
}

void KWindowInfoX11Test::testClientMachine()
{
    const QByteArray oldHostName = QSysInfo::machineHostName().toLocal8Bit();

    KWindowInfo info(window->winId(), NET::Properties(), NET::WM2ClientMachine);
    QCOMPARE(info.clientMachine(), oldHostName);

    // client machine needs to be set through xcb
    const QByteArray newHostName = oldHostName + "2";
    xcb_change_property(QX11Info::connection(),
                        XCB_PROP_MODE_REPLACE,
                        window->winId(),
                        XCB_ATOM_WM_CLIENT_MACHINE,
                        XCB_ATOM_STRING,
                        8,
                        newHostName.size(),
                        newHostName.data());
    xcb_flush(QX11Info::connection());

    // it's just a property change so we can easily refresh
    QX11Info::getTimestamp();

    KWindowInfo info2(window->winId(), NET::Properties(), NET::WM2ClientMachine);
    QCOMPARE(info2.clientMachine(), newHostName);
}

void KWindowInfoX11Test::testName()
{
    // clang-format off
    KWindowInfo info(window->winId(), NET::WMName | NET::WMVisibleName | NET::WMIconName | NET::WMVisibleIconName | NET::WMState | NET::XAWMState);
    QCOMPARE(info.name(),                     QStringLiteral("kwindowinfox11test"));
    QCOMPARE(info.visibleName(),              QStringLiteral("kwindowinfox11test"));
    QCOMPARE(info.visibleNameWithState(),     QStringLiteral("kwindowinfox11test"));
    QCOMPARE(info.iconName(),                 QStringLiteral("kwindowinfox11test"));
    QCOMPARE(info.visibleIconName(),          QStringLiteral("kwindowinfox11test"));
    QCOMPARE(info.visibleIconNameWithState(), QStringLiteral("kwindowinfox11test"));

    window->showMinimized();
    // TODO: improve by using signalspy?
    QTest::qWait(100);
    // should be minimized, now
    QVERIFY(verifyMinimized(window->winId()));

    // that should have changed the visible name
    KWindowInfo info2(window->winId(), NET::WMName | NET::WMVisibleName | NET::WMIconName | NET::WMVisibleIconName | NET::WMState | NET::XAWMState);
    QCOMPARE(info2.name(),                     QStringLiteral("kwindowinfox11test"));
    QCOMPARE(info2.visibleName(),              QStringLiteral("kwindowinfox11test"));
    QCOMPARE(info2.visibleNameWithState(),     QStringLiteral("(kwindowinfox11test)"));
    QCOMPARE(info2.iconName(),                 QStringLiteral("kwindowinfox11test"));
    QCOMPARE(info2.visibleIconName(),          QStringLiteral("kwindowinfox11test"));
    QCOMPARE(info2.visibleIconNameWithState(), QStringLiteral("(kwindowinfox11test)"));

    NETRootInfo rootInfo(QX11Info::connection(), NET::Supported | NET::SupportingWMCheck);
    if (qstrcmp(rootInfo.wmName(), "Openbox") == 0) {
        QSKIP("setting name test fails on openbox");
    }

    // create a low level NETWinInfo to manipulate the name
    NETWinInfo winInfo(QX11Info::connection(), window->winId(), QX11Info::appRootWindow(), NET::WMName, NET::Properties2());
    winInfo.setName("foobar");

    QX11Info::getTimestamp();

    KWindowInfo info3(window->winId(), NET::WMName | NET::WMVisibleName | NET::WMIconName | NET::WMVisibleIconName | NET::WMState | NET::XAWMState);
    QCOMPARE(info3.name(),                     QStringLiteral("foobar"));
    QCOMPARE(info3.visibleName(),              QStringLiteral("foobar"));
    QCOMPARE(info3.visibleNameWithState(),     QStringLiteral("(foobar)"));
    QCOMPARE(info3.iconName(),                 QStringLiteral("foobar"));
    QCOMPARE(info3.visibleIconName(),          QStringLiteral("foobar"));
    QCOMPARE(info3.visibleIconNameWithState(), QStringLiteral("(foobar)"));
    // clang-format on
}

void KWindowInfoX11Test::testTransientFor()
{
    KWindowInfo info(window->winId(), NET::Properties(), NET::WM2TransientFor);
    QCOMPARE(info.transientFor(), WId(0));

    // let's create a second window
    std::unique_ptr<QWidget> window2(new QWidget());
    window2->show();
    QVERIFY(QTest::qWaitForWindowExposed(window2.get()));

    // update the transient for of window1 to window2
    const uint32_t id = window2->winId();
    xcb_change_property(QX11Info::connection(), XCB_PROP_MODE_REPLACE, window->winId(), XCB_ATOM_WM_TRANSIENT_FOR, XCB_ATOM_WINDOW, 32, 1, &id);
    xcb_flush(QX11Info::connection());

    KWindowInfo info2(window->winId(), NET::Properties(), NET::WM2TransientFor);
    QCOMPARE(info2.transientFor(), window2->winId());
}

void KWindowInfoX11Test::testGroupLeader()
{
    // WM_CLIENT_LEADER is set by default
    KWindowInfo info1(window->winId(), NET::Properties(), NET::WM2GroupLeader);
    QVERIFY(info1.groupLeader() != XCB_WINDOW_NONE);

    xcb_connection_t *connection = QX11Info::connection();
    xcb_window_t rootWindow = QX11Info::appRootWindow();

    xcb_window_t leader = xcb_generate_id(connection);
    xcb_create_window(connection, XCB_COPY_FROM_PARENT, leader, rootWindow, 0, 0, 1, 1, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT, XCB_COPY_FROM_PARENT, 0, nullptr);

    xcb_icccm_wm_hints_t hints = {};
    hints.flags = XCB_ICCCM_WM_HINT_WINDOW_GROUP;
    hints.window_group = leader;
    xcb_icccm_set_wm_hints(connection, leader, &hints);
    xcb_icccm_set_wm_hints(connection, window->winId(), &hints);

    KWindowInfo info2(window->winId(), NET::Properties(), NET::WM2GroupLeader);
    QCOMPARE(info2.groupLeader(), leader);
}

void KWindowInfoX11Test::testExtendedStrut()
{
    KWindowInfo info(window->winId(), NET::Properties(), NET::WM2ExtendedStrut);
    NETExtendedStrut strut = info.extendedStrut();
    QCOMPARE(strut.bottom_end, 0);
    QCOMPARE(strut.bottom_start, 0);
    QCOMPARE(strut.bottom_width, 0);
    QCOMPARE(strut.left_end, 0);
    QCOMPARE(strut.left_start, 0);
    QCOMPARE(strut.left_width, 0);
    QCOMPARE(strut.right_end, 0);
    QCOMPARE(strut.right_start, 0);
    QCOMPARE(strut.right_width, 0);
    QCOMPARE(strut.top_end, 0);
    QCOMPARE(strut.top_start, 0);
    QCOMPARE(strut.top_width, 0);

    KX11Extras::setExtendedStrut(window->winId(), 10, 20, 30, 40, 5, 15, 25, 35, 2, 12, 22, 32);

    // it's just an xprop, so one roundtrip is good enough
    QX11Info::getTimestamp();

    KWindowInfo info2(window->winId(), NET::Properties(), NET::WM2ExtendedStrut);
    strut = info2.extendedStrut();
    QCOMPARE(strut.bottom_end, 32);
    QCOMPARE(strut.bottom_start, 22);
    QCOMPARE(strut.bottom_width, 12);
    QCOMPARE(strut.left_end, 30);
    QCOMPARE(strut.left_start, 20);
    QCOMPARE(strut.left_width, 10);
    QCOMPARE(strut.right_end, 15);
    QCOMPARE(strut.right_start, 5);
    QCOMPARE(strut.right_width, 40);
    QCOMPARE(strut.top_end, 2);
    QCOMPARE(strut.top_start, 35);
    QCOMPARE(strut.top_width, 25);
}

void KWindowInfoX11Test::testGeometry()
{
    KWindowInfo info(window->winId(), NET::WMGeometry | NET::WMFrameExtents);
    QCOMPARE(info.geometry().size(), window->geometry().size());
    QCOMPARE(info.frameGeometry().size(), window->frameGeometry().size());

    QSignalSpy spy(KX11Extras::self(), &KX11Extras::windowChanged);
    QVERIFY(spy.isValid());

    // this is tricky, KWin is smart and doesn't allow all geometries we pass in
    // setting to center of screen should work, though
    QRect geo(window->windowHandle()->screen()->geometry().center() - QPoint(window->width() / 2 - 5, window->height() / 2 - 5),
              window->size() + QSize(10, 10));
    window->setGeometry(geo);
    waitForWindow(spy, window->winId(), NET::WMGeometry);

    KWindowInfo info2(window->winId(), NET::WMGeometry | NET::WMFrameExtents);
    QCOMPARE(info2.geometry(), window->geometry());
    QCOMPARE(info2.geometry(), geo);
    QCOMPARE(info2.frameGeometry(), window->frameGeometry());
}

void KWindowInfoX11Test::testDesktopFileName()
{
    KWindowInfo info(window->winId(), NET::Properties(), NET::WM2DesktopFileName);
    QVERIFY(info.valid());
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 1)
    QCOMPARE(info.desktopFileName(), "kwindowinfox11test");
#else
    QCOMPARE(info.desktopFileName(), QString());
#endif
    QSignalSpy spy(KX11Extras::self(), &KX11Extras::windowChanged);
    QVERIFY(spy.isValid());
    // create a NETWinInfo to set the desktop file name
    NETWinInfo netInfo(QX11Info::connection(), window->winId(), QX11Info::appRootWindow(), NET::Properties(), NET::Properties2());
    netInfo.setDesktopFileName("org.kde.foo");
    xcb_flush(QX11Info::connection());

    // it's just a property change so we can easily refresh
    QX11Info::getTimestamp();
    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).value<WId>(), window->winId());
    QCOMPARE(spy.first().at(2).value<NET::Properties2>(), NET::Properties2(NET::WM2DesktopFileName));

    KWindowInfo info2(window->winId(), NET::Properties(), NET::WM2DesktopFileName);
    QVERIFY(info2.valid());
    QCOMPARE(info2.desktopFileName(), QByteArrayLiteral("org.kde.foo"));
}

void KWindowInfoX11Test::testPid()
{
    KWindowInfo info(window->winId(), NET::WMPid);
    QVERIFY(info.valid());
    QCOMPARE(info.pid(), getpid());
}

QTEST_MAIN(KWindowInfoX11Test)

#include "kwindowinfox11test.moc"
