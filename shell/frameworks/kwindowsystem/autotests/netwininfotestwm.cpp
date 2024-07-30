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

Q_DECLARE_METATYPE(NET::State)
Q_DECLARE_METATYPE(NET::States)
Q_DECLARE_METATYPE(NET::Actions)

using Property = UniqueCPointer<xcb_get_property_reply_t>;

// clang-format off
#define INFO NETWinInfo info(m_connection, m_testWindow, m_rootWindow, NET::WMAllProperties, NET::WM2AllProperties, NET::WindowManager);

#define ATOM(name) \
    KXUtils::Atom atom(connection(), QByteArrayLiteral(#name));

#define UTF8 KXUtils::Atom utf8String(connection(), QByteArrayLiteral("UTF8_STRING"));

#define GETPROP(type, length, formatSize) \
    xcb_get_property_cookie_t cookie = xcb_get_property_unchecked(connection(), false, m_testWindow, \
                                       atom, type, 0, length); \
    Property reply(xcb_get_property_reply(connection(), cookie, nullptr)); \
    QVERIFY(reply); \
    QCOMPARE(reply->format, uint8_t(formatSize)); \
    QCOMPARE(reply->value_len, uint32_t(length));

#define VERIFYDELETED(t) \
    xcb_get_property_cookie_t cookieDeleted = xcb_get_property_unchecked(connection(), false, m_testWindow, \
            atom, t, 0, 1); \
    Property replyDeleted(xcb_get_property_reply(connection(), cookieDeleted, nullptr)); \
    QVERIFY(replyDeleted); \
    QVERIFY(replyDeleted->type == XCB_ATOM_NONE);

class NetWinInfoTestWM : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testState_data();
    void testState();
    void testVisibleName();
    void testVisibleIconName();
    void testDesktop_data();
    void testDesktop();
    void testOpacity_data();
    void testOpacity();
    void testAllowedActions_data();
    void testAllowedActions();
    void testFrameExtents();
    void testFrameExtentsKDE();
    void testFrameOverlap();
    void testFullscreenMonitors();

private:
    bool hasAtomFlag(const xcb_atom_t *atoms, int atomsLenght, const QByteArray &actionName);
    void testStrut(xcb_atom_t atom, NETStrut(NETWinInfo:: *getter)(void)const, void (NETWinInfo:: *setter)(NETStrut), NET::Property property, NET::Property2 property2 = NET::Property2(0));
    void waitForPropertyChange(NETWinInfo *info, xcb_atom_t atom, NET::Property prop, NET::Property2 prop2 = NET::Property2(0));
    xcb_connection_t *connection()
    {
        return m_connection;
    }
    xcb_connection_t *m_connection;
    QList<xcb_connection_t*> m_connections;
    std::unique_ptr<QProcess> m_xvfb;
    xcb_window_t m_rootWindow;
    xcb_window_t m_testWindow;
    QByteArray m_displayNumber;
};

void NetWinInfoTestWM::initTestCase()
{
}

void NetWinInfoTestWM::cleanupTestCase()
{
    // close connection
    while (!m_connections.isEmpty()) {
        xcb_disconnect(m_connections.takeFirst());
    }
}

void NetWinInfoTestWM::init()
{
    // first reset just to be sure
    m_connection = nullptr;
    m_rootWindow = XCB_WINDOW_NONE;
    m_testWindow = XCB_WINDOW_NONE;

    const QString xfvbExec = QStandardPaths::findExecutable(QStringLiteral("Xvfb"));
    QVERIFY(!xfvbExec.isEmpty());

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
    m_displayNumber = displayNumber;

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

void NetWinInfoTestWM::cleanup()
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

void NetWinInfoTestWM::waitForPropertyChange(NETWinInfo *info, xcb_atom_t atom, NET::Property prop, NET::Property2 prop2)
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
        if (pe->window != m_testWindow) {
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

bool NetWinInfoTestWM::hasAtomFlag(const xcb_atom_t *atoms, int atomsLength, const QByteArray &actionName)
{
    KXUtils::Atom atom(connection(), actionName);
    if (atom == XCB_ATOM_NONE) {
        qDebug() << "get atom failed";
        return false;
    }
    for (int i = 0; i < atomsLength; ++i) {
        if (atoms[i] == atom) {
            return true;
        }
    }
    return false;
}

void NetWinInfoTestWM::testAllowedActions_data()
{
    QTest::addColumn<NET::Actions>("actions");
    QTest::addColumn<QList<QByteArray> >("names");

    const QByteArray move       = QByteArrayLiteral("_NET_WM_ACTION_MOVE");
    const QByteArray resize     = QByteArrayLiteral("_NET_WM_ACTION_RESIZE");
    const QByteArray minimize   = QByteArrayLiteral("_NET_WM_ACTION_MINIMIZE");
    const QByteArray shade      = QByteArrayLiteral("_NET_WM_ACTION_SHADE");
    const QByteArray stick      = QByteArrayLiteral("_NET_WM_ACTION_STICK");
    const QByteArray maxVert    = QByteArrayLiteral("_NET_WM_ACTION_MAXIMIZE_VERT");
    const QByteArray maxHoriz   = QByteArrayLiteral("_NET_WM_ACTION_MAXIMIZE_HORZ");
    const QByteArray fullscreen = QByteArrayLiteral("_NET_WM_ACTION_FULLSCREEN");
    const QByteArray desktop    = QByteArrayLiteral("_NET_WM_ACTION_CHANGE_DESKTOP");
    const QByteArray close      = QByteArrayLiteral("_NET_WM_ACTION_CLOSE");

    QTest::newRow("move")       << NET::Actions(NET::ActionMove)          << (QList<QByteArray>() << move);
    QTest::newRow("resize")     << NET::Actions(NET::ActionResize)        << (QList<QByteArray>() << resize);
    QTest::newRow("minimize")   << NET::Actions(NET::ActionMinimize)      << (QList<QByteArray>() << minimize);
    QTest::newRow("shade")      << NET::Actions(NET::ActionShade)         << (QList<QByteArray>() << shade);
    QTest::newRow("stick")      << NET::Actions(NET::ActionStick)         << (QList<QByteArray>() << stick);
    QTest::newRow("maxVert")    << NET::Actions(NET::ActionMaxVert)       << (QList<QByteArray>() << maxVert);
    QTest::newRow("maxHoriz")   << NET::Actions(NET::ActionMaxHoriz)      << (QList<QByteArray>() << maxHoriz);
    QTest::newRow("fullscreen") << NET::Actions(NET::ActionFullScreen)    << (QList<QByteArray>() << fullscreen);
    QTest::newRow("desktop")    << NET::Actions(NET::ActionChangeDesktop) << (QList<QByteArray>() << desktop);
    QTest::newRow("close")      << NET::Actions(NET::ActionClose)         << (QList<QByteArray>() << close);

    QTest::newRow("none") << NET::Actions() << QList<QByteArray>();

    QTest::newRow("all") << NET::Actions(NET::ActionMove |
                                    NET::ActionResize |
                                    NET::ActionMinimize |
                                    NET::ActionShade |
                                    NET::ActionStick |
                                    NET::ActionMaxVert |
                                    NET::ActionMaxHoriz |
                                    NET::ActionFullScreen |
                                    NET::ActionChangeDesktop |
                                    NET::ActionClose)
                         << (QList<QByteArray>() << move << resize << minimize << shade <<
                             stick << maxVert << maxHoriz <<
                             fullscreen << desktop << close);
}

void NetWinInfoTestWM::testAllowedActions()
{
    QVERIFY(connection());
    ATOM(_NET_WM_ALLOWED_ACTIONS)
    INFO

    QCOMPARE(info.allowedActions(), NET::Actions());
    QFETCH(NET::Actions, actions);
    info.setAllowedActions(actions);
    QCOMPARE(info.allowedActions(), actions);

    // compare with the X property
    QFETCH(QList<QByteArray>, names);
    QVERIFY(atom != XCB_ATOM_NONE);
    GETPROP(XCB_ATOM_ATOM, names.size(), 32)
    xcb_atom_t *atoms = reinterpret_cast<xcb_atom_t *>(xcb_get_property_value(reply.get()));
    for (int i = 0; i < names.size(); ++i) {
        QVERIFY(hasAtomFlag(atoms, names.size(), names.at(i)));
    }

    // and wait for our event
    waitForPropertyChange(&info, atom, NET::Property(0), NET::WM2AllowedActions);
    QCOMPARE(info.allowedActions(), actions);
}

void NetWinInfoTestWM::testDesktop_data()
{
    QTest::addColumn<int>("desktop");
    QTest::addColumn<uint32_t>("propertyDesktop");

    QTest::newRow("1") << 1 << uint32_t(0);
    QTest::newRow("4") << 4 << uint32_t(3);
    QTest::newRow("on all") << int(NET::OnAllDesktops) << uint32_t(~0);
}

void NetWinInfoTestWM::testDesktop()
{
    QVERIFY(connection());
    ATOM(_NET_WM_DESKTOP)
    INFO

    QCOMPARE(info.desktop(), 0);
    QFETCH(int, desktop);
    info.setDesktop(desktop);
    QCOMPARE(info.desktop(), desktop);

    // compare with the X property
    QVERIFY(atom != XCB_ATOM_NONE);
    GETPROP(XCB_ATOM_CARDINAL, 1, 32)
    QTEST(reinterpret_cast<uint32_t *>(xcb_get_property_value(reply.get()))[0], "propertyDesktop");

    // and wait for our event
    waitForPropertyChange(&info, atom, NET::WMDesktop);
    QCOMPARE(info.desktop(), desktop);

    // delete it
    info.setDesktop(0);
    QCOMPARE(info.desktop(), 0);
    VERIFYDELETED(XCB_ATOM_CARDINAL)

    // and wait for our event
    waitForPropertyChange(&info, atom, NET::WMDesktop);
    QCOMPARE(info.desktop(), 0);
}

void NetWinInfoTestWM::testStrut(xcb_atom_t atom, NETStrut(NETWinInfo:: *getter)(void)const, void (NETWinInfo:: *setter)(NETStrut), NET::Property property, NET::Property2 property2)
{
    INFO

    NETStrut extents = (info.*getter)();
    QCOMPARE(extents.bottom, 0);
    QCOMPARE(extents.left, 0);
    QCOMPARE(extents.right, 0);
    QCOMPARE(extents.top, 0);

    NETStrut newExtents;
    newExtents.bottom = 10;
    newExtents.left   = 20;
    newExtents.right  = 30;
    newExtents.top    = 40;
    (info.*setter)(newExtents);
    extents = (info.*getter)();
    QCOMPARE(extents.bottom, newExtents.bottom);
    QCOMPARE(extents.left,   newExtents.left);
    QCOMPARE(extents.right,  newExtents.right);
    QCOMPARE(extents.top,    newExtents.top);

    // compare with the X property
    QVERIFY(atom != XCB_ATOM_NONE);
    GETPROP(XCB_ATOM_CARDINAL, 4, 32)
    uint32_t *data = reinterpret_cast<uint32_t *>(xcb_get_property_value(reply.get()));
    QCOMPARE(data[0], uint32_t(newExtents.left));
    QCOMPARE(data[1], uint32_t(newExtents.right));
    QCOMPARE(data[2], uint32_t(newExtents.top));
    QCOMPARE(data[3], uint32_t(newExtents.bottom));

    // and wait for our event
    waitForPropertyChange(&info, atom, property, property2);
    extents = (info.*getter)();
    QCOMPARE(extents.bottom, newExtents.bottom);
    QCOMPARE(extents.left,   newExtents.left);
    QCOMPARE(extents.right,  newExtents.right);
    QCOMPARE(extents.top,    newExtents.top);
}

void NetWinInfoTestWM::testFrameExtents()
{
    QVERIFY(connection());
    ATOM(_NET_FRAME_EXTENTS)
    testStrut(atom, &NETWinInfo::frameExtents, &NETWinInfo::setFrameExtents, NET::WMFrameExtents);
}

void NetWinInfoTestWM::testFrameExtentsKDE()
{
    // same as testFrameExtents just with a different atom name
    QVERIFY(connection());
    ATOM(_KDE_NET_WM_FRAME_STRUT)
    testStrut(atom, &NETWinInfo::frameExtents, &NETWinInfo::setFrameExtents, NET::WMFrameExtents);
}

void NetWinInfoTestWM::testFrameOverlap()
{
    QVERIFY(connection());
    ATOM(_NET_WM_FRAME_OVERLAP)
    testStrut(atom, &NETWinInfo::frameOverlap, &NETWinInfo::setFrameOverlap, NET::Property(0), NET::WM2FrameOverlap);
}

void NetWinInfoTestWM::testOpacity_data()
{
    QTest::addColumn<uint32_t>("opacity");

    QTest::newRow("0 %")   << uint32_t(0);
    QTest::newRow("50 %")  << uint32_t(0x0000ffff);
    QTest::newRow("100 %") << uint32_t(0xffffffff);
}

void NetWinInfoTestWM::testOpacity()
{
    QVERIFY(connection());
    ATOM(_NET_WM_WINDOW_OPACITY)
    INFO

    QCOMPARE(info.opacity(), static_cast<unsigned long>(0xffffffffU));
    QFETCH(uint32_t, opacity);
    info.setOpacity(opacity);
    QCOMPARE(info.opacity(), static_cast<unsigned long>(opacity));

    // compare with the X property
    QVERIFY(atom != XCB_ATOM_NONE);
    GETPROP(XCB_ATOM_CARDINAL, 1, 32)
    QCOMPARE(reinterpret_cast<uint32_t *>(xcb_get_property_value(reply.get()))[0], opacity);

    // and wait for our event
    waitForPropertyChange(&info, atom, NET::Property(0), NET::WM2Opacity);
    QCOMPARE(info.opacity(), static_cast<unsigned long>(opacity));
}

void NetWinInfoTestWM::testState_data()
{
    QTest::addColumn<NET::States>("states");
    QTest::addColumn<QList<QByteArray> >("names");

    const QByteArray modal            = QByteArrayLiteral("_NET_WM_STATE_MODAL");
    const QByteArray sticky           = QByteArrayLiteral("_NET_WM_STATE_STICKY");
    const QByteArray maxVert          = QByteArrayLiteral("_NET_WM_STATE_MAXIMIZED_VERT");
    const QByteArray maxHoriz         = QByteArrayLiteral("_NET_WM_STATE_MAXIMIZED_HORZ");
    const QByteArray shaded           = QByteArrayLiteral("_NET_WM_STATE_SHADED");
    const QByteArray skipTaskbar      = QByteArrayLiteral("_NET_WM_STATE_SKIP_TASKBAR");
    const QByteArray skipSwitcher     = QByteArrayLiteral("_KDE_NET_WM_STATE_SKIP_SWITCHER");
    const QByteArray keepAbove        = QByteArrayLiteral("_NET_WM_STATE_ABOVE");
    const QByteArray staysOnTop       = QByteArrayLiteral("_NET_WM_STATE_STAYS_ON_TOP");
    const QByteArray skipPager        = QByteArrayLiteral("_NET_WM_STATE_SKIP_PAGER");
    const QByteArray hidden           = QByteArrayLiteral("_NET_WM_STATE_HIDDEN");
    const QByteArray fullScreen       = QByteArrayLiteral("_NET_WM_STATE_FULLSCREEN");
    const QByteArray keepBelow        = QByteArrayLiteral("_NET_WM_STATE_BELOW");
    const QByteArray demandsAttention = QByteArrayLiteral("_NET_WM_STATE_DEMANDS_ATTENTION");
    const QByteArray focused          = QByteArrayLiteral("_NET_WM_STATE_FOCUSED");

    QTest::newRow("modal")            << NET::States(NET::Modal)            << (QList<QByteArray>() << modal);
    QTest::newRow("sticky")           << NET::States(NET::Sticky)           << (QList<QByteArray>() << sticky);
    QTest::newRow("maxVert")          << NET::States(NET::MaxVert)          << (QList<QByteArray>() << maxVert);
    QTest::newRow("maxHoriz")         << NET::States(NET::MaxHoriz)         << (QList<QByteArray>() << maxHoriz);
    QTest::newRow("shaded")           << NET::States(NET::Shaded)           << (QList<QByteArray>() << shaded);
    QTest::newRow("skipTaskbar")      << NET::States(NET::SkipTaskbar)      << (QList<QByteArray>() << skipTaskbar);
    QTest::newRow("keepAbove")        << NET::States(NET::KeepAbove)        << (QList<QByteArray>() << keepAbove << staysOnTop);
    QTest::newRow("skipPager")        << NET::States(NET::SkipPager)        << (QList<QByteArray>() << skipPager);
    QTest::newRow("hidden")           << NET::States(NET::Hidden)           << (QList<QByteArray>() << hidden);
    QTest::newRow("fullScreen")       << NET::States(NET::FullScreen)       << (QList<QByteArray>() << fullScreen);
    QTest::newRow("keepBelow")        << NET::States(NET::KeepBelow)        << (QList<QByteArray>() << keepBelow);
    QTest::newRow("demandsAttention") << NET::States(NET::DemandsAttention) << (QList<QByteArray>() << demandsAttention);
    QTest::newRow("skipSwitcher")     << NET::States(NET::SkipSwitcher)     << (QList<QByteArray>() << skipSwitcher);
    QTest::newRow("focused")          << NET::States(NET::Focused)          << (QList<QByteArray>() << focused);

    // TODO: it's possible to be keep above and below at the same time?!?
    QTest::newRow("all") << NET::States(NET::Modal |
                                   NET::Sticky |
                                   NET::Max |
                                   NET::Shaded |
                                   NET::SkipTaskbar |
                                   NET::SkipPager |
                                   NET::KeepAbove |
                                   NET::KeepBelow |
                                   NET::Hidden |
                                   NET::FullScreen |
                                   NET::DemandsAttention |
                                   NET::SkipSwitcher  |
                                   NET::Focused)
                         << (QList<QByteArray>() << modal << sticky << maxVert << maxHoriz
                             << shaded << skipTaskbar << keepAbove
                             << skipPager << hidden << fullScreen
                             << keepBelow << demandsAttention << staysOnTop << skipSwitcher << focused);
}

void NetWinInfoTestWM::testState()
{
    QVERIFY(connection());
    ATOM(_NET_WM_STATE)
    INFO

    QCOMPARE(info.state(), NET::States());
    QFETCH(NET::States, states);
    info.setState(states, NET::States());
    QCOMPARE(info.state(), states);

    // compare with the X property
    QFETCH(QList<QByteArray>, names);
    QVERIFY(atom != XCB_ATOM_NONE);
    GETPROP(XCB_ATOM_ATOM, names.size(), 32)
    xcb_atom_t *atoms = reinterpret_cast<xcb_atom_t *>(xcb_get_property_value(reply.get()));
    for (int i = 0; i < names.size(); ++i) {
        QVERIFY(hasAtomFlag(atoms, names.size(), names.at(i)));
    }

    // and wait for our event
    waitForPropertyChange(&info, atom, NET::WMState);
    QCOMPARE(info.state(), states);
}

void NetWinInfoTestWM::testVisibleIconName()
{
    QVERIFY(connection());
    ATOM(_NET_WM_VISIBLE_ICON_NAME)
    UTF8
    INFO

    QVERIFY(!info.visibleIconName());
    info.setVisibleIconName("foo");
    QCOMPARE(info.visibleIconName(), "foo");

    // compare with the X property
    QVERIFY(atom != XCB_ATOM_NONE);
    QVERIFY(utf8String != XCB_ATOM_NONE);
    GETPROP(utf8String, 3, 8)
    QCOMPARE(reinterpret_cast<const char *>(xcb_get_property_value(reply.get())), "foo");

    // and wait for our event
    waitForPropertyChange(&info, atom, NET::WMVisibleIconName);
    QCOMPARE(info.visibleIconName(), "foo");

    // delete the string
    info.setVisibleIconName("");
    QCOMPARE(info.visibleIconName(), "");
    VERIFYDELETED(utf8String)
    // and wait for our event
    waitForPropertyChange(&info, atom, NET::WMVisibleIconName);
    QVERIFY(!info.visibleIconName());

    // set again, to ensure we don't leak on tear down
    info.setVisibleIconName("bar");
    xcb_flush(connection());
    waitForPropertyChange(&info, atom, NET::WMVisibleIconName);
    QCOMPARE(info.visibleIconName(), "bar");
}

void NetWinInfoTestWM::testVisibleName()
{
    QVERIFY(connection());
    ATOM(_NET_WM_VISIBLE_NAME)
    UTF8
    INFO

    QVERIFY(!info.visibleName());
    info.setVisibleName("foo");
    QCOMPARE(info.visibleName(), "foo");

    // compare with the X property
    QVERIFY(atom != XCB_ATOM_NONE);
    QVERIFY(utf8String != XCB_ATOM_NONE);
    GETPROP(utf8String, 3, 8)
    QCOMPARE(reinterpret_cast<const char *>(xcb_get_property_value(reply.get())), "foo");

    // and wait for our event
    waitForPropertyChange(&info, atom, NET::WMVisibleName);
    QCOMPARE(info.visibleName(), "foo");

    // delete the string
    info.setVisibleName("");
    QCOMPARE(info.visibleName(), "");
    VERIFYDELETED(utf8String)

    // and wait for our event
    waitForPropertyChange(&info, atom, NET::WMVisibleName);
    QVERIFY(!info.visibleName());

    // set again, to ensure we don't leak on tear down
    info.setVisibleName("bar");
    xcb_flush(connection());
    waitForPropertyChange(&info, atom, NET::WMVisibleName);
    QCOMPARE(info.visibleName(), "bar");
}

class MockWinInfo : public NETWinInfo
{
public:
    MockWinInfo(xcb_connection_t *connection, xcb_window_t window, xcb_window_t rootWindow)
        : NETWinInfo(connection, window, rootWindow, NET::WMAllProperties, NET::WM2AllProperties, NET::WindowManager)
    {
    }

protected:
    void changeFullscreenMonitors(NETFullscreenMonitors topology) override
    {
        setFullscreenMonitors(topology);
    }
};

void NetWinInfoTestWM::testFullscreenMonitors()
{
    // test case for BUG 391960
    QVERIFY(connection());
    const uint32_t maskValues[] = {
                     XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
                     XCB_EVENT_MASK_KEY_PRESS |
                     XCB_EVENT_MASK_PROPERTY_CHANGE |
                     XCB_EVENT_MASK_COLOR_MAP_CHANGE |
                     XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
                     XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
                     XCB_EVENT_MASK_FOCUS_CHANGE | // For NotifyDetailNone
                     XCB_EVENT_MASK_EXPOSURE
    };
    UniqueCPointer<xcb_generic_error_t> redirectCheck(xcb_request_check(connection(), xcb_change_window_attributes_checked(connection(),
                                                      m_rootWindow, XCB_CW_EVENT_MASK, maskValues)));
    QVERIFY(!redirectCheck);

    KXUtils::Atom atom(connection(), QByteArrayLiteral("_NET_WM_FULLSCREEN_MONITORS"));

    // create client connection
    auto clientConnection = xcb_connect(m_displayNumber.constData(), nullptr);
    QVERIFY(clientConnection);
    QVERIFY(!xcb_connection_has_error(clientConnection));

    NETWinInfo clientInfo(clientConnection, m_testWindow, m_rootWindow, NET::WMAllProperties, NET::WM2AllProperties);
    NETFullscreenMonitors topology;
    topology.top = 1;
    topology.bottom = 2;
    topology.left = 3;
    topology.right = 4;
    clientInfo.setFullscreenMonitors(topology);
    xcb_flush(clientConnection);

    MockWinInfo info(connection(), m_testWindow, m_rootWindow);

    while (true) {
        UniqueCPointer<xcb_generic_event_t> event(xcb_wait_for_event(connection()));
        if (!event) {
            break;
        }
        if ((event->response_type & ~0x80) != XCB_CLIENT_MESSAGE) {
            continue;
        }

        NET::Properties dirtyProtocols;
        NET::Properties2 dirtyProtocols2;
        QCOMPARE(info.fullscreenMonitors().isSet(), false);
        info.event(event.get(), &dirtyProtocols, &dirtyProtocols2);
        QCOMPARE(info.fullscreenMonitors().isSet(), true);
        break;
    }
    xcb_flush(connection());
    // now the property should be updated
    waitForPropertyChange(&info, atom, NET::Property(0), NET::WM2FullscreenMonitors);

    QCOMPARE(info.fullscreenMonitors().top, 1);
    QCOMPARE(info.fullscreenMonitors().bottom, 2);
    QCOMPARE(info.fullscreenMonitors().left, 3);
    QCOMPARE(info.fullscreenMonitors().right, 4);

    xcb_disconnect(clientConnection);
}

QTEST_GUILESS_MAIN(NetWinInfoTestWM)

#include "netwininfotestwm.moc"
