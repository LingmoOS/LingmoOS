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
#include <netwm.h>
#include <qtest_widgets.h>
#include <QProcess>
// system
#include <unistd.h>

class Property : public QScopedPointer<xcb_get_property_reply_t, QScopedPointerPodDeleter>
{
public:
    Property(xcb_get_property_reply_t *p = nullptr) : QScopedPointer<xcb_get_property_reply_t, QScopedPointerPodDeleter>(p) {}
};

#define INFO NETWinInfo info(m_connection, m_testWindow, m_rootWindow, NET::WMAllProperties, NET::WM2AllProperties, NET::Client);

#define ATOM(name) \
    KXUtils::Atom atom(connection(), QByteArrayLiteral(#name));

#define UTF8 KXUtils::Atom utf8String(connection(), QByteArrayLiteral("UTF8_STRING"));

#define GETPROP(type, length, formatSize) \
    xcb_get_property_cookie_t cookie = xcb_get_property_unchecked(connection(), false, m_testWindow, \
                                       atom, type, 0, length); \
    Property reply(xcb_get_property_reply(connection(), cookie, nullptr)); \
    QVERIFY(!reply.isNull()); \
    QCOMPARE(reply->format, uint8_t(formatSize)); \
    QCOMPARE(reply->value_len, uint32_t(length));

#define VERIFYDELETED(t) \
    xcb_get_property_cookie_t cookieDeleted = xcb_get_property_unchecked(connection(), false, m_testWindow, \
            atom, t, 0, 1); \
    Property replyDeleted(xcb_get_property_reply(connection(), cookieDeleted, nullptr)); \
    QVERIFY(!replyDeleted.isNull()); \
    QVERIFY(replyDeleted->type == XCB_ATOM_NONE);

class NetWinInfoTestClient : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testBlockCompositing();
    void testUserTime();
    void testStartupId();
    void testDesktopFileName();
    void testHandledIcons_data();
    void testHandledIcons();
    void testPid();
    void testName();
    void testIconName();
    void testStrut();
    void testExtendedStrut();
    void testIconGeometry();
    void testWindowType_data();
    void testWindowType();

    void testActivities_data();
    void testActivities();
    void testWindowRole();
    void testWindowClass();
    void testClientMachine();
    void testGroupLeader();
    void testUrgency_data();
    void testUrgency();
    void testInput_data();
    void testInput();
    void testInitialMappingState_data();
    void testInitialMappingState();
    void testIconPixmap_data();
    void testIconPixmap();
    void testTransientFor();
    void testProtocols_data();
    void testProtocols();
    void testOpaqueRegion_data();
    void testOpaqueRegion();

private:
    void performNameTest(xcb_atom_t atom, const char *(NETWinInfo:: *getter)(void)const, void (NETWinInfo:: *setter)(const char *), NET::Property property);
    void waitForPropertyChange(NETWinInfo *info, xcb_atom_t atom, NET::Property prop, NET::Property2 prop2 = NET::Property2(0));
    xcb_connection_t *connection()
    {
        return m_connection;
    }
    xcb_connection_t *m_connection;
    QVector<xcb_connection_t*> m_connections;
    QScopedPointer<QProcess> m_xvfb;
    xcb_window_t m_rootWindow;
    xcb_window_t m_testWindow;
};

void NetWinInfoTestClient::initTestCase()
{
    qsrand(QDateTime::currentMSecsSinceEpoch());
}

void NetWinInfoTestClient::cleanupTestCase()
{
    // close connection
    while (!m_connections.isEmpty()) {
        xcb_disconnect(m_connections.takeFirst());
    }
}

void NetWinInfoTestClient::init()
{
    // first reset just to be sure
    m_connection = nullptr;
    m_rootWindow = XCB_WINDOW_NONE;
    m_testWindow = XCB_WINDOW_NONE;
    // start Xvfb
    m_xvfb.reset(new QProcess);
    // use pipe to pass fd to Xvfb to get back the display id
    int pipeFds[2];
    QVERIFY(pipe(pipeFds) == 0);
    m_xvfb->start(QStringLiteral("Xvfb"), QStringList{ QStringLiteral("-displayfd"), QString::number(pipeFds[1]) });
    QVERIFY(m_xvfb->waitForStarted());
    QCOMPARE(m_xvfb->state(), QProcess::Running);

    // reads from pipe, closes write side
    close(pipeFds[1]);

    QFile readPipe;
    QVERIFY(readPipe.open(pipeFds[0], QIODevice::ReadOnly, QFileDevice::AutoCloseHandle));
    QByteArray displayNumber = readPipe.readLine();
    readPipe.close();

    displayNumber.prepend(QByteArray(":"));
    displayNumber.remove(displayNumber.size() -1, 1);

    // create X connection
    int screen = 0;
    m_connection = xcb_connect(displayNumber.constData(), &screen);
    QVERIFY(m_connection);
    QVERIFY(!xcb_connection_has_error(m_connection));
    m_rootWindow = KXUtils::rootWindow(m_connection, screen);

    // create test window
    m_testWindow = xcb_generate_id(m_connection);
    uint32_t values[] = {XCB_EVENT_MASK_PROPERTY_CHANGE};
    xcb_create_window(m_connection, XCB_COPY_FROM_PARENT, m_testWindow,
                      m_rootWindow,
                      0, 0, 100, 100, 0, XCB_COPY_FROM_PARENT,
                      XCB_COPY_FROM_PARENT, XCB_CW_EVENT_MASK, values);
    // and map it
    xcb_map_window(m_connection, m_testWindow);
}

void NetWinInfoTestClient::cleanup()
{
    // destroy test window
    xcb_unmap_window(m_connection, m_testWindow);
    xcb_destroy_window(m_connection, m_testWindow);
    m_testWindow = XCB_WINDOW_NONE;

    // delay till clenupTestCase as otherwise xcb reuses the same memory address
    m_connections << connection();
    // kill Xvfb
    m_xvfb->terminate();
    m_xvfb->waitForFinished();
}

void NetWinInfoTestClient::waitForPropertyChange(NETWinInfo *info, xcb_atom_t atom, NET::Property prop, NET::Property2 prop2)
{
    while (true) {
        KXUtils::ScopedCPointer<xcb_generic_event_t> event(xcb_wait_for_event(connection()));
        if (event.isNull()) {
            break;
        }
        if ((event->response_type & ~0x80) != XCB_PROPERTY_NOTIFY) {
            continue;
        }
        xcb_property_notify_event_t *pe = reinterpret_cast<xcb_property_notify_event_t *>(event.data());
        if (pe->window != m_testWindow) {
            continue;
        }
        if (pe->atom != atom) {
            continue;
        }
        NET::Properties dirty;
        NET::Properties2 dirty2;
        info->event(event.data(), &dirty, &dirty2);
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

void NetWinInfoTestClient::testBlockCompositing()
{
    QVERIFY(connection());
    ATOM(_KDE_NET_WM_BLOCK_COMPOSITING)
    INFO

    QVERIFY(!info.isBlockingCompositing());
    info.setBlockingCompositing(true);
    QVERIFY(info.isBlockingCompositing());

    // compare with the X property
    QVERIFY(atom != XCB_ATOM_NONE);
    GETPROP(XCB_ATOM_CARDINAL, 1, 32)
    QCOMPARE(reinterpret_cast<uint32_t *>(xcb_get_property_value(reply.data()))[0], uint32_t(1));

    // and wait for our event
    waitForPropertyChange(&info, atom, NET::Property(0), NET::WM2BlockCompositing);
    QVERIFY(info.isBlockingCompositing());

    // setting false should delete the property again
    info.setBlockingCompositing(false);
    QVERIFY(!info.isBlockingCompositing());
    VERIFYDELETED(XCB_ATOM_CARDINAL)

    // and wait for our event
    waitForPropertyChange(&info, atom, NET::Property(0), NET::WM2BlockCompositing);
    QVERIFY(!info.isBlockingCompositing());
}

void NetWinInfoTestClient::testUserTime()
{
    QVERIFY(connection());
    ATOM(_NET_WM_USER_TIME)
    INFO

    QCOMPARE(info.userTime(), uint32_t(-1));
    info.setUserTime(500);
    QCOMPARE(info.userTime(), uint32_t(500));

    // compare with the X property
    QVERIFY(atom != XCB_ATOM_NONE);
    GETPROP(XCB_ATOM_CARDINAL, 1, 32)
    QCOMPARE(reinterpret_cast<uint32_t *>(xcb_get_property_value(reply.data()))[0], uint32_t(500));

    // and wait for our event
    waitForPropertyChange(&info, atom, NET::Property(0), NET::WM2UserTime);
    QCOMPARE(info.userTime(), uint32_t(500));
}

void NetWinInfoTestClient::testStartupId()
{
    QVERIFY(connection());
    ATOM(_NET_STARTUP_ID)
    UTF8
    INFO

    QVERIFY(!info.startupId());
    info.setStartupId("foo");
    QCOMPARE(info.startupId(), "foo");

    // compare with the X property
    QVERIFY(atom != XCB_ATOM_NONE);
    QVERIFY(utf8String != XCB_ATOM_NONE);
    GETPROP(utf8String, 3, 8)
    QCOMPARE(reinterpret_cast<const char *>(xcb_get_property_value(reply.data())), "foo");

    // and wait for our event
    waitForPropertyChange(&info, atom, NET::Property(0), NET::WM2StartupId);
    QCOMPARE(info.startupId(), "foo");
}

void NetWinInfoTestClient::testDesktopFileName()
{
    QVERIFY(connection());
    ATOM(_KDE_NET_WM_DESKTOP_FILE)
    UTF8
    INFO

    QVERIFY(!info.desktopFileName());
    info.setDesktopFileName("foo");
    QCOMPARE(info.desktopFileName(), "foo");

    // compare with the X property
    QVERIFY(atom != XCB_ATOM_NONE);
    QVERIFY(utf8String != XCB_ATOM_NONE);
    GETPROP(utf8String, 3, 8)
    QCOMPARE(reinterpret_cast<const char *>(xcb_get_property_value(reply.data())), "foo");

    // and wait for our event
    waitForPropertyChange(&info, atom, NET::Property(0), NET::WM2DesktopFileName);
    QCOMPARE(info.desktopFileName(), "foo");
}

void NetWinInfoTestClient::testHandledIcons_data()
{
    QTest::addColumn<bool>("handled");
    QTest::addColumn<uint32_t>("value");

    QTest::newRow("enabled") << true << uint32_t(1);
    QTest::newRow("disabled") << false << uint32_t(0);
}

void NetWinInfoTestClient::testHandledIcons()
{
    QVERIFY(connection());
    ATOM(_NET_WM_HANDLED_ICONS)
    INFO

    QVERIFY(!info.handledIcons());
    QFETCH(bool, handled);
    info.setHandledIcons(handled);
    QCOMPARE(info.handledIcons(), handled);

    // compare with the X property
    QVERIFY(atom != XCB_ATOM_NONE);
    GETPROP(XCB_ATOM_CARDINAL, 1, 32)
    QTEST(reinterpret_cast<uint32_t *>(xcb_get_property_value(reply.data()))[0], "value");

    // and wait for our event
    waitForPropertyChange(&info, atom, NET::WMHandledIcons);
    QCOMPARE(info.handledIcons(), handled);
}

void NetWinInfoTestClient::testPid()
{
    QVERIFY(connection());
    ATOM(_NET_WM_PID)
    INFO

    QCOMPARE(info.pid(), 0);
    info.setPid(m_xvfb->pid());
    QCOMPARE(info.pid(), m_xvfb->pid());

    // compare with the X property
    QVERIFY(atom != XCB_ATOM_NONE);
    GETPROP(XCB_ATOM_CARDINAL, 1, 32)
    QCOMPARE(reinterpret_cast<uint32_t *>(xcb_get_property_value(reply.data()))[0], uint32_t(m_xvfb->pid()));

    // and wait for our event
    waitForPropertyChange(&info, atom, NET::WMPid);
    QCOMPARE(info.pid(), m_xvfb->pid());
}

void NetWinInfoTestClient::performNameTest(xcb_atom_t atom, const char *(NETWinInfo:: *getter)(void)const, void (NETWinInfo:: *setter)(const char *), NET::Property property)
{
    UTF8
    INFO

    QVERIFY(!(info.*getter)());
    (info.*setter)("foo");
    QCOMPARE((info.*getter)(), "foo");

    // compare with the X property
    QVERIFY(atom != XCB_ATOM_NONE);
    QVERIFY(utf8String != XCB_ATOM_NONE);
    GETPROP(utf8String, 3, 8)
    QCOMPARE(reinterpret_cast<const char *>(xcb_get_property_value(reply.data())), "foo");

    // and wait for our event
    waitForPropertyChange(&info, atom, property);
    QCOMPARE((info.*getter)(), "foo");

    // delete the string
    (info.*setter)("");
    QCOMPARE((info.*getter)(), "");
    VERIFYDELETED(utf8String)

    // and wait for our event
    waitForPropertyChange(&info, atom, property);
    QVERIFY(!(info.*getter)());

    // set it again, to ensure that we don't leak on tear-down
    (info.*setter)("bar");
    QCOMPARE((info.*getter)(), "bar");
    xcb_flush(connection());
    waitForPropertyChange(&info, atom, property);
    QCOMPARE((info.*getter)(), "bar");
}

void NetWinInfoTestClient::testIconName()
{
    QVERIFY(connection());
    ATOM(_NET_WM_ICON_NAME)
    performNameTest(atom, &NETWinInfo::iconName, &NETWinInfo::setIconName, NET::WMIconName);
}

void NetWinInfoTestClient::testName()
{
    QVERIFY(connection());
    ATOM(_NET_WM_NAME)
    performNameTest(atom, &NETWinInfo::name, &NETWinInfo::setName, NET::WMName);
}

void NetWinInfoTestClient::testStrut()
{
    QVERIFY(connection());
    ATOM(_NET_WM_STRUT)
    INFO

    NETStrut extents = info.strut();
    QCOMPARE(extents.bottom, 0);
    QCOMPARE(extents.left, 0);
    QCOMPARE(extents.right, 0);
    QCOMPARE(extents.top, 0);

    NETStrut newExtents;
    newExtents.bottom = 10;
    newExtents.left   = 20;
    newExtents.right  = 30;
    newExtents.top    = 40;
    info.setStrut(newExtents);
    extents = info.strut();
    QCOMPARE(extents.bottom, newExtents.bottom);
    QCOMPARE(extents.left,   newExtents.left);
    QCOMPARE(extents.right,  newExtents.right);
    QCOMPARE(extents.top,    newExtents.top);

    // compare with the X property
    QVERIFY(atom != XCB_ATOM_NONE);
    GETPROP(XCB_ATOM_CARDINAL, 4, 32)
    uint32_t *data = reinterpret_cast<uint32_t *>(xcb_get_property_value(reply.data()));
    QCOMPARE(data[0], uint32_t(newExtents.left));
    QCOMPARE(data[1], uint32_t(newExtents.right));
    QCOMPARE(data[2], uint32_t(newExtents.top));
    QCOMPARE(data[3], uint32_t(newExtents.bottom));

    // and wait for our event
    waitForPropertyChange(&info, atom, NET::WMStrut);
    extents = info.strut();
    QCOMPARE(extents.bottom, newExtents.bottom);
    QCOMPARE(extents.left,   newExtents.left);
    QCOMPARE(extents.right,  newExtents.right);
    QCOMPARE(extents.top,    newExtents.top);
}

void NetWinInfoTestClient::testExtendedStrut()
{
    QVERIFY(connection());
    ATOM(_NET_WM_STRUT_PARTIAL)
    INFO

    NETExtendedStrut extents = info.extendedStrut();
    QCOMPARE(extents.left_width, 0);
    QCOMPARE(extents.right_width, 0);
    QCOMPARE(extents.top_width, 0);
    QCOMPARE(extents.bottom_width, 0);
    QCOMPARE(extents.left_start, 0);
    QCOMPARE(extents.left_end, 0);
    QCOMPARE(extents.right_start, 0);
    QCOMPARE(extents.right_end, 0);
    QCOMPARE(extents.top_start, 0);
    QCOMPARE(extents.top_end, 0);
    QCOMPARE(extents.bottom_start, 0);
    QCOMPARE(extents.bottom_end, 0);

    NETExtendedStrut newExtents;
    newExtents.left_width   = 10;
    newExtents.right_width  = 20;
    newExtents.top_width    = 30;
    newExtents.bottom_width = 40;
    newExtents.left_start   = 50;
    newExtents.left_end     = 60;
    newExtents.right_start  = 70;
    newExtents.right_end    = 80;
    newExtents.top_start    = 90;
    newExtents.top_end      = 91;
    newExtents.bottom_start = 92;
    newExtents.bottom_end   = 93;
    info.setExtendedStrut(newExtents);
    extents = info.extendedStrut();
    QCOMPARE(extents.left_width,   newExtents.left_width);
    QCOMPARE(extents.right_width,  newExtents.right_width);
    QCOMPARE(extents.top_width,    newExtents.top_width);
    QCOMPARE(extents.bottom_width, newExtents.bottom_width);
    QCOMPARE(extents.left_start,   newExtents.left_start);
    QCOMPARE(extents.left_end,     newExtents.left_end);
    QCOMPARE(extents.right_start,  newExtents.right_start);
    QCOMPARE(extents.right_end,    newExtents.right_end);
    QCOMPARE(extents.top_start,    newExtents.top_start);
    QCOMPARE(extents.top_end,      newExtents.top_end);
    QCOMPARE(extents.bottom_start, newExtents.bottom_start);
    QCOMPARE(extents.bottom_end,   newExtents.bottom_end);

    // compare with the X property
    QVERIFY(atom != XCB_ATOM_NONE);
    GETPROP(XCB_ATOM_CARDINAL, 12, 32)
    uint32_t *data = reinterpret_cast<uint32_t *>(xcb_get_property_value(reply.data()));
    QCOMPARE(data[ 0], uint32_t(newExtents.left_width));
    QCOMPARE(data[ 1], uint32_t(newExtents.right_width));
    QCOMPARE(data[ 2], uint32_t(newExtents.top_width));
    QCOMPARE(data[ 3], uint32_t(newExtents.bottom_width));
    QCOMPARE(data[ 4], uint32_t(newExtents.left_start));
    QCOMPARE(data[ 5], uint32_t(newExtents.left_end));
    QCOMPARE(data[ 6], uint32_t(newExtents.right_start));
    QCOMPARE(data[ 7], uint32_t(newExtents.right_end));
    QCOMPARE(data[ 8], uint32_t(newExtents.top_start));
    QCOMPARE(data[ 9], uint32_t(newExtents.top_end));
    QCOMPARE(data[10], uint32_t(newExtents.bottom_start));
    QCOMPARE(data[11], uint32_t(newExtents.bottom_end));

    // and wait for our event
    waitForPropertyChange(&info, atom, NET::Property(0), NET::WM2ExtendedStrut);
    extents = info.extendedStrut();
    QCOMPARE(extents.left_width,   newExtents.left_width);
    QCOMPARE(extents.right_width,  newExtents.right_width);
    QCOMPARE(extents.top_width,    newExtents.top_width);
    QCOMPARE(extents.bottom_width, newExtents.bottom_width);
    QCOMPARE(extents.left_start,   newExtents.left_start);
    QCOMPARE(extents.left_end,     newExtents.left_end);
    QCOMPARE(extents.right_start,  newExtents.right_start);
    QCOMPARE(extents.right_end,    newExtents.right_end);
    QCOMPARE(extents.top_start,    newExtents.top_start);
    QCOMPARE(extents.top_end,      newExtents.top_end);
    QCOMPARE(extents.bottom_start, newExtents.bottom_start);
    QCOMPARE(extents.bottom_end,   newExtents.bottom_end);
}

void NetWinInfoTestClient::testIconGeometry()
{
    QVERIFY(connection());
    ATOM(_NET_WM_ICON_GEOMETRY)
    INFO

    NETRect geo = info.iconGeometry();
    QCOMPARE(geo.pos.x, 0);
    QCOMPARE(geo.pos.y, 0);
    QCOMPARE(geo.size.width, 0);
    QCOMPARE(geo.size.height, 0);

    NETRect newGeo;
    newGeo.pos.x       = 10;
    newGeo.pos.y       = 20;
    newGeo.size.width  = 30;
    newGeo.size.height = 40;
    info.setIconGeometry(newGeo);
    geo = info.iconGeometry();
    QCOMPARE(geo.pos.x,       newGeo.pos.x);
    QCOMPARE(geo.pos.y,       newGeo.pos.y);
    QCOMPARE(geo.size.width,  newGeo.size.width);
    QCOMPARE(geo.size.height, newGeo.size.height);

    // compare with the X property
    QVERIFY(atom != XCB_ATOM_NONE);
    GETPROP(XCB_ATOM_CARDINAL, 4, 32)
    uint32_t *data = reinterpret_cast<uint32_t *>(xcb_get_property_value(reply.data()));
    QCOMPARE(data[0], uint32_t(newGeo.pos.x));
    QCOMPARE(data[1], uint32_t(newGeo.pos.y));
    QCOMPARE(data[2], uint32_t(newGeo.size.width));
    QCOMPARE(data[3], uint32_t(newGeo.size.height));

    // and wait for our event
    waitForPropertyChange(&info, atom, NET::WMIconGeometry);
    geo = info.iconGeometry();
    QCOMPARE(geo.pos.x,       newGeo.pos.x);
    QCOMPARE(geo.pos.y,       newGeo.pos.y);
    QCOMPARE(geo.size.width,  newGeo.size.width);
    QCOMPARE(geo.size.height, newGeo.size.height);
}

Q_DECLARE_METATYPE(NET::WindowType)
void NetWinInfoTestClient::testWindowType_data()
{
    QTest::addColumn<NET::WindowType>("type");
    QTest::addColumn<int>("length");
    QTest::addColumn<QByteArray>("typeAtom");
    QTest::addColumn<QByteArray>("secondaryTypeAtom");

    QTest::newRow("override")     << NET::Override     << 2 << QByteArrayLiteral("_KDE_NET_WM_WINDOW_TYPE_OVERRIDE")  << QByteArrayLiteral("_NET_WM_WINDOW_TYPE_NORMAL");
    QTest::newRow("TopMenu")      << NET::TopMenu      << 2 << QByteArrayLiteral("_KDE_NET_WM_WINDOW_TYPE_TOPMENU")   << QByteArrayLiteral("_NET_WM_WINDOW_TYPE_DOCK");
    QTest::newRow("Utility")      << NET::Utility      << 2 << QByteArrayLiteral("_NET_WM_WINDOW_TYPE_UTILITY")       << QByteArrayLiteral("_NET_WM_WINDOW_TYPE_DIALOG");
    QTest::newRow("Splash")       << NET::Splash       << 2 << QByteArrayLiteral("_NET_WM_WINDOW_TYPE_SPLASH")        << QByteArrayLiteral("_NET_WM_WINDOW_TYPE_DOCK");
    // TODO: this should be 2
    QTest::newRow("DropdownMenu") << NET::DropdownMenu << 1 << QByteArrayLiteral("_NET_WM_WINDOW_TYPE_DROPDOWN_MENU") << QByteArrayLiteral("_NET_WM_WINDOW_TYPE_MENU");
    // TODO: this should be 2
    QTest::newRow("PopupMenu")    << NET::PopupMenu    << 1 << QByteArrayLiteral("_NET_WM_WINDOW_TYPE_POPUP_MENU")    << QByteArrayLiteral("_NET_WM_WINDOW_TYPE_MENU");
    // TODO: this should be 2
    QTest::newRow("Notification") << NET::Notification << 1 << QByteArrayLiteral("_NET_WM_WINDOW_TYPE_NOTIFICATION")  << QByteArrayLiteral("_NET_WM_WINDOW_TYPE_UTILITY");
    QTest::newRow("Dialog")       << NET::Dialog       << 1 << QByteArrayLiteral("_NET_WM_WINDOW_TYPE_DIALOG")   << QByteArray();
    QTest::newRow("Menu")         << NET::Menu         << 1 << QByteArrayLiteral("_NET_WM_WINDOW_TYPE_MENU")     << QByteArray();
    QTest::newRow("Toolbar")      << NET::Toolbar      << 1 << QByteArrayLiteral("_NET_WM_WINDOW_TYPE_TOOLBAR")  << QByteArray();
    QTest::newRow("Dock")         << NET::Dock         << 1 << QByteArrayLiteral("_NET_WM_WINDOW_TYPE_DOCK")     << QByteArray();
    QTest::newRow("Desktop")      << NET::Desktop      << 1 << QByteArrayLiteral("_NET_WM_WINDOW_TYPE_DESKTOP")  << QByteArray();
    QTest::newRow("Tooltip")      << NET::Tooltip      << 1 << QByteArrayLiteral("_NET_WM_WINDOW_TYPE_TOOLTIP")  << QByteArray();
    QTest::newRow("ComboBox")     << NET::ComboBox     << 1 << QByteArrayLiteral("_NET_WM_WINDOW_TYPE_COMBOBOX") << QByteArray();
    QTest::newRow("DNDIcon")      << NET::DNDIcon      << 1 << QByteArrayLiteral("_NET_WM_WINDOW_TYPE_DND")      << QByteArray();
    QTest::newRow("Normal")       << NET::Normal       << 1 << QByteArrayLiteral("_NET_WM_WINDOW_TYPE_NORMAL")   << QByteArray();
    QTest::newRow("OnScreenDisplay") << NET::OnScreenDisplay << 1 << QByteArrayLiteral("_KDE_NET_WM_WINDOW_TYPE_ON_SCREEN_DISPLAY") << QByteArray();
}

void NetWinInfoTestClient::testWindowType()
{
    QVERIFY(connection());
    ATOM(_NET_WM_WINDOW_TYPE)
    INFO

    QVERIFY(info.hasWindowType());
    QVERIFY(!info.hasNETSupport());
    QCOMPARE(info.windowType(NET::AllTypesMask), NET::Unknown);
    QFETCH(NET::WindowType, type);
    info.setWindowType(type);
    // it does not update the internal type!
    QCOMPARE(info.windowType(NET::AllTypesMask), NET::Unknown);
    QFETCH(int, length);
    QFETCH(QByteArray, typeAtom);

    // compare the X property
    KXUtils::Atom type1(connection(), typeAtom);
    QVERIFY(atom != XCB_ATOM_NONE);
    GETPROP(XCB_ATOM_ATOM, length, 32)
    xcb_atom_t *atoms = reinterpret_cast<xcb_atom_t *>(xcb_get_property_value(reply.data()));
    QCOMPARE(atoms[0], xcb_atom_t(type1));
    if (reply->value_len > 1) {
        QFETCH(QByteArray, secondaryTypeAtom);
        KXUtils::Atom type2(connection(), secondaryTypeAtom);
        QVERIFY(type2 != XCB_ATOM_NONE);
        QCOMPARE(atoms[1], xcb_atom_t(type2));
    }

    waitForPropertyChange(&info, atom, NET::WMWindowType);
    QCOMPARE(info.windowType(NET::AllTypesMask), type);
    QVERIFY(info.hasNETSupport());
}

void NetWinInfoTestClient::testClientMachine()
{
    QVERIFY(connection());
    INFO

    QVERIFY(!info.clientMachine());

    // client machine needs to be changed using xcb
    xcb_change_property(connection(), XCB_PROP_MODE_REPLACE, m_testWindow,
                        XCB_ATOM_WM_CLIENT_MACHINE, XCB_ATOM_STRING, 8, 9, "localhost");
    xcb_flush(connection());

    // only updated after event
    waitForPropertyChange(&info, XCB_ATOM_WM_CLIENT_MACHINE, NET::Property(0), NET::WM2ClientMachine);
    QCOMPARE(info.clientMachine(), "localhost");
}

void NetWinInfoTestClient::testGroupLeader()
{
    QVERIFY(connection());
    INFO

    QVERIFY(info.groupLeader() == XCB_WINDOW_NONE);

    // group leader needs to be changed through wm hints
    uint32_t values[] = {
        1 << 6, /* WindowGroupHint*/
        1, /* Input */
        1, /* Normal State */
        XCB_NONE, /* icon pixmap */
        XCB_NONE, /* icon window */
        XCB_NONE, /* icon x */
        XCB_NONE, /* icon y */
        XCB_NONE, /* icon mask */
        m_rootWindow /* group leader */
    };
    xcb_change_property(connection(), XCB_PROP_MODE_REPLACE, m_testWindow,
                        XCB_ATOM_WM_HINTS, XCB_ATOM_WM_HINTS, 32, 9, values);
    xcb_flush(connection());

    // only updated after event
    waitForPropertyChange(&info, XCB_ATOM_WM_HINTS, NET::Property(0), NET::WM2GroupLeader);
    QCOMPARE(info.groupLeader(), m_rootWindow);
}

void NetWinInfoTestClient::testUrgency_data()
{
    QTest::addColumn<quint32>("flags");
    QTest::addColumn<bool>("expected");

    QTest::newRow("urgency") << quint32(1 << 8) << true;
    QTest::newRow("none") << quint32(0) << false;
    QTest::newRow("group_urgency") << quint32((1 << 6) | (1 << 8)) << true;
    QTest::newRow("input") << quint32(1) << false;
}

void NetWinInfoTestClient::testUrgency()
{
    QVERIFY(connection());
    INFO

    QVERIFY(!info.urgency());
    QFETCH(quint32, flags);

    // group leader needs to be changed through wm hints
    uint32_t values[] = {
        flags,
        1, /* Input */
        1, /* Normal State */
        XCB_NONE, /* icon pixmap */
        XCB_NONE, /* icon window */
        XCB_NONE, /* icon x */
        XCB_NONE, /* icon y */
        XCB_NONE, /* icon mask */
        XCB_NONE /* group leader */
    };
    xcb_change_property(connection(), XCB_PROP_MODE_REPLACE, m_testWindow,
                        XCB_ATOM_WM_HINTS, XCB_ATOM_WM_HINTS, 32, 9, values);
    xcb_flush(connection());

    // only updated after event
    waitForPropertyChange(&info, XCB_ATOM_WM_HINTS, NET::Property(0), NET::WM2Urgency);
    QTEST(info.urgency(), "expected");
}

void NetWinInfoTestClient::testInput_data()
{
    QTest::addColumn<quint32>("flags");
    QTest::addColumn<quint32>("input");
    QTest::addColumn<bool>("expected");

    QTest::newRow("flag_input")      << quint32(1) << quint32(1) << true;
    QTest::newRow("flag_noinput")    << quint32(1) << quint32(0) << false;
    QTest::newRow("noflag_input")    << quint32(0) << quint32(1) << true;
    QTest::newRow("noflag_noinput")  << quint32(0) << quint32(0) << true;
    QTest::newRow("flag_with_other_input")   << quint32(1 | 1 << 8) << quint32(1) << true;
    QTest::newRow("flag_with_other_noinput") << quint32(1 | 1 << 8) << quint32(0) << false;
}

void NetWinInfoTestClient::testInput()
{
    QVERIFY(connection());
    INFO

    QVERIFY(info.input());
    QFETCH(quint32, flags);
    QFETCH(quint32, input);

    // group leader needs to be changed through wm hints
    uint32_t values[] = {
        flags,
        input, /* Input */
        1, /* Normal State */
        XCB_NONE, /* icon pixmap */
        XCB_NONE, /* icon window */
        XCB_NONE, /* icon x */
        XCB_NONE, /* icon y */
        XCB_NONE, /* icon mask */
        XCB_NONE /* group leader */
    };
    xcb_change_property(connection(), XCB_PROP_MODE_REPLACE, m_testWindow,
                        XCB_ATOM_WM_HINTS, XCB_ATOM_WM_HINTS, 32, 9, values);
    xcb_flush(connection());

    // only updated after event
    waitForPropertyChange(&info, XCB_ATOM_WM_HINTS, NET::Property(0), NET::WM2Urgency);
    QTEST(info.input(), "expected");
}

Q_DECLARE_METATYPE(NET::MappingState)

void NetWinInfoTestClient::testInitialMappingState_data()
{
    QTest::addColumn<quint32>("flags");
    QTest::addColumn<quint32>("state");
    QTest::addColumn<NET::MappingState>("expected");

    QTest::newRow("flag-iconic") << quint32(2) << quint32(3) << NET::Iconic;
    QTest::newRow("flag-normal") << quint32(2) << quint32(1) << NET::Visible;
    QTest::newRow("flag-invalid") << quint32(2) << quint32(8) << NET::Withdrawn;
    QTest::newRow("noflag-iconic") << quint32(256) << quint32(3) << NET::Withdrawn;
    QTest::newRow("noflag-normal") << quint32(256) << quint32(1) << NET::Withdrawn;
}

void NetWinInfoTestClient::testInitialMappingState()
{
    QVERIFY(connection());
    INFO

    QCOMPARE(info.initialMappingState(), NET::Withdrawn);
    QFETCH(quint32, flags);
    QFETCH(quint32, state);

    // group leader needs to be changed through wm hints
    uint32_t values[] = {
        flags,
        1, /* Input */
        state, /* Normal State */
        XCB_NONE, /* icon pixmap */
        XCB_NONE, /* icon window */
        XCB_NONE, /* icon x */
        XCB_NONE, /* icon y */
        XCB_NONE, /* icon mask */
        XCB_NONE /* group leader */
    };
    xcb_change_property(connection(), XCB_PROP_MODE_REPLACE, m_testWindow,
                        XCB_ATOM_WM_HINTS, XCB_ATOM_WM_HINTS, 32, 9, values);
    xcb_flush(connection());

    // only updated after event
    waitForPropertyChange(&info, XCB_ATOM_WM_HINTS, NET::Property(0), NET::WM2InitialMappingState);
    QTEST(info.initialMappingState(), "expected");
}

void NetWinInfoTestClient::testIconPixmap_data()
{
    QTest::addColumn<quint32>("flags");
    QTest::addColumn<quint32>("icon");
    QTest::addColumn<quint32>("mask");
    QTest::addColumn<quint32>("expectedPixmap");
    QTest::addColumn<quint32>("expectedMask");

    QTest::newRow("invalid-flags") << 1u << 2u << 3u << 0u << 0u;
    QTest::newRow("pixmap-flags") << 4u << 2u << 3u << 2u << 0u;
    QTest::newRow("mask-flags") << 32u << 2u << 3u << 0u << 3u;
    QTest::newRow("pixmap-mask-flags") << 36u << 2u << 3u << 2u << 3u;
}

void NetWinInfoTestClient::testIconPixmap()
{
    QVERIFY(connection());
    INFO

    QCOMPARE(info.icccmIconPixmap(), 0u);
    QCOMPARE(info.icccmIconPixmapMask(), 0u);
    QFETCH(quint32, flags);
    QFETCH(quint32, icon);
    QFETCH(quint32, mask);

    // icon pixmap needs to be changed through wm hints
    uint32_t values[] = {
        flags,
        1, /* Input */
        XCB_NONE, /* Normal State */
        icon,     /* icon pixmap */
        XCB_NONE, /* icon window */
        XCB_NONE, /* icon x */
        XCB_NONE, /* icon y */
        mask,     /* icon mask */
        XCB_NONE  /* group leader */
    };
    xcb_change_property(connection(), XCB_PROP_MODE_REPLACE, m_testWindow,
                        XCB_ATOM_WM_HINTS, XCB_ATOM_WM_HINTS, 32, 9, values);
    xcb_flush(connection());
    // only updated after event
    waitForPropertyChange(&info, XCB_ATOM_WM_HINTS, NET::Property(0), NET::WM2IconPixmap);
    QTEST(info.icccmIconPixmap(), "expectedPixmap");
    QTEST(info.icccmIconPixmapMask(), "expectedMask");
}

void NetWinInfoTestClient::testTransientFor()
{
    QVERIFY(connection());
    INFO

    QVERIFY(info.transientFor() == XCB_WINDOW_NONE);
    // transient for needs to be changed using xcb
    xcb_change_property(connection(), XCB_PROP_MODE_REPLACE, m_testWindow,
                        XCB_ATOM_WM_TRANSIENT_FOR, XCB_ATOM_WINDOW, 32, 1, &m_rootWindow);
    xcb_flush(connection());

    // only updated after event
    waitForPropertyChange(&info, XCB_ATOM_WM_TRANSIENT_FOR, NET::Property(0), NET::WM2TransientFor);
    QCOMPARE(info.transientFor(), m_rootWindow);
}

void NetWinInfoTestClient::testWindowClass()
{
    QVERIFY(connection());
    INFO

    QVERIFY(!info.windowClassClass());
    QVERIFY(!info.windowClassName());

    // window class needs to be changed using xcb
    xcb_change_property(connection(), XCB_PROP_MODE_REPLACE, m_testWindow,
                        XCB_ATOM_WM_CLASS, XCB_ATOM_STRING, 8, 7, "foo\0bar");
    xcb_flush(connection());

    // only updated after event
    waitForPropertyChange(&info, XCB_ATOM_WM_CLASS, NET::Property(0), NET::WM2WindowClass);
    QCOMPARE(info.windowClassName(), "foo");
    QCOMPARE(info.windowClassClass(), "bar");
}

void NetWinInfoTestClient::testWindowRole()
{
    QVERIFY(connection());
    ATOM(WM_WINDOW_ROLE)
    INFO

    QVERIFY(!info.windowRole());

    // window role needs to be changed using xcb
    xcb_change_property(connection(), XCB_PROP_MODE_REPLACE, m_testWindow,
                        atom, XCB_ATOM_STRING, 8, 3, "bar");
    xcb_flush(connection());

    // only updated after event
    waitForPropertyChange(&info, atom, NET::Property(0), NET::WM2WindowRole);
    QCOMPARE(info.windowRole(), "bar");
}

void NetWinInfoTestClient::testActivities_data()
{
    QTest::addColumn<QByteArray>("activities");
    QTest::addColumn<QByteArray>("expectedActivities");

    const QByteArray testActivities = QByteArrayLiteral("foo,bar");
    const QByteArray allActivities = QByteArrayLiteral(KDE_ALL_ACTIVITIES_UUID);

    QTest::newRow("activites") << testActivities << testActivities;
    QTest::newRow("empty") << QByteArray() << allActivities;
    QTest::newRow("\\0") << QByteArrayLiteral("\0") << allActivities;
}

void NetWinInfoTestClient::testActivities()
{
    QVERIFY(connection());
    ATOM(_KDE_NET_WM_ACTIVITIES)
    INFO

    QVERIFY(!info.activities());
    QFETCH(QByteArray, activities);

    // activities needs to be changed using xcb
    info.setActivities(activities.isNull() ? nullptr : activities.constData());
    xcb_flush(connection());

    // only updated after event
    waitForPropertyChange(&info, atom, NET::Property(0), NET::WM2Activities);
    QTEST(QByteArray(info.activities()), "expectedActivities");
}

Q_DECLARE_METATYPE(NET::Protocols)
void NetWinInfoTestClient::testProtocols_data()
{
    QTest::addColumn<NET::Protocols>("protocols");
    QTest::addColumn<bool>("takeFocus");
    QTest::addColumn<bool>("deleteWindow");
    QTest::addColumn<bool>("ping");
    QTest::addColumn<bool>("sync");
    QTest::addColumn<bool>("context");

    const NET::Protocol t = NET::TakeFocusProtocol;
    const NET::Protocol d = NET::DeleteWindowProtocol;
    const NET::Protocol p = NET::PingProtocol;
    const NET::Protocol s = NET::SyncRequestProtocol;
    const NET::Protocol c = NET::ContextHelpProtocol;

    QTest::newRow("none") << NET::Protocols(NET::NoProtocol) << false << false << false << false << false;

    QTest::newRow("t") << NET::Protocols(t) << true << false << false << false << false;
    QTest::newRow("d") << NET::Protocols(d) << false << true << false << false << false;
    QTest::newRow("p") << NET::Protocols(p) << false << false << true << false << false;
    QTest::newRow("s") << NET::Protocols(s) << false << false << false << true << false;
    QTest::newRow("c") << NET::Protocols(c) << false << false << false << false << true;

    // all two combinations with t
    QTest::newRow("t/d") << NET::Protocols(t | d) << true << true  << false << false << false;
    QTest::newRow("t/p") << NET::Protocols(t | p) << true << false << true  << false << false;
    QTest::newRow("t/s") << NET::Protocols(t | s) << true << false << false << true  << false;
    QTest::newRow("t/c") << NET::Protocols(t | c) << true << false << false << false << true;
    // all two combinations with d
    QTest::newRow("d/p") << NET::Protocols(d | p) << false << true << true  << false << false;
    QTest::newRow("d/s") << NET::Protocols(d | s) << false << true << false << true  << false;
    QTest::newRow("d/c") << NET::Protocols(d | c) << false << true << false << false << true;
    // all two combinations with p
    QTest::newRow("p/s") << NET::Protocols(p | s) << false << false << true << true  << false;
    QTest::newRow("p/c") << NET::Protocols(p | c) << false << false << true << false << true;
    // and remaining two combination
    QTest::newRow("s/c") << NET::Protocols(s | c) << false << false << false << true << true;

    // all three combinations with t
    QTest::newRow("t/d/p") << NET::Protocols(t | d | p) << true << true  << true  << false << false;
    QTest::newRow("t/d/s") << NET::Protocols(t | d | s) << true << true  << false << true  << false;
    QTest::newRow("t/d/c") << NET::Protocols(t | d | c) << true << true  << false << false << true;
    QTest::newRow("t/p/s") << NET::Protocols(t | p | s) << true << false << true  << true  << false;
    QTest::newRow("t/p/c") << NET::Protocols(t | p | c) << true << false << true  << false << true;
    QTest::newRow("t/s/c") << NET::Protocols(t | s | c) << true << false << false << true  << true;
    // all three combinations with d
    QTest::newRow("d/p/s") << NET::Protocols(d | p | s) << false << true << true  << true  << false;
    QTest::newRow("d/p/c") << NET::Protocols(d | p | c) << false << true << true  << false << true;
    QTest::newRow("d/s/c") << NET::Protocols(d | s | c) << false << true << false << true  << true;
    // and remaining
    QTest::newRow("p/s/c") << NET::Protocols(p | s | c) << false << false << true << true << true;

    QTest::newRow("t/d/p/s") << NET::Protocols(t | d | p | s) << true  << true  << true  << true  << false;
    QTest::newRow("t/d/p/c") << NET::Protocols(t | d | p | c) << true  << true  << true  << false << true;
    QTest::newRow("t/d/s/c") << NET::Protocols(t | d | s | c) << true  << true  << false << true  << true;
    QTest::newRow("t/p/s/c") << NET::Protocols(t | p | s | c) << true  << false << true  << true  << true;
    QTest::newRow("d/p/s/c") << NET::Protocols(d | p | s | c) << false << true  << true  << true  << true;

    QTest::newRow("all") << NET::Protocols(t | d | p | s | c) << true << true << true << true << true;
}

void NetWinInfoTestClient::testProtocols()
{
    QVERIFY(connection());
    ATOM(WM_PROTOCOLS)
    KXUtils::Atom takeFocus(connection(), QByteArrayLiteral("WM_TAKE_FOCUS"));
    KXUtils::Atom deleteWindow(connection(), QByteArrayLiteral("WM_DELETE_WINDOW"));
    KXUtils::Atom ping(connection(), QByteArrayLiteral("_NET_WM_PING"));
    KXUtils::Atom syncRequest(connection(), QByteArrayLiteral("_NET_WM_SYNC_REQUEST"));
    KXUtils::Atom contextHelp(connection(), QByteArrayLiteral("_NET_WM_CONTEXT_HELP"));
    INFO

    QVERIFY(!info.supportsProtocol(NET::TakeFocusProtocol));
    QVERIFY(!info.supportsProtocol(NET::DeleteWindowProtocol));
    QVERIFY(!info.supportsProtocol(NET::PingProtocol));
    QVERIFY(!info.supportsProtocol(NET::SyncRequestProtocol));
    QVERIFY(!info.supportsProtocol(NET::ContextHelpProtocol));
    QCOMPARE(info.protocols(), NET::Protocols(NET::NoProtocol));

    QVector<xcb_atom_t> props;
    QFETCH(NET::Protocols, protocols);
    if (protocols.testFlag(NET::TakeFocusProtocol)) {
        props << takeFocus;
    }
    if (protocols.testFlag(NET::DeleteWindowProtocol)) {
        props << deleteWindow;
    }
    if (protocols.testFlag(NET::PingProtocol)) {
        props << ping;
    }
    if (protocols.testFlag(NET::SyncRequestProtocol)) {
        props << syncRequest;
    }
    if (protocols.testFlag(NET::ContextHelpProtocol)) {
        props << contextHelp;
    }

    xcb_change_property(connection(), XCB_PROP_MODE_REPLACE, m_testWindow, atom, XCB_ATOM_ATOM, 32, props.size(), props.constData());
    xcb_flush(connection());

    // only updated after event
    waitForPropertyChange(&info, atom, NET::Property(0), NET::WM2Protocols);
    QCOMPARE(info.protocols(), protocols);
    QTEST(info.supportsProtocol(NET::TakeFocusProtocol), "takeFocus");
    QTEST(info.supportsProtocol(NET::DeleteWindowProtocol), "deleteWindow");
    QTEST(info.supportsProtocol(NET::PingProtocol), "ping");
    QTEST(info.supportsProtocol(NET::SyncRequestProtocol), "sync");
    QTEST(info.supportsProtocol(NET::ContextHelpProtocol), "context");

    xcb_delete_property(connection(), m_testWindow, atom);
    xcb_flush(connection());
    waitForPropertyChange(&info, atom, NET::Property(0), NET::WM2Protocols);
    QVERIFY(!info.supportsProtocol(NET::TakeFocusProtocol));
    QVERIFY(!info.supportsProtocol(NET::DeleteWindowProtocol));
    QVERIFY(!info.supportsProtocol(NET::PingProtocol));
    QVERIFY(!info.supportsProtocol(NET::SyncRequestProtocol));
    QVERIFY(!info.supportsProtocol(NET::ContextHelpProtocol));
    QCOMPARE(info.protocols(), NET::Protocols(NET::NoProtocol));
}

void NetWinInfoTestClient::testOpaqueRegion_data()
{
    QTest::addColumn<QVector<QRect> >("geometries");

    QTest::newRow("none") << QVector<QRect>();
    QTest::newRow("empty") << QVector<QRect>({QRect(0, 0, 0, 0)});
    QTest::newRow("one rect") << QVector<QRect>({QRect(10, 20, 30, 40)});
    QTest::newRow("two rect") << QVector<QRect>({QRect(10, 20, 30, 40), QRect(1, 2, 4, 5)});
    QTest::newRow("multiple") << QVector<QRect>({QRect(10, 20, 30, 40),
                                                 QRect(1, 2, 4, 5),
                                                 QRect(100, 0, 200, 400),
                                                 QRect(1, 2, 4, 5)});
}

void NetWinInfoTestClient::testOpaqueRegion()
{
    QVERIFY(connection());
    ATOM(_NET_WM_OPAQUE_REGION)
    INFO

    QCOMPARE(info.opaqueRegion().size(), std::size_t(0));

    QFETCH(QVector<QRect>, geometries);
    QVector<qint32> data;
    for (auto it = geometries.constBegin(); it != geometries.constEnd(); ++it) {
        const QRect &r = *it;
        data << r.x();
        data << r.y();
        data << r.width();
        data << r.height();
    }

    xcb_change_property(connection(), XCB_PROP_MODE_REPLACE, m_testWindow, atom, XCB_ATOM_CARDINAL, 32, data.size(), data.constData());
    xcb_flush(connection());

    // only updated after event
    waitForPropertyChange(&info, atom, NET::Property(0), NET::WM2OpaqueRegion);
    const auto opaqueRegion = info.opaqueRegion();
    QCOMPARE(opaqueRegion.size(), std::size_t(geometries.size()));

    for (std::size_t i = 0; i < opaqueRegion.size(); ++i) {
        auto r1 = opaqueRegion.at(i);
        auto r2 = geometries.at(i);
        QCOMPARE(r1.pos.x, r2.x());
        QCOMPARE(r1.pos.y, r2.y());
        QCOMPARE(r1.size.width, r2.width());
        QCOMPARE(r1.size.height, r2.height());
    }

    xcb_delete_property(connection(), m_testWindow, atom);
    xcb_flush(connection());
    waitForPropertyChange(&info, atom, NET::Property(0), NET::WM2OpaqueRegion);
    QCOMPARE(info.opaqueRegion().size(), std::size_t(0));
}

QTEST_GUILESS_MAIN(NetWinInfoTestClient)

#include "netwininfotestclient.moc"
