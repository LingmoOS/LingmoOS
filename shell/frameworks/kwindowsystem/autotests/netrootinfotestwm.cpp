/*
    SPDX-FileCopyrightText: 2013 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "nettesthelper.h"
#include <netwm.h>

#include <QProcess>
#include <QStandardPaths>
#include <qtest_widgets.h>

// system
#include <unistd.h>

using Property = UniqueCPointer<xcb_get_property_reply_t>;

Q_DECLARE_METATYPE(NET::Orientation)
Q_DECLARE_METATYPE(NET::DesktopLayoutCorner)

static const char *s_wmName = "netrootinfotest";

class NetRootInfoTestWM : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testCtor();
    void testSupported();
    void testClientList();
    void testClientListStacking();
    void testNumberOfDesktops();
    void testCurrentDesktop();
    void testDesktopNames();
    void testDesktopLayout_data();
    void testDesktopLayout();
    void testDesktopGeometry();
    void testDesktopViewports();
    void testShowingDesktop_data();
    void testShowingDesktop();
    void testWorkArea();
    void testActiveWindow();
    void testVirtualRoots();
    void testDontCrashMapViewports();

private:
    void waitForPropertyChange(NETRootInfo *info, xcb_atom_t atom, NET::Property prop, NET::Property2 prop2 = NET::Property2(0));
    xcb_connection_t *connection()
    {
        return m_connection;
    }
    xcb_connection_t *m_connection;
    QList<xcb_connection_t *> m_connections;
    std::unique_ptr<QProcess> m_xvfb;
    xcb_window_t m_supportWindow;
    xcb_window_t m_rootWindow;
};

void NetRootInfoTestWM::cleanupTestCase()
{
    while (!m_connections.isEmpty()) {
        xcb_disconnect(m_connections.takeFirst());
    }
}

void NetRootInfoTestWM::initTestCase()
{
}

void NetRootInfoTestWM::init()
{
    // first reset just to be sure
    m_connection = nullptr;
    m_supportWindow = XCB_WINDOW_NONE;
    // start Xvfb
    const QString xfvbExec = QStandardPaths::findExecutable(QStringLiteral("Xvfb"));
    QVERIFY(!xfvbExec.isEmpty());

    m_xvfb.reset(new QProcess);
    // use pipe to pass fd to Xvfb to get back the display id
    int pipeFds[2];
    QVERIFY(pipe(pipeFds) == 0);
    m_xvfb->start(xfvbExec, QStringList{QStringLiteral("-displayfd"), QString::number(pipeFds[1])});
    QVERIFY(m_xvfb->waitForStarted());
    QCOMPARE(m_xvfb->state(), QProcess::Running);

    // reads from pipe, closes write side
    close(pipeFds[1]);

    QFile readPipe;
    QVERIFY(readPipe.open(pipeFds[0], QIODevice::ReadOnly, QFileDevice::AutoCloseHandle));
    QByteArray displayNumber = readPipe.readLine();
    readPipe.close();

    displayNumber.prepend(QByteArray(":"));
    displayNumber.remove(displayNumber.size() - 1, 1);

    // create X connection
    int screen = 0;
    m_connection = xcb_connect(displayNumber.constData(), &screen);
    QVERIFY(m_connection);
    QVERIFY(!xcb_connection_has_error(m_connection));
    m_rootWindow = KXUtils::rootWindow(m_connection, screen);
    uint32_t values[] = {XCB_EVENT_MASK_PROPERTY_CHANGE};
    xcb_change_window_attributes(m_connection, m_rootWindow, XCB_CW_EVENT_MASK, values);

    // create support window
    values[0] = true;
    m_supportWindow = xcb_generate_id(m_connection);
    xcb_create_window(m_connection,
                      XCB_COPY_FROM_PARENT,
                      m_supportWindow,
                      m_rootWindow,
                      0,
                      0,
                      1,
                      1,
                      0,
                      XCB_COPY_FROM_PARENT,
                      XCB_COPY_FROM_PARENT,
                      XCB_CW_OVERRIDE_REDIRECT,
                      values);
    const uint32_t lowerValues[] = {XCB_STACK_MODE_BELOW};
    // we need to do the lower window with a roundtrip, otherwise NETRootInfo is not functioning
    UniqueCPointer<xcb_generic_error_t> error(
        xcb_request_check(m_connection, xcb_configure_window_checked(m_connection, m_supportWindow, XCB_CONFIG_WINDOW_STACK_MODE, lowerValues)));
    QVERIFY(!error);
}

void NetRootInfoTestWM::cleanup()
{
    // destroy support window
    xcb_destroy_window(connection(), m_supportWindow);
    m_supportWindow = XCB_WINDOW_NONE;

    // close connection
    // delay till clenupTestCase as otherwise xcb reuses the same memory address
    m_connections << connection();
    // kill Xvfb
    m_xvfb->terminate();
    m_xvfb->waitForFinished();
    m_xvfb.reset();
}

void NetRootInfoTestWM::waitForPropertyChange(NETRootInfo *info, xcb_atom_t atom, NET::Property prop, NET::Property2 prop2)
{
    while (true) {
        UniqueCPointer<xcb_generic_event_t> event(xcb_wait_for_event(connection()));
        if (!event) {
            break;
        }
        if ((event->response_type & ~0x80) != XCB_PROPERTY_NOTIFY) {
            continue;
        }
        xcb_property_notify_event_t *pe = reinterpret_cast<xcb_property_notify_event_t *>(event.get());
        if (pe->window != m_rootWindow) {
            continue;
        }
        if (pe->atom != atom) {
            continue;
        }
        NET::Properties dirty;
        NET::Properties2 dirty2;
        info->event(event.get(), &dirty, &dirty2);
        if (prop != 0) {
            QVERIFY(dirty & prop);
        }
        if (prop2 != 0) {
            QVERIFY(dirty2 & prop2);
        }
        if (!prop) {
            QCOMPARE(dirty, NET::Properties());
        }
        if (!prop2) {
            QCOMPARE(dirty2, NET::Properties2());
        }
        break;
    }
}

void NetRootInfoTestWM::testCtor()
{
    QVERIFY(connection());
    NETRootInfo
        rootInfo(connection(), m_supportWindow, s_wmName, NET::WMAllProperties, NET::AllTypesMask, NET::States(~0u), NET::WM2AllProperties, NET::Actions(~0u));
    QCOMPARE(rootInfo.xcbConnection(), connection());
    QCOMPARE(rootInfo.rootWindow(), m_rootWindow);
    QCOMPARE(rootInfo.supportWindow(), m_supportWindow);
    QCOMPARE(rootInfo.wmName(), s_wmName);
    QCOMPARE(rootInfo.supportedProperties(), NET::WMAllProperties);
    QCOMPARE(rootInfo.supportedProperties2(), NET::WM2AllProperties);
    QCOMPARE(rootInfo.supportedActions(), NET::Actions(~0u));
    QCOMPARE(rootInfo.supportedStates(), NET::States(~0u));
    QCOMPARE(rootInfo.supportedWindowTypes(), NET::AllTypesMask);

    QCOMPARE(rootInfo.passedProperties(), NET::WMAllProperties);
    QCOMPARE(rootInfo.passedProperties2(), NET::WM2AllProperties);
    QCOMPARE(rootInfo.passedActions(), NET::Actions(~0u));
    QCOMPARE(rootInfo.passedStates(), NET::States(~0u));
    QCOMPARE(rootInfo.passedWindowTypes(), NET::AllTypesMask);
}

void NetRootInfoTestWM::testSupported()
{
    KXUtils::Atom supported(connection(), QByteArrayLiteral("_NET_SUPPORTED"));
    KXUtils::Atom wmCheck(connection(), QByteArrayLiteral("_NET_SUPPORTING_WM_CHECK"));
    KXUtils::Atom wmName(connection(), QByteArrayLiteral("_NET_WM_NAME"));
    KXUtils::Atom utf8String(connection(), QByteArrayLiteral("UTF8_STRING"));
    QVERIFY(connection());
    NETRootInfo
        rootInfo(connection(), m_supportWindow, s_wmName, NET::WMAllProperties, NET::AllTypesMask, NET::States(~0u), NET::WM2AllProperties, NET::Actions(~0u));
    int count = 0;
    for (int i = 0; i < 34; ++i) {
        if (i == 12) {
            continue;
        }
        QVERIFY(rootInfo.isSupported(NET::Property(1 << i)));
        count++;
    }
    for (int i = 0; i < 22; ++i) {
        QVERIFY(rootInfo.isSupported(NET::Property2(1 << i)));
        count++;
    }

    QVERIFY(rootInfo.isSupported(NET::WM2GTKShowWindowMenu));
    count++;

    for (int i = 0; i < 17; ++i) {
        QVERIFY(rootInfo.isSupported(NET::WindowTypeMask(1 << i)));
        count++;
    }
    for (int i = 0; i < 13; ++i) {
        QVERIFY(rootInfo.isSupported(NET::State(1 << i)));
        count++;
    }
    for (int i = 0; i < 10; ++i) {
        QVERIFY(rootInfo.isSupported(NET::Action(1 << i)));
        count++;
    }
    // NET::WMFrameExtents has two properties
    count += 1;
    // XAWState, WMGeometry, WM2TransientFor, WM2GroupLeader, WM2WindowClass, WM2WindowRole, WM2ClientMachine
    count -= 7;
    // WM2BlockCompositing has 3 properties
    count += 2;
    // Add _GTK_FRAME_EXTENTS
    ++count;

    QVERIFY(supported != XCB_ATOM_NONE);
    QVERIFY(utf8String != XCB_ATOM_NONE);
    QVERIFY(wmCheck != XCB_ATOM_NONE);
    QVERIFY(wmName != XCB_ATOM_NONE);

    // we should have got some events
    waitForPropertyChange(&rootInfo, supported, NET::Supported);
    waitForPropertyChange(&rootInfo, wmCheck, NET::SupportingWMCheck);

    // get the cookies of the things to check
    xcb_get_property_cookie_t supportedCookie = xcb_get_property_unchecked(connection(), false, rootInfo.rootWindow(), supported, XCB_ATOM_ATOM, 0, 101);
    xcb_get_property_cookie_t wmCheckRootCookie = xcb_get_property_unchecked(connection(), false, rootInfo.rootWindow(), wmCheck, XCB_ATOM_WINDOW, 0, 1);
    xcb_get_property_cookie_t wmCheckSupportWinCookie = xcb_get_property_unchecked(connection(), false, m_supportWindow, wmCheck, XCB_ATOM_WINDOW, 0, 1);
    xcb_get_property_cookie_t wmNameCookie = xcb_get_property_unchecked(connection(), false, m_supportWindow, wmName, utf8String, 0, 16);

    Property supportedReply(xcb_get_property_reply(connection(), supportedCookie, nullptr));
    QVERIFY(supportedReply);
    QCOMPARE(supportedReply->format, uint8_t(32));
    QCOMPARE(supportedReply->value_len, uint32_t(count));
    // TODO: check that the correct atoms are set?
    Property wmCheckRootReply(xcb_get_property_reply(connection(), wmCheckRootCookie, nullptr));
    QVERIFY(wmCheckRootReply);
    QCOMPARE(wmCheckRootReply->format, uint8_t(32));
    QCOMPARE(wmCheckRootReply->value_len, uint32_t(1));
    QCOMPARE(reinterpret_cast<xcb_window_t *>(xcb_get_property_value(wmCheckRootReply.get()))[0], m_supportWindow);

    Property wmCheckSupportReply(xcb_get_property_reply(connection(), wmCheckSupportWinCookie, nullptr));
    QVERIFY(wmCheckSupportReply);
    QCOMPARE(wmCheckSupportReply->format, uint8_t(32));
    QCOMPARE(wmCheckSupportReply->value_len, uint32_t(1));
    QCOMPARE(reinterpret_cast<xcb_window_t *>(xcb_get_property_value(wmCheckSupportReply.get()))[0], m_supportWindow);

    Property wmNameReply(xcb_get_property_reply(connection(), wmNameCookie, nullptr));
    QVERIFY(wmNameReply);
    QCOMPARE(wmNameReply->format, uint8_t(8));
    QCOMPARE(wmNameReply->value_len, uint32_t(15));
    QCOMPARE(reinterpret_cast<const char *>(xcb_get_property_value(wmNameReply.get())), s_wmName);

    // disable some supported
    rootInfo.setSupported(NET::WMFrameExtents, false);
    rootInfo.setSupported(NET::WM2KDETemporaryRules, false);
    rootInfo.setSupported(NET::ActionChangeDesktop, false);
    rootInfo.setSupported(NET::FullScreen, false);
    QVERIFY(rootInfo.isSupported(NET::ToolbarMask));
    QVERIFY(rootInfo.isSupported(NET::OnScreenDisplayMask));
    QVERIFY(rootInfo.isSupported(NET::DockMask));
    rootInfo.setSupported(NET::ToolbarMask, false);
    rootInfo.setSupported(NET::OnScreenDisplayMask, false);

    QVERIFY(!rootInfo.isSupported(NET::WMFrameExtents));
    QVERIFY(!rootInfo.isSupported(NET::WM2KDETemporaryRules));
    QVERIFY(!rootInfo.isSupported(NET::ActionChangeDesktop));
    QVERIFY(!rootInfo.isSupported(NET::FullScreen));
    QVERIFY(!rootInfo.isSupported(NET::ToolbarMask));
    QVERIFY(!rootInfo.isSupported(NET::OnScreenDisplayMask));
    QVERIFY(rootInfo.isSupported(NET::DockMask));

    // lets get supported again
    supportedCookie = xcb_get_property_unchecked(connection(), false, rootInfo.rootWindow(), supported, XCB_ATOM_ATOM, 0, 90);
    supportedReply.reset(xcb_get_property_reply(connection(), supportedCookie, nullptr));
    QVERIFY(supportedReply);
    QCOMPARE(supportedReply->format, uint8_t(32));
    QCOMPARE(supportedReply->value_len, uint32_t(count - 7));

    for (int i = 0; i < 5; ++i) {
        // we should have got some events
        waitForPropertyChange(&rootInfo, supported, NET::Supported);
        waitForPropertyChange(&rootInfo, wmCheck, NET::SupportingWMCheck);
    }

    // turn something off, just to get another event
    rootInfo.setSupported(NET::WM2BlockCompositing, false);
    // lets get supported again
    supportedCookie = xcb_get_property_unchecked(connection(), false, rootInfo.rootWindow(), supported, XCB_ATOM_ATOM, 0, 90);
    supportedReply.reset(xcb_get_property_reply(connection(), supportedCookie, nullptr));
    QVERIFY(supportedReply);
    QCOMPARE(supportedReply->format, uint8_t(32));
    QCOMPARE(supportedReply->value_len, uint32_t(count - 9));
    NETRootInfo clientInfo(connection(), NET::Supported | NET::SupportingWMCheck);
    waitForPropertyChange(&clientInfo, supported, NET::Supported);
    waitForPropertyChange(&clientInfo, wmCheck, NET::SupportingWMCheck);
}

void NetRootInfoTestWM::testClientList()
{
    KXUtils::Atom atom(connection(), QByteArrayLiteral("_NET_CLIENT_LIST"));
    QVERIFY(connection());
    NETRootInfo
        rootInfo(connection(), m_supportWindow, s_wmName, NET::WMAllProperties, NET::AllTypesMask, NET::States(~0u), NET::WM2AllProperties, NET::Actions(~0u));
    QCOMPARE(rootInfo.clientListCount(), 0);
    QVERIFY(!rootInfo.clientList());

    xcb_window_t windows[] = {xcb_generate_id(connection()),
                              xcb_generate_id(connection()),
                              xcb_generate_id(connection()),
                              xcb_generate_id(connection()),
                              xcb_generate_id(connection())};
    rootInfo.setClientList(windows, 5);
    QCOMPARE(rootInfo.clientListCount(), 5);
    const xcb_window_t *otherWins = rootInfo.clientList();
    for (int i = 0; i < 5; ++i) {
        QCOMPARE(otherWins[i], windows[i]);
    }

    // compare with the X property
    QVERIFY(atom != XCB_ATOM_NONE);
    xcb_get_property_cookie_t cookie = xcb_get_property_unchecked(connection(), false, rootInfo.rootWindow(), atom, XCB_ATOM_WINDOW, 0, 5);
    Property reply(xcb_get_property_reply(connection(), cookie, nullptr));
    QVERIFY(reply);
    QCOMPARE(reply->format, uint8_t(32));
    QCOMPARE(reply->value_len, uint32_t(5));
    const xcb_window_t *propWins = reinterpret_cast<xcb_window_t *>(xcb_get_property_value(reply.get()));
    for (int i = 0; i < 5; ++i) {
        QCOMPARE(propWins[i], windows[i]);
    }

    // wait for our property
    NETRootInfo clientInfo(connection(), NET::Supported | NET::SupportingWMCheck | NET::ClientList);
    waitForPropertyChange(&clientInfo, atom, NET::ClientList);
    QCOMPARE(clientInfo.clientListCount(), 5);
    const xcb_window_t *otherWins2 = clientInfo.clientList();
    for (int i = 0; i < 5; ++i) {
        QCOMPARE(otherWins2[i], windows[i]);
    }
}

void NetRootInfoTestWM::testClientListStacking()
{
    KXUtils::Atom atom(connection(), QByteArrayLiteral("_NET_CLIENT_LIST_STACKING"));
    QVERIFY(connection());
    NETRootInfo
        rootInfo(connection(), m_supportWindow, s_wmName, NET::WMAllProperties, NET::AllTypesMask, NET::States(~0u), NET::WM2AllProperties, NET::Actions(~0u));
    QCOMPARE(rootInfo.clientListStackingCount(), 0);
    QVERIFY(!rootInfo.clientListStacking());

    xcb_window_t windows[] = {xcb_generate_id(connection()),
                              xcb_generate_id(connection()),
                              xcb_generate_id(connection()),
                              xcb_generate_id(connection()),
                              xcb_generate_id(connection())};
    rootInfo.setClientListStacking(windows, 5);
    QCOMPARE(rootInfo.clientListStackingCount(), 5);
    const xcb_window_t *otherWins = rootInfo.clientListStacking();
    for (int i = 0; i < 5; ++i) {
        QCOMPARE(otherWins[i], windows[i]);
    }

    // compare with the X property
    QVERIFY(atom != XCB_ATOM_NONE);
    xcb_get_property_cookie_t cookie = xcb_get_property_unchecked(connection(), false, rootInfo.rootWindow(), atom, XCB_ATOM_WINDOW, 0, 5);
    Property reply(xcb_get_property_reply(connection(), cookie, nullptr));
    QVERIFY(reply);
    QCOMPARE(reply->format, uint8_t(32));
    QCOMPARE(reply->value_len, uint32_t(5));
    const xcb_window_t *propWins = reinterpret_cast<xcb_window_t *>(xcb_get_property_value(reply.get()));
    for (int i = 0; i < 5; ++i) {
        QCOMPARE(propWins[i], windows[i]);
    }

    // wait for our property
    waitForPropertyChange(&rootInfo, atom, NET::ClientListStacking);
    QCOMPARE(rootInfo.clientListStackingCount(), 5);
    const xcb_window_t *otherWins2 = rootInfo.clientListStacking();
    for (int i = 0; i < 5; ++i) {
        QCOMPARE(otherWins2[i], windows[i]);
    }
}

void NetRootInfoTestWM::testVirtualRoots()
{
    KXUtils::Atom atom(connection(), QByteArrayLiteral("_NET_VIRTUAL_ROOTS"));
    QVERIFY(connection());
    NETRootInfo
        rootInfo(connection(), m_supportWindow, s_wmName, NET::WMAllProperties, NET::AllTypesMask, NET::States(~0u), NET::WM2AllProperties, NET::Actions(~0u));
    QCOMPARE(rootInfo.virtualRootsCount(), 0);
    QVERIFY(!rootInfo.virtualRoots());

    xcb_window_t windows[] = {xcb_generate_id(connection()),
                              xcb_generate_id(connection()),
                              xcb_generate_id(connection()),
                              xcb_generate_id(connection()),
                              xcb_generate_id(connection())};
    rootInfo.setVirtualRoots(windows, 5);
    QCOMPARE(rootInfo.virtualRootsCount(), 5);
    const xcb_window_t *otherWins = rootInfo.virtualRoots();
    for (int i = 0; i < 5; ++i) {
        QCOMPARE(otherWins[i], windows[i]);
    }

    // compare with the X property
    QVERIFY(atom != XCB_ATOM_NONE);
    xcb_get_property_cookie_t cookie = xcb_get_property_unchecked(connection(), false, rootInfo.rootWindow(), atom, XCB_ATOM_WINDOW, 0, 5);
    Property reply(xcb_get_property_reply(connection(), cookie, nullptr));
    QVERIFY(reply);
    QCOMPARE(reply->format, uint8_t(32));
    QCOMPARE(reply->value_len, uint32_t(5));
    const xcb_window_t *propWins = reinterpret_cast<xcb_window_t *>(xcb_get_property_value(reply.get()));
    for (int i = 0; i < 5; ++i) {
        QCOMPARE(propWins[i], windows[i]);
    }

    // wait for our property - reported to a Client NETRootInfo
    NETRootInfo clientInfo(connection(), NET::VirtualRoots);
    waitForPropertyChange(&clientInfo, atom, NET::VirtualRoots);
    QCOMPARE(rootInfo.virtualRootsCount(), 5);
    const xcb_window_t *otherWins2 = rootInfo.virtualRoots();
    for (int i = 0; i < 5; ++i) {
        QCOMPARE(otherWins2[i], windows[i]);
    }
}

void NetRootInfoTestWM::testNumberOfDesktops()
{
    KXUtils::Atom atom(connection(), QByteArrayLiteral("_NET_NUMBER_OF_DESKTOPS"));
    QVERIFY(connection());
    NETRootInfo
        rootInfo(connection(), m_supportWindow, s_wmName, NET::WMAllProperties, NET::AllTypesMask, NET::States(~0u), NET::WM2AllProperties, NET::Actions(~0u));
    QCOMPARE(rootInfo.numberOfDesktops(), 1);
    rootInfo.setNumberOfDesktops(4);
    QCOMPARE(rootInfo.numberOfDesktops(), 4);

    // compare with the X property
    QVERIFY(atom != XCB_ATOM_NONE);
    xcb_get_property_cookie_t cookie = xcb_get_property_unchecked(connection(), false, rootInfo.rootWindow(), atom, XCB_ATOM_CARDINAL, 0, 1);
    Property reply(xcb_get_property_reply(connection(), cookie, nullptr));
    QVERIFY(reply);
    QCOMPARE(reply->format, uint8_t(32));
    QCOMPARE(reply->value_len, uint32_t(1));
    QCOMPARE(reinterpret_cast<uint32_t *>(xcb_get_property_value(reply.get()))[0], uint32_t(4));

    // wait for our property
    waitForPropertyChange(&rootInfo, atom, NET::NumberOfDesktops);
    QCOMPARE(rootInfo.numberOfDesktops(), 4);
}

void NetRootInfoTestWM::testCurrentDesktop()
{
    KXUtils::Atom atom(connection(), QByteArrayLiteral("_NET_CURRENT_DESKTOP"));
    // TODO: verify that current desktop cannot be higher than number of desktops
    QVERIFY(connection());
    NETRootInfo
        rootInfo(connection(), m_supportWindow, s_wmName, NET::WMAllProperties, NET::AllTypesMask, NET::States(~0u), NET::WM2AllProperties, NET::Actions(~0u));
    QCOMPARE(rootInfo.currentDesktop(), 1);
    rootInfo.setCurrentDesktop(5);
    QCOMPARE(rootInfo.currentDesktop(), 5);

    // compare with the X property
    QVERIFY(atom != XCB_ATOM_NONE);
    xcb_get_property_cookie_t cookie = xcb_get_property_unchecked(connection(), false, rootInfo.rootWindow(), atom, XCB_ATOM_CARDINAL, 0, 1);
    Property reply(xcb_get_property_reply(connection(), cookie, nullptr));
    QVERIFY(reply);
    QCOMPARE(reply->format, uint8_t(32));
    QCOMPARE(reply->value_len, uint32_t(1));
    // note: API starts counting at 1, but property starts counting at 5, because of that subtracting one
    QCOMPARE(reinterpret_cast<uint32_t *>(xcb_get_property_value(reply.get()))[0], uint32_t(5 - 1));

    // wait for our property
    waitForPropertyChange(&rootInfo, atom, NET::CurrentDesktop);
    QCOMPARE(rootInfo.currentDesktop(), 5);
}

void NetRootInfoTestWM::testDesktopNames()
{
    KXUtils::Atom atom(connection(), QByteArrayLiteral("_NET_DESKTOP_NAMES"));
    KXUtils::Atom utf8String(connection(), QByteArrayLiteral("UTF8_STRING"));
    QVERIFY(connection());
    NETRootInfo
        rootInfo(connection(), m_supportWindow, s_wmName, NET::WMAllProperties, NET::AllTypesMask, NET::States(~0u), NET::WM2AllProperties, NET::Actions(~0u));
    QVERIFY(!rootInfo.desktopName(0));
    QVERIFY(!rootInfo.desktopName(1));
    QVERIFY(!rootInfo.desktopName(2));
    rootInfo.setDesktopName(1, "foo");
    rootInfo.setDesktopName(2, "bar");
    rootInfo.setNumberOfDesktops(2);
    QCOMPARE(rootInfo.desktopName(1), "foo");
    QCOMPARE(rootInfo.desktopName(2), "bar");

    // compare with the X property
    QVERIFY(atom != XCB_ATOM_NONE);
    QVERIFY(utf8String != XCB_ATOM_NONE);
    xcb_get_property_cookie_t cookie = xcb_get_property_unchecked(connection(), false, rootInfo.rootWindow(), atom, utf8String, 0, 10000);
    Property reply(xcb_get_property_reply(connection(), cookie, nullptr));
    QVERIFY(reply);
    QCOMPARE(reply->format, uint8_t(8));
    QCOMPARE(reply->value_len, uint32_t(8));
    QCOMPARE(reinterpret_cast<const char *>(xcb_get_property_value(reply.get())), "foo\0bar");

    // wait for our property
    waitForPropertyChange(&rootInfo, atom, NET::DesktopNames);
    QCOMPARE(rootInfo.desktopName(1), "foo");
    QCOMPARE(rootInfo.desktopName(2), "bar");
    // there should be two events
    waitForPropertyChange(&rootInfo, atom, NET::DesktopNames);
    QCOMPARE(rootInfo.desktopName(1), "foo");
    QCOMPARE(rootInfo.desktopName(2), "bar");
}

void NetRootInfoTestWM::testActiveWindow()
{
    KXUtils::Atom atom(connection(), QByteArrayLiteral("_NET_ACTIVE_WINDOW"));
    QVERIFY(connection());
    NETRootInfo
        rootInfo(connection(), m_supportWindow, s_wmName, NET::WMAllProperties, NET::AllTypesMask, NET::States(~0u), NET::WM2AllProperties, NET::Actions(~0u));
    QVERIFY(rootInfo.activeWindow() == XCB_WINDOW_NONE);
    // rootinfo doesn't verify whether our window is a window, so we just generate an ID
    xcb_window_t activeWindow = xcb_generate_id(connection());
    rootInfo.setActiveWindow(activeWindow);
    QCOMPARE(rootInfo.activeWindow(), activeWindow);

    // compare with the X property
    QVERIFY(atom != XCB_ATOM_NONE);
    xcb_get_property_cookie_t cookie = xcb_get_property_unchecked(connection(), false, rootInfo.rootWindow(), atom, XCB_ATOM_WINDOW, 0, 1);
    Property reply(xcb_get_property_reply(connection(), cookie, nullptr));
    QVERIFY(reply);
    QCOMPARE(reply->format, uint8_t(32));
    QCOMPARE(reply->value_len, uint32_t(1));
    QCOMPARE(reinterpret_cast<xcb_window_t *>(xcb_get_property_value(reply.get()))[0], activeWindow);

    // wait for our property
    waitForPropertyChange(&rootInfo, atom, NET::ActiveWindow);
    QCOMPARE(rootInfo.activeWindow(), activeWindow);
}

void NetRootInfoTestWM::testDesktopGeometry()
{
    KXUtils::Atom atom(connection(), QByteArrayLiteral("_NET_DESKTOP_GEOMETRY"));
    QVERIFY(connection());
    NETRootInfo
        rootInfo(connection(), m_supportWindow, s_wmName, NET::WMAllProperties, NET::AllTypesMask, NET::States(~0u), NET::WM2AllProperties, NET::Actions(~0u));
    QCOMPARE(rootInfo.desktopGeometry().width, 0);
    QCOMPARE(rootInfo.desktopGeometry().height, 0);

    NETSize size;
    size.width = 1000;
    size.height = 800;
    rootInfo.setDesktopGeometry(size);
    QCOMPARE(rootInfo.desktopGeometry().width, size.width);
    QCOMPARE(rootInfo.desktopGeometry().height, size.height);

    // compare with the X property
    QVERIFY(atom != XCB_ATOM_NONE);
    xcb_get_property_cookie_t cookie = xcb_get_property_unchecked(connection(), false, rootInfo.rootWindow(), atom, XCB_ATOM_CARDINAL, 0, 2);
    Property reply(xcb_get_property_reply(connection(), cookie, nullptr));
    QVERIFY(reply);
    QCOMPARE(reply->format, uint8_t(32));
    QCOMPARE(reply->value_len, uint32_t(2));
    uint32_t *data = reinterpret_cast<uint32_t *>(xcb_get_property_value(reply.get()));
    QCOMPARE(data[0], uint32_t(size.width));
    QCOMPARE(data[1], uint32_t(size.height));

    // wait for our property
    waitForPropertyChange(&rootInfo, atom, NET::DesktopGeometry);
    QCOMPARE(rootInfo.desktopGeometry().width, size.width);
    QCOMPARE(rootInfo.desktopGeometry().height, size.height);
}

void NetRootInfoTestWM::testDesktopLayout_data()
{
    QTest::addColumn<NET::Orientation>("orientation");
    QTest::addColumn<QSize>("columnsRows");
    QTest::addColumn<NET::DesktopLayoutCorner>("corner");

    QTest::newRow("h/1/1/tl") << NET::OrientationHorizontal << QSize(1, 1) << NET::DesktopLayoutCornerTopLeft;
    QTest::newRow("h/1/0/tr") << NET::OrientationHorizontal << QSize(1, 0) << NET::DesktopLayoutCornerTopRight;
    QTest::newRow("h/0/1/bl") << NET::OrientationHorizontal << QSize(0, 1) << NET::DesktopLayoutCornerBottomLeft;
    QTest::newRow("h/1/2/br") << NET::OrientationHorizontal << QSize(1, 2) << NET::DesktopLayoutCornerBottomRight;
    QTest::newRow("v/3/2/tl") << NET::OrientationVertical << QSize(3, 2) << NET::DesktopLayoutCornerTopLeft;
    QTest::newRow("v/5/4/tr") << NET::OrientationVertical << QSize(5, 4) << NET::DesktopLayoutCornerTopRight;
    QTest::newRow("v/2/1/bl") << NET::OrientationVertical << QSize(2, 1) << NET::DesktopLayoutCornerBottomLeft;
    QTest::newRow("v/3/2/br") << NET::OrientationVertical << QSize(3, 2) << NET::DesktopLayoutCornerBottomRight;
}

void NetRootInfoTestWM::testDesktopLayout()
{
    KXUtils::Atom atom(connection(), QByteArrayLiteral("_NET_DESKTOP_LAYOUT"));
    QVERIFY(connection());
    NETRootInfo
        rootInfo(connection(), m_supportWindow, s_wmName, NET::WMAllProperties, NET::AllTypesMask, NET::States(~0u), NET::WM2AllProperties, NET::Actions(~0u));
    QFETCH(NET::Orientation, orientation);
    QFETCH(QSize, columnsRows);
    QFETCH(NET::DesktopLayoutCorner, corner);

    rootInfo.setDesktopLayout(orientation, columnsRows.width(), columnsRows.height(), corner);
    QCOMPARE(rootInfo.desktopLayoutOrientation(), orientation);
    QCOMPARE(rootInfo.desktopLayoutColumnsRows(), columnsRows);
    QCOMPARE(rootInfo.desktopLayoutCorner(), corner);

    // compare with the X property
    QVERIFY(atom != XCB_ATOM_NONE);
    xcb_get_property_cookie_t cookie = xcb_get_property_unchecked(connection(), false, rootInfo.rootWindow(), atom, XCB_ATOM_CARDINAL, 0, 4);
    Property reply(xcb_get_property_reply(connection(), cookie, nullptr));
    QVERIFY(reply);
    QCOMPARE(reply->format, uint8_t(32));
    QCOMPARE(reply->value_len, uint32_t(4));
    uint32_t *data = reinterpret_cast<uint32_t *>(xcb_get_property_value(reply.get()));
    QCOMPARE(data[0], uint32_t(orientation));
    QCOMPARE(data[1], uint32_t(columnsRows.width()));
    QCOMPARE(data[2], uint32_t(columnsRows.height()));
    QCOMPARE(data[3], uint32_t(corner));

    // wait for our property
    waitForPropertyChange(&rootInfo, atom, NET::Property(0), NET::WM2DesktopLayout);
    QCOMPARE(rootInfo.desktopLayoutOrientation(), orientation);
    QCOMPARE(rootInfo.desktopLayoutColumnsRows(), columnsRows);
    QCOMPARE(rootInfo.desktopLayoutCorner(), corner);

    NETRootInfo info2(connection(), NET::WMAllProperties, NET::WM2AllProperties);
    QCOMPARE(info2.desktopLayoutColumnsRows(), columnsRows);
}

void NetRootInfoTestWM::testDesktopViewports()
{
    KXUtils::Atom atom(connection(), QByteArrayLiteral("_NET_DESKTOP_VIEWPORT"));
    QVERIFY(connection());
    NETRootInfo
        rootInfo(connection(), m_supportWindow, s_wmName, NET::WMAllProperties, NET::AllTypesMask, NET::States(~0u), NET::WM2AllProperties, NET::Actions(~0u));
    // we need to know the number of desktops, therefore setting it
    rootInfo.setNumberOfDesktops(4);
    NETPoint desktopOne;
    desktopOne.x = 100;
    desktopOne.y = 50;
    NETPoint desktopTwo;
    desktopTwo.x = 200;
    desktopTwo.y = 100;
    rootInfo.setDesktopViewport(1, desktopOne);
    rootInfo.setDesktopViewport(2, desktopTwo);

    const NETPoint compareZero = rootInfo.desktopViewport(0);
    QCOMPARE(compareZero.x, 0);
    QCOMPARE(compareZero.y, 0);
    const NETPoint compareOne = rootInfo.desktopViewport(1);
    QCOMPARE(compareOne.x, desktopOne.x);
    QCOMPARE(compareOne.y, desktopOne.y);
    const NETPoint compareTwo = rootInfo.desktopViewport(2);
    QCOMPARE(compareTwo.x, desktopTwo.x);
    QCOMPARE(compareTwo.y, desktopTwo.y);
    const NETPoint compareThree = rootInfo.desktopViewport(3);
    QCOMPARE(compareThree.x, 0);
    QCOMPARE(compareThree.y, 0);

    // compare with the X property
    QVERIFY(atom != XCB_ATOM_NONE);
    xcb_get_property_cookie_t cookie = xcb_get_property_unchecked(connection(), false, rootInfo.rootWindow(), atom, XCB_ATOM_CARDINAL, 0, 8);
    Property reply(xcb_get_property_reply(connection(), cookie, nullptr));
    QVERIFY(reply);
    QCOMPARE(reply->format, uint8_t(32));
    QCOMPARE(reply->value_len, uint32_t(8));
    uint32_t *data = reinterpret_cast<uint32_t *>(xcb_get_property_value(reply.get()));
    QCOMPARE(data[0], uint32_t(desktopOne.x));
    QCOMPARE(data[1], uint32_t(desktopOne.y));
    QCOMPARE(data[2], uint32_t(desktopTwo.x));
    QCOMPARE(data[3], uint32_t(desktopTwo.y));
    QCOMPARE(data[4], uint32_t(0));
    QCOMPARE(data[5], uint32_t(0));
    QCOMPARE(data[6], uint32_t(0));
    QCOMPARE(data[7], uint32_t(0));

    // wait for our property - two events
    waitForPropertyChange(&rootInfo, atom, NET::DesktopViewport);
    waitForPropertyChange(&rootInfo, atom, NET::DesktopViewport);
    const NETPoint compareOne2 = rootInfo.desktopViewport(1);
    QCOMPARE(compareOne2.x, desktopOne.x);
    QCOMPARE(compareOne2.y, desktopOne.y);
    const NETPoint compareTwo2 = rootInfo.desktopViewport(2);
    QCOMPARE(compareTwo2.x, desktopTwo.x);
    QCOMPARE(compareTwo2.y, desktopTwo.y);
    const NETPoint compareThree2 = rootInfo.desktopViewport(3);
    QCOMPARE(compareThree2.x, 0);
    QCOMPARE(compareThree2.y, 0);
}

void NetRootInfoTestWM::testShowingDesktop_data()
{
    QTest::addColumn<bool>("set");
    QTest::addColumn<uint32_t>("setValue");

    QTest::newRow("true") << true << uint32_t(1);
    QTest::newRow("false") << false << uint32_t(0);
}

void NetRootInfoTestWM::testShowingDesktop()
{
    KXUtils::Atom atom(connection(), QByteArrayLiteral("_NET_SHOWING_DESKTOP"));
    QVERIFY(connection());
    NETRootInfo
        rootInfo(connection(), m_supportWindow, s_wmName, NET::WMAllProperties, NET::AllTypesMask, NET::States(~0u), NET::WM2AllProperties, NET::Actions(~0u));
    QFETCH(bool, set);
    rootInfo.setShowingDesktop(set);
    QCOMPARE(rootInfo.showingDesktop(), set);

    // compare with the X property
    QVERIFY(atom != XCB_ATOM_NONE);
    xcb_get_property_cookie_t cookie = xcb_get_property_unchecked(connection(), false, rootInfo.rootWindow(), atom, XCB_ATOM_CARDINAL, 0, 1);
    Property reply(xcb_get_property_reply(connection(), cookie, nullptr));
    QVERIFY(reply);
    QCOMPARE(reply->format, uint8_t(32));
    QCOMPARE(reply->value_len, uint32_t(1));
    QTEST(reinterpret_cast<uint32_t *>(xcb_get_property_value(reply.get()))[0], "setValue");

    // wait for our property
    waitForPropertyChange(&rootInfo, atom, NET::Property(0), NET::WM2ShowingDesktop);
    QCOMPARE(rootInfo.showingDesktop(), set);
}

void NetRootInfoTestWM::testWorkArea()
{
    KXUtils::Atom atom(connection(), QByteArrayLiteral("_NET_WORKAREA"));
    QVERIFY(connection());
    NETRootInfo
        rootInfo(connection(), m_supportWindow, s_wmName, NET::WMAllProperties, NET::AllTypesMask, NET::States(~0u), NET::WM2AllProperties, NET::Actions(~0u));
    // we need to know the number of desktops, therefore setting it
    rootInfo.setNumberOfDesktops(4);
    NETRect desktopOne;
    desktopOne.pos.x = 10;
    desktopOne.pos.y = 5;
    desktopOne.size.width = 1000;
    desktopOne.size.height = 800;
    NETRect desktopTwo;
    desktopTwo.pos.x = 20;
    desktopTwo.pos.y = 10;
    desktopTwo.size.width = 800;
    desktopTwo.size.height = 750;
    rootInfo.setWorkArea(1, desktopOne);
    rootInfo.setWorkArea(2, desktopTwo);

    const NETRect compareZero = rootInfo.workArea(0);
    QCOMPARE(compareZero.pos.x, 0);
    QCOMPARE(compareZero.pos.y, 0);
    QCOMPARE(compareZero.size.width, 0);
    QCOMPARE(compareZero.size.height, 0);
    const NETRect compareOne = rootInfo.workArea(1);
    QCOMPARE(compareOne.pos.x, desktopOne.pos.x);
    QCOMPARE(compareOne.pos.y, desktopOne.pos.y);
    QCOMPARE(compareOne.size.width, desktopOne.size.width);
    QCOMPARE(compareOne.size.height, desktopOne.size.height);
    const NETRect compareTwo = rootInfo.workArea(2);
    QCOMPARE(compareTwo.pos.x, desktopTwo.pos.x);
    QCOMPARE(compareTwo.pos.y, desktopTwo.pos.y);
    QCOMPARE(compareTwo.size.width, desktopTwo.size.width);
    QCOMPARE(compareTwo.size.height, desktopTwo.size.height);
    const NETRect compareThree = rootInfo.workArea(3);
    QCOMPARE(compareThree.pos.x, 0);
    QCOMPARE(compareThree.pos.y, 0);
    QCOMPARE(compareThree.size.width, 0);
    QCOMPARE(compareThree.size.height, 0);

    // compare with the X property
    QVERIFY(atom != XCB_ATOM_NONE);
    xcb_get_property_cookie_t cookie = xcb_get_property_unchecked(connection(), false, rootInfo.rootWindow(), atom, XCB_ATOM_CARDINAL, 0, 16);
    Property reply(xcb_get_property_reply(connection(), cookie, nullptr));
    QVERIFY(reply);
    QCOMPARE(reply->format, uint8_t(32));
    QCOMPARE(reply->value_len, uint32_t(16));
    uint32_t *data = reinterpret_cast<uint32_t *>(xcb_get_property_value(reply.get()));
    QCOMPARE(data[0], uint32_t(desktopOne.pos.x));
    QCOMPARE(data[1], uint32_t(desktopOne.pos.y));
    QCOMPARE(data[2], uint32_t(desktopOne.size.width));
    QCOMPARE(data[3], uint32_t(desktopOne.size.height));
    QCOMPARE(data[4], uint32_t(desktopTwo.pos.x));
    QCOMPARE(data[5], uint32_t(desktopTwo.pos.y));
    QCOMPARE(data[6], uint32_t(desktopTwo.size.width));
    QCOMPARE(data[7], uint32_t(desktopTwo.size.height));
    QCOMPARE(data[8], uint32_t(0));
    QCOMPARE(data[9], uint32_t(0));
    QCOMPARE(data[10], uint32_t(0));
    QCOMPARE(data[11], uint32_t(0));
    QCOMPARE(data[12], uint32_t(0));
    QCOMPARE(data[13], uint32_t(0));
    QCOMPARE(data[14], uint32_t(0));
    QCOMPARE(data[15], uint32_t(0));

    // wait for our property - two events
    waitForPropertyChange(&rootInfo, atom, NET::WorkArea);
    waitForPropertyChange(&rootInfo, atom, NET::WorkArea);
    const NETRect compareOne2 = rootInfo.workArea(1);
    QCOMPARE(compareOne2.pos.x, desktopOne.pos.x);
    QCOMPARE(compareOne2.pos.y, desktopOne.pos.y);
    QCOMPARE(compareOne2.size.width, desktopOne.size.width);
    QCOMPARE(compareOne2.size.height, desktopOne.size.height);
    const NETRect compareTwo2 = rootInfo.workArea(2);
    QCOMPARE(compareTwo2.pos.x, desktopTwo.pos.x);
    QCOMPARE(compareTwo2.pos.y, desktopTwo.pos.y);
    QCOMPARE(compareTwo2.size.width, desktopTwo.size.width);
    QCOMPARE(compareTwo2.size.height, desktopTwo.size.height);
    const NETRect compareThree2 = rootInfo.workArea(3);
    QCOMPARE(compareThree2.pos.x, 0);
    QCOMPARE(compareThree2.pos.y, 0);
    QCOMPARE(compareThree2.size.width, 0);
    QCOMPARE(compareThree2.size.height, 0);
}

void NetRootInfoTestWM::testDontCrashMapViewports()
{
    QProcess p;
    const QString processName = QFINDTESTDATA("dontcrashmapviewport");
    QVERIFY(!processName.isEmpty());

    p.start(processName, QStringList());
    QVERIFY(p.waitForFinished());
    QCOMPARE(p.exitStatus(), QProcess::NormalExit);
    QCOMPARE(p.exitCode(), 0);
}

QTEST_GUILESS_MAIN(NetRootInfoTestWM)

#include "netrootinfotestwm.moc"
